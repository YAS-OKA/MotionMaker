#pragma once
#include"../EC.hpp"
#include"Transform.h"
#include"../Figure.h"

//当たり判定は今のところ回転できない！ 2Dをのぞく
class CollideBox
{
public:
	class CollideFigure;
	using Shape = std::variant<Box, Cylinder, CollideFigure>;

	class CollideFigure
	{
	protected:
		Figure figure;
		double rad = 0;
	public:
		//bool layerd;
		CollideBox* parent;

		CollideFigure(const Figure& fig);

		Figure getFigure()const;

		virtual Polygon getScaledFig()const;

		void setFigure(const Figure& fig);

		void operator=(const Figure& fig);

		bool intersects(const CollideFigure& fig)const;

		void setWH(double w, double h);

		void setR(double r);

		virtual void setPos(const Vec3& pos);

		virtual void rotateAt(const Vec2& pos,double rad);

		void setAngle(const Vec2& pos, double rad);

		void scale(const Vec2& s);

		RectF boudingRectF()const;
	};

	//FigureはFigureとしか衝突できない
	Shape shape;

	Vec3 relative{ 0,0,0 };
	
	Transform* t;

	void update();

	template<class Shape>
	CollideBox(const Shape& _shape,const Vec3& relative)
		:shape(_shape),relative(relative)
	{
		if (shape.index() == 2) {
			auto& tmp=std::get<2>(shape);
			tmp.parent = this;
			tmp.setFigure(tmp.getFigure());
		}
	}

	Box boudingBox()const;

	bool intersects(const CollideBox& other)const;
private:
	bool intersects(const Box& box)const;
	bool intersects(const Cylinder& cylinder)const;
	bool intersects(const Box& box, const Cylinder& cylinder)const;
	bool intersects(const CollideFigure& fig)const;
};

enum class ColliderCategory
{
	hero,
	enemy,
	object,
	non,
};

class Collider :public Component
{
private:
	//自分の
	ColliderCategory category = ColliderCategory::non;
public:
	CollideBox hitbox;

	static HashSet<ColliderCategory> AllCategory;

	Borrow<Transform> transform;

	bool collidable = true;

	template<class Shape>
	Collider(const Shape& shape, const Vec3& relative = { 0,0,0 })
		:hitbox(shape, relative)
	{}

	Collider(const CollideBox& box)
		:hitbox(box)
	{}

	~Collider();

	void start()override;

	void update(double dt)override;

	Array<Borrow<Entity>> intersects(const HashSet<ColliderCategory> targets)const;

	Array<Borrow<Entity>> intersects(const ColliderCategory& target)const;

	Array<Borrow<Entity>> intersectsAll()const;

	bool intersects(const Borrow<Collider>& target)const;

	void setCategory(const ColliderCategory& category);

	const ColliderCategory getCategory()const;

	Figure getFig()const
	{
		return std::get<2>(hitbox.shape).getFigure();
	};
};
