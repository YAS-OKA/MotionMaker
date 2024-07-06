#include "../stdafx.h"
#include"../Game/Utilities.h"
#include "Transform.h"

Transform::Vector::Vector() {
	resetVector({ 0,0,0 });
}

void Transform::Vector::calculate()
{
	delta = vec - pre;
	pre = vec;
}

void Transform::Vector::resetVector(const Vec3& v)
{
	vec = pre = v;
	delta = { 0,0,0 };
}

Vec3 Transform::Vector::operator=(const Vec3& v)
{
	vec = v;
	return vec;
}

Vec3 Transform::Vector::operator+(const Vec3& v)
{
	return vec + v;
}

Vec3 Transform::Vector::operator*(const Vec3& v)
{
	return vec* v;
}

Vec3 Transform::Vector::operator*=(const Vec3& v)
{
	vec = vec * v;
	return vec;
}

Transform::Scale::Scale()
{
	aspect = { 1,1,1 };
	dir = { 1,0,0 };
	verticalDir = { 0,1,0 };
}

void Transform::Scale::setScale(double scale, double xRotate, double yRotate, double zRotate)
{
	setAspect({ scale,scale,scale }, xRotate,yRotate,zRotate);
}

void Transform::Scale::setAspect(const Vec3& aspect, double xRotate, double yRotate, double zRotate)
{
	this->aspect = aspect;
	auto q = Quaternion::RotateX(xRotate) * Quaternion::RotateY(yRotate) * Quaternion::RotateZ(zRotate);
	this->dir = q * Vec3 { 1, 0, 0 };
	this->verticalDir = q * Vec3{ 0,1,0 };
}

std::pair<Vec3, Vec3> Transform::Scale::getDir() const
{
	return std::pair<Vec3, Vec3>(dir,verticalDir);
}

void Transform::Scale::addScale(double scale)
{
	addAspect({ scale,scale,scale });
}

void Transform::Scale::addAspect(const Vec3& aspect)
{
	setAspect(Scale::aspect + aspect);
}

void Transform::Scale::giveScale(double scale)
{
	giveAspect({ scale,scale,scale });
}

void Transform::Scale::giveAspect(const Vec3& aspect)
{
	setAspect(Scale::aspect * aspect);
}

void Transform::Scale::operator=(double scale)
{
	setScale(scale);
}

void Transform::Scale::operator=(const Vec3& aspect)
{
	setAspect(aspect);
}

void Transform::Scale::operator+=(double scale)
{
	addScale(scale);
}

void Transform::Scale::operator+=(const Vec3& aspect)
{
	addAspect(aspect);
}

void Transform::Scale::operator*=(double scale)
{
	giveScale(scale);
}

void Transform::Scale::operator*=(const Vec3& aspect)
{
	giveAspect(aspect);
}

Transform::Direction::Direction()
	:vector({ 1,0,0 })
	, vertical({ 0,1,0 })
	,accum(Quaternion(0,0,0,1))
{
}

std::pair<Vec3, Vec3> Transform::Direction::asPair()const
{
	return std::make_pair(vector, vertical);
}

void Transform::Direction::setDirection(const Vec3& dir, double rad)
{
	rotate(Quaternion::FromUnitVectors(vector, dir));
	if (rad != 0)rotate(vector, rad);
}

void Transform::Direction::rotate(Vec3 axis, double rad)
{
	//rotate(Quaternion{ axis.x * sin(rad / 2), axis.y * sin(rad / 2), axis.z * sin(rad / 2), cos(rad / 2) });
	rotate(Quaternion::RotationAxis(axis, rad));
}

void Transform::Direction::rotate(const Quaternion& qua)
{
	q = qua;
	accum *= qua;
	vector = q * vector;
	vertical = q * vertical;
	vector.normalize();
	vertical.normalize();
}
////あえて3次元のrotateは使わない
//void Transform::Direction::rotateXY(double rad)
//{
//	q = Quaternion{ 0 * sin(rad / 2), 0 * sin(rad / 2), 1 * sin(rad / 2), cos(rad / 2) };
//	accum *= qua;
//}

