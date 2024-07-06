#include "../stdafx.h"
#include "Parts.h"
#include"../Game/Camera.h"
#include"../Util/Cmd.hpp"
#include"../Util/CmdDecoder.h"
#include"../Util/Util.h"
#include"MotionCmd.h"

namespace
{
	constexpr double frameZ = 10000; 
}

namespace mot
{
	Array<String> PartsParams::Names = {
			U"name",
			U"path",
			U"drawing",
			U"parent",
			U"pos",
			U"scale",
			U"angle",
			U"z",
			U"rotatePos",
			U"color"
	};

	void Parts::onTrashing()
	{
		Object::onTrashing();
		auto& arr = parts_manager->partsArray;
		//partsArrayから削除
		for (auto it = arr.begin(); it != arr.end();)
		{
			if (*it == this)
			{
				it = arr.erase(it);
			}
			else
			{
				++it;
			}
		}
		if (parts_manager->master == this)parts_manager->master = nullptr;
	}

	void Parts::setParent(const String& name, bool setLocalPos)
	{
		auto nextParent = parts_manager->find(name);
		if (not nextParent)return;

		partsRelation.clearParents();

		partsRelation.setParent(nextParent);

		parent = nextParent;

		params.parent = parent->name;

		transform->setParent(parent->transform);

		if (setLocalPos) {
			//絶対を相対に！
			const auto& p = transform->getPos();
			transform->setLocalX(p.x);
			transform->setLocalY(p.y);
			transform->setLocalDirection(transform->getDirection());
		}
		else
		{
			setPos(transform->getLocalPos().xy());
		}

		params.angle = getAngle();
	}

	void Parts::setPos(const Vec2& pos)
	{
		transform->setLocalXY(pos, true);
	}

	Borrow<Parts> Parts::clone()
	{
		return parts_manager->addParts(params);
	}

	void Parts::transfer(const Borrow<PartsManager>& to)
	{
		if (parts_manager)
		{
			if (parts_manager == to)return;//移籍する必要がない

			parts_manager->partsArray.remove(*this);
		}
		parts_manager = to;
	}

	void Parts::setTexture(const PartsDrawing& drawing)
	{
		if (tex)remove(*tex);
		switch (drawing.index())
		{
		case 0:
			tex = addComponent<DrawTexture>(&parts_manager->dm->drawing, std::get<0>(drawing));
			tex->relative = { -std::get<0>(drawing).size() / 2, 0 };
			break;
		case 1:
			tex = addComponent<Draw2D<Figure>>(&parts_manager->dm->drawing, std::get<1>(drawing));
			break;
		}
		params.drawing = drawing;
	}

	void Parts::build(const PartsParams& params)
	{
		if (transform == nullptr)return;
		//名前
		setName(params.name);
		//親
		if (name != U"master")setParent(params.parent, true);
		//path
		if (params.path)this->params.path = params.path;
		//座標
		setPos(params.pos);
		//アングル
		setAngle(params.angle);
		//スケール
		setScale(params.scale);
		//z
		setZ(params.z);
		//パーツのテクスチャ
		setTexture(params.drawing);
		//回転中心
		setRotatePos(params.rotatePos);
		//カラー
		tex->color = params.color;
	}

	void Parts::update(double dt)
	{
		//これいらないかも
		//params.pos = transform->getXY();

		Object::update(dt);
	}

	//MultiPolygonの中から最大の面積のものをを当たり判定にしてる
	Borrow<Collider> Parts::createHitbox(const Vec2& pos, const MultiPolygon& fig)
	{
		auto maxAreaFig = util::GetMax(fig.asArray(), [](Polygon p) {return p.area(); });

		if (collider)remove(*collider);

		collider = addComponent<Collider>(CollideBox::CollideFigure(maxAreaFig));

		collider->hitbox.relative = { maxAreaFig.centroid() - pos,0 };

		std::get<2>(collider->hitbox.shape).rotateAt({ 0,0 }, getAngle() * 1_deg);

		return collider;
	}

