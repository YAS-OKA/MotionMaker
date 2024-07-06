#include "../stdafx.h"
#include "Object.h"
#include"../GameMaster.h"
#include"../Component/Transform.h"
#include"Scenes.h"

void Object::start()
{
	Entity::start();
	name = U"Object";
	transform = addComponentNamed<Transform>(U"original");
	scene->entitysTransform << transform;
}

void Object::startAction(StringView actionName)
{
	actman.act(actionName);
}

void Object::stopAction(StringView actionName)
{
	actman.stop(actionName);
}

void Object::update(double dt)
{
	dt *= timeScale;
	Entity::update(dt);
	actman.update(dt);
}

Vec3 Object::convertAbsPos(const Vec3& relativePos)
{
	return relativePos + transform->getPos();
}

//アクションの追加、削除などはアクションの外で行う
prg::Actions& Object::ACreate(StringView actionName,bool startAction,bool loopAction)
{
	return actman.create(actionName, startAction, loopAction);
}

void Object::onTrashing()
{
	Entity::onTrashing();
}

void Object::die()
{
	owner->kill(*this);
}
