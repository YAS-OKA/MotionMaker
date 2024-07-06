#include "stdafx.h"
#include "GameMaster.h"
#include"Component/Transform.h"
#include"Game/Scenes.h"
#include"Game/Object.h"

struct GameMaster::Impl
{
	EntityManager manager;
	double timeScale = 1.0;
	Borrow<GameMaster> master;
	Borrow<my::Scene> scene;
	Impl()
	{

	}

	void update(double dt)
	{
		manager.update(dt);
	}
};

void GameMaster::start()
{
	my::Scene::master = *this;
	p_impl = std::make_shared<Impl>();
	p_impl->master = *this;
}

#if _DEBUG&&0
bool push = false;
#endif

void GameMaster::update(double dt)
{
#if _DEBUG&&0
	if (Key0.down())push = true;
	if (not push)dt = 0;
#endif
	p_impl->update(dt);
}

void GameMaster::addScene(my::Scene* scene)
{
	p_impl->manager.add(scene);
}

EntityManager& GameMaster::getManager()
{
	return p_impl->manager;
}

