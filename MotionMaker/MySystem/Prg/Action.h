#pragma once
#include"Condition.h"
#include"../Util/Borrow.h"

namespace prg
{
	class ConditionArray;

	class Actions;
	class StateActions;
	class ActCluster;

	class IAction :public Borrowable
	{
		ConditionArray* editingCondition = nullptr;
	public:
		IAction(const Optional<double>& time = none);

		void setStartCondition(ConditionArray&& condition = ConditionArray());

		void setEndCondition(ConditionArray&& condition = ConditionArray());
		//開始条件をセット
		template<class C = FuncCondition, class Self, class... Args>
		auto startIf(this Self&& self, Args&& ...args) -> decltype(self)
		{
			self.editingCondition = &self.startCondition;
			self.editingCondition->set<C>(std::forward<Args>(args)...);
			return std::forward<Self>(self);
		}
		template<class C = FuncCondition, class Self, class... Args>
		auto startIfNot(this Self&& self, Args&& ...args) -> decltype(self)
		{
			self.editingCondition = &self.startCondition;
			self.editingCondition->set<C>(std::forward<Args>(args)...)->Not();
			return std::forward<Self>(self);
		}
		//終了条件をセット
		template<class C = FuncCondition, class Self, class... Args>
		auto endIf(this Self&& self, Args&& ...args) -> decltype(self)
		{
			self.editingCondition = &self.endCondition;
			self.editingCondition->set<C>(std::forward<Args>(args)...);
			return std::forward<Self>(self);
		}
		//終了条件をセット
		template<class C = FuncCondition, class Self, class... Args>
		auto endIfNot(this Self&& self, Args&& ...args) -> decltype(self)
		{
			self.editingCondition = &self.endCondition;
			self.editingCondition->set<C>(std::forward<Args>(args)...)->Not();
			return std::forward<Self>(self);
		}
		//条件追加
		template<class C = FuncCondition, class Self, class... Args>
		auto andIf(this Self&& self, Args&& ...args) -> decltype(self)
		{
			if (self.editingCondition)self.editingCondition->add<C>(std::forward<Args>(args)...);
			return std::forward<Self>(self);
		}
		template<class C = FuncCondition, class Self, class... Args>
		auto andIfNot(this Self&& self, Args&& ...args) -> decltype(self)
		{
			if (self.editingCondition)self.editingCondition->add<C>(std::forward<Args>(args)...)->Not();
			return std::forward<Self>(self);
		}
		template<class C = FuncCondition, class Self, class... Args>
		auto andStartIf(this Self&& self, Args&& ...args) -> decltype(self)
		{
			self.editingCondition = &self.startCondition;
			self.editingCondition->add<C>(std::forward<Args>(args)...);

			return std::forward<Self>(self);
		}
		template<class C = FuncCondition, class Self, class... Args>
		auto andStartIfNot(this Self&& self, Args&& ...args) -> decltype(self)
		{
			self.editingCondition = &self.startCondition;
			self.editingCondition->add<C>(std::forward<Args>(args)...)->Not();
			return std::forward<Self>(self);
		}
		template<class C = FuncCondition, class Self, class... Args>
		auto andEndIf(this Self&& self, Args&& ...args) -> decltype(self)
		{
			self.editingCondition = &self.endCondition;
			self.editingCondition->add<C>(std::forward<Args>(args)...);

			return std::forward<Self>(self);
		}
		template<class C = FuncCondition, class Self, class... Args>
		auto andEndIfNot(this Self&& self, Args&& ...args) -> decltype(self)
		{
			self.editingCondition = &self.endCondition;
			self.editingCondition->add<C>(std::forward<Args>(args)...)->Not();
			return std::forward<Self>(self);
		}

		//他のアクションと開始終了を合わせる
		template<class Self>
		auto same(this Self&& self, const Borrow<IAction>& other) -> decltype(self)
		{
			auto tmp = self.editingCondition;//エディティングコンディションの状態を保存しておく
			self.startIf(other, ActState::start);
			self.endIf(other, ActState::end);
			self.editingCondition = tmp;//最初の状態に戻す
			return std::forward<Self>(self);
		}

