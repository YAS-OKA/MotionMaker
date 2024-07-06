#pragma once
#include"../EC.hpp"
#include"../Prg/Init.h"
#include"../Component/Field.h"

namespace my {
	class Scene;
}
namespace skill {
	class ISkillEffect;
}
class Transform;
class Collider;
using SkillEffectContainer = WeakTypeContainer<skill::ISkillEffect>;

//前方宣言↑
//シーンオブジェクト
class Object :public Entity
{
public:
	Borrow<my::Scene> scene;

	Borrow<Transform> transform;

	SkillEffectContainer skillEffects;

	ActionManager actman;

	double timeScale = 1.0;

	//相対距離を絶対距離に
	Vec3 convertAbsPos(const Vec3& relativePos);

	prg::Actions& ACreate(StringView actionName, bool immediatelyStart = false, bool loopAction = false);

	virtual void onTrashing()override;

	virtual void start()override;

	virtual void update(double dt)override;

	void startAction(StringView actionName);

	void stopAction(StringView actionName);

	void die();
};
