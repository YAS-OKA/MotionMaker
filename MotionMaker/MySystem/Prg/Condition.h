#pragma once
#include"../Util/TypeContainer.hpp"
#include"../Util/Borrow.h"

namespace prg
{
	class ICondition
	{
	public:
		ICondition(size_t waitFrame = 0);
		virtual ~ICondition() {};
		void forced(bool flag = true);

		virtual void countFrame();

		void setWaitFrame(size_t frame);

		void addWaitFrame(size_t frame);

		bool enoughFrame()const;

		bool commonCheck()const;

		size_t getWaitFrame()const;

		ICondition* Not(bool flag = true);

		virtual void reset();
	protected:
		virtual bool check()const;
		bool TNot = false;//これがtrueならNot
		bool flag;
	private:
		size_t waitFrame;
		size_t frameCounter;
	};

	enum class Type {
		Every,
		Any
	};

	class ConditionArray :public ICondition
	{
	private:
		TypeContainer<ICondition> conditions;
	public:
		using ICondition::ICondition;

		ConditionArray(const Type& t);

		ConditionArray(ConditionArray&&) = default;
		ConditionArray& operator = (ConditionArray&&) = default;

		ConditionArray(const ConditionArray&) = delete;
		ConditionArray& operator = (const ConditionArray&) = delete;

		size_t getConditionSize()const;

		void countFrame()override;

		template<class C = FuncCondition, class... Args>
		C* add(Args&& ...args)
		{
			return conditions.add<C>(std::forward<Args>(args)...);
		}
		template<class C = FuncCondition, class... Args>
		C* addIn(const String& id, Args&& ...args)
		{
			return conditions.addIn<C>(id, std::forward<Args>(args)...);
		}

		template<class C = FuncCondition, class... Args>
		C* set(Args&& ...args)
		{
			conditions.removeAll();
			return conditions.set<C>(std::forward<Args>(args)...);
		}

		template<class C = FuncCondition, class... Args>
		C* setIn(const String& id, Args&& ...args)
		{
			conditions.removeAll();
			return conditions.setIn<C>(id, std::forward<Args>(args)...);
		}

		template<class C = FuncCondition>
		C* get(Optional<StringView> id = none)
		{
			if (id)return conditions.get<C>(*id);
			else return conditions.get<C>();
		}

		void reset()override;
		//Every すべてのconditionがtrueならtrue
		//Any いずれかのconditionがtrueならtrue
		Type checkType = Type::Every;

	protected:
		virtual bool check()const override;
	};

	class TimeCondition :public ICondition
	{
	public:
		Borrow<class IAction> act;

		double time;
		//actが開始してからtime秒後という条件
		TimeCondition(const Borrow<IAction>& act, double time, size_t waitFrame = 0);

	protected:
		bool check()const override;
	};

	enum class ActState
	{
		start, active, end
	};

	enum class InputState
	{
		d,u,p
	};

	//関数でチェック
	class FuncCondition :public ICondition
	{
	public:
		//ラムダ式などで関数を渡す　関数の戻り値はbool
		FuncCondition(std::function<bool()> _function, size_t waitFrame = 0)
			:ICondition(waitFrame), m_function{ _function } {}

		FuncCondition(const Borrow<class IAction>& act, const ActState& state, size_t waitFrame = 0);

		FuncCondition(const Input& key, const InputState& state = InputState::d , size_t waitFrame = 0);

		std::function<bool()> m_function;
	protected:
		bool check()const override;
	};
	//Actions内で、他のアクションがスタートしたかを監視
	class ActivesChecker:public ICondition
	{
	public:
		ActivesChecker(const Borrow<class Actions>& act, int32 threshold = 1, size_t waitFrame = 0);

		template<class... Args>
		void without(Args&& ...args) { exception.emplace(args...); }
	protected:
		HashSet<String> exception;
		Borrow<Actions> actions;
		int32 threshold;

		bool check()const override;
	};
}

#include"../Component/Collider.h"

class Entity;

namespace prg
{
	class Hitbox;

	class Hit :public ICondition
	{
		Borrow<Hitbox> box;
	public:
		HashSet<ColliderCategory> target;

		Hit(const Borrow<Hitbox>& box, HashSet<ColliderCategory> targets = {}, size_t waitFrame = 0);
		Hit(const Borrow<Hitbox>& box, const ColliderCategory& target, size_t waitFrame = 0);

	protected:
		bool check()const override;
	};

	class Touch:public ICondition
	{
	public:
		Collider* collider;

		ColliderCategory targetCategory;

		Touch(Collider* collider, const ColliderCategory& groundCategory, size_t waitFrame = 0);

	protected:
		bool check()const override;
	};

	class TouchGround :public ICondition
	{
	public:
		Collider* collider;
		ColliderCategory targetCategory;

	};
}

namespace ui
{
	class Button;
}

namespace prg
{
	class ButtonChecker :public ICondition
	{
	public:
		Borrow<ui::Button> button;

		ButtonChecker(const Borrow<ui::Button>& button, size_t waitFrame = 0);
		void setButton(const Borrow<ui::Button>& button);

	protected:
		bool check()const override;
	};
}
