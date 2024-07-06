#include "../stdafx.h"
#include "Action.h"
#include"Condition.h"
#include"Actions.h"

using namespace prg;

IAction::IAction(const Optional<double>& t)
	:updatePriority(0)
	, timer(0)
	, timeScale(1.0)
	, id(U"")
	, startCondition(ConditionArray())
	, endCondition(ConditionArray())
{
	if (t)endCondition.add<TimeCondition>(*this, *t);
}

void IAction::setStartCondition(ConditionArray&& condition)
{
	startCondition = std::move(condition);
}

void IAction::setEndCondition(ConditionArray&& condition)
{
	endCondition = std::move(condition);
}

String prg::IAction::getId() const
{
	return id;
}

bool IAction::isStarted()
{
	return started;
}

bool IAction::isEnded()
{
	return ended;
}

bool IAction::isActive()
{
	return started and (not ended);
}

void prg::IAction::StateCheckType(bool f)
{
	m_stateCheckerType = f;
	startCondition.checkType = Type::Any;
	endCondition.checkType = Type::Any;//こっちはやらなくていいかも
}

void prg::IAction::setOwner(const Borrow<Actions>& owner)
{
	this->owner = owner;
}

Borrow<Actions> prg::IAction::getOwner() const
{
	return owner;
}

void IAction::update(double dt)
{
	timer += dt * timeScale;
};

void IAction::reset()
{
	startCondition.reset();
	endCondition.reset();

	timer = 0;
	started = false;
	ended = false;
}

double TimeAction::progress() const
{
	return std::clamp(timer / time, 0.0, 1.0);
}

double TimeAction::dtPerTime(double dt) const
{
	if (time == 0)return 1;
	//超過分を減らす
	if (timer >= time)dt += time - timer;

	return dt / time;
}

prg::TimeAction::TimeAction(double time)
	:IAction(time), time(time)
{
}

MyPrint::MyPrint(const String& text,const Optional<double>& t)
	:text(text), IAction(t)
{}

void MyPrint::update(double dt)
{
	IAction::update(dt);
	Print << text;
}

FuncAction::FuncAction(const UpdateFunc& upd, const Optional<double>& time)
	:upd(upd), IAction(time) {}

prg::FuncAction::FuncAction(const UpdateFunc& upd, const TermEvent& fin, const Optional<double>& time)
	:upd(upd), fin(fin), IAction(time)
{
}

FuncAction::FuncAction(const TermEvent& ini, const Optional<double>& time)
	:ini(ini), IAction(time) {}

prg::FuncAction::FuncAction(const TermEvent& ini, const TermEvent& fin, const Optional<double>& time)
	:ini(ini), fin(fin), IAction(time){}

FuncAction::FuncAction(const TermEvent& ini, const UpdateFunc& upd, const Optional<double>& time)
	:ini(ini), upd(upd), IAction(time) {}

prg::FuncAction::FuncAction(const TermEvent& ini, const UpdateFunc& upd, const TermEvent& fin, const Optional<double>& time)
	:ini(ini), upd(upd), fin(fin), IAction(time) {}


void FuncAction::update(double dt)
{
	IAction::update(dt);

	if (holds_alternative<FullUpdate>(upd))
	{
		std::get<FullUpdate>(upd)(dt, this);
	}
	else {
		std::get<SimpleUpdate>(upd)(dt);
	}
}

void FuncAction::start()
{
	IAction::start();

	ini();
}

void FuncAction::end()
{
	IAction::end();

	fin();
}

//void prg::Wait::update(double dt)
//{
//	IAction::update(dt);
//	Print << U"Waiting";
//}
