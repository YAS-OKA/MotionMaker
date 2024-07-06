#pragma once
#include"../EC.hpp"
#include"Transform.h"
#include"../Game/Camera.h"
#include"../Game/Utilities.h"
#include"../Figure.h"
#include"Field.h"

class DrawManager;
class IDraw2D;
//なぜかTransformer2D::Scale(aspect_)でaspect_が0,1の時にアサートが出るので,0はzeroで代用
constexpr double zero = 0.001;

namespace draw_helper {
	//　スケーリング
	class ScaleHelper2D
	{
	public:
		virtual Vec2 getScalePos()const { return { 0,0 }; };

		virtual double operator ()() const = 0;
	};
	// とくにスケーリングしない　通常
	class NonScaleByCamera :public ScaleHelper2D
	{
	public:
		virtual double operator ()() const { return 1; };
	};
	//透視図法のスケーリング
	class ScaleCalculation2D:public ScaleHelper2D
	{
	public:
		util::Convert2DScale converter;

		Borrow<Transform> transform;

		ScaleCalculation2D(const Borrow<Transform>& transform, DrawManager* m, double baseLength=100,const Camera::DistanceType& type=Camera::DistanceType::XYZ);

		virtual double operator ()() const;
	};
	//drawManagerはこれをもとに描く順番を決める。
	class DrawShallow
	{
	public:
		double layer = 0;

		double m_depth = 0;

		Borrow<IDraw2D> owner;

		DrawShallow(const Borrow<IDraw2D>& owner);

		virtual void cal_depth();

		virtual double getDepth()const;

		bool shouldReplace(DrawShallow* other)const;
	};

}

using namespace draw_helper;

struct Influence
{
	double XScale = 1;
	double YScale = 1;
	double XMovement = 1;
	double YMovement = 1;

	double Rotate = 1;

	void SetScaleInfluence(double per)
	{
		SetScaleXInfluence(per);
		SetScaleYInfluence(per);
	}

	void SetScaleXInfluence(double per)
	{
		XScale = per;
	};

	void SetScaleYInfluence(double per)
	{
		YScale = per;
	};

	void SetMoveInfluence(double per)
	{
		SetMoveXInfluence(per);
		SetMoveYInfluence(per);
	}

	void SetMoveXInfluence(double per)
	{
		XMovement = per;
	}

	void SetMoveYInfluence(double per)
	{
		YMovement = per;
	}

	void SetInfluence(double scalePer, double movePer,double rotatePer)
	{
		SetScaleInfluence(scalePer);
		SetMoveInfluence(movePer);
		Rotate = rotatePer;
	}

	Vec2 getScale()const
	{
		return { XScale,YScale };
	}

	Vec2 getMovement()const
	{
		return { XMovement,YMovement };
	}
};

class IDrawing :public Component
{
	double m_distanceFromCamera = 0;
	Vec3 m_drawPos;
public:
	bool visible = true;
	//transformの影響
	bool transformDirectionAffectable = true;
	bool transformScaleAffectable = true;

	ColorF color = Palette::White.removeSRGBCurve();
	//xyz比
	Transform::Scale aspect;

	Borrow<Transform> transform;
	//相対座標
	Vec3 relative{ 0,0,0 };

	Transform::Direction direction;

	Vec3 rotateCenter{ 0,0,0 };

	DrawManager* manager;

	IDrawing(DrawManager* manager);

	virtual void start();

	virtual void draw()const = 0;

	void cal_drawPos();

	virtual Vec3 getDrawPos()const;

	void cal_distanceFromCamera();

	double distanceFromCamera()const;
};

#include"../DrawManager.h"

//:3D描画
class IDraw3D :public IDrawing
{
public:
	using IDrawing::IDrawing;

	virtual ~IDraw3D();

	void start() override {
		IDrawing::start();
		manager->set3D(*this);
	};
	virtual void draw()const = 0;
};

template<class Draw>
struct Frame {
	Frame(const Draw& fig, double t)
		:figure(fig), t(t) {}

	double t = 2;
	Draw figure;
	void draw(const ColorF& color)const {
		figure.drawFrame(t, color);
	};
	Draw movedBy(const Vec2& pos)const
	{
		return figure.movedBy(pos);
	}
};

//2D描画
class IDraw2D :public IDrawing
{
public:
	ScaleHelper2D* _scale_calculation = nullptr;
	DrawShallow* shallow = nullptr;
	
