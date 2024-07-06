#pragma once
#include"Action.h"
#include"../Component/Transform.h"
#include"../Component/Draw.h"
#include"../Game/Scenes.h"
#include"../Component/Collider.h"
#include"Actions.h"
#include"../Util/Util.h"

class Object;
class Collider;
class Character;

namespace prg
{
	//アクションを終了させる
	class EndAction final :public IAction
	{
	public:
		Borrow<class Actions> actions;

		Array<Borrow<IAction>>targets{};

		template<class... Args>
		EndAction(const Borrow<Actions>& actions,Optional<double> time, Args&& ...ids)
			:IAction(time), actions(actions)
		{
			setTargets(ids...);
		}
		template<class ...Args>
		void withoutId(Args&& ...ids)
		{
			for (const String& id : std::initializer_list<String>{ ids... }) addException(id);
		}

		template<class ...Args>
		void without(Args&& ...acts)
		{
			for (const Borrow<IAction>& act : std::initializer_list<Borrow<IAction>>{ acts... }) addException(act);
		}

		void endAll();

		//fに渡されるIActionのnull判定はしなくていい
		void endOtherIf(const std::function<bool(IAction*)>& f);

		template<class... Args>
		void setTargets(Args&& ...ids)
		{
			for (const String& id : std::initializer_list<String>{ ids... }) addTarget(id);
		}
	protected:
		Array<Borrow<IAction>> exception{};
		std::function<bool (IAction*)> f = nullptr;
		void addException(StringView id);
		void addException(const Borrow<IAction>& act);
		void addTarget(StringView ids);
		void start()override;
		void update(double dt)override;
	};

	class ActCluster :public IAction
	{
	public:
		using IAction::IAction;

		template<class T = FuncAction, class... Args>
		T& add(Args&&... args)
		{
			auto action = new T(args...);

			return addAct<T>(action);
		}
		template<class T>
		T& addAct(T* action, const String& id = U"")
		{
			acts << action;

			if (id != U"")action->id = id;

			return *action;
		}

		void update(double dt)override;

		void end()override;

		Array<IAction*> acts;
	};

	class MoveAct :public IAction
	{
	public:
		Vec3 initVel{ 0,0,0 };
		Vec3 acc{ 0,0,0 };
		Borrow<Transform> transform;

		MoveAct(const Borrow<Transform>& transform, const Vec3& initVel, const Optional<double>& t = none);

		MoveAct(const Vec3& initVel, const Optional<double>& t = none);

		void setTransform(const Borrow<Transform>& t);

		Vec3 getVel()const;
	protected:
		friend class MulMove;
		Vec3 vel;
		virtual void start()override;
		virtual void update(double dt)override;
	};

	class MulMove :public IAction
	{
	public:
		Array<util::uPtr<MoveAct>> moves;

		Optional<double> constantSpeed;

		Optional<double> limitingSpeed;
		//getter
		Vec3 vel;

		Borrow<Transform> transform;

		MulMove(const Borrow<Transform>& transform, const Optional<double>& constantSpeed = none, const Optional<double>& t = none);

		void setLimit(double limitingSpeed);

		void addMove(MoveAct&& move);
	protected:
		virtual void update(double dt)override;
		virtual void end()override;
	};

	class FreeFall :public MoveAct
	{
	public:
		FreeFall(const Borrow<Transform>& transform, const Vec3& initVel, double acc, const Optional<double>& t = none);

		void setAcc(double a);
	};
	//このアクションは終了すると対象のオブジェクトが死ぬ
	class LifeSpan :public IAction
	{
	public:
		Borrow<Object> target;

		LifeSpan(const Borrow<Object>& target, const Optional<double>& time = none);
	protected:
		void end()override;
	};

	class Hitbox:public IAction
	{
	public:
		Borrow<Collider> collider;

		Borrow<Object> hitbox;
		//衝突するカテゴリー
		HashSet<ColliderCategory> targetCategory{};
		//衝突したエンティティを保管 一フレームごとに更新
		HashTable<ColliderCategory, HashSet<Entity*>> collidedEntitys{};