	String PartsManager::_resolveNameDuplication(const String& name)
	{
		HashSet<int32> dNums;
		const auto& len = name.length();
		//かぶってる名前があったらdNumsに0を追加
		//「かぶってる名前＋(n)」ですでに使われてるnをdNumsに追加。
		for (const auto& p : partsArray)
		{
			const auto& str = p->getName();
			const auto& slen = str.length();
			//strが「name+U"..."」の形になっているか
			if (slen >= len and util::strEqual(str, 0, len, name))
			{
				//strとnameの長さが一致してたらstr=「name」のはず
				if (slen == len)
				{
					dNums.emplace(0);
				}
				//strが「name+U"(..."」の形になっているか
				else if (str[len] == U'(' and slen >= len + 3)
				{
					auto closePos = str.indexOf(U")", len + 2);
					//strが「name+U"(...)"」の形になっているか
					if (not (closePos == String::npos))
					{
						//strが「name+U"(数字)"」の形になっているか
						if (auto num = ParseOpt<size_t>(util::slice(str, len + 1, closePos)))
						{
							//数字を追加
							dNums.emplace(*num);
						}
					}
				}
			}
		}
		size_t num=0;
		//かぶらない数字を探す
		while (dNums.contains(num))num++;
		//オリジナルのままでもかぶらない
		if (num == 0)return name;
		//オリジナルに数字を添える
		else return name + U"({})"_fmt(num);
	}

	void PartsManager::scaleHelperParamsSetting(double baseLength, const Camera::DistanceType& distanceType)
	{
		scaleHelperBaseLength = baseLength;
		distanceTypeUsedInScaleHelper = distanceType;
	}

	Borrow<Parts> PartsManager::birthParts()
	{
		return scene->birth<Parts>(*this);
	}

	Borrow<Parts> PartsManager::addParts(const PartsParams& params, bool resolveNameDuplication)
	{
		if (master == nullptr and params.name != U"master")return nullptr;

		auto parts = birthParts();

		parts->build(params);

		parts->base = params;

		parts->params = params;

		return addParts(parts, resolveNameDuplication);
	}

	Borrow<Parts> PartsManager::addParts(const Borrow<Parts>& parts, bool resolveNameDuplication)
	{
		if (master == nullptr and parts->name != U"master")return nullptr;

		if (resolveNameDuplication)
		{
			parts->setName(_resolveNameDuplication(parts->getName()));
		}

		parts->transfer(*this);

		if (parts->name == U"master")
		{
			setMaster(parts);
		}
		else
		{
			partsArray << parts;
			parts->followDestiny(master);//masterが死んだらパーツが死ぬようにする
		}

		return parts;
	}

	Borrow<Parts> PartsManager::setMaster(const Borrow<Parts>& masterParts)
	{
		if (master) {
			execute(new KillParts(U"master", true), this->lend());
		}

		masterParts->setName(U"master");

		master = masterParts;

		partsArray << master;

		master->transform->setParent(transform);

		return master;
	}

	Borrow<Parts> PartsManager::createMaster()
	{
		//masterパーツを作成
		auto m = birthParts();

		m->build(PartsParams(U"tmp"));

		m->tex->visible = false;

		m->setColor(ColorF{ 0,0,0,0 });

		return setMaster(m);
	}

	Borrow<Parts> PartsManager::find(const String& name)
	{
		for (auto itr = partsArray.begin(), en = partsArray.end(); itr != en; ++itr)
		{
			if ((*itr)->name == name)return *itr;
		}
		return nullptr;
	}

	void PartsManager::killParts(const Borrow<Parts>& parts)
	{
		parts->die();
	}

	void PartsManager::killParts(const String& name){
		killParts(find(name));
	}

	void PartsManager::setPartsVisibility(bool visible)
	{
		for (auto parts : partsArray)
		{
			parts->tex->visible = visible;
		}
	}

