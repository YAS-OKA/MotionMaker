#include "../stdafx.h"
#include "Scenes.h"
#include"../GameMaster.h"
#include"Object.h"
#include"../Motions/Parts.h"
#include"../Util/Util.h"

namespace my
{
	void createCollider(const Borrow<Entity>& ent, const Box& shape, const Vec3& relative)
	{
		auto col = ent->addComponentNamed<Collider>(U"hitbox", shape, relative);
		auto box = col->hitbox.boudingBox();
		auto s = shape;
		s.h = Min(1.0, shape.h * 0.05);
		s.w = shape.w * 0.7;
		s.d = shape.d * 0.7;
		ent->addComponentNamed<Collider>(U"bottom", s, box.bottomCenter());
		ent->addComponentNamed<Collider>(U"top", s, box.topCenter());
	}
	void createCollider(const Borrow<Entity>& ent, const Cylinder& shape, const Vec3& relative)
	{
		auto col = ent->addComponentNamed<Collider>(U"hitbox", shape, relative);
		auto box = col->hitbox.boudingBox();
		auto s = shape;
		s.h = Min(1.0, shape.h * 0.05);
		s.r = shape.r * 0.7;
		ent->addComponentNamed<Collider>(U"bottom", s, box.bottomCenter());
		ent->addComponentNamed<Collider>(U"top", s, box.topCenter());
	}

	void Scene::hitboxVisible(const MeshData& data)
	{
	}

	Borrow<GameMaster> Scene::master = nullptr;

	Scene::Scene()
		:drawManager(ColorF{ 0.4, 0.6, 0.8 }.removeSRGBCurve())
	{
		partsLoader = new mot::LoadParts(*this);
	}

	Scene::~Scene()
	{
		delete partsLoader;
	}

	EntityManager* Scene::getEntityManager()
	{
		return &entityManager;
	}

	DrawManager* Scene::getDrawManager()
	{
		return &drawManager;
	}

	void Scene::start()
	{
		Entity::start();

		transform = addComponent<Transform>();
	}

	void Scene::update(double dt)
	{
		Entity::update(dt);

		actman.update(dt);

		entityManager.update(dt);
		
		drawManager.update();

		if (drawOnUpdating)draw();
	}

	void Scene::draw() const
	{
		Transformer2D t{ Mat3x2::Translate(-screenPos.asPoint()) };
		ScopedViewport2D v{ transform->getXY().asPoint(),screenSize};

		const Transformer2D t1{ Mat3x2::Scale(transform->scale.aspect.vec.xy(),transform->getPos().xy()),TransformCursor::Yes};
		const Transformer2D t2{ Mat3x2::Rotate(util::getRad(transform->getDirection().xy())) };
		drawManager.draw();
	}

	void Scene::updateTransform(double dt)
	{
		entitysTransform.remove_if([](const auto& p) {return not p; });
		for (auto& t : entitysTransform)t->calUpdate(dt);
	}
}