	Borrow<Field<Influence>> dManagerInfluence;

	Borrow<Field<Influence>> cameraInfluence;//まだ使ってない

	IDraw2D(DrawManager* manager);

	virtual Quad getRegion()const { return Quad{}; };

	virtual ~IDraw2D();

	RectF viewport{ 0,0,Scene::Size() };
	void start() override{
		IDrawing::start();
		manager->set2D(*this);
		_scale_calculation = new NonScaleByCamera();
		shallow = new DrawShallow(*this);
		dManagerInfluence = owner->getComponent<Field<Influence>>(U"dManagerInfluence");
		if (not dManagerInfluence)dManagerInfluence = owner->addComponentNamed<Field<Influence>>(U"dManagerInfluence", Influence());
	};

	template<class ScaleHelper,class... Args>
	ScaleHelper* setScaleHelper(Args&&... args)
	{
		if (_scale_calculation != nullptr)delete _scale_calculation;

		auto ret= new ScaleHelper(transform, manager, args...);

		_scale_calculation = ret;

		return ret;
	}

	Vec2 getScale()const;

	Vec2 getScalePos()const;

	template<class Shallow, class... Args>
	Shallow* setShallow(Args&&... args)
	{
		if (shallow != nullptr)	delete shallow;
		
		auto res = new Shallow(this, args...);

		shallow = res;

		return res;
	}

	DrawShallow* getShallow()const;

	virtual Transformer2D getTransformer()const;

	virtual void draw()const = 0;
};

class Draw3D :public IDraw3D
{
public:
	Mesh mesh{};
	Texture tex;

	Draw3D(DrawManager* manager, const MeshData& data);

	virtual void setAssetName(const String& name);

	virtual void draw()const override;
};

//class Billboard final :public Draw3D
//{
//public:
//	Billboard(DrawManager* manager);
//
//	void draw()const override;
//};

template<class Drawing2D>
class Draw2D :public IDraw2D
{
public:
	Drawing2D drawing;

	template<class... Args>
	Draw2D(DrawManager* manager, Args&&... args)
		:IDraw2D(manager), drawing(args...)
	{}

	Drawing2D getDrawing()const
	{
		Vec2 aspect_ = getScale();
		auto fig = drawing.movedBy(getDrawPos().xy()- manager->getCamera()->transform->getPos().xy() - viewport.tl());

		if constexpr (typeid(Drawing2D) == typeid(RectF))
		{
			return fig.scaled(aspect_);
		}

		return fig.scaled(aspect_.x);
	};

	virtual void draw()const override
	{
		if (not visible)return;

		//const ScopedViewport2D v{ viewport.asRect() };

		const Transformer2D t1(getTransformer());

		drawing.draw(color);
	};
};
//今のところフレームを描く処理はしてない
template<>
class Draw2D<Font> :public IDraw2D {
public:
	Font drawing;
	String text = U"";

	double getEndX()const
	{
		return getDrawPos().x + drawing(text).getXAdvances().sumF();
	}

	template<class... Args>
	Draw2D<Font>(DrawManager* manager, Args&&... args)
		:IDraw2D(manager)
		, drawing(args...)
	{}

	virtual void draw()const override
	{
		if (not (visible and text))return;
		
		//const ScopedViewport2D v{ viewport.asRect() };

		const Transformer2D t1(getTransformer());

		drawing(text).draw(0, 0, color);
	};
};

template<>
class Draw2D<DrawManager> :public IDraw2D
{
public:
	DrawManager drawing;

	template<class... Args>
	Draw2D<DrawManager>(DrawManager* manager, Args&&... args)
		:IDraw2D(manager)
		, drawing(args...)
	{}

	virtual void draw()const override
	{
		if (not visible)return;

		drawing.draw(false);
	};
};

template<>
class Draw2D<std::function<void()>> :public IDraw2D
{
public:
	std::function<void()> drawing;

	template<class... Args>
	Draw2D<std::function<void()>>(DrawManager* manager, Args&&... args)
		: IDraw2D(manager), drawing(args...)
	{}

	virtual void draw()const override
	{
		if(visible)drawing();
	}
};

using DrawRect = Draw2D<Rect>;
using DrawRectF = Draw2D<RectF>;
using DrawCircle = Draw2D<Circle>;
using DrawTexture = Draw2D<Texture>;

using DrawFont = Draw2D<Font>;