	void PartsManager::start()
	{
		Object::start();

		auto scene_draw_manager = scene->getDrawManager();

		dm = addComponent<Draw2D<DrawManager>>(scene_draw_manager, scene_draw_manager->getCamera(), MSRenderTexture{ 0,0,HasDepth::No }, ColorF{ 0,0,0,0 });

		createMaster();
	}

	void PartsManager::update(double dt)
	{
		dm->drawing.scalePos = transform->getPos().xy() - util::sc();//反転の中心を更新
		Object::update(dt);
		dm->drawing.update();//drawManagerをアプデ
	}

	String GetParamStr(const PartsParams& p, StringView name)
	{
		if (name == U"path" and p.path)
		{
			return *p.path;
		}
		if (name == U"drawing")
		{
			if (p.path)return *p.path;
			else return get<1>(p.drawing).getName();
		}
		if (name == U"parent")
		{
			return p.parent;
		}
		if (name == U"pos")
		{
			return Format(p.pos);
		}
		if (name == U"posX")
		{
			return Format(p.pos.x);
		}
		if (name == U"posY")
		{
			return Format(p.pos.y);
		}
		if (name == U"scale")
		{
			return Format(p.scale);
		}
		if (name == U"scaleX")
		{
			return Format(p.scale.x);
		}
		if (name == U"scaleY")
		{
			return Format(p.scale.y);
		}
		if (name == U"angle")
		{
			return Format(p.angle);
		}
		if (name == U"z")
		{
			return Format(p.z);
		}
		if (name == U"rotatePos")
		{
			return Format(p.rotatePos);
		}
		if (name == U"color")
		{
			return Format(p.color);
		}
		return String();
	}
	String GetParamDefault(StringView name)
	{
		if (name == U"pos" or name == U"scale" or name == U"rotatePos")
		{
			return U"(0,0)";
		}
		if (name == U"color")
		{
			return U"(0,0,0,0)";
		}
		return U"0";
	}
}

namespace mot
{
	void loadPartsTexture(const Borrow<my::Scene>& scene, const String& relativePath)
	{
		//アセットが登録されていなければ
		if (not scene->r.textures.contains(relativePath))
		{
			scene->r.type = RegisterAssets::AssetType::texture;
			using namespace resource;
			texture::Register(scene->r(relativePath), localPath + relativePath, TextureDesc::Mipped);
			resource::texture::Load(relativePath);
		}
	}

	bool savePartsJson(const Borrow<PartsManager>& pm, const String& path)
	{
		Array<PartsParams> many_parts;
		for (const auto& parts : pm->partsArray)
		{
			many_parts << PartsParams(parts->params);
		}

		JSON json;

		for (const auto& parts : many_parts)
		{
			json[parts.name][U"parent"] = parts.parent;
			String texture = U"";
			if (parts.path)
			{
				texture = *parts.path;
			}
			else
			{
				const auto& fig = std::get<1>(parts.drawing);
				const auto& figName = fig.getName();
				if (figName == U"Circle") {
					texture = U"cir {}"_fmt(fig.getCircle().r);
				}
				else if (figName == U"RectF") {
					const auto& rect = fig.getRectF();
					texture = U"rect {} {}"_fmt(rect.w, rect.h);
				}
				else if (figName == U"Triangle")
				{
					const auto& tri = fig.getTriangle();
					const auto& deg = util::angleOf2Vec(tri.p1 - tri.p0, tri.p2 - tri.p0) / 1_deg;
					const auto& len1 = (tri.p0 - tri.p1).length();
					const auto& len2 = (tri.p0 - tri.p2).length();
					texture = U"tri {} {} {}"_fmt(deg, len1, len2);
				}
			}
			json[parts.name][U"texture"] = texture;
			json[parts.name][U"pos"] = parts.pos;
			json[parts.name][U"z"] = parts.z;
			json[parts.name][U"scale"] = parts.scale;
			json[parts.name][U"angle"] = parts.angle;
			json[parts.name][U"rotatePos"] = parts.rotatePos;
			json[parts.name][U"color"] = parts.color;
		}

		return json.save(path);
	}

