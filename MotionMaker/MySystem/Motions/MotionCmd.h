#pragma once
#include"../Prg/Init.h"
#include"../Figure.h"
#include"../Util/Util.h"
#include"Motion.h"

namespace mot
{
	//セットしたsceneにパーツを生成
	class MakeParts :public prg::IAction
	{
	private:
		Borrow<class Parts> createdParts;
	public:
		String name;

		Optional<String> path = none;

		Figure fig;

		double x, y;

		Borrow<class PartsManager> pmanager;

		bool createHitbox = false;

		MakeParts(String name, String path, double x, double y)
			:name(name), IAction(0), path(path), x(x), y(y) {};

		MakeParts(String name, String figure, double x, double y, double w, double h)
			:name(name), IAction(0), x(x), y(y) {
			if (figure == U"rect")
			{
				fig = RectF{ -w / 2,-h / 2,w,h };
			}
		};

		MakeParts(String name, String figure, double x, double y, double deg, double len1, double len2)
			:name(name), IAction(0), x(x), y(y)
		{
			if (figure == U"tri") {
				deg *= 1_deg;
				fig = Triangle{ {0,0},util::polar(-deg / 2,len1),util::polar(deg / 2,len2) };
				fig.setCenter(0, 0);
			}
		}

		MakeParts(String name, String figure, double x, double y, double r)
			:name(name), IAction(0), x(x), y(y)
		{
			if (figure == U"cir") {
				fig = Circle{ r };
			}
		}

		MakeParts* build(const Borrow<PartsManager>& pmanager, bool createHitbox)
		{
			this->pmanager = pmanager;
			this->createHitbox = createHitbox;
			return this;
		};
		//keepParts=falseの場合、現在のcreatedPartsを返して、このクラスのcreatedParts=nullptrにする。
		Borrow<Parts> getCreatedParts(bool keepParts = false);

	private:
		virtual void start()override;
	};

	class FindParts :public prg::IAction
	{
	private:
		Borrow<class Parts> foundParts;
	public:
		String name;

		Borrow<class PartsManager> pmanager;

		FindParts(const String& name)
			:name(name) {};

		FindParts* build(const Borrow<PartsManager>& pmanager)
		{
			this->pmanager = pmanager;
			return this;
		};

		Borrow<Parts> getFindParts(bool keepParts = false);
	private:
		virtual void start()override;
	};

	class KillParts :public prg::IAction
	{
	private:
		HashSet<class Parts*> killedParts;
	public:
		String name;

		bool killChildren;

		Borrow<class PartsManager> pmanager;

		KillParts(const String& name, bool killChildren = false)
			:name(name), killChildren(killChildren)
		{
		}

		KillParts* build(const Borrow<PartsManager>& pmanager)
		{
			this->pmanager = pmanager;
			return this;
		}

		HashSet<Parts*> getKilledParts(bool keepParts = false);
	private:
		virtual void start()override;
	};

	class StartMotion :public prg::IAction
	{
	public:
		bool loopedMotion;
		Borrow<PartsManager> pMan;
		String motionName;

		StartMotion(String motionName, bool loop = false);

		StartMotion* build(const Borrow<PartsManager>& pMan)
		{
			this->pMan = pMan;
			return this;
		}
	private:
		void start();
	};

	template<class T,class Enable = void> class SetMotion{};

	template<class T>
	class SetMotion<T, std::enable_if_t<std::is_base_of_v<PartsMotion, T>>>:public prg::IAction
	{
	public:
		Borrow<PartsManager> pMan;
		String targetName;
		T* action=nullptr;
		bool addParallel;
		Borrow<prg::Actions> actions;
		//actionsに指定したモーションを追加する

		template<class... Args>
		SetMotion(String target, bool addParallel, Args&& ...args)
			:addParallel(addParallel),targetName(target), action(new T(nullptr, args...))
		{
		}

		SetMotion<T>* build(const Borrow<PartsManager>& pMan, const Borrow<prg::Actions>& actions)
		{
			this->actions = actions;
			this->pMan = pMan;
			return this;
		}

		T* getCreatedMotion()const
		{
			return action;
		};
	private:
		void start()override
		{
			prg::IAction::start();
			auto targetParts = pMan->find(targetName);
			if (not targetParts)return;
			action->target = targetParts;
			if (addParallel)
			{
				actions->addActParallel(action);
			}
			else
			{
				actions->addAct(action);
			}
		}
	};

	template<class T>
	class SetMotion<T, std::enable_if_t<!std::is_base_of_v<PartsMotion, T>>> :public prg::IAction
	{
	public:
		Borrow<PartsManager> pMan;
		T* action = nullptr;
		bool addParallel;
		Borrow<prg::Actions> actions;
		//actionsに指定したモーションを追加する

		template<class... Args>
		SetMotion(String _, bool addParallel, Args&& ...args)
			:addParallel(addParallel), action(new T(args...))
		{
		}

		SetMotion<T>* build(const Borrow<PartsManager>& pMan, const Borrow<prg::Actions>& actions)
		{
			this->actions = actions;
			this->pMan = pMan;
			return this;
		}

		T* getCreatedMotion()const
		{
			return action;
		};
	private:
		void start()override
		{
			prg::IAction::start();
			if (addParallel)
			{
				actions->addActParallel(action);
			}
			else
			{
				actions->addAct(action);
			}
		}
	};

	template<>
	inline SetMotion<StartMotion>* SetMotion<StartMotion>::build(const Borrow<PartsManager>& pMan, const Borrow<prg::Actions>& actions)
	{
		this->actions = actions;
		this->pMan = pMan;
		action->build(pMan);
		return this;
	}

	template<>
	inline SetMotion<PartsParamsVariable>* SetMotion<PartsParamsVariable>::build(const Borrow<PartsManager>& pMan, const Borrow<prg::Actions>& actions)
	{
		this->actions = actions;
		this->pMan = pMan;
		action->pMan = pMan;
		return this;
	}


	class EraseMotion:public prg::IAction
	{
	public:
		Borrow<PartsManager> pMan;
		String motionName;

		EraseMotion(StringView motionName);

		EraseMotion* build(const Borrow<PartsManager>& pman);
	private:
		void start()override;
	};

	class LoadMotionScript:public prg::IAction
	{
	public:
		Borrow<PartsManager> pman;
		FilePath path;
		String motionName;

		LoadMotionScript(FilePath path,String motionName);

		LoadMotionScript* build(const Borrow<PartsManager>& pman);

	protected:
		void start();
	};

	class WriteMotionScript :public prg::IAction
	{
	public:
		Borrow<PartsManager> pman;
		FilePath path;
		String motionName;
		Optional<String> time;
		Optional<String> len;
		HashSet<String> targets;

		WriteMotionScript(FilePath path, String motionName, Optional<String> len = none, Optional<String> time = none);

		WriteMotionScript* build(const Borrow<PartsManager>& pmana);

		Array<String> createMotionText()const;

	protected:
		void start();
	};
}
