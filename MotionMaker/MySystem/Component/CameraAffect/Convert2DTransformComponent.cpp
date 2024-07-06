#include "../../stdafx.h"
#include "Convert2DTransformComponent.h"

Convert2DTransformComponent::Convert2DTransformComponent(const Borrow<Transform>& t, const Convert2DTransform& converter)
	:target(t), converter(converter)
{
}

Convert2DTransformComponent::Convert2DTransformComponent(const Borrow<Transform>& t, DrawManager* dm, const ProjectionType& type)
	:target(t), converter(Convert2DTransform(dm, type))
{
}

void Convert2DTransformComponent::convert()
{
	target->setPos(converter.convert(ownerTransform));
}

void Convert2DTransformComponent::start()
{
	Component::start();

	ownerTransform = owner->getComponent<Transform>(U"original");

	convert();
}

void Convert2DTransformComponent::update(double dt)
{
	Component::update(dt);

	convert();
}
