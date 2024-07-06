#pragma once
#include"../Component/Transform.h"
#include"../Game/Object.h"
#include"../Component/Draw.h"
#include"../Game/Scenes.h"
#include"../Figure.h"
//#include"../Game/Utilities.h"

class Camera;

namespace draw_helper {
	class CameraScaleOfParts;
	class ZShallow;
}

namespace mot
{
	static String localPath{ U"asset/motion/" };

	struct PartsParams
	{
		String name;
		Optional<String> path;
		std::variant<Texture, Figure> drawing;
		String parent;
		Vec2 pos;
		Vec2 scale;
		double angle;
		double z;
		Vec2 rotatePos;
		ColorF color;
		//パーツのパラメータ一覧
		static Array<String> Names;

		PartsParams() {};

		PartsParams(const PartsParams& other)
		{
			build(other.name, other.parent, other.pos, other.scale, other.angle, other.z, other.rotatePos, other.color, other.path, other.drawing);
		}

		PartsParams(
			const String& _name,
			const Optional<String>& _path = none,
			const std::variant<Texture, Figure>& _drawing = RectF{},
			const String& _parent = U"",
			const Vec2& _pos = { 0,0 },
			const Vec2& _scale = { 1,1 },
			double _angle = 0,
			double _z = 100,
			const Vec2& _rotatePos = { 0,0 },
			const ColorF& _color = Palette::White)
		{
			build(_name,
				_parent,
				_pos,
				_scale,
				_angle,
				_z,
				_rotatePos,
				_color,
				_path,
				_drawing);
		}

		void build(
			const String& _name,
			const String& _parent,
			const Vec2& _pos,
			const Vec2& _scale,
			double _angle,
			double _z,
			const Vec2& _rotatePos,
			const ColorF& _color,
			const Optional<String> _path,
			const std::variant<Texture, Figure>& _drawing)
		{
			name = _name;
			parent = _parent;
			pos = _pos;
			scale = _scale;
			angle = _angle;
			z = _z;
			rotatePos = _rotatePos;
			color = _color;
			path = _path;
			drawing = _drawing;
		};
	};

	String GetParamStr(const PartsParams& p, StringView name);

	String GetParamDefault(StringView name);

	class PartsManager;

	using PartsDrawing = std::variant<Texture, Figure>;

	class Parts :public Object
	{
		Vec3 m_localPos;
		double m_z;
	public:
		PartsParams base;
		
		PartsParams params;

		Borrow<IDraw2D> tex;

		Borrow<Collider> collider;

		Borrow<PartsManager> parts_manager;

		Borrow<Parts> parent;

		util::PCRelationship<Parts> partsRelation;

		Parts(const Borrow<PartsManager>& manager)
			:parts_manager(manager),partsRelation(this) {};

		Parts() :partsRelation(this) {};

		void onTrashing()override;

		Borrow<Collider> createHitbox(const Vec2& pos, const MultiPolygon& fig);

		void setParent(const String& name, bool setLocalPos = false);

		void setZ(double z)
		{
			//この値は子に影響しないようにする
			//transform->affectToChildren = false;
			params.z = z;
			transform->pos.vec.z = z;
			transform->calculate();
			//transform->affectToChildren = true;
		}

		void setPos(const Vec2& pos);

		void setAbsPos(const Vec2& pos)
		{
			transform->setXY(pos);
		}

		Borrow<Parts> clone();
		//移籍する
		void transfer(const Borrow<PartsManager>& to);

	private:
		void rotateCollider(double ang)
		{
			if (collider)std::get<2>(collider->hitbox.shape).rotateAt({ 0,0 }, ang * 1_deg);
			for (auto& c : partsRelation.getChildren())
			{
				c->rotateCollider(ang);
			}
		}

	public:
		void setAngle(double angle)
		{
			double ang = Math::Fmod(angle - getAngle(), 360);

			auto rp = transform->pos.vec.xy() + getRotatePos().rotated(getAbsAngle() * 1_deg);

			transform->rotateAt({ rp, 0}, {0,0,1}, ang * 1_deg);

			rotateCollider(ang);

			params.angle = angle;

			setScale(transform->scale.aspect.vec.xy());
		}
		//ただその場で回る　ローテートポスを無視　recursive=trueなら子のpureRotateを呼ぶ
		void pureRotate(double angle, bool recursive = true)
		{
			double ang = Math::Fmod(angle, 360);
			auto tmp = transform->affectToChildren;
			transform->affectToChildren = false;
			transform->rotate({ 0,0,1 }, ang * 1_deg);
			transform->affectToChildren = tmp;
			if (collider != nullptr)std::get<2>(collider->hitbox.shape).rotateAt({ 0,0 }, ang * 1_deg);
			setScale(transform->scale.aspect.vec.xy());
			//子どもも回す
			if (recursive)for (auto& c : partsRelation.getChildren())dynamic_cast<Parts*>(c)->pureRotate(ang, true);
		}

