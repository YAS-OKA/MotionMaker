#pragma once
#include"Actions.h"

using namespace prg;

class ActionManager
{
private:
	HashTable<String, std::unique_ptr<Actions>> actions;
	HashSet<Actions*> oneShotSet;
public:
	Actions& create(StringView name, bool startImmediately = false, bool loop = false);
	//アクション終了時に消去されるアクションを生成
	Actions& createOneShot(StringView name, bool startImmediately = false, bool loop = false);

	Actions& create(StringView name, Actions&& act, bool startImmediately = false, Optional<bool> loop = none);

	Actions& createOneShot(StringView name, Actions&& act, bool startImmediately = false, Optional<bool> loop = none);

	bool erase(StringView name);
	//開始
	void act(StringView nam);

	void restart(StringView name);

	void stop(StringView name);

	void endAll();

	void end(StringView name);

	void update(double dt);

	Actions& get(StringView name);

	Actions& operator [](StringView name);

	bool operator ()(StringView name);
};
