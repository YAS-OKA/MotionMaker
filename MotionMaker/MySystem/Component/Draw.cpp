#include "../stdafx.h"
#include "Draw.h"
#include"Transform.h"
#include"../DrawManager.h"
#include"../Util/Util.h"

IDrawing::IDrawing(DrawManager* manager)
	:manager(manager)
{
}

void IDrawing::start()
{
	//cameraInfluence = owner->addComponentNamed<Field<Influence>>(U"cameraInfluence", Influence());
	transform=owner->getComponent<Transform>();
}

void IDrawing::cal_drawPos()
{
	Vec3 localPos = transformScaleAffectable ? relative * transform->getAspect() : relative;
	//回転
	auto q = direction.accum;

	auto rq = q.inverse();

	if (transformDirectionAffectable) {
		q = q * transform->get2Direction().accum;
	}
	localPos = rq * localPos;
	localPos = q * localPos;
	//基準に戻し、目的の方向を向かせたものをgetPosに足す
	m_drawPos = transform->getPos() + localPos;
}

Vec3 IDrawing::getDrawPos() const
{
	//Vec3 localPos = transformScaleAffectable ? relative*transform->getAspect():relative;
	////回転
	//auto q = direction.accum;

	//auto rq = q.inverse();

	//if (transformDirectionAffectable) {
	//	q = q * transform->get2Direction().accum;
	//}
	//localPos = rq * localPos;
	//localPos = q * localPos;
	////基準に戻し、目的の方向を向かせたものをgetPosに足す
	//return transform->getPos() + localPos;
	return m_drawPos;
}

void IDrawing::cal_distanceFromCamera()
{
	m_distanceFromCamera = (getDrawPos() - manager->getCamera()->transform->getPos()).length();
}

double IDrawing::distanceFromCamera()const
{
	return m_distanceFromCamera;
}

Draw3D::Draw3D(DrawManager* manager, const MeshData& data)
	:IDraw3D(manager)
	,mesh(data)
{
}

void Draw3D::setAssetName(const String& name)
{
	tex = TextureAsset(name);
}

void Draw3D::draw()const
{
	mesh.draw(
		getDrawPos()
		, transform->get2Direction().accum
		, tex
		, color.removeSRGBCurve()
		);
}

IDraw2D::IDraw2D(DrawManager* manager)
	:IDrawing(manager)
{
}

IDraw2D::~IDraw2D()
{
	delete _scale_calculation;
	delete shallow;
	if (manager)manager->remove2D(*this);
}

Vec2 IDraw2D::getScale() const
{
	Vec2 aspect_ = (transformScaleAffectable ? transform->getAspect() * aspect.aspect.vec : aspect.aspect.vec).xy();

	return aspect_ * (*_scale_calculation)();
}

Vec2 IDraw2D::getScalePos() const
{
	return _scale_calculation->getScalePos();
}

DrawShallow* IDraw2D::getShallow()const
{
	return shallow;
}

Transformer2D IDraw2D::getTransformer() const
{
	Vec2 aspect_ = getScale();//(transformScaleAffectable ? transform->getAspect() * aspect.aspect.vec : aspect.aspect.vec).xy();
	Vec2 aspDir = (transformScaleAffectable ? Vec3(Quaternion::FromUnitVectorPairs({ {1,0,0,},{0,1,0} }, transform->getScaleDir()) * aspect.dir) : aspect.dir).xy();
	double rotation = transformDirectionAffectable ? transform->getDirection().xy().getAngle() - Vec2{ 1,0 }.getAngle() + direction.vector.xy().getAngle() : direction.vector.xy().getAngle();
	rotation -= Vec2{ 1,0 }.getAngle();
	//motioneditorでxscaleを0にするとバグるので対策
	if (aspect_.x == 0)aspect_.x = zero;
	if (aspect_.y == 0)aspect_.y = zero;

	auto drawPos = getDrawPos();

	double scaleRad = util::getRad(aspDir);

	const auto& scaleMat = Mat3x2::Rotate(-scaleRad+rotation) * Mat3x2::Scale(aspect_, getScalePos()) * Mat3x2::Rotate(-rotation+scaleRad);

	return Transformer2D{
		scaleMat
		* Mat3x2::Rotate(rotation,rotateCenter.xy())
		* Mat3x2::Translate(drawPos.xy() - viewport.tl())
		,TransformCursor::Yes
	};
}

draw_helper::ScaleCalculation2D::ScaleCalculation2D(const Borrow<Transform>& transform, DrawManager* m, double baseLength, const Camera::DistanceType& type)
	:converter(baseLength, m, type),transform(transform)
{
}

double draw_helper::ScaleCalculation2D::operator() ()const
{
	//カメラとの距離が0だったら0
	return converter.distanceRate(transform->getPos());
}

draw_helper::DrawShallow::DrawShallow(const Borrow<IDraw2D>& owner)
	:owner(owner)
{
}

void draw_helper::DrawShallow::cal_depth()
{
	const auto & camera = owner->manager->getCamera();
	m_depth = camera->distance(owner->getDrawPos());
}

double draw_helper::DrawShallow::getDepth() const
{
	return m_depth;
}

bool draw_helper::DrawShallow::shouldReplace(DrawShallow* other) const
{
	//相手の描画が優先される(あとから描写される)場合trueを返すというようにすればいい
	if (layer == other->layer)
	{
		return getDepth() > other->getDepth();
	}
	else
	{
		return layer < other->layer;
	}
}

IDraw3D::~IDraw3D()
{
	if (manager)manager->remove3D(*this);
}
