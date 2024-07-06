#include"../stdafx.h"
#include"../Component/Transform.h"
#include"../Util/Util.h"
#include"Utilities.h"
#include"Camera.h"
#include"Scenes.h"
#include"../Motions/Parts.h"
#include"../Component/Draw.h"

namespace util
{
	void MouseObject::start()
	{
		Object::start();

		mouseHitbox = addComponent<Collider>(CollideBox::CollideFigure{ Circle{ 0.5 } });

		manager = scene->getDrawManager();

		//transform->setParent(manager->getCamera()->transform);
	}

	void MouseObject::update(double dt)
	{
		Object::update(dt);

		transform->setLocalPos({ Cursor::PosF(), 0});
	}

	Array<Borrow<Collider>>& MouseObject::getColliderArray(StringView kind)
	{
		if (not collideDict.contains(kind)) {
			collideDict[kind] = Array<Borrow<Collider>>();
		}
		return collideDict[kind];
	}

	Array<Borrow<Collider>> MouseObject::getColliderArray(StringView kind)const
	{
		if (not collideDict.contains(kind)) {
			return Array<Borrow<Collider>>();
		}
		return collideDict.at(kind);
	}

	Vec3 MouseObject::getCursorPos(Influence* influence)const
	{
		Vec2 pos = transform->getPos().xy();
		pos += influence->getMovement() * manager->translate;
		pos += influence->getScale() * (manager->scalePos + util::sc() - Cursor::PosF()) * (Vec2{ 1,1 } - Vec2{ 1,1 } / manager->scale);

		return { pos,transform->getPos().z };
	}

	Array<Borrow<Entity>> MouseObject::getClickedObjects(const Array<Borrow<Collider>>& colliders)const
	{
		Array<Borrow<Entity>> ret{};
		const auto& camera = manager->getCamera();
		
		const auto& fp = camera->getForcusDir();

		//クリックした当たり判定からエンティティを取得
		for (auto& collider : colliders)
		{
			auto dInfluence = collider->owner->getComponent<Field<Influence>>(U"dManagerInfluence");
			Vec2 move{ 0,0 };
			if (dInfluence != nullptr)
			{
				move = dInfluence->value.getMovement() * manager->translate;
				move += dInfluence->value.getScale() * (manager->scalePos + util::sc() - Cursor::PosF()) * (Vec2{ 1,1 } - Vec2{ 1,1 } / manager->scale);
			}

			Ray ray = camera->getCamera().screenToRay(Cursor::PosF());
			if (move.length() != 0)
			{
				Vec3 move3D = Quaternion::RotationAxis(fp, move.getAngle()) * camera->getCamera().getUpDirection() * move.length();
				auto c = camera->getCamera();
				c.setView(camera->getEyePos() + move3D, camera->getForcusPos() + move3D);
				ray = c.screenToRay(Cursor::PosF());
			}

			if (dInfluence != nullptr)std::get<2>(mouseHitbox->hitbox.shape).setPos(getCursorPos(&dInfluence->value));

			
			const auto& hitbox = collider->hitbox;

			switch (hitbox.shape.index())
			{
			case 0:
				if (std::get<0>(hitbox.shape).intersects(ray))ret << *collider->owner;
				break;
			case 1:
				if (ray.intersects(std::get<1>(hitbox.shape)))ret << *collider->owner;
				break;
			case 2:
				if (collider->intersects(mouseHitbox))ret << *collider->owner;
				break;
			}
		}
		return ret;
	}

	Array<Borrow<Entity>> MouseObject::getClickedObjects(StringView collideKind) const
	{
		return getClickedObjects(getColliderArray(collideKind));
	}

	Borrow<Entity> MouseObject::getClickedSurfaceObject(Array<Borrow<Collider>> colliders,const SurfaceType& type)
	{
		auto obj = getClickedObjects(colliders);

		Borrow<Entity> ret;
		//表面のエンティティを求める
		if (type == depth)
		{
			double distance = Math::Inf;
			for (const auto& ent : obj)
			{
				double _distance = manager->getCamera()->distance(ent->getComponent<Transform>(U"original")->getPos());
				if (distance > _distance) { distance = _distance; ret = ent; }
			}
		}
		else if (type == z)
		{
			double z = -Math::Inf;
			for (const auto& ent : obj)
			{
				double zz = ent->getComponent<Transform>()->getPos().z;
				if (z < zz) { z = zz; ret = ent; }
			}
		}

		return ret;
	}

