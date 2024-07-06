#pragma once
#include"../../EC.hpp"

#include"../../Game/Utilities.h"
#include"../../Motions/Parts.h"

using namespace util;
using namespace mot;

//ownerTransformを2D上に投影した座標をtargetにセットする
class Convert2DTransformComponent :public Component
{
public:
	Convert2DTransform converter;

	Borrow<Transform> target;

	Borrow<Transform> ownerTransform;
	
	Convert2DTransformComponent(const Borrow<Transform>& convTarget, const Convert2DTransform& converter);

	Convert2DTransformComponent(const Borrow<Transform>& convTarget, DrawManager* dm, const ProjectionType& type = ProjectionType::Parallel);

	void convert();
	
	void start()override;
	
	void update(double dt)override;
};
