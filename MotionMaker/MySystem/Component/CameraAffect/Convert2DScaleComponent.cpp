#include "../../stdafx.h"
#include "Convert2DScaleComponent.h"

Convert2DScaleComponent::Convert2DScaleComponent(const Borrow<Transform>& convTarget, const util::Convert2DScale& converter)
	:converter(converter),target(convTarget)
{
}

Convert2DScaleComponent::Convert2DScaleComponent(const Borrow<Transform>& convTarget, double baseLength, DrawManager* dm, const Camera::DistanceType& type)
	:converter(util::Convert2DScale(baseLength, dm, type)), target(convTarget)
{
}

void Convert2DScaleComponent::convert()
{
	target->scale.aspect = converter.convert(ownerTransform);
}

void Convert2DScaleComponent::start()
{
	Component::start();

	ownerTransform = owner->getComponent<Transform>(U"original");

	convert();
}

void Convert2DScaleComponent::update(double dt)
{
	Component::update(dt);

	convert();
}

