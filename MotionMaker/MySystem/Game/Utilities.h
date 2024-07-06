#pragma once
#include"../Prg/Action.h"
#include"../EC.hpp"
#include"Object.h"
#include"Camera.h"

class Collider;
class Character;
class DrawManager;
class Transform;
class Influence;

namespace mot {
	class PartsManager;
}

namespace my{
	class Scene;
}

namespace util
{
	enum class ProjectionType
	{
		Parallel,
		Parse,
	};

	void loadTexture(my::Scene* scene, const String& name, const String& path);
	//三次元に投影
	class Convert2DTransform
	{
	public:
		ProjectionType type;

		DrawManager* dManager;

		Convert2DTransform(DrawManager* dManager, const ProjectionType& type=ProjectionType::Parallel);

		Vec3 convert(const Vec3& pos, const BasicCamera3D& camera)const;

		Vec3 convert(const Vec3& pos, const ProjectionType& type)const;

		Vec3 convert(const Vec3& pos)const;

		Vec3 convert(const Borrow<Transform>& transform)const;
	};
	//カメラからの距離をもとにスケーリングする　遠近法
	class Convert2DScale
	{
	public:
		Camera::DistanceType type;

		Borrow<Camera> camera = nullptr;

		DrawManager* dManager;

		double baseLength;
		//カメラの距離に応じて
		Convert2DScale(double baseLength, DrawManager* dManager);
		//typeに応じて
		Convert2DScale(double baseLength,DrawManager* dManager, const Camera::DistanceType& type);
		//baseLength/distanceを返す
		double distanceRate(const Vec3& pos, const Camera::DistanceType& type)const;
		double distanceRate(const Vec3& pos)const;

		Vec3 convert(const Vec3&scale, const Vec3& pos, const Camera::DistanceType& type)const;

		Vec3 convert(const Vec3& scale,const Vec3& pos)const;

		Vec3 convert(const Borrow<Transform>& transform)const;
	};

	class MouseObject :public Object
	{
	public:
		DrawManager* manager;

		Borrow<Collider> mouseHitbox;

		HashTable<String, Array<Borrow<Collider>>> collideDict;

		void start()override;

		void update(double dt)override;

		Array<Borrow<Collider>>& getColliderArray(StringView kind);
		Array<Borrow<Collider>> getColliderArray(StringView kind) const;

		Vec3 getCursorPos(Influence* dManagerInfluence)const;

		Array<Borrow<Entity>> getClickedObjects(const Array<Borrow<Collider>>& colliders) const;

		Array<Borrow<Entity>> getClickedObjects(StringView collideKind)const;

		enum SurfaceType {
			depth,
			z,
		};
		//クリックされた表面のエンティティを返す
		Borrow<Entity> getClickedSurfaceObject(Array<Borrow<Collider>> collider, const SurfaceType& type = depth);
		//クリックされた表面のエンティティを返す　2D(FigureCollider)を優先する
		Borrow<Entity> getClickedSurfaceObject2DPrior(Array<Borrow<Collider>> colliders, const SurfaceType& type2d = z, const SurfaceType& type3d = depth);

		//クリックされた表面のエンティティを返す
		Borrow<Entity> getClickedSurfaceObject(StringView collideKind, const SurfaceType& type = depth);
		//クリックされた表面のエンティティを返す　2D(FigureCollider)を優先する
		Borrow<Entity> getClickedSurfaceObject2DPrior(StringView collideKind, const SurfaceType& type2d = z, const SurfaceType& type3d = depth);
	};	
}
