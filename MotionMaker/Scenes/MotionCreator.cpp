#include "../stdafx.h"
#include "MotionCreator.h"
//#include"../Component/Transform.h"
//#include"../Game/Object.h"
//#include"../Game/UI.h"
//#include"Parts.h"
//#include"../Game/Utilities.h"
//#include"../Util/Util.h"
//#include"../Util/Cmd.hpp"
//#include"../Util/CmdDecoder.h"
//#include"MotionCmd.h"
//#include"../Component/Draw.h"
//#include"Motion.h"
//#include"../Game/Scenes/GameScene.h"
//#include"StateViewScene.h"
//#include"../GameMaster.h"
#include"../MySystem/Motions/Includes.h"
#include"../MySystem/Prg/Includes.h"

namespace {
	constexpr auto UiZvalue = 0;//-100000;
	constexpr auto CameraZvalue = -200000;
	constexpr double UiPriority = Math::Inf;
	Borrow<util::MouseObject> mouse;
	Borrow<Entity> clickedSurfaceUiEntity;
	Borrow<Entity> clickedSurfaceParts;
	Array<Borrow<Collider>>uiCollider;//ui
	Array<Borrow<Collider>>partsCollider;//parts
	Array<Borrow<Collider>>pointCollider;//rotatePoint/scalePoint
	Borrow<mot::PartsManager> pmanager;
	Borrow<mot::Parts> selecting;
	Borrow<mot::Parts> grabbing;
}

namespace mot
{
	class DrawPartsFrame :public IDraw2D
	{
	private:
		Borrow<Parts> parts;
	public:
		double thickness=1.0;

		DrawPartsFrame(DrawManager* manager, const Borrow<Parts>& parts)
			:IDraw2D(manager), parts(parts)
		{
		}

		void update(double dt)override
		{
			relative.z = UiZvalue - 1 - transform->getPos().z;//前面に表示　ただしUIに隠れる
		}

		void draw()const override
		{
			std::get<2>(parts->collider->hitbox.shape).getScaledFig().drawFrame(thickness, color);
		}
	};
	using Event = std::function<void()>;
	template<class... Args>
	Borrow<ui::Button> ButtonEvent(const Borrow<Object>& act_owner, const String& eventName, Array<Event> events, Args&& ...args)
	{
		auto button = ui::createButton(act_owner->scene, args...);
		button->name = eventName + U"Button";
		//eventの数によって条件を付ける場所を変える こうするとsizeが1のときショートカット実行がやりやすい
		if (events.size() == 1)
		{
			auto& act = act_owner->ACreate(eventName);
			//ボタンが押され、かつそのボタンが前面にあるなら
			act.startIf<prg::ButtonChecker>(button)
				.andIf([=] {return button->collider->owner == clickedSurfaceUiEntity;});
			act.add(events[0]);
		}
		else {
			auto& act = act_owner->ACreate(eventName, true, true);

			for (const auto& e : events)
			{
				//ボタンが押され、かつそのボタンが前面にあるなら
				act.add(e)
					.startIf<prg::ButtonChecker>(button)
					.andIf([=] {return button->collider->owner == clickedSurfaceUiEntity; });
			}
		}

		button->box->transform->setZ(UiZvalue - CameraZvalue);

		uiCollider << button->collider;

		//Buttonを先にアップデートしないとclickedSurfaceUiEntityがnullptrにされちゃうよ		そんなことない？
		button->priority.setPriority(UiPriority);

		return button;
	}

	template<class... Args>
	Borrow<ui::Button> ButtonEvent(const Borrow<Object> act_owner, const String& eventName, const Event& event, Args&& ...args)
	{
		return ButtonEvent(act_owner, eventName, Array<Event>{ event }, args...);
	}

	class PartsDetailWindow:public Object
	{
	private:
		void createTextBoxs(DrawManager* dm,double box_w)
		{
			for (const auto& elems : all_params_key)
			{
				for (const auto& elem : elems)
				{
					params[elem] = ui::createSimpleInputBox(scene, { 0,0 }, box_w / elems.size());
					params[elem]->transform->setParent(transform);

					auto f = ui::makeUiLike(params[elem]->owner->addComponent<DrawFont>(dm, 16));
					f->text = elem;
					f->color = Palette::Black;
					f->relative = { -f->getEndX() - 5,5,0 };

					uiCollider << params[elem]->owner->getComponent<Collider>();
				}
			}
		}

		std::tuple<double,double> textBoxSetting(DrawManager* dm,double init_x,double init_y,double w,double box_w)
		{
			const double inter = 40;
			double x = init_x;
			double y = init_y;
			for (const auto& elems : all_params_key)for (const auto& elem : elems)
			{
				params[elem]->owner->getComponent<DrawFont>()->visible = false;
				params[elem]->visible = false;
			}

			for (const auto& elems : params_key)
			{
				x = init_x;
				for (const auto& elem : elems)
				{
					double size = elems.size();
					auto f = params[elem]->owner->getComponent<DrawFont>();
					x -= f->relative.x;
					params[elem]->transform->setLocalPos({ x,y,-1});
					params[elem]->owner->getComponent<DrawFont>()->visible = true;
					params[elem]->visible = true;
					x += box_w / size + 10;
				}
				y += inter;
			}
			return { x,y };
		}