		void setScale(const Vec2& s)
		{
			if (params.scale == s)return;

			transform->scale.setAspect({ s,1 }, 0, 0, getAngle() * 1_deg);

			if (params.scale.x != 0 and params.scale.y != 0)
			{
				setRotatePos(params.rotatePos * s / params.scale);
			}

			params.scale = s;
		}

		void setColor(const ColorF& color)
		{
			tex->color = color;
			params.color = color;
		}

		void setTexture(const PartsDrawing& drawing);

		void setRotatePos(const Vec2& pos)
		{
			params.rotatePos = pos;
		}

		void setName(const String& name)
		{
			this->name = name;
			params.name = name;
		}

		void build(const PartsParams& params);

		String getParent()const {
			auto parent = transform->getParent();
			return parent ? parent->owner->name : U"None Parent";
		}

		Vec2 getPos()const { return transform->getLocalPos().xy(); }
		//親の回転を消した相対座標
		Vec2 getRelativePos()const {
			return parent ? getPos().rotate(-parent->getAbsAngle() * 1_deg) : getPos();
		}

		String getName()const { return params.name; }

		double getZ()const { return transform->getPos().z; }

		double getAngle()const {
			//return params.angle;
			return (transform->getLocalDirection().xy().getAngle() - Vec2{ 1,0 }.getAngle())/1_deg;
		};

		double getAbsAngle()const {
			if (parent)return getAngle() + parent->getAbsAngle();
			else return getAngle();
		}

		Vec2 getRotatePos()const {
			return params.rotatePos;
		}

		ColorF getColor()const { return tex->color; }

		Vec2 getScale()const { return params.scale; }
		
		void update(double dt)override;
	};

	class PartsManager:public Object
	{
	public:
		//名前被りの解決 name+(n)
		String _resolveNameDuplication(const String& name);
		//マスターパーツ
		Borrow<Parts> master;
		Array<Borrow<Parts>> partsArray;
		Borrow<Draw2D<DrawManager>> dm;

		Camera::DistanceType distanceTypeUsedInScaleHelper=Camera::Screen;
		double scaleHelperBaseLength=100;

		//EntityManager ema;

		void scaleHelperParamsSetting(double baseLength, const Camera::DistanceType& distanceType = Camera::Screen);

		Borrow<Parts> birthParts();

		Borrow<Parts> addParts(const PartsParams& params, bool resolveNameDuplication = true);

		Borrow<Parts> addParts(const Borrow<Parts>& parts, bool resolveNameDuplication = true);

		Borrow<Parts> setMaster(const Borrow<Parts>& masterParts);

		Borrow<Parts> createMaster();

		Borrow<Parts> find(const String& name);

		void killParts(const Borrow<Parts>& parts);

		void killParts(const String& name);

		void setPartsVisibility(bool visible=false);

		void start()override;

		void update(double dt)override;
	};
}

namespace mot
{
	//path->localPath+relativePath
	void loadPartsTexture(const Borrow<my::Scene>& scene, const String& relativePath);

	bool savePartsJson(const Borrow<PartsManager>& pm, const String& path);

	class LoadParts
	{
	private:
		Borrow<my::Scene> m_scene;
	public:
		LoadParts(const Borrow<my::Scene>& scene) :m_scene(scene) {};

		void setScene(const Borrow<my::Scene>& scene) { m_scene = scene; };

		Borrow<PartsManager> create(const String& jsonPath, bool createCollider);

		Borrow<PartsManager> create(const String& jsonPath, const Borrow<PartsManager>& pmanager,bool createCollider);
	};
	Borrow<Parts> CreateParts(const PartsParams& params, const Borrow<PartsManager>& pmanager, bool createCollider);

	Borrow<Parts> CreateParts(const PartsParams& params, const Borrow<PartsManager>& pmanager, const Figure& collider);

	Borrow<Parts> CreateParts(const PartsParams& params, const Borrow<PartsManager>& pmanager, const String& path);
}

namespace draw_helper
{
	//Masterパーツの情報を参照してスケール。
	class CameraScaleOfParts :public ScaleHelper2D
	{
	private:
		ScaleHelper2D* helper = nullptr;
		Borrow<mot::Parts> parts;
	public:
		CameraScaleOfParts(const Borrow<mot::Parts>& parts)
			: parts(parts)
		{
		}

		~CameraScaleOfParts() { delete helper; }

		template <class ScaleHelper, class... Args>
		ScaleHelper* setHelper(Args&&... args)
		{
			if (helper != nullptr)delete helper;

			auto ret = new ScaleHelper(args...);

			helper = ret;
			
			return ret;
		}

		virtual Vec2 getScalePos()const override;

		virtual double operator () () const override;
	};

	class PartsShallow :public DrawShallow
	{
	public:
		Borrow<mot::PartsManager> pmanager;

		PartsShallow(const Borrow<mot::PartsManager>& p, const Borrow<IDraw2D>& d);

		double getDepth()const override;
	};
}