	Borrow<Entity> MouseObject::getClickedSurfaceObject2DPrior(Array<Borrow<Collider>> colliders, const SurfaceType& type2d, const SurfaceType& type3d)
	{
		Array<Borrow<Collider>> col3D{};
		Array<Borrow<Collider>> col2D{};

		//クリックした当たり判定からエンティティを取得
		for (const auto& collider : colliders)
		{
			switch (collider->hitbox.shape.index())
			{
			case 0:
				col3D << collider;
				break;
			case 1:
				col2D << collider;
				break;
			case 2:
				col2D << collider;
				break;
			}
		}

		Borrow<Entity> ret;

		ret = getClickedSurfaceObject(col2D, type2d);

		return (bool)ret ? ret : getClickedSurfaceObject(col3D, type3d);
	}

	Borrow<Entity> MouseObject::getClickedSurfaceObject(StringView collideKind, const SurfaceType& type)
	{
		return getClickedSurfaceObject(getColliderArray(collideKind), type);
	}

	Borrow<Entity> MouseObject::getClickedSurfaceObject2DPrior(StringView collideKind, const SurfaceType& type2d, const SurfaceType& type3d)
	{
		return getClickedSurfaceObject2DPrior(getColliderArray(collideKind), type2d, type3d);
	}

	Convert2DTransform::Convert2DTransform(DrawManager* dmanager,const ProjectionType& type)
		:dManager(dmanager),type(type)
	{
	}

	Vec3 Convert2DTransform::convert(const Vec3& pos, const BasicCamera3D& camera)const
	{
		const SIMD_Float4 worldPos = DirectX::XMVector3TransformCoord(
			SIMD_Float4{ pos, 0.0f },
			camera.getViewProj());

		auto v = worldPos.xy();

		v.y *= -1.0f;
		v.y += 1.0f;
		v.x += 1.0f;
		v.y *= (camera.getSceneSize().y * 0.5f);
		v.x *= (camera.getSceneSize().x * 0.5f);

		//カメラのフォーカス方向との内積をとる(スクリーン距離)
		const auto& dotProduct = (pos - camera.getEyePosition()) * ((camera.getFocusPosition() - camera.getEyePosition()).normalize());
		const auto& dotProductValue = dotProduct.x + dotProduct.y + dotProduct.z;

		return { v,dotProductValue };
	}

	Vec3 Convert2DTransform::convert(const Vec3& pos,const ProjectionType& type)const
	{
		const auto& myCamera = dManager->getCamera();
		auto camera = myCamera->getCameraLatest();

		auto tmp = convert(pos, camera);
		if (type == ProjectionType::Parse)return tmp;
		if (type == ProjectionType::Parallel)
		{
			Vec2 v; double dotProductValue;
			v = tmp.xy(); dotProductValue = tmp.z;
			const auto& screenDistance = myCamera->getScreenDistance();
			const auto& dotProductParScreenDistance = dotProductValue / screenDistance;//(camera.screenToWorldPoint(util::sc(), 0.1f)-camera.getEyePosition()).length();
			return { v * (1 + dotProductParScreenDistance) - util::sc() * dotProductParScreenDistance,dotProductValue };
		}
	}

	Vec3 Convert2DTransform::convert(const Vec3& pos)const
	{
		return convert(pos, type);
	}

	Vec3 Convert2DTransform::convert(const Borrow<Transform>& transform)const
	{
		return convert(transform->getPos());
	}

	Convert2DScale::Convert2DScale(double baseLength, DrawManager* dManager)
		:baseLength(baseLength), dManager(dManager),camera(dManager->getCamera())
	{
	}

	Convert2DScale::Convert2DScale(double baseLength, DrawManager* dmanager, const Camera::DistanceType& type)
		:baseLength(baseLength), dManager(dmanager), type(type)
	{
	}

	double Convert2DScale::distanceRate(const Vec3& pos, const Camera::DistanceType& type) const
	{
		const auto& cameraDistance = dManager->getCamera()->distance(pos, type);
		return cameraDistance == 0 ? 0 : baseLength / cameraDistance;
	}

	double Convert2DScale::distanceRate(const Vec3& pos) const
	{
		return distanceRate(pos, type);
	}

	Vec3 Convert2DScale::convert(const Vec3& scale, const Vec3& pos, const Camera::DistanceType& type)const
	{
		return scale * distanceRate(pos, type);
	}

	Vec3 Convert2DScale::convert(const Vec3& scale,const Vec3& pos)const
	{
		if (camera)return convert(scale, pos, camera->type);
		else return convert(scale,pos, type);
	}

	Vec3 Convert2DScale::convert(const Borrow<Transform>& transform)const
	{
		return convert(transform->scale.aspect.vec, transform->getPos());
	}
	void loadTexture(my::Scene* scene, const String& name, const String& path)
	{
		//アセットが登録されていなければ
		if (not scene->r.textures.contains(name))
		{
			scene->r.type = RegisterAssets::AssetType::texture;
			using namespace resource;
			texture::Register(scene->r(name), path, TextureDesc::Mipped);
			resource::texture::Load(name);
		}
	}
}