		template <class Shape>
		Hitbox(const Borrow<Object>& chara,const Shape& shape, const Vec3& relative, const Optional<double> time = none)
			:IAction(time)
		{
			hitbox = chara->scene->birthObject(shape, Vec3{ 0,0,0 });
			hitbox->transform->setParent(chara->transform);
			hitbox->transform->setLocalPos(relative);
			collider = hitbox->getComponent<Collider>();
			collider->collidable = false;
			auto vis = hitbox->getComponent<Draw3D>(U"hitbox");
			vis->color = ColorF{ Palette::Yellow }.removeSRGBCurve();
			vis->visible = false;
			//このアクションが終わったらヒットボックスオブジェクトをkill
			hitbox->ACreate(U"killSelf", true) += LifeSpan(hitbox).same(*this);
		}
		
		Hitbox* setTarget(HashSet<ColliderCategory> category) {
			targetCategory = category;
			return this;
		}
		
	protected:
		virtual void reset()override;
		virtual void start()override;
		virtual void update(double dt)override;
		virtual void end()override;
	};

	template<class T1, class T2, class T3, class T4>
	MulMove Move4D(const Borrow<Transform>& transform ,double speed, const T1& upCondition, const T2& downCondition, const T3& leftCondition, const T4& rightCondition)
	{
		return MulMove(transform, speed)
			* MoveAct({ 0,0,1 }).activeIf<T1>(upCondition)
			* MoveAct({ 0,0,-1 }).activeIf<T2>(downCondition)
			* MoveAct({ -1,0,0 }).activeIf<T3>(leftCondition)
			* MoveAct({ 1,0,0 }).activeIf<T4>(rightCondition);
	}

	template<class T1, class T2, class T3, class T4>
	ActCluster Look4D(const Borrow<Transform>& transform, const T1& upCondition, const T2& downCondition, const T3& leftCondition, const T4& rightCondition)
	{
		return FuncAction([=] {transform->setDirection({ 0,0,1 }); }).startIf<T1>(upCondition)
			* FuncAction([=] {transform->setDirection({ 0,0,-1 }); }).startIf<T2>(downCondition)
			* FuncAction([=] {transform->setDirection({ -1,0,0 }); }).startIf<T3>(leftCondition)
			* FuncAction([=] {transform->setDirection({ 1,0,0 }); }).startIf<T4>(rightCondition);
	}
	//便利?
	namespace use {
		template<class T1, class T2, class T3, class T4>
		ActCluster Move4D(const Borrow<Transform>& transform, double speed, const T1& upCondition, const T2& downCondition, const T3& leftCondition, const T4& rightCondition)
		{
			return prg::Move4D(transform,
						speed,
						upCondition,
						downCondition,
						leftCondition,
						rightCondition).startIf([] {return true; })
				* Look4D(transform,
						upCondition,
						downCondition,
						leftCondition,
						rightCondition).startIf([] {return true; });
		}
	}

}

//アクトクラスターを作る
template<class Act1, class Act2>
typename std::enable_if_t<std::is_base_of_v<IAction, Act1>&& std::is_base_of_v<IAction, Act2>, ActCluster>
operator * (Act1&& a, Act2&& b)
{
	ActCluster cl;
	cl.addAct(new Act1(std::forward<Act1>(a)));
	cl.addAct(new Act2(std::forward<Act2>(b)));
	return cl;
}
template<class Act2>
typename std::enable_if_t<std::is_base_of_v<IAction, Act2>, ActCluster&&>
operator * (ActCluster&& a, Act2&& b)
{
	a.addAct(new Act2(std::forward<Act2>(b)));
	return std::forward<ActCluster>(a);
}
template<class Act2>
typename std::enable_if_t<std::is_base_of_v<IAction, Act2>, ActCluster&&>
operator |= (ActCluster&& a, Act2&& b)
{
	a.addAct(new Act2(std::forward<Act2>(b)));

	return std::forward<ActCluster>(a);
}
//MulMoveのおぺ
template<class Act2>
typename std::enable_if_t<std::is_base_of_v<MoveAct, Act2>, MulMove&&>
operator * (MulMove& a, Act2&& b)
{
	a.addMove(std::forward<Act2>(b));
	return std::forward<MulMove>(a);
}
template<class Act2>
typename std::enable_if_t<std::is_base_of_v<MoveAct, Act2>, MulMove&&>
operator * (MulMove&& a, Act2&& b)
{
	a.addMove(std::forward<Act2>(b));
	return std::forward<MulMove>(a);
}