		enum class PartsType {
			master,
			texture,
			rect,
			triangle,
			circle,
		};
		void partsParamsSetting(PartsType type){
			this->type = type;
			switch (type)
			{
			case PartsType::master:
				params_key = all_params_key.filter([=](Array<String> p) {
					return not(
						p == Array<String>{U"W", U"H"}
					or p == Array<String>{U"Radius"}
					or p == Array<String>{U"Deg"}
					or p == Array<String>{U"Len1", U"Len2"}
					or p == Array<String>{U"Texture"});
					});
				break;
			case PartsType::texture:
				params_key = all_params_key.filter([=](Array<String> p) {
					return not(
						p == Array<String>{U"W", U"H"}
					or p == Array<String>{U"Radius"}
					or p == Array<String>{U"Deg"}
					or p == Array<String>{U"Len1", U"Len2"});
					});
				break;
			case PartsType::rect:
				params_key = all_params_key.filter([=](Array<String> p) {
					return not(
						p == Array<String>{U"Texture"}
					or p == Array<String>{U"Radius"}
					or p == Array<String>{U"Deg"}
					or p == Array<String>{U"Len1", U"Len2"});
				});
				break;
			case PartsType::triangle:
				params_key = all_params_key.filter([=](Array<String> p) {
					return not(
						p == Array<String>{U"Texture"}
					or p == Array<String>{U"Radius"}
					or p == Array<String>{U"W",U"H"});
				});
				break;
			case PartsType::circle:
				params_key = all_params_key.filter([=](Array<String> p) {
					return not(
						p == Array<String>{U"Texture"}
					or p == Array<String>{U"W", U"H"}
					or p == Array<String>{U"Deg"}
					or p == Array<String>{U"Len1", U"Len2"});
				});
				break;
			}			
		}

		void windowSetting(DrawManager* dm)
		{
			//画面の構成
			auto [x, y] = textBoxSetting(dm, init_x, init_y, w, box_w);

			window->drawing.w = w;
			window->drawing.h = y;

			std::get<2>(windowCollider->hitbox.shape) = RectF{ 0,0,w,y };

			fitButton->transform->setLocalPos({ init_x, y + 10, 0 });
		}
	public:
		bool visible=true;
		HashTable<String, ui::SimpleInputBox*> params;
		Array<Array<String>> all_params_key{
			{U"Name"},
			{U"Parent"},
			{U"Texture"},
			{U"W",U"H"},
			{U"Deg"},
			{U"Len1",U"Len2" },
			{U"Radius"},
			{U"X",U"rdX"},
			{U"Y",U"rdY"},
			{U"Z"},
			{U"Angle"},
			{U"RP X",U"RP Y"},
			{U"S X",U"S Y"},
			{U"R",U"G",U"B",U"A"}
		};
		Array<Array<String>> params_key;
		Object* frame = nullptr;
		Object* rotatePoint = nullptr;
		Object* scalePoint = nullptr;
		prg::IAction* selectAct;
		PartsType type;

		String getText(const String& key) {
			return params[key]->getText();
		}

		void createPartsFrame()
		{
			frame = scene->birth();
			auto tmp = frame->addComponent<DrawPartsFrame>(scene->getDrawManager(), selecting);
			tmp->transform->setPos(selecting->transform->getPos());
			tmp->transform->setParent(selecting->transform);
			tmp->color = Palette::Cyan;
		}

		void createRotatePoint()
		{
			rotatePoint = scene->birth();
			const double r = 6;
			auto tmp = rotatePoint->addComponent<DrawCircle>(scene->getDrawManager(), r);
			tmp->color = Palette::Orange;
			rotatePoint->ACreate(U"move", true)
				.add(
				//アドレスのコピー　これをしないとthis->...を参照してしまう
				[rp = rotatePoint, s = selecting](double dt) {
					rp->transform->setPos(
						{ s->getRotatePos().rotated(s->getAbsAngle()*1_deg) + s->getPos() + s->transform->getParent()->getPos().xy(),UiZvalue/2 - 11}
					);
				}
			);
			pointCollider << rotatePoint->addComponent<Collider>(CollideBox::CollideFigure(Circle{ 0,0,r }));
			//つかんだ時の処理
			std::shared_ptr<Vec2> offset{ new Vec2{0,0} };
			rotatePoint->ACreate(U"grab")
				.add(
				[=, rp = rotatePoint]
				{
					*offset = rp->transform->getPos().xy() - (mouse->getCursorPos(&rp->getComponent<Field<Influence>>(U"dManagerInfluence")->value)).xy();
				},
				[=, rp = rotatePoint, s = selecting](double dt)
				{
					s->setRotatePos(((mouse->getCursorPos(&rp->getComponent<Field<Influence>>(U"dManagerInfluence")->value)).xy() + *offset - s->transform->getPos().xy()).rotate(-s->getAngle() * 1_deg));
					setText(U"RP X", Format(s->getRotatePos().x));
					setText(U"RP Y", Format(s->getRotatePos().y));
				}
			).endIf([] {return MouseL.up(); });
		}

		const double init_x = 10;
		const double init_y = 10;
		const double w = 330;
		const double box_w = 200;
		Borrow<DrawRectF> window;//自分のコンポーネント
		Borrow<Collider> windowCollider;
		Borrow<ui::Button> fitButton;