		//開始と終了条件が同じ
		template<class C = FuncCondition, class Self, class... Args >
		auto activeIf(this Self&& self, Args&& ...args) -> decltype(self)
		{
			auto tmp = self.editingCondition;//エディティングコンディションの状態を保存しておく
			self.startIf<C>(args...);
			self.endIfNot<C>(std::forward<Args>(args)...);
			self.editingCondition = tmp;//最初の状態に戻す
			return std::forward<Self>(self);
		}
		//開始と終了条件が同じ
		template<class C = FuncCondition, class Self, class... Args>
		auto activeIfNot(this Self&& self, Args&& ...args) -> decltype(self)
		{
			auto tmp = self.editingCondition;//エディティングコンディションの状態を保存しておく
			self.startIfNot<C>(args...);
			self.endIf<C>(std::forward<Args>(args)...);
			self.editingCondition = tmp;//最初の状態に戻す
			return std::forward<Self>(self);
		}

		template<class C = FuncCondition, class Self, class... Args >
		auto andActiveIf(this Self&& self, Args&& ...args) -> decltype(self)
		{
			auto tmp = self.editingCondition;
			self.andStartIf<C>(args...);
			self.andEndIfNot<C>(std::forward<Args>(args)...);
			self.editingCondition = tmp;
			return std::forward<Self>(self);
		}
		template<class C = FuncCondition, class Self, class... Args>
		auto andActiveIfNot(this Self&& self, Args&& ...args) -> decltype(self)
		{
			auto tmp = self.editingCondition;
			self.andStartIfNot<C>(args...);
			self.andEndIf<C>(std::forward<Args>(args)...);
			self.editingCondition = tmp;
			return std::forward<Self>(self);
		}

		template<class Self>
		auto setId(this Self&& self, StringView id) -> decltype(self)
		{
			self.id = id;
			return std::forward<Self>(self);
		};

		String getId()const;

		bool isStarted();

		bool isEnded();

		bool isActive();

		double updatePriority;

		double timer;

		double timeScale;

		void StateCheckType(bool f = true);

		ConditionArray startCondition;
		ConditionArray endCondition;

		void setOwner(const Borrow<Actions>& owner);

		Borrow<Actions> getOwner()const;
	protected:
		Borrow<Actions> owner;

		bool m_stateCheckerType = false;

		String id;
		bool started = false;
		bool ended = false;
		friend Actions;
		friend StateActions;
		friend ActCluster;

		virtual void reset();

		virtual void start()
		{
			timer = 0;
			started = true;
		};
		virtual void end()
		{
			ended = true;
		};

		virtual void update(double dt);
	};

	//IActionはWaitとしても使える
	using Wait = IAction;

	/*class Wait :public IAction
	{
	public:
		using IAction::IAction;
	private:
		void update(double dt)override;
	};*/

	class TimeAction :public IAction
	{
	public:
		double time;

		double progress()const;
		//dtが0～timeまでdtを加算していったとき、その和が1となるようなdt/timeを出力する
		double dtPerTime(double dt)const;

		TimeAction(double time = 0);
	};

	class MyPrint final :public IAction
	{
	public:
		MyPrint(const String& text, const Optional<double>& time = none);

		String text;

	private:
		void update(double dt);
	};

	class FuncAction :public IAction
	{
	public:
		using Self = FuncAction*;
		using FullUpdate = std::function<void(double, Self)>;
		using SimpleUpdate = std::function<void(double)>;
		using TermEvent = std::function<void()>;
		using UpdateFunc = std::variant< FullUpdate, SimpleUpdate>;

		UpdateFunc upd = [](double) {};
		TermEvent ini = [] {};
		TermEvent fin = [] {};

		FuncAction() = default;

		FuncAction(const UpdateFunc& upd, const Optional<double>& time = none);

		FuncAction(const UpdateFunc& upd, const TermEvent& fin,const Optional<double>& time = none);

		FuncAction(const TermEvent& ini, const Optional<double>& time = 0);

		FuncAction(const TermEvent& ini, const TermEvent& fin, const Optional<double>& time = 0);

		FuncAction(const TermEvent& ini, const UpdateFunc& upd, const Optional<double>& time = none);

		FuncAction(const TermEvent& ini, const UpdateFunc& upd, const TermEvent& fin, const Optional<double>& time = none);
	private:
		void update(double dt);
		void start();
		void end();
	};
}