	Borrow<PartsManager> LoadParts::create(const String& jsonPath, bool createCollider)
	{
		auto pm = m_scene->birth<PartsManager>();

		return create(jsonPath, pm, createCollider);
	}

	Borrow<PartsManager> LoadParts::create(const String& jsonPath, const Borrow<PartsManager>& pmanager, bool createCollider)
	{
		auto& pm = pmanager;

		JSON json = JSON::Load(jsonPath);

		if (not json) { throw Error{U"PartsのLoad中jsonの読み込みに失敗しました"}; }

		HashTable<Parts*, String> parent_list;
		HashTable<Parts*, double> partsAngle;

		CmdDecoder deco;

		DecoderSet(&deco).registerMakePartsCmd(pm, createCollider);

		for (const auto& elms : json)
		{
			Parts* createdParts = nullptr;

			if (elms.key != U"master")
			{
				auto tmp = elms.value[U"texture"].getString();

				auto texArray = tmp.split(' ');
				texArray.insert(texArray.begin() + 1, U"0 0");
				for (tmp = U""; const auto & tex : texArray)
				{
					tmp += U" " + tex;
				}
				//mkpar name pos param をうちこむ
				deco.input(U"mkpar " + elms.key + tmp)->decode()->execute();
				//作成したパーツを取得
				createdParts = pm->partsArray.back();
			}
			else
			{
				createdParts = pm->master;
			}

			createdParts->setPos(elms.value[U"pos"].get<Vec2>());
			createdParts->setScale(elms.value[U"scale"].get<Vec2>());
			createdParts->setRotatePos(elms.value[U"rotatePos"].get<Vec2>());
			//createdParts->pureRotate(elms.value[U"angle"].get<double>());
			createdParts->setZ(elms.value[U"z"].get<double>());
			createdParts->setColor(elms.value[U"color"].get<ColorF>());

			partsAngle[createdParts] = elms.value[U"angle"].get<double>();

			if (elms.key != U"master")parent_list.emplace(createdParts, elms.value[U"parent"].getString());
		}
		for (auto& p : parent_list)p.first->setParent(p.second);//親子関係は最後に結ぶ(存在しない親を参照してしまうから)

		for (auto& [p, v] : partsAngle)
		{
			p->pureRotate(v, false);		//その場で回転させる
			p->params.angle = v;
		}

		return pm;
	}

	Borrow<Parts> CreateParts(const PartsParams& params, const Borrow<PartsManager>& pmanager, bool createCollider)
	{
		Borrow<Parts> parts;
		if (createCollider)
		{
			if (params.path)
			{
				parts = CreateParts(params, pmanager, *params.path);
			}
			else
			{
				parts = CreateParts(params, pmanager, std::get<1>(params.drawing));
			}
		}
		else
		{
			parts = pmanager->addParts(params);
		}

		return parts;
	}

	Borrow<Parts> CreateParts(const PartsParams& params, const Borrow<PartsManager>& pmanager, const Figure& collider)
	{
		auto parts = pmanager->addParts(params);

		parts->createHitbox({ 0,0 }, { collider.asPolygon() });

		return parts;
	}

	Borrow<Parts> CreateParts(const PartsParams& params, const Borrow<PartsManager>& pmanager, const String& path)
	{
		auto parts = pmanager->addParts(params);

		parts->createHitbox(TextureAsset(path).size() / 2.0, Image{ localPath + path }.alphaToPolygons());

		return parts;
	}
}

double draw_helper::CameraScaleOfParts::operator () () const
{
	return (*helper)();
}

draw_helper::PartsShallow::PartsShallow(const Borrow<mot::PartsManager>& p, const Borrow<IDraw2D>& d)
	:pmanager(p), DrawShallow(d)
{

}

double draw_helper::PartsShallow::getDepth() const
{
	return 0.0;
}