		void start()override
		{
			Object::start();

			DrawManager* dm = scene->getDrawManager();
			//画面の構成
			window = ui::makeUiLike(addComponent<DrawRectF>(dm, 0, 0, 0, 0));
			windowCollider = addComponent<Collider>(CollideBox::CollideFigure{ RectF{ 0,0,0,0 } }, Vec3{ 0,0,0 });
			uiCollider << windowCollider;

			//textBoxのセット
			createTextBoxs(scene->getDrawManager(), box_w);

			partsParamsSetting(PartsType::master);

			auto button = ButtonEvent(*this, U"fitting", [=] {
				if (selecting != nullptr)fit();
			}, U"適応する", 18, Vec2{ 0,0 });

			button->transform->setParent(transform);

			fitButton = button;

			windowSetting(dm);

			//座標をセット カメラを追従するようにする カメラを動かした後zがUiZvalueとなるように-CameraZvalueをおいておく
			transform->setPos({ util::sw() - w,0,UiZvalue });

			//パーツ選択
			ACreate(U"selectParts").add(
				[=] {
					auto pre = selecting;
					Borrow<Parts> selectedParts;
					for (const auto& parts : pmanager->partsArray)
					{
						if (parts == clickedSurfaceParts)
						{
							selectedParts = parts;
							//選択済みのパーツだったら　つかむ
							if (pre == selectedParts) {
								grabbing = selectedParts;
							}
							break;
						};
					}
					select(selectedParts);
				}
			);

			//パーツを動かす
			std::shared_ptr<Vec2> offset{ new Vec2{0,0} };
			ACreate(U"move")
				.startIf([=] {return grabbing; })
				.add(
				[=]
				{
				*offset = grabbing->getPos() - (mouse->getCursorPos(&grabbing->getComponent<Field<Influence>>(U"dManagerInfluence")->value) - grabbing->transform->getParent()->getPos()).xy();
				},
					[=](double dt)
				{
					grabbing->setAbsPos(mouse->getCursorPos(&grabbing->getComponent<Field<Influence>>(U"dManagerInfluence")->value).xy() + *offset);/*.rotate(-grabbing->getAngle() * 1_deg));*/
					auto p = grabbing->getPos();
					setText(U"X", Format(p.x));
					setText(U"Y", Format(p.y));
					setText(U"RP X", Format(grabbing->getRotatePos().x));
					setText(U"RP Y", Format(grabbing->getRotatePos().y));
				},
					[=]
				{
					grabbing = nullptr;
				}
			).endIf([] {return MouseL.up(); });
		}

		void update(double dt)override
		{
			//set();

			//アクションをアップデートするより先に
			if (MouseL.down())
			{
				clickedSurfaceUiEntity = nullptr;
				clickedSurfaceUiEntity = mouse->getClickedSurfaceObject(uiCollider);
				if (clickedSurfaceUiEntity == nullptr) {
					//rotatePosとかscalePosとか
					auto ent = mouse->getClickedSurfaceObject(pointCollider);
					if (ent != nullptr) {
						dynamic_cast<Object*>(ent.get())->startAction(U"grab");
					}
					else {
						clickedSurfaceParts = nullptr;
						clickedSurfaceParts = mouse->getClickedSurfaceObject(partsCollider);
						startAction(U"selectParts");
					}
				}
			}

			if (KeyControl.pressed() and KeyEnter.down())
				startAction(U"fitting");

			Object::update(dt);
		}

		void setText(const String& param,const String& te)
		{
			params[param]->tex.text = te;
		};

		void select(const Borrow<Parts>& target)
		{
			selecting = target;
			//フレームなどを描く

			if (frame != nullptr) {
				frame->die();
				frame = nullptr;
			}
			if (rotatePoint != nullptr) {
				rotatePoint->die();
				rotatePoint = nullptr;
			}
			if (scalePoint) {
				scalePoint->die();
				scalePoint = nullptr;
			}

			pointCollider.clear();

			set();

			if (selecting)
			{
				//フレームを生成
				if(selecting->collider) createPartsFrame();

				//rotatePointの生成
				createRotatePoint();
			}
		}

