#include "../stdafx.h"
#include "ActionManager.h"

Actions& ActionManager::create(StringView name, bool immediatelyStart, bool loop)
{
	//同名のアクションがあれば消して上書き
	erase(name);
	actions.emplace(name, new Actions());
	if (immediatelyStart)act(name);
	actions[name]->loop = loop;
	return *actions[name];
}

Actions& ActionManager::createOneShot(StringView name, bool startImmediately, bool loop)
{
	auto& act = create(name, startImmediately, loop);
	oneShotSet.emplace(&act);
	return act;
}

Actions& ActionManager::create(StringView name, Actions&& action, bool immediatelyStart, Optional<bool> loop)
{
	erase(name);
	actions.emplace(name, new Actions(std::move(action)));
	if (immediatelyStart)act(name);
	if (loop)actions[name]->loop = *loop;
	return *actions[name];
}

Actions& ActionManager::createOneShot(StringView name, Actions&& action, bool immediatelyStart, Optional<bool> loop)
{
	auto& act = create(name, std::forward<Actions>(action), immediatelyStart, loop);
	oneShotSet.emplace(&act);
	return act;
}

bool ActionManager::erase(StringView name)
{
	if (not actions.contains(name))return false;
	else {
		//oneShotSet.erase(actions[name]);
		actions.erase(name);
	};
	return true;
}

void ActionManager::act(StringView name)
{
	if (not actions.contains(name))return;
	if (actions[name]->isActive() and actions[name]->stopped)
		actions[name]->stopped = false;
	else
		actions[name]->startCondition.forced();
}

void ActionManager::restart(StringView name)
{
	if (not actions.contains(name))return;
	actions[name]->restart();
}

void ActionManager::stop(StringView name)
{
	if (not actions.contains(name))return;
	actions[name]->stopped = true;
}

void ActionManager::endAll()
{
	for (auto& a : actions)if (a.second->isActive())a.second->end();
}

void ActionManager::end(StringView name)
{
	if (actions.contains(name) and actions[name]->isActive())actions[name]->end();
}

void ActionManager::update(double dt)
{
	for (auto& [key, act] : actions) {
		if ((not act->isStarted()) and (not act->isEnded()))
		{
			if (act->startCondition.commonCheck())act->start();
			else act->startCondition.countFrame();//カウント
		}
	}

	for (auto& [key, act] : actions) { if (act->isActive()) act->update(dt); }

	//終了したアクションがいればリセットをかける
	for (auto& [key, act] : actions) { if (act->isEnded())act->reset(); }

	//一度きりのアクションが終了したら、名前をこのリストに入れる。
	Array<String> eraseList{};
	for (auto& [key, act] : actions)
	{
		if (act->isActive()) {
			if (act->endCondition.commonCheck())
			{
				act->end();
				if (oneShotSet.contains(act)) eraseList << key;
			}
			else {
				act->endCondition.countFrame();
			}
		}
	}
	//eraseListに入ってるアクションを消す。
	for (const auto& name : eraseList)erase(name);
}

Actions& ActionManager::get(StringView name)
{
	if (not actions.contains(name))return create(name);
	return *actions[name];
}

Actions& ActionManager::operator[](StringView name)
{
	return get(name);
}

bool ActionManager::operator()(StringView name)
{
	return actions.contains(name);
}
