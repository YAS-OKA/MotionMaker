#include "../../stdafx.h"
#include "PartsMirrored.h"
#include"../../Motions/Parts.h"
#include"../../Component/Draw.h"

PartsMirrored::PartsMirrored(const Borrow<mot::PartsManager>& pman, bool m)
	:pman(pman), active(false), mirrored(false)
{
	if (m)
	{
		pman->dm->drawing.scale.x *= -1;
	}
}

bool PartsMirrored::getMirrored() const
{
	return mirrored;
}

void PartsMirrored::start()
{
	Component::start();
	transform = owner->getComponent<Transform>();
}

void PartsMirrored::update(double dt)
{
	Component::update(dt);
	//反転を検知
	if (not active)
	{
		if ((mirrored and transform->direction.vector.x > 0.5) or
			((not mirrored) and transform->direction.vector.x < -0.5))
		{
			active = true;
			firstScale = pman->dm->drawing.scale.x;
			timer = 0;
		}
	}
	//反転中
	if (active)
	{
		timer += dt;
		if (timer >= mirroredTime) {
			timer = mirroredTime;
			mirrored = !mirrored;
			active = false;
		}
		auto& s = pman->dm->drawing.scale.x;
		s = firstScale * (1 - 2 * timer / mirroredTime);
		if (0 < s and s < zero)s = zero;
		if (-zero < s and s <= 0)s = -zero;
	}
}