		void set()
		{
			for (const auto& param : params)param.second->tex.text = U"";

			if (not selecting)return;

			setText(U"Name", selecting->getName());
			setText(U"Parent", selecting->getParent());
			if (selecting->params.name == U"master")
			{
				partsParamsSetting(PartsType::master);
			}
			else if (selecting->params.path)
			{
				setText(U"Texture", *selecting->params.path);
				partsParamsSetting(PartsType::texture);
			}
			else
			{
				const auto& fig = std::get<1>(selecting->params.drawing);
				auto index = fig.getIndex();
				if (index == 0)
				{
					//circle
					setText(U"Radius", Format(fig.getCircle().r));
					partsParamsSetting(PartsType::circle);
				}
				else if (index == 1)
				{
					const auto& rect = fig.getRectF();
					setText(U"W", Format(rect.w));
					setText(U"H", Format(rect.h));
					partsParamsSetting(PartsType::rect);
				}
				else if(index==3)
				{
					const auto& tri = fig.getTriangle();
					setText(U"Deg", Format(util::angleOf2Vec(tri.p1 - tri.p0, tri.p2 - tri.p0)/1_deg));
					setText(U"Len1", Format((tri.p0 - tri.p1).length()));
					setText(U"Len2", Format((tri.p0 - tri.p2).length()));
					partsParamsSetting(PartsType::triangle);
				}
			}
			setText(U"Angle", Format(selecting->getAngle()));
			setText(U"X", Format(selecting->getPos().x));
			setText(U"Y", Format(selecting->getPos().y));
			setText(U"rdX", Format(selecting->getRelativePos().x));
			setText(U"rdY", Format(selecting->getRelativePos().y));
			setText(U"Z", Format(selecting->getZ()));
			setText(U"S X", Format(selecting->transform->scale.aspect.vec.x));
			setText(U"S Y", Format(selecting->transform->scale.aspect.vec.y));
			setText(U"RP X", Format(selecting->getRotatePos().x));
			setText(U"RP Y", Format(selecting->getRotatePos().y));
			const auto& color = selecting->getColor();
			setText(U"R", Format(color.r));
			setText(U"G", Format(color.g));
			setText(U"B", Format(color.b));
			setText(U"A", Format(color.a));

			windowSetting(scene->getDrawManager());
		}
		/* TODO: 例外処理をつけるため、警告ポップアップを作ろう*/
		void fit()
		{
			if (type == PartsType::texture) {
				//存在しないパスだとバグる　例外処理をつけよう createHitbox
				const auto& path = getText(U"Texture");
				bool changed = false;
				if (selecting->params.path) {
					changed = selecting->params.path != path;
				}
				else {
					changed = not path.isEmpty();
				}
				if (changed) {
					partsCollider.remove(selecting->collider);
					selecting->remove(selecting->collider);
					auto pos = Texture{ AssetManager::myAsset(localPath + path) }.size() / 2;
					
					partsCollider << selecting->createHitbox(
						pos,
						Image{ AssetManager::myAsset(localPath + path) }.alphaToPolygons().rotateAt(pos, selecting->getAbsAngle() *1_deg)
					);

					frame->die();
					createPartsFrame();
				}
				selecting->params.path = path;
				loadPartsTexture(scene, path);
				selecting->setTexture(resource::texture(path));
			}
			else if (type == PartsType::rect)
			{
				Vec2 hw{ Parse<double>(getText(U"W")),Parse<double>(getText(U"H")) };
				RectF rect{ -hw / 2,hw };

				partsCollider.remove(selecting->collider);
				selecting->remove(selecting->collider);
				partsCollider << selecting->createHitbox({ 0,0 }, { rect.asPolygon().rotate(selecting->getAbsAngle() * 1_deg)});

				frame->die();
				createPartsFrame();

				selecting->setTexture(rect);
			}
			else if (type == PartsType::circle)
			{
				double r=Parse<double>(getText(U"Radius"));
				Circle cir{ 0,0,r };

				partsCollider.remove(selecting->collider);
				selecting->remove(selecting->collider);
				partsCollider << selecting->createHitbox({ 0,0 }, { cir.asPolygon() });

				frame->die();
				createPartsFrame();

				selecting->setTexture(cir);
			}
			else if (type == PartsType::triangle)
			{
				double deg = Parse<double>(getText(U"Deg"));
				double l1 = Parse<double>(getText(U"Len1"));
				double l2 = Parse<double>(getText(U"Len2"));
				auto p0 = std::get<1>(selecting->params.drawing).getTriangle().p0;
				Triangle tri{ p0,p0 + util::polar(l1,-deg * 1_deg / 2),p0 + util::polar(l2,deg * 1_deg / 2) };

				partsCollider.remove(selecting->collider);
				selecting->remove(selecting->collider);
				partsCollider << selecting->createHitbox({ 0,0 }, { tri.asPolygon().rotate(selecting->getAbsAngle() * 1_deg) });

				frame->die();
				createPartsFrame();

				selecting->setTexture(tri);
			}

			selecting->setName(params[U"Name"]->getText());

			selecting->setRotatePos({ Parse<double>(params[U"RP X"]->getText()), Parse<double>(params[U"RP Y"]->getText()) });
			selecting->setPos({ Parse<double>(params[U"X"]->getText()),Parse<double>(params[U"Y"]->getText()) });

			selecting->setScale({ Parse<double>(params[U"S X"]->getText()),Parse<double>(params[U"S Y"]->getText()) });
			setText(U"RP X", Format(selecting->getRotatePos().x));
			setText(U"RP Y", Format(selecting->getRotatePos().y));

			selecting->setZ(Parse<double>(params[U"Z"]->getText()));
			selecting->setColor({
				Parse<double>(params[U"R"]->getText()),
				Parse<double>(params[U"G"]->getText()),
				Parse<double>(params[U"B"]->getText()),
				Parse<double>(params[U"A"]->getText()) });

			selecting->setAngle(Parse<double>(params[U"Angle"]->getText()));
			//回転すると変わるので更新
			setText(U"X", Format(selecting->getPos().x));
			setText(U"Y", Format(selecting->getPos().y));
			setText(U"Z", Format(selecting->getZ()));

			selecting->setParent(params[U"Parent"]->getText());
			//parentをセットすると相対座標が変わるので更新
			setText(U"X", Format(selecting->getPos().x));
			setText(U"Y", Format(selecting->getPos().y));
			setText(U"Angle", Format(selecting->getAngle()));
		}
	};

	class PartsEditor::Impl;

	class CmdArea :public Object
	{
	public:
		ui::SimpleInputArea* area;
		CmdDecoder decoder;
		PartsDetailWindow* pd;
		Borrow<PartsEditor::Impl> editor;

		void start()override
		{
			Object::start();

			pd = scene->getEntityManager()->findOne<PartsDetailWindow>();

			double w, h;
			Vec2 pos;

			w = 600;
			h = 40;
			pos = { (util::sc().x - w / 2)/2, util::sh() - h - 15 };

			area = ui::createSimpleInputArea(scene, {0,0}, w, h);

			area->transform->setPos({ pos,UiZvalue });

			uiCollider << area->owner->getComponent<Collider>();

			auto f = ui::makeUiLike(area->owner->addComponent<DrawFont>(scene->getDrawManager(), Font(20, Typeface::Bold)));
			f->text = U"Command >";
			f->relative = { -130,0,0 };
			f->color = Palette::White;

			area->transform->setPos({ pos,UiZvalue });

			buildDecoder();			
		}
		//コマンドをフィールドコンポーネントから簡単に追加する
		void addCmd(const String& name,const String& componentName)
		{
			decoder.add_original<FuncAction>(
											name,
											Eventa<FuncAction>::NonEvent,
											[=](FuncAction* act)
											{
												act->ini = getComponent<Field<std::function<void()>>>(componentName)->value;
												act->endIf<TimeCondition>(act->lend(), 0);
												return act;
											}
											);
		};

