#pragma once
#include<Siv3D.hpp>
#include"Util/PCTree.hpp"
#include"Util/TypeContainer.hpp"
#include"Util/Borrow.h"

class Entity;

class Priority
{
	//値が大きい方が先に更新される
	double priority = 0;
public:
	double getPriority() const { return priority; };
	void setPriority(double priority) { Priority::priority = priority; };
};

//コンポーネントの基底クラス
class Component :public Borrowable
{
public:
	virtual void start() {}
	virtual void update(double dt) {}

	Priority priority;

	Entity* owner = nullptr;
	//追加番号　priorityが等しい場合、これをもとにソート
	int32 index;

private:
	friend Entity;

	std::type_index compType{ typeid(Component) };
	String id{ U"" };
};

class EntityManager;

//コンポーネントを適用した基底クラス
//HashTableを使っているので計算量は基本O(1)？
//同一の型のコンポーネントを追加することは可能　その場合idを指定しないとGetComponentはできない（バグ防止）
//型が重複してるならGetComponentArrを使うことを推奨
class Entity :public Borrowable
{
public:
	EntityManager* owner;

	util::PCRelationship<Entity> relation;

	Array<Borrow<Entity>> sameDestiny;

	Entity()
		:relation(this), owner(nullptr)
	{}
	//killされた直後実行する
	virtual void onTrashing() {};

	virtual void start() {};

	virtual void update(double dt) {};

	//運命共同体。相手が死ぬと自分も死ぬし、自分が死ぬと相手も死ぬ
	void setSameDestiny(const Borrow<Entity>& other)
	{
		sameDestiny << other;
		other->sameDestiny << *this;
	}
	//相手が死ぬと自分も死ぬ
	void followDestiny(const Borrow<Entity>& owner)
	{
		relation.setParent(owner.get());
	}

	//コンポーネントをアップデート
	virtual void update_components(double dt)
	{
		if (components.garbages.size() > 0)components.deleteGarbages();

		allComponentForUpdate.remove_if([](const auto& p) {return not p; });//消されたコンポーネントを除外

		std::stable_sort(allComponentForUpdate.begin(), allComponentForUpdate.end(), [=](const auto& com1, const auto& com2) {return _replace_flag(com1, com2); });

		for (auto& com : allComponentForUpdate)com->update(dt);
	};

	//一致するコンポーネントを削除 下のオーバーロードの関数を呼び出す
	void remove(const Borrow<Component>& com)
	{
		remove(com->compType, com->id);
	}
	//コンポーネントの削除 コンポーネントの型が重複している場合下のif文でid指定で消す 削除したコンポーネントはgarbagesへ
	template<class T>
	void remove(const String& id = U"")
	{
		if (id == U"") components.remove<T>();
		else remove(typeid(T), id);
	}

	void remove(const std::type_index& compType, const String& id)
	{
		components.remove(compType, id);
	}

	//コンポーネントの追加　idがかぶったら上書き
	template<class T, class... Args>
	Borrow<T> addComponentNamed(const String& id, Args&&... args)
	{
		auto component = components.addIn<T>(id, args...);
		component->owner = this;
		component->start();
		component->index = componentsNum;
		component->compType = typeid(T);
		component->id = id;
		allComponentForUpdate << *component;
		componentsNum++;
		return *component;
	}
	//コンポーネントの追加
	template<class T, class... Args>
	Borrow<T> addComponent(Args&&... args)
	{
		auto component = components.add<T>(args...);
		component->owner = this;
		component->start();
		component->index = componentsNum;
		component->compType = typeid(T);
		component->id = Format(int32(components.ids[typeid(T)]) - 1);
		componentsNum++;
		allComponentForUpdate << *component;
		return *component;
	}

	//コンポーネントの取得　型の重複はなし
	template<class T>
	Borrow<T> getComponent()
	{
		if (auto res = components.get<T>())
		{
			return res->lend();
		}
		else {
			return Borrow<T>();
		}
	}

	template<class T>
	const Borrow<T> getComponent() const
	{
		if (auto res = components.get<T>())
		{
			return res->lend();
		}
		else {
			return Borrow<T>();
		}
	}

	//コンポーネントの取得 id指定。使いどころは複数のコンポーネントが同一の型で重複しているときとか。
	template<class T>
	Borrow<T> getComponent(const String& id)
	{
		if (auto res = components.get<T>(id))
		{
			return res->lend();
		}
		else {
			return Borrow<T>();
		}
	}

	String name = U"";

	Priority priority;
private:
	TypeContainer<Component> components;
	int32 componentsNum = 0;
	//すべてのコンポーネントをここにぶち込む priorityでソートするため 所有権は持たない
	Array<Borrow<Component>> allComponentForUpdate;

	//優先度で入れ替えを行うかどうか
	bool _replace_flag(const Borrow<Component>& s, const Borrow<Component>& other)
	{
		if (s->priority.getPriority() != other->priority.getPriority())return s->priority.getPriority() > other->priority.getPriority();
		return s->index < other->index;
	};

	template<class T>
	Optional<String> _get_id(const Borrow<Component>& com)
	{
		if (not components.arr.contains(typeid(T)))return none;
		for (const auto& component : components.arr[typeid(T)])
		{
			if (component.second == com)
			{
				return component.first;
			}
		}
		return none;
	}
};

class EntityManager {
	using EntityContainer = Array<std::pair<std::type_index, Entity*>>;