void Transform::setParent(Transform* parent, bool maintainAbsPosition)
{
	relation.clearParents();

	relation.setParent(parent);

	m_parent = parent;

	if (not maintainAbsPosition) {
		//今までの絶対座標を相対座標にする
		setLocalPos(getPos());
		setLocalDirection(getDirection());
		setLocalPos(m_parent->direction.accum * getLocalPos());
	}
}

Transform* Transform::getParent()const
{
	return relation.getParent();
}

Array<Transform*> Transform::getChildren()const
{
	return relation.getChildren();
}

void Transform::relaseParent(Transform* parent)
{
	parent->relaseChild(this);
	//if (m_parent == parent)m_parent = nullptr;
}

void Transform::relaseChild(Transform* child)
{
	relation.removeChild(child);
}

Transform* Transform::getParent() {
	return m_parent;
}

Transform::Transform()
	:relation(this)
{
	measureVel = measureDirVel = { 0,0,0 };
	correctPos = { 0,0,0 };
	m_parent = nullptr;
}

void Transform::setDirection(const Vec3& dir, double verRad)
{
	direction.setDirection(dir, verRad);
	affectChildren();
}

void Transform::setDirAndPreDir(const Vec3& dir, double verRad)
{
	direction.setDirection(dir, verRad);
	frameDir.vec = frameDir.pre = dir;
}

void Transform::setLocalDirection(const Vec3& dir, double verRad)
{
	auto parent = getParent();
	if (parent == nullptr) {
		setDirection(dir,verRad);
		return;
	}

	auto q = Quaternion::FromUnitVectors({ 1,0,0 }, dir);
	direction.setDirection(q * parent->getDirection(),verRad);
	affectChildren();
}

void Transform::setLocalDirAndPreDir(const Vec3& dir, double verRad)
{
	auto parent = getParent();
	if (parent == nullptr) {
		setDirAndPreDir(dir);
		return;
	}

	auto q = Quaternion::FromUnitVectors({ 1,0,0 }, dir);
	direction.setDirection(q * parent->getDirection(), verRad);
	frameDir.vec = frameDir.pre = getDirection();
}

void Transform::setPosAndPrePos(const Vec3& p)
{
	pos.vec = pos.pre = p;
	framePos.vec = framePos.pre = p;
}

void Transform::setPos(const Vec3& p)
{
	pos.vec = p;
	
	affectChildren();
}

void Transform::setXY(const Vec2& p)
{
	pos.vec.x = p.x;
	pos.vec.y = p.y;
	affectChildren();
}

void Transform::setLocalXY(const Vec2& p,bool gridScaling)
{
	if (gridScaling)
	{
		auto asp = getAspect();
		_setLocalX(p.x, asp.x);
		_setLocalY(p.y, asp.y);
	}
	else {
		_setLocalX(p.x);
		_setLocalY(p.y);
	}
	affectChildren();
}

void Transform::setLocalPos(const Vec3& p, bool gridScaling)
{
	if (auto parent = getParent()) {
		if (gridScaling)
		{
			const auto& s1 = parent->getAspect();
			if (s1.x * s1.y * s1.z == 0)return;//0の成分あったら計算しない(本当は0の成分以外は計算すべき？)
			pos.vec = p / s1 + parent->pos.vec;
		}
		else {
			pos.vec = p + parent->pos.vec;
		}

		affectChildren();
	}
	else {
		setPos(p);
	}
}

void Transform::setLocalPosAndPrePos(const Vec3& p)
{
	if (auto parent = getParent())
	{
		pos.vec
			= pos.pre
			= framePos.vec
			= framePos.pre
			= p + parent->getPos();
	}
	else{
		setPosAndPrePos(p);	
	}
}

void Transform::setX(double x)
{
	pos.vec.x = x;
	affectChildren();
}

void Transform::setLocalX(double x,bool gridScaling)
{
	if (gridScaling)
	{
		auto asp = getAspect();
		_setLocalX(x, asp.x);
	}
	else {
		_setLocalX(x);
	}
	affectChildren();
}
void Transform::setLocalY(double y, bool gridScaling)
{
	if (gridScaling)
	{
		auto asp = getAspect();
		_setLocalY(y, asp.y);
	}
	else {
		_setLocalY(y);
	}
	affectChildren();
}

