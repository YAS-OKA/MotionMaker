#include "../stdafx.h"
#include "Condition.h"
#include"Action.h"
#include"Actions.h"

#include"../EC.hpp"
#include"../Prg/GameAction.h"
#include"../Game/UIProducts.h"

using namespace prg;

ICondition::ICondition(size_t waitFrame)
	:flag(false), waitFrame(waitFrame), frameCounter(0)
{
}

void ICondition::forced(bool _flag)
{
	flag = _flag;
}

void ICondition::countFrame()
{
	frameCounter++;
}

void ICondition::setWaitFrame(size_t frame)
{
	waitFrame = frame;
}

void prg::ICondition::addWaitFrame(size_t frame)
{
	waitFrame += frame;
}

bool ICondition::enoughFrame() const
{
	return waitFrame <= frameCounter;
}

bool prg::ICondition::commonCheck()const
{
	return enoughFrame() and TNot != (flag or check());
}

size_t prg::ICondition::getWaitFrame() const
{
	return waitFrame;
}

ICondition* prg::ICondition::Not(bool flag)
{
	TNot = flag;
	return this;
}

void ICondition::reset()
{
	flag = false;
	frameCounter = 0;
}

bool ICondition::check()const
{
	return false;
}

prg::ConditionArray::ConditionArray(const Type& t)
	:checkType(t)
{
}

size_t prg::ConditionArray::getConditionSize() const
{
	size_t res = 0;
	for (auto& conditionTable : conditions.arr)
		for (auto& [_, condition] : conditionTable.second)
			res++;
	return res;
}

void prg::ConditionArray::countFrame()
{
	ICondition::countFrame();
	for (auto& conditionTable : conditions.arr)
		for (auto& [_, condition] : conditionTable.second)
			condition->countFrame();
	
}

bool ConditionArray::check()const
{
	if (conditions.arr.empty())return false;//無条件の場合、自分(flag)が条件となる
	
	//すべてand または すべてor
	bool tmp = checkType == Type::Any;

	for (auto& conditionTable : conditions.arr)
		for (auto& [_, condition] : conditionTable.second)
			if (tmp == condition->commonCheck())return tmp;

	return not tmp;
}

void ConditionArray::reset()
{
	ICondition::reset();
	for (auto& conditionTable : conditions.arr)
		for (auto& [_, condition] : conditionTable.second)
			condition->reset();
}

Hit::Hit(const Borrow<Hitbox>& box, HashSet<ColliderCategory> target, size_t waitFrame)
	:box(box), target(target), ICondition(waitFrame)
{
}

Hit::Hit(const Borrow<Hitbox>& box, const ColliderCategory& target, size_t waitFrame)
	:box(box), target({ target }), ICondition(waitFrame)
{
}

bool Hit::check()const
{
	for (auto& c : target)
	{
		return box->collidedEntitys.contains(c) and not box->collidedEntitys.at(c).empty();
	}
	return false;
}

prg::TimeCondition::TimeCondition(const Borrow<IAction>& _act, double time, size_t waitFrame)
	:ICondition(waitFrame), act(_act), time(time)
{
}

bool prg::TimeCondition::check()const
{
	return act and act->timer >= time;
}

prg::FuncCondition::FuncCondition(const Borrow<IAction>& act, const ActState& state, size_t waitFrame)
	:ICondition(waitFrame)
{
	using enum ActState;

	switch (state){
	case start: m_function = [act] {return act and act->isStarted(); }; break;
	case active: m_function = [act] {return act and act->isActive(); }; break;
	case end: m_function = [act] {return (not act) or act->isEnded(); }; break;
	default: break;
	}
}

prg::FuncCondition::FuncCondition(const Input& key, const InputState& state, size_t waitFrame)
	:ICondition(waitFrame)
{
	using enum InputState;

	switch (state) {
	case d: m_function = [key] {return key.down(); }; break;
	case u: m_function = [key] {return key.up(); }; break;
	case p: m_function = [key] {return key.pressed(); }; break;
	default: break;
	}
}

bool prg::FuncCondition::check() const
{
	return m_function();
}

prg::ActivesChecker::ActivesChecker(const Borrow<class Actions>& act, int32 threshold, size_t waitFrame)
	:ICondition(waitFrame), actions(act), threshold(threshold)
{
}

bool prg::ActivesChecker::check() const
{
	int32 activeNum = actions->getActiveNum();
	//例外のアクションがカウントされていたらその分ひく
	for (const auto& name : exception)
		if (actions->getAction(name)->isActive())
			activeNum--;

	return activeNum >= threshold;
}

prg::Touch::Touch(Collider* collider, const ColliderCategory& targetCategory, size_t waitFrame)
	:collider(collider), targetCategory(targetCategory), ICondition(waitFrame)
{
}

bool prg::Touch::check() const
{
	return not collider->intersects(targetCategory).isEmpty();
}

ButtonChecker::ButtonChecker(const Borrow<ui::Button>& button, size_t waitFrame)
	:button(button), ICondition(waitFrame)
{}

void ButtonChecker::setButton(const Borrow<ui::Button>& button)
{
	ButtonChecker::button = button;
}

bool ButtonChecker::check()const
{
	return button->pushed();
}