		// クラス特有の命令一覧
		// mkpar
		// find
		// kill
		//
		void buildDecoder();

		void update(double dt)override
		{
			if (area->getText() == U"\n")area->tex.text.clear();
			if (area->isActive() and KeyEnter.down())
			{
				if (area->getText() != U"") {
					decoder.input(area->getText());
					area->tex.text.clear();
					if (auto cmd = decoder.decode())cmd->execute();
					else throw Error{ U"命令:{}が見つかりませんでした"_fmt(decoder.token[0]) };
				}
			}
			Object::update(dt);
		}
	};

	class CameraController:public Object
	{
	private:
		bool touch_thumb;
		bool touch_bar_thumb;
		double x, y, r;
		Borrow<DrawRectF> bar_thumb;
		Borrow<DrawRectF> scale_bar;
		Borrow<DrawCircle> range;
		Borrow<DrawCircle> thumb;
		double sensitivity;
		double mouse_sensitivity;

		double _scale_func(double t)
		{
			if (t > 0)
			{
				return 1.0 + scale_range * t;
			}
			else
			{
				return 1.0 + (1 - 1.0 / scale_range) * t;
			}
		}

		Borrow<Collider> c1;
		Borrow<Collider> c2;
	public:
		DrawManager* manager;

		double get_x() { return x; }
		double get_y() { return y; }

		double scale_range;

		void start()override
		{
			Object::start();
			manager = scene->getDrawManager();
			transform->setParent(scene->getDrawManager()->getCamera()->transform);
			transform->setZ(UiZvalue-CameraZvalue);
			sensitivity = 0.1;
			mouse_sensitivity = -10;
			scale_range = 5.0;
			touch_thumb = touch_bar_thumb = false;
			double distance_from_Scene = 20, bar_h = 20, bar_thumb_w = 15, bar_from_range = 10;
			r = 80;
			x = 0;
			y = 0;
			range = ui::makeUiLike(addComponent<DrawCircle>(scene->getDrawManager(), x, y, r));
			thumb = ui::makeUiLike(addComponent<DrawCircle>(scene->getDrawManager(), x, y, r / 4));
			bar_thumb = ui::makeUiLike(addComponent<DrawRectF>(scene->getDrawManager(), RectF{ Arg::center(x, y - r - bar_h / 2 - bar_from_range),bar_thumb_w,bar_h }));
			scale_bar = ui::makeUiLike(addComponent<DrawRectF>(scene->getDrawManager(), x - r, y - r - bar_h - bar_from_range, 2 * r, bar_h));

			c1 = addComponent<Collider>(CollideBox::CollideFigure(range->drawing));
			c2 = addComponent<Collider>(CollideBox::CollideFigure(scale_bar->drawing));
			c1->hitbox.relative = { x, y,0 };
			c2->hitbox.relative = { x, y - r - bar_h/2 - bar_from_range,0 };

			uiCollider << c1;
			uiCollider << c2;

			bar_thumb->relative.z = -1;

			scale_bar->color=ColorF(0.2, 0.2, 0.2);
			bar_thumb->color=Palette::Gray;
			range->color=ColorF(0.2, 0.2, 0.2);
			thumb->color = Palette::Gray;

			transform->setPos({ util::sw() - r - distance_from_Scene ,util::sh() - r,0 });
		}

		void update(double dt)override
		{
			//UIの操作
			if (MouseL.up()) {
				touch_thumb = touch_bar_thumb = false;
				thumb->drawing.setCenter(0, 0);
			}
			else if (MouseL.down()) {
				touch_thumb = thumb->getDrawing().leftClicked();
				if ((not touch_thumb) and range->getDrawing().leftClicked())
				{
					Vec2 p = transform->getXY();
					thumb->drawing.setCenter((Cursor::Pos() - p).setLength(Min((Cursor::Pos() - p).length(), range->drawing.r - thumb->drawing.r - 1)));
					touch_thumb = true;
				}
				touch_bar_thumb = bar_thumb->getDrawing().leftClicked();
				if ((not touch_bar_thumb) and scale_bar->getDrawing().leftClicked())
				{
					double px = transform->getXY().x;

					if (abs(Cursor::Pos().x - px) < (scale_bar->drawing.w - bar_thumb->drawing.w) / 2) {
						bar_thumb->drawing.setCenter(Vec2{ Cursor::Pos().x - px,bar_thumb->drawing.centerY() });
					}
					else {
						bar_thumb->drawing.setCenter({
							Cursor::Pos().x - px > 0 ? (scale_bar->drawing.w - bar_thumb->drawing.w) / 2 : -(scale_bar->drawing.w - bar_thumb->drawing.w) / 2
							,bar_thumb->drawing.centerY()
							});
					}
					touch_bar_thumb = true;
				}
			};
			//UIを動かす
			if (touch_thumb) {
				if (thumb->drawing.movedBy(Cursor::DeltaF()).intersectsAt(range->drawing).has_value()
					and thumb->drawing.movedBy(Cursor::DeltaF()).intersectsAt(range->drawing)->isEmpty())
						thumb->drawing.moveBy(Cursor::DeltaF());
			}
			else if (touch_bar_thumb) {
				if (scale_bar->drawing.intersects(bar_thumb->drawing.movedBy(Cursor::DeltaF().x, 0).left()) and scale_bar->drawing.intersects(bar_thumb->drawing.movedBy(Cursor::DeltaF().x, 0).right()))
					bar_thumb->drawing.moveBy(Cursor::DeltaF().x, 0);
			}

			if (scale_bar->drawing.intersects(bar_thumb->drawing.movedBy(mouse_sensitivity * int32(Mouse::Wheel()), 0).left())
				and scale_bar->drawing.intersects(bar_thumb->drawing.movedBy(mouse_sensitivity * int32(Mouse::Wheel()), 0).right()))
			{
				bar_thumb->drawing.moveBy(mouse_sensitivity * int32(Mouse::Wheel()), 0);
			}

			manager->translate += sensitivity * (thumb->drawing.center - Vec2{ x,y });
			manager->scale = Vec2{ 1,1 }*_scale_func((bar_thumb->drawing.centerX() - scale_bar->drawing.centerX()) * 2.0 / (scale_bar->drawing.w - bar_thumb->drawing.w));
		}
	};