void Transform::setLocalZ(double z, bool gridScaling)
{
	if (gridScaling)
	{
		auto asp = getAspect();
		_setLocalZ(z, asp.z);
	}
	else {
		_setLocalZ(z);
	}
	affectChildren();
}

void Transform::setY(double y)
{
	const auto& p = getPos();
	setPos({ p.x,y,p.z });
}

void Transform::setZ(double z)
{
	setPos({ getPos().xy(),z });
}

void Transform::addX(double x)
{
	addPos(Vec3{ x,0,0 });
}

void Transform::addY(double y)
{
	addPos(Vec3{ 0,y,0 });
}

void Transform::addZ(double z)
{
	setPos(Vec3{ 0,0,z });
}

void Transform::addPos(const Vec3& pos)
{
	moveBy(pos);
}

void Transform::moveBy(const Vec3& delta)
{
	pos.vec += delta;
	affectChildren();
}

Vec3 Transform::getAspect()const
{
	auto parent = getParent();

	Vec3 res = scale.aspect.vec;
	if (parentScalingAffectable) {
		while (parent)
		{
			res *= parent->scale.aspect.vec;
			parent = parent->getParent();
		}
	}
	return res;
	//再帰だとなぜか遅い...
	//return parentScalingAffectable and parent ? parent->getAspect()*scale.aspect.vec : scale.aspect.vec;
}

std::pair<Vec3,Vec3> Transform::getScaleDir() const
{
	if (not rotatableAspect)return { {1,0,0},{0,1,0} };

	auto [dir, ver] = scale.getDir();
	auto parent = getParent();
	while (parent and parent->rotatableAspect)
	{
		const auto& q = Quaternion::FromUnitVectorPairs({ {1,0,0},{0,1,0} }, parent->scale.getDir());
		dir = q * dir;
		ver = q * ver;
		parent = parent->getParent();
	}

	return { dir,ver };

	//if (parentScalingAffectable) {
	//	if (auto parent = getParent())
	//	{
	//		const auto& q = Quaternion::FromUnitVectorPairs({ {1,0,0},{0,1,0} }, parent->getScaleDir());
	//		return { q * dir,q * ver };
	//	}
	//}
	//return { dir , ver };
}

Vec3 Transform::getPos()const
{
	return correctPos;
}

Vec2 Transform::getXY()const
{
	return getPos().xy();
}

Vec3 Transform::getLocalPos()const
{
	if (auto parent = getParent())
	{
		const auto& q = Quaternion::FromUnitVectorPairs({ {1,0,0},{0,1,0} }, parent->getScaleDir());
		return q * ((q.inverse() * (pos.vec - parent->pos.vec)) * parent->getAspect());
	}
	else
	{
		return pos.vec;
	}
}

Vec3 Transform::getVel()const
{
	return measureVel;
}

Vec3 Transform::getDirection()const
{
	return direction.vector;
}

Transform::Direction Transform::get2Direction()const
{
	return direction;
}

Vec3 Transform::getLocalDirection()const
{
	if (auto parent = getParent())
	{
		//親がいる場合
		auto q = Quaternion::FromUnitVectors(parent->getDirection(), getDirection());
		return q * Vec3{ 1,0,0 };
	}
	else
	{
		return getDirection();
	}
}

Vec3 Transform::getAngulerVel()const
{
	return measureDirVel;
}

void Transform::affectChildren()
{
	calculate();
	if (affectToChildren)
	{
		for (auto& child : getChildren())
		{
			if (child->followParent){
				child->pos.vec += pos.delta;//child->moveBy(pos.delta);
				//つじつま合わせ
				child->pos.delta = child->pos.vec - child->pos.pre;
				child->correctPos = getPos() + child->getLocalPos();
			}
			
			if (child->parentRotationAffectable)child->rotateAt(pos.vec, direction.q);
		}
	}
	direction.q.set(0, 0, 0, 1);
}

