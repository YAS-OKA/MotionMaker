#pragma once
#include"../../EC.hpp"

#include"../../Game/Utilities.h"

class Convert2DScaleComponent:public Component
{
public:
	util::Convert2DScale converter;

	Borrow<Transform> target;

	Borrow<Transform> ownerTransform;

	Convert2DScaleComponent(const Borrow<Transform>& convTarget, const util::Convert2DScale& converter);

	Convert2DScaleComponent(const Borrow<Transform>& convTarget, double baseLength,DrawManager* dm, const Camera::DistanceType& type = Camera::DistanceType::Screen);

	void convert();

	void start()override;

	void update(double dt)override;	
};