	class PartsEditor::Impl :public Object
	{
	public:
		PartsDetailWindow* dw;
		CmdArea* c;
		Optional<PartsParams> copiedParams = none;
		Optional<String> srcPath = none;
		//モーションスクリプトに出力される
		Array<Borrow<Parts>> choosingParts;

		Borrow<Parts> addParts(const PartsParams& params,const String& p)
		{
			auto parts = pmanager->addParts(params);
			partsCollider << parts->createHitbox(resource::texture(p).size()/2, Image{AssetManager::myAsset(localPath+p)}.alphaToPolygons());
			return parts;
		}

		void birthPartsManager()
		{
			if (pmanager)pmanager->die();
			pmanager = scene->birth<PartsManager>();
			pmanager->transform->setXY(Vec2{ util::sw(),util::sh() } / 2);
			pmanager->scaleHelperParamsSetting(Abs(CameraZvalue), Camera::Z);
		}
		//右クリック選択
		void chooseParts(const Borrow<Parts>& parts,bool affectChildren)
		{
			choosingParts<<parts;

			auto frame = parts->addComponentNamed<DrawPartsFrame>(U"RClickSelecting", scene->getDrawManager(), parts);//赤いフレームを追加
			frame->thickness = 2.0;
			frame->color = ColorF{ Palette::Red,0.5 };
			
			if (affectChildren)
			{
				for (auto& child : parts->partsRelation.getChildren())
				{
					chooseParts(*child, affectChildren);
				}
			}
		}
		//右クリック選択中の解除
		void releaseChoosing(const Borrow<Parts>& parts, bool affectChildren)
		{
			choosingParts.remove(parts);
			parts->remove<DrawPartsFrame>(U"RClickSelecting");//赤いフレーム削除
			if (affectChildren)
			{
				for (auto& child : parts->partsRelation.getChildren())
				{
					releaseChoosing(*child, affectChildren);
				}
			}
		}

