#include "../stdafx.h"
#include "Camera.h"
#include"../Component/Transform.h"
#include"Utilities.h"
#include"../Util/Util.h"

Camera::Camera(const BasicCamera3D& camera)
	:camera(camera),followTarget(nullptr)
{
	name = U"Camera";

	screenDistance = camera.getSceneSize().y / (2 * Tan(camera.getVerticalFOV() / 2));

	baseScreenSize = camera.getSceneSize();
}

void Camera::start()
{
	Object::start();

	transform->setPos(getEyePos());

	transform->direction.vector = { 0,0,1 };
	transform->direction.vertical = { 0,1,0 };

}

void Camera::update(double dt)
{
	Object::update(dt);
//#if _DEBUG
//	{
//		Vec3 delta{0,0,0};
//		if (KeyZ.pressed())
//		{
//			delta += Vec3{ 0,-100 * dt,0 };
//		}
//		if (KeyX.pressed())
//		{
//			delta += Vec3{ 0,100 * dt,0 };
//		}
//		if (KeyRight.pressed())
//		{
//			delta += Vec3{ 100 * dt,0,0 };
//		}
//		if (KeyLeft.pressed())
//		{
//			delta += Vec3{ -100 * dt,0,0 };
//		}
//		if (KeyUp.pressed())
//		{
//			delta += Vec3{ 0,0,100 * dt };
//		}
//		if (KeyDown.pressed())
//		{
//			delta += Vec3{ 0,0,-100 * dt };
//		}
//		transform->moveBy(delta);
//	}
//#endif
		
	auto targetPos = not(followTarget) ? Vec3{0,0,0} : followTarget->transform->getPos();

	//if (KeyQ.pressed()) {
	//	theta += 5 * dt;
	//	transform->direction.vector = Quaternion::FromUnitVectorPairs({ {0,0,1},{0,1,0} }, { {Sin(theta),0,Cos(theta)} ,{0,1,0} }) * Vec3 { 0, 0,1};
	//	transform->direction.vertical = { 0,1,0 };// Quaternion::FromUnitVectorPairs({ {0,0,1},{0,1,0} }, { {0,Sin(theta),Cos(theta)} ,{0,0,1} })* Vec3 { 0, 0, 1 }
	//}

	//if (KeyE.pressed())
	//{
	//	theta += 5 * dt;
	//	transform->direction.vertical = Quaternion::FromUnitVectorPairs({ {0,0,1},{0,1,0} }, { {0,0,1} ,{Sin(theta),Cos(theta),0} }) * Vec3 { 0, 1, 0 };
	//	transform->direction.vector = { 0,0,1 };// Quaternion::FromUnitVectorPairs({ {0,0,1},{0,1,0} }, { {0,Sin(theta),Cos(theta)} ,{0,0,1} })* Vec3 { 0, 0, 1 }
	//}

	camera.setView(transform->getPos(), targetPos);
	//カメラを回す
	camera.setUpDirection(
		camera.screenToWorldPoint(Vec2{ 0,-util::sh() }.rotated(screenAngle * 1_deg), 1)
		- camera.screenToWorldPoint({ 0,0 }, 1));
}

double Camera::distance(const Vec3& pos,const DistanceType& type)const
{
	const auto& p = transform->getPos();
	if (type == XYZ) return (p - pos).length();
	else if (type == X)return Abs(p.x - pos.x);
	else if (type == Y)return Abs(p.y - pos.y);
	else if (type == Z)return Abs(p.z - pos.z);
	else if (type == XY)return Math::Sqrt(std::pow(p.x - pos.x, 2) + std::pow(p.y - pos.y, 2));
	else if (type == XZ)return Math::Sqrt(std::pow(p.x - pos.x, 2) + std::pow(p.z - pos.z, 2));
	else if (type == YZ)return Math::Sqrt(std::pow(p.z - pos.z, 2) + std::pow(p.y - pos.y, 2));
	else if (type == Screen){
		const auto& dotProduct = (pos - getEyePos()) * ((getForcusPos() - transform->getPos()).normalize());
		return dotProduct.x + dotProduct.y + dotProduct.z;
	}

	throw Error{ U"未定義の距離\t{} \nCamera.cpp camera.distance"_fmt(int32(type))};
	return 0.0;
}

double Camera::distance(const Vec3& pos)const
{
	return distance(pos, type);
}

BasicCamera3D Camera::getCamera()const
{
	return camera;
}

BasicCamera3D Camera::getCameraLatest() const
{
	BasicCamera3D c = camera;
	const auto& p = transform->getPos();
	c.setView(p, p+getForcusDir());
	return c;
}

Vec3 Camera::getForcusDir() const
{
	return (camera.getFocusPosition()- camera.getEyePosition()).normalize();
}

void Camera::setFollowTarget(const Borrow<Object>& target)
{
	followTarget = target;
	transform->setParent(target->transform);
	transform->parentRotationAffectable = false;
}

Point Camera::getSceneSize() const
{
	return camera.getSceneSize();
}

void Camera::setSceneSize(const Point& size)
{
	camera.setSceneSize(size);
}

void Camera::setIFOV(const double ifov)
{
	camera.setProjection(getSceneSize(), ifov);

	screenDistance = camera.getSceneSize().y / (2 * Tan(ifov / 2));
}

double Camera::getIFOV() const
{
	return camera.getVerticalFOV();
}

Vec3 Camera::getEyePos() const
{
	return camera.getEyePosition();
}

Vec3 Camera::getForcusPos() const
{
	return camera.getFocusPosition();
}

Vec2 Camera::getScreenPos(const Vec3& pos) const
{
	//3d->2d
	const SIMD_Float4 worldPos = DirectX::XMVector3TransformCoord(SIMD_Float4{ pos, 0.0f }, camera.getViewProj());

	auto v = worldPos.xy();

	v.y *= -1.0f;
	v.y += 1.0f;
	v.x += 1.0f;
	v.y *= (camera.getSceneSize().y * 0.5f);
	v.x *= (camera.getSceneSize().x * 0.5f);
	return v;
}

double Camera::getScreenDistance() const
{
	return screenDistance;
}