	EntityContainer pre;
	EntityContainer entitys;
	Array<Entity*> garbages;
	HashSet<Entity*> trashList;
private:
	EntityContainer MargedEntitys() {
		EntityContainer ret;
		return ret.append(entitys).append(pre);
	};

	void freshGarbages() {
		if (garbages.isEmpty())return;
		for (auto& ent : garbages)delete ent;
		garbages.clear();
	}

public:

	~EntityManager() {
		clean();
		freshGarbages();
	}

	void update(double dt) {
		//消去
		freshGarbages();

		//preからentitysに移す
		entitys.append(pre);
		pre.clear();

		//ソート
		std::stable_sort(
			entitys.begin(),
			entitys.end(),
			[=](const auto& ent1, const auto& ent2) { return ent1.second->priority.getPriority() > ent2.second->priority.getPriority(); }
		);

		//更新
		for (auto& entity : entitys) {
			entity.second->update(dt);
			entity.second->update_components(dt);
		}

		//killされたentityをがーべじへ
		for (auto it = entitys.begin(); it != entitys.end();) {
			if (trashList.contains(it->second)) {
				garbages << it->second;
				it = entitys.erase(it);
			}
			else {
				++it;
			}
		}
		for (auto it = pre.begin(); it != pre.end();) {
			if (trashList.contains(it->second)) {
				garbages << it->second;
				it = pre.erase(it);
			}
			else {
				++it;
			}
		}
		trashList.clear();
	}

	Array<Borrow<Entity>> allEntitys() {
		Array<Borrow<Entity>> result;
		for (auto& ent : MargedEntitys())result << *ent.second;
		return result;
	};

	template<class T>
	Array<Borrow<T>> find(Optional<String> name = none)
	{
		Array<Borrow<T>> result;
		std::type_index info = typeid(T);
		for (auto& ent : MargedEntitys())
		{
			if (ent.first == info)
			{
				//名前が指定されていない場合
				if (!name)result << static_cast<Borrow<T>>(*ent.second);
				//名前が指定されている場合
				else if (*name == ent.second->name)result << static_cast<Borrow<T>>(*ent.second);
			}
		}
		return result;
	}

	template<class T>
	Borrow<T> findOne(Optional<String> name = none)
	{
		auto ent = find<T>(name);
		if (ent.isEmpty())return Borrow<T>();
		else return *ent[0];
	}

	//Entityを作る
	template<class T = Entity, class... Args>
	Borrow<T> birth(Args&&... args) {
		auto entity = new T(args...);
		pre << std::pair<std::type_index, Entity*>(typeid(T), entity);
		entity->owner = this;
		entity->start();
		return *entity;
	}

	template<class T = Entity, class... Args>
	Borrow<T> birthNonStart(Args&&... args) {
		auto entity = new T(args...);
		pre << std::pair<std::type_index, Entity*>(typeid(T), entity);
		entity->owner = this;
		return *entity;
	}

	template<class T, std::enable_if_t<std::is_base_of_v<Entity, T>>* = nullptr>
	void relase(const Borrow<T>& ent)
	{
		//一致するentityを除外
		pre.remove_if([&](std::pair<std::type_index, Entity*> x) {return x.second == ent; });
		entitys.remove_if([&](std::pair<std::type_index, Entity*> x) {return x.second == ent; });
		garbages.remove_if([&](Entity* e) {return e == ent; });
		if (ent->owner == this)ent->owner = nullptr;
	}

	template<class T, std::enable_if_t<std::is_base_of_v<Entity, T>>* = nullptr>
	Borrow<T> add(T* ent)
	{
		//追加
		pre << std::pair<std::type_index, Entity*>(typeid(T), ent);
		//すでに所有者がいる場合
		if (auto other = ent->owner)
		{
			//entの所有権を手放させる
			other->relase(ent->lend());
		}
		else
		{
			//所有者がいないということはまだstartを走らせていない(そうじゃない場合はどこかでowner=nullptrをやってしまっている。カプセル化してできないようにすればよかった...)
			ent->start();
		}
		//entを所有
		ent->owner = this;

		return *ent;
	}

	//すべてガーベージに
	void clean()
	{
		for (auto& ent : MargedEntitys())garbages << ent.second;
		pre.clear();
		entitys.clear();
	}

	//killが確認できたらtrue 
	bool isKilled(const Borrow<Entity>& entity)
	{
		if (not entity)return false;
		//生ポインタをゲット　trashListまたはgarbagesに入っているかをチェック
		const auto& ptr = entity.get();
		return trashList.contains(ptr) or garbages.contains(ptr);
	}

	//一致するEntityをgarbagesへ
	void kill(const Borrow<Entity>& ent) {
		//子をkillする
		for (auto& child : ent->relation.getChildren())
		{
			child->owner->kill(*child);
		}
		//運命共同体もkillする
		for (auto& other : ent->sameDestiny)
		{
			other->sameDestiny.remove(*ent);
			other->owner->kill(other);
		}
		//trashListへ
		if (not trashList.contains(ent.get())) {
			ent->onTrashing();
			trashList.emplace(ent);
		}
	}
	//名前が一致するEntityをgarbagesへ
	void kill(const String& name)
	{
		for (auto it = entitys.begin(), en = entitys.end(); it != en; ++it)
		{
			if (it->second->name == name) {
				kill(*it->second);
			}
		}
		for (auto it = pre.begin(), en = pre.end(); it != en; ++it)
		{
			if (it->second->name == name) {
				kill(*it->second);
			}
		}
	}
};

static bool IsKilled(const Borrow<Entity>& entity)
{
	return entity->owner->isKilled(entity);
}