		void start()override
		{
			Object::start();
			birthPartsManager();
			mouse = scene->birth<util::MouseObject>();
			dw = scene->birth<PartsDetailWindow>();
			dw->priority.setPriority(priority.getPriority() + 1);
			auto cont = scene->birth<CameraController>();
			c = scene->birth<CmdArea>();
			c->editor = *this;

			//画像読み込み
			ButtonEvent(*this, U"readImg", [=] {
				Array<String>path = Dialog::OpenFiles({ FileFilter::AllImageFiles() });
				if (path.isEmpty())return;
				//パーツ追加
				for (const auto& tmp : path)
				{
					auto p = FileSystem::RelativePath(tmp, FileSystem::CurrentDirectory() + localPath);
					auto name = FileSystem::BaseName(tmp);
					c->decoder.input(U"mkpar " + name + U" " + p + U" 0 0")->decode()->execute();
				}
			}, U"画像読み込み", 20, Vec2{ 10,10 });

			//JSON読み込み
			ButtonEvent(*this, U"loadJSON", [=] {
				Optional<FilePath> path = Dialog::OpenFile({ FileFilter::JSON() });
				if (not path)return;
				//masterパーツを殺して新しく構築
				c->decoder.input(U"kill master")->decode()->execute();
				scene->partsLoader->create(*path, pmanager, true);
				//当たり判定を取得
				for (auto& p : pmanager->partsArray)
				{
					if (p->collider)partsCollider << p->collider;
				}
			}, U"JSON読み込み", 20, Vec2{ 10,50 });

			//保存
			ButtonEvent(*this, U"save", [=] {
				Optional<FilePath> path = Dialog::SaveFile({ FileFilter::JSON() });
				if (path)savePartsJson(pmanager, *path);
			}, U"保存", 20, Vec2{ 10,90 });

			//スクリプトを登録
			ButtonEvent(*this, U"getScriptPath", [&] {
				srcPath= Dialog::OpenFile({ FileFilter::Text() });
				if (srcPath)
				{
					system(util::toStr(*srcPath).c_str());
					*srcPath = FileSystem::RelativePath(*srcPath, FileSystem::InitialDirectory());
				}
			}, U"スクリプト", 20, Vec2{ 10,180 });

			auto motionInputBox = ui::createSimpleInputBox(scene, Vec2{ 60,230 }, 150);

			//モーション読み込み
			ButtonEvent(*this, U"playMotion", [&, box = motionInputBox->lend(), cmd = c->lend()] {
				if (not srcPath)return;
				//モーションを消去して読み込み
				auto names = MotionScript::GetMotionNames(*srcPath);
				for (const auto& name : names)
				{
					cmd->decoder.input(U"em {}"_fmt(name))->decode()->execute();
					cmd->decoder.input(U"load {} {}"_fmt(*srcPath, name))->decode()->execute();
				}
				//1フレーム後に実行する
				this->actman.createOneShot(U"").startIf([] {return true; }, 1)
					.add([=] {cmd->decoder.input(U"start {}"_fmt(box->getText()))->decode()->execute(); });
			}, U"実行", 20, Vec2{ 10,230 });

			//操作方法切り替え
			ButtonEvent(*this, U"keyMoveModeChange", Array<Event>{
				[=] {
					auto button = scene->findOne<ui::Button>(U"keyMoveModeChangeButton");
					button->setText(U"キー操作終了");
					button->box->fitSize();
					startAction(U"keyMove");
				}, [=] {
					auto button = scene->findOne<ui::Button>(U"keyMoveModeChangeButton");
					button->setText(U"キー操作");
					button->box->fitSize();
					stopAction(U"keyMove");
				}
			}, U"キー操作", 20, Vec2{ util::sw() - 470,10 });

			//パーツをキー入力で移動させる　回転　拡大させる
			ACreate(U"keyMove").add(
				[=](double dt) {
					if (selecting == nullptr or c->area->isActive())return;
					Vec2 moving{ 0,0 };
					double dAngle = 0;
					Vec2 dZoom{ 0,0 };
					double speed = 100;
					bool needToUpdate = false;
					if (KeyShift.pressed())speed *= 2.5;
					if (KeyA.pressed())moving += Vec2{ -1,0 };
					if (KeyD.pressed())moving += Vec2{ 1,0 };
					if (KeyS.pressed())moving += Vec2{ 0,1 };
					if (KeyW.pressed())moving += Vec2{ 0,-1 };
					if (moving.length())
					{
						moving.setLength(speed);
						selecting->setPos(selecting->getPos() + moving * dt);
						needToUpdate = true;
					}
					if (KeyQ.pressed())dAngle -= 1;
					if (KeyE.pressed())dAngle += 1;
					if (dAngle)selecting->setAngle(selecting->getAngle() + dAngle * speed * dt);
					if (KeyJ.pressed())dZoom += Vec2{ -1,0 };
					if (KeyL.pressed())dZoom += Vec2{ 1,0 };
					if (KeyI.pressed())dZoom += Vec2{ 0,1 };
					if (KeyK.pressed())dZoom += Vec2{ 0,-1 };
					if (dZoom.length())
					{
						dZoom.setLength(speed / 150);
						selecting->setScale(selecting->getScale() + dZoom * dt);
						needToUpdate = true;
					}
					if (needToUpdate)dw->set();
				}
			);
			//ゲームビューを生成
			ACreate(U"GameView").add(
				[=] {
					auto& masterManager = Scene::master->getManager();

					auto gameScene = new StateViewScene();

					Scene::master->addScene(gameScene);

					gameScene->followDestiny(scene);

					auto _dm = gameScene->getDrawManager();

					int32 w = 300;

					gameScene->screenSize = Size(w, util::sh());

					gameScene->screenPos.x = util::sw() / 2 - w / 2;

					gameScene->transform->setPos({util::sw() - w , 0, 0 });

					dw->transform->addX(-w);

					cont->transform->addX(-w);

					c->area->w *= 0.8;

					scene->findOne<ui::Button>(U"keyMoveModeChangeButton")->transform->addX(-w);
				}
			);

			ACreate(U"ChooseParts").startIf(MouseR, InputState::d)
				.add(
					[=] {
						//パーツの当たり判定を取得
						auto colliders = util::Generate<Borrow<Collider>>(pmanager->partsArray, [](const Borrow<Parts>& p) {return p->collider; });
						//右クリックされたものを取得
						if (auto clickedParts = mouse->getClickedSurfaceObject(partsCollider))
						{
							//選択済みなら削除　そうでなければ選択
							if (choosingParts.contains(clickedParts))
							{
								releaseChoosing(clickedParts, KeyShift.pressed());
							}
							else {
								chooseParts(clickedParts, KeyShift.pressed());
							}
						}
						else
						{
							//すべて選択解除
							auto _copyed = choosingParts;
							for (auto& parts : _copyed)
							{
								releaseChoosing(parts->lend(), false);
							}
						}
					}
				);

			//ゲームビューを立ち上げる処理の起動関数を渡す
			c->addComponentNamed<Field<std::function<void()>>>(U"gameViewFunction", [b = lend()] {b->startAction(U"GameView"); });

			ACreate(U"Shot").add(
				//キャラクターの画像を生成
				[=] {
					pmanager->dm->drawing.createPicture(U"shot.png", { 0,0 }, s3d::Scene::Size());
				}
			);
			//ゲームビューを立ち上げる処理の起動関数を渡す
			c->addComponentNamed<Field<std::function<void()>>>(U"ShotFunction", [b = lend()] {b->startAction(U"Shot"); });

			auto plusMark = scene->birth();

			ui::makeUiLike(plusMark->addComponent<Draw2D<Polygon>>(scene->getDrawManager(), Shape2D::Plus(6, 1).asPolygon()));

			plusMark->transform->setPos({ util::sc(),0 });
		}

		void update(double dt)override
		{
			//死んだ奴らを除外する
			uiCollider.remove_if([](const Borrow<Collider>& c) {return not c; });
			partsCollider.remove_if([](const Borrow<Collider>& c) {return not c; });
			pointCollider.remove_if([](const Borrow<Collider>& c) {return not c; });
			choosingParts.remove_if([](const Borrow<Parts>& p) {return not p; });

			Object::update(dt);
			//コピー
			if (KeyC.down() and KeyControl.pressed())
			{
				if (selecting)
				{
					copiedParams = selecting->params;//選択中のパーツの現在の状態をクローン
				}
			}
			//貼り付け
			if (KeyV.down() and KeyControl.pressed())
			{
				if (copiedParams)
				{
					auto p = CreateParts(*copiedParams, pmanager, true);
					partsCollider << p->collider;
				}
			}
		}

