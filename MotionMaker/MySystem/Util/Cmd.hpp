#pragma once
#include"../Figure.h"
#include"../Prg/Action.h"
#include"../Game/Utilities.h"

namespace my {
	class Scene;
}

template<class T> using EventFunction = std::function<void(T*)>;

template <class T>
class Eventa
{
public:
	static EventFunction<T> NonEvent;

	EventFunction<T> f;

	Eventa(const EventFunction<T>& functa)
		: f(functa) {}

	virtual void operator = (const EventFunction<T>& functa)
	{
		f = functa;
	}

	virtual void operator ()(T* arg)
	{
		f(arg);
	}
};

template <class T>
EventFunction<T> Eventa<T>::NonEvent = [](T*) {};

class ICMD
{
public:
	virtual void execute() {};
};

template <class T = prg::IAction>
class CMD :public ICMD
{
public:
	T* action = nullptr;

	Eventa<T> e;

	CMD(T* action, const EventFunction<T>& e = Eventa<T>::NonEvent)
		:action(action), e(e) {}

	virtual ~CMD() { if (action != nullptr)delete action; }

	void set(T* action, const EventFunction<T>& e = Eventa<T>::NonEvent)
	{
		this->action = action;
		this->e = e;
	}

	void execute()override
	{
		//作成されたアクションとマネージャーはstart()した後削除される。updateは呼び出されないので,updateが必要ないコマンドに対して適切に動作する。
		prg::Actions actManager;
		actManager.addAct(action);
		actManager.start(true);
		e(action);
		action = nullptr;
		//ここでactManagerもactionも削除される
	}
};

template <class Act>using PostProcessing = std::function<Act* (Act*)>;

template <class T, class... Args>
PostProcessing<T> buildProcess(Args ...args)
{
	return [=](T* t) {
		return t->build(args...);
	};
}

// 使い方こんな感じ
// 1->	execute(buildProcess(...), arg1, arg2, ...);
// 2->	execute([](Act* act){return act->build(...);}, arg1, arg2, ...);
template<class Act, class... Args>
static void execute(PostProcessing<Act> postProcessing, Args&& ...actArgs)
{
	CMD<Act>(postProcessing(new Act(actArgs...)), Eventa<Act>::NonEvent).execute();
}
// 使い方こんな感じ
// 1->	execute([](Act*){return;}, buildProcess(...), arg1, arg2, ...);
// 2->	execute([](Act*){return;}, [](Act* act){return act;}, arg1, arg2, ...);
template<class Act, class... Args>
static void execute(const EventFunction<Act>& ef, PostProcessing<Act> postProcessing, Args&& ...actArgs)
{
	CMD<Act>(postProcessing(new Act(actArgs...)), Eventa<Act>::NonEvent).execute();
}
// 使い方こんな感じ
// execute(util::ptr<Act>(arg1, arg2, ...), barg1, barg2, ...);
template<class Act, class... BuildArgs>
static void execute(Act* act, BuildArgs&& ...buildArgs)
{
	CMD<Act>(buildProcess<Act>(buildArgs...)(act), Eventa<Act>::NonEvent).execute();
}