void Transform::calculate()
{
	pos.calculate();

	if (auto parent = getParent())
	{
		correctPos = parent->getPos() + getLocalPos();
	}
	else
	{
		correctPos = pos.vec;
	}
}

void Transform::rotateAt(const Vec3& center, Vec3 axis, double rad)
{
	axis = axis.withLength(1);
	rotateAt(center, Quaternion{ axis.x * sin(rad / 2), axis.y * sin(rad / 2), axis.z * sin(rad / 2), cos(rad / 2) });
}

void Transform::rotateAt(const Vec3& center,const Quaternion& qua)
{
	direction.rotate(qua);
	pos.vec = qua * (pos.vec - center) + center;
	affectChildren();
}

void Transform::rotate(Vec3 axis, double rad)
{
	axis = axis.withLength(1);
	rotateAt(pos.vec, Quaternion{ axis.x * sin(rad / 2), axis.y * sin(rad / 2), axis.z * sin(rad / 2), cos(rad / 2) });
}

void Transform::rotate(const Quaternion& q)
{
	rotateAt(pos.vec, q);
}

void Transform::rotate(const std::pair<Vec3, Vec3>& from, const std::pair<Vec3, Vec3>& to)
{
	rotate(Quaternion::FromUnitVectorPairs(from, to));
}

void Transform::scaleAt(const Vec3& pos, const Vec3& asp)
{
	Vec3 delta = getPos() - pos;
	scale.setAspect(asp);
	setPos(delta * asp + pos);
}

void Transform::scaleAt(const Vec3& pos, double s)
{
	scaleAt(pos, { s,s,s });
}

void Transform::calUpdate(double dt)
{
	if (dt == 0)return;
	const auto& nowPos = getPos();
	const auto& nowDir = getDirection();
	//速度を求める
	measureVel = (nowPos - framePos.vec) / dt;
	measureDirVel = (nowDir - frameDir.vec) / dt;
	//framePos,frameDirの更新
	framePos.vec = nowPos;
	frameDir.vec = nowDir;
}

Vec3 Transform::operator+(const Vec3& vec)
{
	return pos + vec;
}

Vec3 Transform::operator+=(const Vec3& vec)
{
	moveBy(vec);
	return pos.vec;
}

void Transform::_setLocalX(double x, Optional<double> asp)
{
	if (auto parent = getParent()) {
		if (asp)
		{
			if (*asp == 0)return;//0の成分あったら計算しない(本当は0の成分以外は計算すべき？)
			pos.vec.x = x / *asp + parent->pos.vec.x;
		}
		else {
			pos.vec.x = x + parent->pos.vec.x;
		}
	}
	else {
		setX(x);
	}
}

void Transform::_setLocalY(double y, Optional<double> asp)
{
	if (auto parent = getParent()) {
		if (asp)
		{
			if (*asp == 0)return;//0の成分あったら計算しない(本当は0の成分以外は計算すべき？)
			pos.vec.y = y / *asp + parent->pos.vec.y;
		}
		else {
			pos.vec.y = y + parent->pos.vec.y;
		}
	}
	else {
		setY(y);
	}
}

void Transform::_setLocalZ(double z, Optional<double> asp)
{
	if (auto parent = getParent()) {
		if (asp)
		{
			if (*asp == 0)return;//0の成分あったら計算しない(本当は0の成分以外は計算すべき？)
			pos.vec.z = z / *asp + parent->pos.vec.z;
		}
		else {
			pos.vec.z = z + parent->pos.vec.z;
		}
	}
	else {
		setZ(z);
	}
}

void Transform::scaleXYAt(const Vec2& _pos, double scale, double rad)
{
	scaleXYAt(_pos, scale, rad);
}

void Transform::scaleXYAt(const Vec2& _pos, const Vec2& scale, double rad)
{
	Vec2 relative = (getPos().xy() - _pos).rotate(rad);

	relative *= scale;

	setXY(relative.rotate(-rad) + _pos);

	this->scale.setAspect({ scale,this->scale.aspect.vec.z }, 0, 0, rad);
}