		Array<PartsParams> createAllPartsParams()const
		{
			Array<PartsParams> ret;
			for (const auto& parts : pmanager->partsArray)
			{
				ret << PartsParams(parts->params);
			}
			return ret;
		}
	};

	void CmdArea::buildDecoder()
	{
		DecoderSet sets(&decoder);
		//パーツを作る命令
		sets.registerMakePartsCmd(pmanager, true, [=](MakeParts* p) { partsCollider << p->getCreatedParts()->collider; });
		//パーツを選択する命令
		decoder.add_event_cmd<FindParts, String>(U"find", [=](FindParts* act) { pd->select(act->getFindParts()); }, pmanager);
		//パーツを削除する命令
		EventFunction<KillParts> killPartsEvent
			= [=](KillParts* act) {
			auto killedParts = act->getKilledParts();
			for (const auto& p : killedParts)
			{
				partsCollider.remove(p->collider);
				if (selecting == p)pd->select(nullptr);
				//マスターが殺されてたら自動生成
				if (p->getName() == U"master")pmanager->createMaster();
			}
			};
		decoder.add_event_cmd<KillParts, String, bool>(U"kill", killPartsEvent, pmanager);
		decoder.add_event_cmd<KillParts, String>(U"kill", killPartsEvent, pmanager);
		sets.motionScriptCmd(pmanager, nullptr);

		{
			auto processing = [=](WriteMotionScript* act) {
				act->targets.clear();
				for (auto parts : editor->choosingParts)
				{
					act->targets.emplace(parts->name);
				}
				return act->build(pmanager);
				};

			decoder.add_original<WriteMotionScript, FilePath, String>(U"write", Eventa<WriteMotionScript>::NonEvent, processing);
			decoder.add_original<WriteMotionScript, FilePath, String, Optional<String>>(U"write", Eventa<WriteMotionScript>::NonEvent, processing);
			decoder.add_original<WriteMotionScript, FilePath, String, Optional<String>, Optional<String>>(U"write", Eventa<WriteMotionScript>::NonEvent, processing);
		}

		addCmd(U"gameview", U"gameViewFunction");
		addCmd(U"Shot", U"ShotFunction");
	}

	void PartsEditor::start()
	{
		Scene::start();

		camera = birth<Camera>(BasicCamera3D(drawManager.getRenderTexture().size(), 50_deg, Vec3{ 0,0,-10 }));

		camera->type = Camera::Z;

		drawManager.setting(camera);

		impl = birth<PartsEditor::Impl>();
		camera->transform->setZ(CameraZvalue);

		//transform->setXY(-Vec2{util::sw(),util::sh()} / 2);
	}

	void StateViewScene::start()
	{
		Scene::start();
		drawManager.backGroundColor = ColorF{ 0.4,0.5,0.6 };

		auto s = Size{ 700,700 };
		const MSRenderTexture uvChecker{ s,ColorF{0.12,0.27,0.12}.removeSRGBCurve() };
		auto ground = birthObject<Object>(Box{ 500,2,500 }, { 0,0,0 });
		ground->getComponent<Collider>(U"hitbox")->setCategory(ColliderCategory::object);
		auto ground_texture = ground->addComponent<Draw3D>(&drawManager, MeshData::OneSidedPlane({ 500,500 }, { 8,8 }));
		//地面の模様作成
		{
			const ScopedRenderTarget2D target{ uvChecker };
			const ScopedRenderStates2D blend{ BlendState::AdditiveRGB };

			for (int32 i = 0; i < 200; ++i)
			{
				auto r = Random<int32>(1, 3);
				Vec2 pos = RandomVec2(Rect{ r, r, s.x - r, s.y - r });
				Circle{ pos,r }.draw(ColorF{ Random(0.2) + 0.1,Random(0.2) + 0.1,Random(0.1) + 0.05 }.removeSRGBCurve());
			}

			for (int32 i = 0; i < 300; ++i)
			{
				auto wh = Random<int32>(4, 8);
				Vec2 pos = RandomVec2(Rect{ 6, 6, s.x - 6, s.y - 6 });
				double theta = Random<double>(0, 90);
				RectF{ pos,wh }.rotated(2 * theta * Math::Pi).draw(ColorF{ Random(0.2) + 0.1,Random(0.2) + 0.1,Random(0.1) + 0.05 }.removeSRGBCurve());
			}

			for (int32 i = 0; i < 400; ++i)
			{
				auto r = Random<double>(4, 10);
				Vec2 pos = RandomVec2(Rect{ 6, 6, s.x - 6, s.y - 6 });
				double theta = Random<double>(0, 120);
				Triangle{ pos,r }.rotated(2 * theta * Math::Pi).draw(ColorF{ Random(0.2) + 0.1,Random(0.2) + 0.1,Random(0.1) + 0.05 }.removeSRGBCurve());
			}

			// 2D 描画をフラッシュ
			Graphics2D::Flush();

			// マルチサンプル・テクスチャをリゾルブ
			uvChecker.resolve();
		}
		ground_texture->tex = uvChecker;
		//カメラ
		camera = birth<Camera>(BasicCamera3D(drawManager.getRenderTexture().size(), 50_deg, Vec3{ 0,12,-25 }));
		drawManager.setting(camera);
		camera->type = Camera::DistanceType::Z;

		actman.create(U"CameraControl", true).add([=](double dt)
		{
				if (KeyUp.pressed())
				{
					camera->transform->addY(10 * dt);
				}
				if (KeyDown.pressed())
				{
					camera->transform->addY(-10 * dt);
				}
				if (KeyRight.pressed())
				{
					camera->transform->addX(10 * dt);
				}
				if (KeyLeft.pressed())
				{
					camera->transform->addX(-10 * dt);
				}
		});
	}
}
