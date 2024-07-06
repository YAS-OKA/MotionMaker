#pragma once
#include"../EC.hpp"
#include"../DrawManager.h"
#include"../Component/Draw.h"
#include"../Component/Collider.h"
#include"../Asset/AssetManager.h"

class GameMaster;
class Camera;

namespace mot
{
	class LoadParts;
}

namespace my
{
	//template<class Shape>
	//void createCollider(Entity* ent,const Shape& shape, const Vec3& relative)
	//{
	///*	auto col=ent->addComponentNamed<Collider>(U"hitbox",shape, relative);
	//	auto box = col->hitbox.boudingBox();*/
	//	//auto s = shape;
	//	//s.h = Min(1.0, shape.h * 0.05);
	//	//ent->addComponentNamed<Collider>(U"bottom",s, box.bottomCenter());
	//	//ent->addComponentNamed<Collider>(U"top", s, box.topCenter());

	//}
	//シーンを持ったオブジェクト
	template<class T>
	concept HasMemberScene = requires (T & x)
	{
		x.scene;
	};

	void createCollider(const Borrow<Entity>& ent, const Box& shape, const Vec3& relative);
	void createCollider(const Borrow<Entity>& ent, const Cylinder& shape, const Vec3& relative);

	class Scene :public Entity
	{
	protected:
		EntityManager entityManager;
		DrawManager drawManager;
		void hitboxVisible(const MeshData& data);
	public:
		static Borrow<GameMaster> master;

		bool drawOnUpdating = true;

		Size screenSize=s3d::Scene::Size();
		Vec2 screenPos{ 0,0 };

		Borrow<Camera> camera;

		RegisterAssets r;
		mot::LoadParts* partsLoader;
		Array<Borrow<Transform>> entitysTransform;
		//シーンを描画する位置
		//他のシーンと親子関係が結べるようになる。
		Borrow<Transform> transform;

		ActionManager actman;

		Scene();

		virtual ~Scene();

		//entityManager.birth<T>(...)->scene=thisを省略
		template<HasMemberScene T = Object , class ...Args>
		Borrow<T> birth(Args&& ...args)
		{
			auto obj = entityManager.birthNonStart<T>(args...);
			obj->scene = *this;
			obj->start();
			return obj;
		}

		template<class T, class ...Args>
		Borrow<T> birth(Args&& ...args)
		{
			auto obj = entityManager.birth<T>(args...);
			return obj;
		}
		
		//Hitboxがついてくる Box
		template<class T=Object,class ...Args>
		Borrow<T> birthObject(const Box& shape,const Vec3& relative,Args&& ...args)
		{
			auto obj=entityManager.birthNonStart<T>(args...);
			obj->scene = *this;
			obj->start();
			createCollider(obj,shape, relative);
			auto vis = obj->addComponentNamed<Draw3D>(U"hitbox", getDrawManager(), MeshData::Box(shape.size));
			vis->relative = relative;
			vis->color = ColorF{ Palette::Red,0.5 };
			vis->visible = false;
			return obj;
		}
		//Hitboxがついてくる Cylinder
		template<class T = Object, class ...Args>
		Borrow<T> birthObject(const Cylinder& shape, const Vec3& relative, Args&& ...args)
		{
			auto obj = entityManager.birthNonStart<T>(args...);
			obj->scene = *this;
			obj->start();
			createCollider(obj,shape, relative);
			auto vis = obj->addComponentNamed<Draw3D>(U"hitbox", getDrawManager(), MeshData::Cylinder(shape.r, shape.h));
			vis->relative = relative;
			vis->color = ColorF{ Palette::Red,0.5 };
			vis->visible = false;
			return obj;
		}
		//Hitboxがついてくる
		template<class T = Object, class ...Args>
		Borrow<T> birthObject(const Figure& shape, const Vec3& relative, Args&& ...args)
		{
			auto obj = entityManager.birthNonStart<T>(args...);
			obj->scene = *this;
			obj->start();
			obj->addComponent<Collider>(shape, relative);
			//createCollider(obj, shape, relative);
			auto vis = obj->addComponentNamed<Draw2D<Figure>>(U"hitbox", getDrawManager(), shape);
			vis->relative = relative;
			vis->color = ColorF{ Palette::Red,0.6 };
			vis->visible = false;
			return obj;
		}
		template<class T>
		Borrow<T> findOne(Optional<String> name=none)
		{
			return entityManager.findOne<T>(name);
		}
		template<class T>
		Array<Borrow<T>> find(Optional<String> name=none)
		{
			return entityManager.find<T>(name);
		}

		EntityManager* getEntityManager();

		DrawManager* getDrawManager();

		virtual void start()override;

		virtual void update(double dt);

		virtual void draw()const;

		void updateTransform(double dt);
	};
}
