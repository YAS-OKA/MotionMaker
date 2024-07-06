#pragma once
#include"Object.h"

class Camera :public Object
{
	BasicCamera3D camera;
	Borrow<Object> followTarget;
	Vec2 baseScreenSize;
	double screenDistance;
public:
	//x軸距離、y軸距離、z軸距離、xy平面距離,xz平面距離,yz平面距離,xyz空間距離、スクリーン距離
	enum DistanceType
	{
		X,
		Y,
		Z,
		XY,
		XZ,
		YZ,
		XYZ,
		Screen,
	}type;
	//画面のスケーリング
	Vec2 scale{ 1,1 };
	Vec2 scalePos{ 0,0 };
	//スクリーンの傾きが何度か
	double screenAngle = 0;
	//まだ作ってない
	/*void setScale(const Vec2& s, const Vec2& pos = { 0,0 });

	void setScale(double s, const Vec2& pos = { 0,0 });*/

	Camera(const BasicCamera3D& camera);

	virtual void start()override;

	virtual void update(double dt) override;
	//カメラとの距離を返す
	double distance(const Vec3& pos, const DistanceType& type)const;
	//カメラとの距離を返す　現在のカメラのtypeを適用する
	double distance(const Vec3& pos)const;

	BasicCamera3D getCamera()const;

	BasicCamera3D getCameraLatest()const;

	Vec3 getForcusDir()const;

	void setFollowTarget(const Borrow<Object>& obj);

	Point getSceneSize()const;

	void setSceneSize(const Point& size);

	void setIFOV(const double ifov);
	
	double getIFOV()const;

	Vec3 getEyePos()const;

	Vec3 getForcusPos()const;

	Vec2 getScreenPos(const Vec3& pos)const;

	double getScreenDistance()const;

	/*void addSceneSize*/
};
