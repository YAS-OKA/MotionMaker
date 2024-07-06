#pragma once
#include"../EC.hpp"
#include"../Util/PCTree.hpp"

class Transform :public Component
{
public:
	struct Vector
	{
		Vec3 vec;
		Vec3 pre;
		Vec3 delta;

		Vector();

		void resetVector(const Vec3& v);

		void calculate();

		Vec3 operator =(const Vec3& v);

		Vec3 operator +(const Vec3& v);

		Vec3 operator*(const Vec3& v);

		Vec3 operator *=(const Vec3& v);
	};
	
	struct Direction
	{
		Direction();
		Vec3 vector;
		Vec3 vertical;
		Quaternion q;//回転のための
		Quaternion accum;

		std::pair<Vec3, Vec3> asPair()const;

		//dir方向を向かせる、verRadはdirを軸に時計回りに回転させる
		void setDirection(const Vec3& dir, double verRad=0);
		void rotate(Vec3 axis, double rad);
		void rotate(const Quaternion& qua);
		void rotateXY(double rad);
	};

	struct Scale
	{
		Vec3 verticalDir;
	public:
		Vector aspect;
		Vec3 dir;//スケールxの方向

		Scale();

		//以下の関数はすべてsetAspectを呼び出す

		std::pair<Vec3, Vec3> getDir()const;
		//代入
		void setScale(double scale, double xRotate=0, double yRotate = 0, double zRotate = 0);
		void setAspect(const Vec3& aspect, double xRotate=0, double yRotate = 0, double zRotate = 0);
		//加算
		void addScale(double scale);
		void addAspect(const Vec3& aspect);
		//乗算
		void giveScale(double scale);
		void giveAspect(const Vec3& aspect);

		void operator = (double scale);
		void operator = (const Vec3& aspect);
		void operator +=(double scale);
		void operator +=(const Vec3& aspect);
		void operator *= (double scale);
		void operator *= (const Vec3& aspect);
	};
private:
	//速度測定用 フレーム毎に更新
	Vec3 measureVel;
	Vector framePos;
	Vec3 measureDirVel;
	Vector frameDir;

	util::PCRelationship<Transform> relation;

	Transform* m_parent;

	Vec3 correctPos;
public:
	//位置
	Vector pos;
	//方向
	Direction direction;

	Scale scale;

	void calculate();

	void affectChildren();

	//親の影響に関する設定
	bool followParent = true;//親の移動に追従
	bool parentRotationAffectable = true;//親の回転に追従
	bool parentScalingAffectable = true;//親のスケールに追従
	//子に影響するか
	bool affectToChildren = true;

	//aspectを回転させるか
	bool rotatableAspect = true;

	Transform();

	void setParent(Transform* parent,bool maintainAbsPosition=true);

	Transform* getParent()const;

	Array<Transform*> getChildren()const;

	void relaseParent(Transform* parent);

	void relaseChild(Transform* child);

	Transform* getParent();

	Vec3 getAspect()const;

	std::pair<Vec3,Vec3> getScaleDir()const;
	//絶対座標を返す
	Vec3 getPos()const;
	
	Vec2 getXY()const;
	//親からの相対座標　親なしなら絶対座標 unityと逆になってしまった...
	Vec3 getLocalPos()const;

	Vec3 getVel()const;
	//方向の1軸をゲット
	Vec3 getDirection()const;
	//方向の2軸をゲット
	Direction get2Direction()const;

	Vec3 getLocalDirection()const;

	Vec3 getAngulerVel()const;

	void setDirection(const Vec3& dir, double verRad = 0);

	void setDirAndPreDir(const Vec3& dir, double verRad = 0);

	void setLocalDirection(const Vec3& dir, double verRad = 0);

	void setLocalDirectionByAxis(const Vec3& axis, double rad = 0);

	void setLocalDirAndPreDir(const Vec3& dir, double verRad = 0);

	void setPos(const Vec3& pos);

	void addPos(const Vec3& pos);

	//void setAbsPos(const Vec3& pos);

	void setXY(const Vec2& pos);

	void setLocalXY(const Vec2& pos,bool gridScaling=false);

	void setPosAndPrePos(const Vec3& pos);
	//第2引数がtrueならスケーリングされた座標系で座標をセットする
	void setLocalPos(const Vec3& pos, bool gridScaling = false);

	void setLocalPosAndPrePos(const Vec3& pos);

	void setX(double x);
	void setY(double y);
	void setZ(double z);

	void addX(double x);
	void addY(double y);
	void addZ(double z);

	void setLocalX(double x, bool gridScaling = false);
	void setLocalY(double y, bool gridScaling = false);
	void setLocalZ(double z, bool gridScaling = false);

	void moveBy(const Vec3& delta);

	void rotateAt(const Vec3& center,Vec3 axis, double rad);

	void rotateAt(const Vec3& center,const Quaternion& q);

	void rotate(Vec3 axis, double rad);

	void rotate(const std::pair<Vec3, Vec3>& from, const std::pair<Vec3, Vec3>& to);

	void rotate(const Quaternion& q);

	void rotateXY(double rad);

	void scaleAt(const Vec3& pos, const Vec3& asp);

	void scaleAt(const Vec3& pos, double scale);

	void scaleXYAt(const Vec2& pos, double scale, double rad = 0);

	void scaleXYAt(const Vec2& pos, const Vec2& scale, double rad = 0);

	void calUpdate(double dt);

	Vec3 operator+(const Vec3& vec);

	Vec3 operator+=(const Vec3& vec);
private:
	void _setLocalX(double x, Optional<double> asp = none);
	void _setLocalY(double y, Optional<double> asp = none);
	void _setLocalZ(double z, Optional<double> asp = none);
};
