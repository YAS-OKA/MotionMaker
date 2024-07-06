#include "../stdafx.h"
#include "UIProducts.h"
#include"Scenes.h"
#include"../Component/Transform.h"

namespace ui
{
	void ProgressBar::setting(const Vec3& pos, double w, double h, double round)
	{
		setting(pos, w, h, ColorF(0.25), { { 1.0, ColorF(0.1, 0.8, 0.2) } }, round);
	}

	void ProgressBar::setting(const Vec3& pos, double w, double h, const ColorF& backgroundColor, const ColorF& barColor, double round)
	{
		setting(pos, w, h, backgroundColor, { { 1.0, barColor } }, round);
	}

	void ProgressBar::setting(const Vec3& pos, double w, double h, const ColorF& backgroundColor, const Array<std::pair<double, ColorF>>& barColors, double round)
	{
		m_barColors = barColors;
		m_barColors.sort_by([](const auto& a, const auto& b) { return a.first > b.first; });
		transform->setPos(pos);
		auto dm = scene->getDrawManager();
		if (round != 0)
		{
			back = addComponentNamed<Draw2D<RoundRect>>(U"back", dm, 0.0, 0.0, w, h, round);
			rect = addComponentNamed<Draw2D<RoundRect>>(U"gage", dm, 0.0, 0.0, 0.0, h, round);
		}
		else
		{
			back = addComponentNamed<DrawRectF>(U"back", dm, w, h);
			rect = addComponentNamed<DrawRectF>(U"gage", dm, 0.0, h);
		}
		m_w = w;
		//backより前に描く
		std::visit([](auto& x) { x->relative.z -= 0.001; }, rect);

		back->color = backgroundColor;
	}

	double ProgressBar::rate(double r)
	{
		m_rate = Clamp(r, 0.0, 1.0);
		std::visit([&](auto& x) {
			x->drawing.w = m_w * m_rate;
			}, rect);//長さを設定

		return rate();
	}

	double ProgressBar::rate()const
	{
		return m_rate;
	}

	bool ProgressBar::visible(bool vis)
	{
		back->visible = vis;

		std::visit([&](auto& x)->void {x->visible = vis; }, rect);

		return visible();
	}

	bool ProgressBar::visible()const
	{
		return back->visible and std::visit([](auto& x)->bool {return x->visible; }, rect);
	}

	Borrow<Text> Text::setting(StringView text, StringView assetName, const Vec2& pos, const ColorF& fontColor)
	{
		Text::setting(text, 0, pos, fontColor);
		font->drawing = FontAsset(assetName);
		return *this;
	}

	Borrow<Text> Text::setting(StringView text, const int32& fontSize, const Vec2& pos, const ColorF& fontColor)
	{
		font->drawing = Font{ fontSize };
		font->text = text;
		transform->setPos({ pos ,0 });
		font->color = fontColor;
		return *this;
	}

	void Text::setText(StringView text)
	{
		font->text = text;
	}

	void Text::start()
	{
		Object::start();

		auto draw_manager = scene->getDrawManager();

		font = makeUiLike(addComponent<DrawFont>(draw_manager, Font(20)));

		font->color = Palette::White;
	}

	void TextBox::fitSize()
	{
		const auto& tex = font->drawing(font->text);
		RectF rect = tex.region();
		rect.w += x_margin * 2;
		rect.h += y_margin * 2;
		box->drawing = rect;
	}

	Borrow<TextBox> TextBox::setting(StringView text, const int32& fontSize, const Vec2& pos, double w, double h, const ColorF& fontColor, const ColorF& boxColor)
	{
		font->drawing = Font{ fontSize };
		font->text = text;
		transform->setPos({ pos ,0 });
		box->drawing = RectF{ 0,0,w,h };
		font->color = fontColor;
		box->color = boxColor;
		return *this;
	}

	Borrow<TextBox> TextBox::setting(StringView text, const int32& fontSize, const Vec2& pos, const ColorF& fontColor, const ColorF& boxColor)
	{
		TextBox::setting(text, fontSize, pos, 0, 0, fontColor, boxColor);
		fitSize();
		return *this;
	}

	Borrow<TextBox> TextBox::setting(StringView text, StringView assetName, const Vec2& pos, double w, double h, const ColorF& fontColor, const ColorF& boxColor)
	{
		TextBox::setting(text, 0, pos, w, h, fontColor, boxColor);
		font->drawing = FontAsset(assetName);
		return *this;
	}

	Borrow<TextBox> TextBox::setting(StringView text, StringView assetName, const Vec2& pos, const ColorF& fontColor, const ColorF& boxColor)
	{
		TextBox::setting(text, assetName, pos, 0, 0, fontColor, boxColor);
		fitSize();
		return *this;
	}

	void TextBox::setText(StringView text)
	{
		font->text = text;
	}

	void TextBox::start()
	{
		Object::start();

		textObject = scene->birth<Object>();
		textObject->transform->setParent(transform);
		setSameDestiny(*textObject);
		auto draw_manager = scene->getDrawManager();

		box = makeUiLike(addComponent<DrawRectF>(draw_manager, RectF{}));

		font = makeUiLike(addComponent<DrawFont>(draw_manager, Font(20)));

		font->color = Palette::White;

		frame = makeUiLike(addComponent<DrawFrame>(draw_manager, RectF{}, 3));

		frame->color = Palette::Gray;

		frame->relative.z = box->relative.z - 0.5;
	}

	void TextBox::update(double dt)
	{
		Object::update(dt);

		//font->viewport = box->getDrawing();
		const auto& tex = font->drawing(font->text);
		const auto& rect = box->drawing;
		double w = tex.region(0, 0).w;
		double h = tex.region(0, 0).h;

		frame->drawing.figure = box->drawing;

		switch (arrange)
		{
		case ui::Arrangement::tl:
			font->relative = { 0,0,0 };
			break;
		case ui::Arrangement::top:
			font->relative = { (rect.w - w) / 2,0 ,0 };
			break;
		case ui::Arrangement::tr:
			font->relative = { rect.w - w,0 ,0 };
			break;
		case ui::Arrangement::left:
			font->relative = { 0,(rect.h - h) / 2 ,0 };
			break;
		case ui::Arrangement::center:
			font->relative = { (rect.w - w) / 2,(rect.h - h) / 2  ,0 };
			break;
		case ui::Arrangement::right:
			font->relative = { rect.w - w, (rect.h - h) / 2,0 };
			break;
		case ui::Arrangement::bl:
			font->relative = { 0,rect.h - h ,0 };
			break;
		case ui::Arrangement::bottom:
			font->relative = { (rect.w - w) / 2 ,rect.h - h ,0 };
			break;
		case ui::Arrangement::br:
			font->relative = { rect.w - w,rect.h - h ,0 };
			break;
		default:
			break;
		}
		//箱より上に描く
		font->relative += { x_margin, y_margin, -1 };
	}

	Borrow<TextBox> createTextBox(const Borrow<my::Scene>& scene, StringView text, const int32& fontSize, const Vec2& pos, double w, double h, const ColorF& fontColor, const ColorF& boxColor)
	{
		auto textbox = scene->birth<TextBox>();
		textbox->setting(text, fontSize, pos, w, h, fontColor, boxColor);
		return textbox;
	}

	Borrow<TextBox> createTextBox(const Borrow<my::Scene>& scene, StringView text, const int32& fontSize, const Vec2& pos, const ColorF& fontColor, const ColorF& boxColor)
	{
		auto textbox = scene->birth<TextBox>();
		textbox->setting(text, fontSize, pos, fontColor, boxColor);
		return textbox;
	}

	Borrow<TextBox> createTextBox(const Borrow<my::Scene>& scene, StringView text, StringView assetName, const Vec2& pos, double w, double h, const ColorF& fontColor, const ColorF& boxColor)
	{
		auto textbox = scene->birth<TextBox>();
		textbox->setting(text, assetName, pos, w, h, fontColor, boxColor);
		return textbox;
	}

	Borrow<TextBox> createTextBox(const Borrow<my::Scene>& scene, StringView text, StringView assetName, const Vec2& pos, const ColorF& fontColor, const ColorF& boxColor)
	{
		auto textbox = scene->birth<TextBox>();
		textbox->setting(text, assetName, pos, fontColor, boxColor);
		return textbox;
	}

	void InputBox::input(const int32& start, const int32& end, StringView t)
	{
		auto& text = box->font->text;
		auto b = start;
		auto e = end;

		if (b > e) std::swap(b, e);
		b = std::clamp(b, 0, int32(text.size()));
		e = std::clamp(e, 0, int32(text.size()));

		text.insert(e, t);
		text.erase(text.begin() + b, text.begin() + e);
		//cursorPosの更新
		if (e <= cursorPos and b < cursorPos)cursorPos -= e - b;

		cursorPos += t.size() - (e - b);
	}

	String InputBox::getText(int32 b, int32 e)const
	{
		String result = U"";
		auto& text = box->font->text;

		if (b > e) std::swap(b, e);
		b = std::clamp(b, 0, int32(text.size()));
		e = std::clamp(e, 0, int32(text.size()));

		for (int32 i = b; i < e; i++)
		{
			//選択範囲内の文字は返さない
			if (i < editS or editE <= i)result << text[i];
		}

		return result;
	}

	void InputBox::keyInput(StringView text)
	{
		double base_height = box->font->drawing.height();

		Array<String> tmp = box->font->text.split(U'\n');
		if (tmp.isEmpty())tmp << U"";
		//barPos barPointの更新
		for (int32 i = 0; i < text.size(); ++i)
		{
			const auto& s = text[i];
			if (s == U'\n') {
				tmp[barPoint.x].insert(barPoint.y, U"\n");
				auto tmp2 = tmp[barPoint.x].split(U'\n');
				if (tmp2.isEmpty()) tmp2 = { U"",U"" };
				if (tmp2.size() == 1)tmp2 << U"";
				tmp[barPoint.x] = tmp2[1];
				tmp.insert(tmp.begin() + barPoint.x, tmp2[0]);
				barPos.y += base_height;
				barPos.x = box->font->transform->getPos().x;
				barPoint.y = 0;
				++barPoint.x;
			}
			else if (s == U'\b' and not (barPoint.y == 0 and barPoint.x == 0))
			{
				//端でバックスペースを押したか
				if (barPoint.y == 0)
				{
					barPos.y -= base_height;
					barPos.x = box->font->drawing(tmp[barPoint.x - 1]).getXAdvances().sum();

					tmp[barPoint.x - 1] += tmp[barPoint.x];
					tmp.erase(tmp.begin() + barPoint.x);

					--barPoint.x;
					barPoint.y = tmp[barPoint.x].size() - 1;
				}
				else {
					auto pop = tmp[barPoint.x][barPoint.y - 1];//消される文字を保持

					tmp[barPoint.x].erase(tmp[barPoint.x].begin() + barPoint.y - 1);//消す

					barPos.x -= box->font->drawing(pop).getXAdvances()[0];//消した分減らす

					--barPoint.y;
				}
			}
			else {
				tmp[barPoint.x].insert(barPoint.y, Format(s));
				barPos.x += box->font->drawing(s).getXAdvances()[0];
				++barPoint.y;
			}
		}
		//tmp[barPoint.x].insert(barPoint.y, text);

		//textの更新
		box->font->text = U"";
		for (const auto& s : tmp)
		{
			box->font->text.append(s + U'\n');
		}
		box->font->text.pop_back();
	}

	int32 InputBox::getCharaNum(Point pos)const
	{
		const auto& splitedText = box->font->text.split(U'\n');
		int32 result = 0;
		for (int32 i = 0; i < pos.y - 1; ++i)
		{
			//+1は改行の分
			result += splitedText[i].size() + 1;
		}
		result += pos.x;
		return result;
	}

	void InputBox::setBarPosition()
	{
		const auto& pos = Cursor::Pos();
		const auto& region = box->font->drawing(box->font->text).region();
		double base_height = box->font->drawing.height();
		//カーソルに最も近い行を求める
		int32 row =
			Max(
				1,
				Min(
					int32(box->font->text.split(U'\n').size()),
					int32((pos.y - box->font->transform->getPos().y) / base_height) + 1
				)
			);
		//一番左のｘ距離
		double tmp = box->font->transform->getPos().x;
		double distance = (pos.x - tmp) * (pos.x - tmp);
		//barPosを行の一番左側にセット
		barPos = box->font->transform->getPos().xy() + Vec2{ 0,base_height * (row - 1) };
		barPoint = Point{ row - 1,0 };//行,列
		if (box->font->text.isEmpty())return;//空だったらここで終了
		for (auto p : box->font->drawing(box->font->text.split(U'\n')[row - 1]).getXAdvances())
		{
			tmp += p;
			double next_distance = (tmp - pos.x) * (tmp - pos.x);
			//x距離が遠くなったら
			if (distance < next_distance)
			{
				return;
			}
			//x距離の更新とbarPos,barPointの更新
			distance = next_distance;
			barPos.x += p;
			++barPoint.y;
		}

		cursorPos = getCharaNum(barPoint);
	}

	void InputBox::start()
	{
		Object::start();

		box = scene->birth<TextBox>();

		setSameDestiny(box);
		//左揃え
		box->arrange = Arrangement::tl;

		transform->setParent(box->transform);
		//w:h=1:10の長方形
		auto bar = makeUiLike(addComponent<DrawRectF>(scene->getDrawManager(), RectF{ 0,0,1,10 }));

		bar->visible = false;

		bar->color = Palette::Black;

		//キー入力 入力したら発動
		ACreate(U"input", false, true)
			.startIf([=] {return isActive(); })
			.endIf([=] {return not isActive(); });
		actman[U"input"].add(
			[=] {
				Print << cursorPos;
				const auto& pre = getText(cursorPos - 1, cursorPos + 1);
				String t = pre;
				TextInput::UpdateText(t, t.size() > 0 ? 1 : 0);
				if (t == pre)return;

				if (editS != editE)
				{
					input(editS, editE, U"");
					//eraseかdeleteを押してなかったら
					if (t.size() > pre.size()) {
						input(cursorPos - 1, cursorPos + 1, t);
					};
					editS = editE;
				}
				else {
					input(cursorPos - 1, cursorPos + 1, t);
				}
			}
		);
		//棒の点滅アクション  入力できる設定&テキストボックスの幅が0より大きい&入力待ち状態　なら開始
		ACreate(U"barFlashing", false, true)
			.startIf([=] {return isActive(); })
			.endIf([=] {return not isActive(); });
		//棒を0.5秒表示
		actman[U"barFlashing"].add(
			[=] {
				//棒を表示
				bar->visible = true;
				transform->setPos({ barPos,box->transform->getPos().z - 1 });
			}, [=](double dt) {
				double base_height = box->font->drawing.height();
				bar->aspect.setScale(base_height / bar->drawing.h * 0.95);
				transform->setPos({ barPos,box->transform->getPos().z - 1 });
				}, [=] {
					//棒を非表示
					bar->visible = false;
					}, 0.5
					);
		actman[U"barFlashing"].add<prg::Wait>(0.4);
	}

	bool InputBox::isActive()const
	{
		return canInput and box->box->drawing.w > 0 and active;
	}

	void InputBox::update(double dt)
	{
		Object::update(dt);

		//箱をクリックしたら入力待ち状態
		auto fig = box->box->getDrawing();
		if (fig.leftClicked())
		{
			active = true;

			setBarPosition();
		}
	}

	Borrow<InputBox> createInputBox(const Borrow<my::Scene>& scene, const int32& fontSize, const Vec2& pos, double w, double h, StringView text, const ColorF& fontColor, const ColorF& boxColor)
	{
		auto inputbox = scene->birth<InputBox>();
		inputbox->box->setting(text, fontSize, pos, w, h, fontColor, boxColor);
		return inputbox;
	}

	Borrow<InputBox> createInputBox(const Borrow<my::Scene>& scene, StringView assetName, const Vec2& pos, double w, double h, StringView text, const ColorF& fontColor, const ColorF& boxColor)
	{
		auto inputbox = scene->birth<InputBox>();
		inputbox->box->setting(text, assetName, pos, w, h, fontColor, boxColor);
		return inputbox;
	}

	SimpleInputArea::SimpleInputArea(DrawManager* manager, double w, double h, StringView text)
		:w(w), h(h), IDraw2D(manager)
	{
		tex.text = text;
	}

	bool SimpleInputArea::isActive()
	{
		return tex.active;
	}

	String SimpleInputArea::getText()const
	{
		return tex.text;
	}

	void SimpleInputArea::draw()const
	{
		const Transformer2D t1(getTransformer());

		SimpleGUI::TextArea(tex, { 0,0 }, { w,h }, 50000, canInput);
	}

	Borrow<SimpleInputArea> createSimpleInputArea(const Borrow<my::Scene>& scene, const Vec2& pos, double w, double h, StringView text)
	{
		auto area = scene->birthObject(RectF{ 0,0,w,h }, { 0,0,0 });

		area->transform->setPos({ pos,0 });

		return makeUiLike(area->addComponent<SimpleInputArea>(scene->getDrawManager(), w, h, text));
	}

	SimpleInputBox::SimpleInputBox(DrawManager* manager, double w, StringView text)
		:w(w), IDraw2D(manager)
	{
		tex.text = text;
	}

	String SimpleInputBox::getText()const
	{
		return tex.text;
	}

	bool SimpleInputBox::isEditing()const
	{
		return tex.active;
	}

	void SimpleInputBox::draw()const
	{
		const Transformer2D t1(IDraw2D::getTransformer());

		SimpleGUI::TextBox(tex, { 0,0 }, w, none, canInput);
	}

	Borrow<SimpleInputBox> createSimpleInputBox(const Borrow<my::Scene>& scene, const Vec2& pos, double w, StringView text)
	{
		auto area = scene->birthObject(RectF{ 0,0,w,40 }, { 0,0,0 });

		area->transform->setPos({ pos,0 });

		return makeUiLike(area->addComponent<SimpleInputBox>(scene->getDrawManager(), w, text));
	}

	void Button::setText(StringView text)
	{
		box->setText(text);
	}

	void Button::start()
	{
		Object::start();

		box = scene->birthObject<TextBox>(RectF{}, { 0,0,0 });

		setSameDestiny(box);

		box->name = U"Button";

		//box->box->visible = false;

		box->transform->setParent(transform);

		box->arrange = Arrangement::center;

		collider = box->getComponent<Collider>();
	}

	void Button::update(double dt)
	{
		Object::update(dt);
		flag = false;

		if (not active)return;

		auto fig = box->box->getDrawing();

		std::get<2>(collider->hitbox.shape).setWH(fig.w, fig.h);

		if (fig.leftClicked()) {
			flag = true;
		}
	}

	bool Button::pushed()
	{
		return flag;
	}

	Borrow<Button> createButton(const Borrow<my::Scene>& scene, StringView text, const int32& fontSize, const Vec2& pos, double w, double h, const ColorF& fontColor, const ColorF& boxColor)
	{
		auto button = scene->birth<Button>();
		button->transform->setPos({ pos, 0 });
		button->box->setting(text, fontSize, pos, w, h, fontColor, boxColor);
		return button;
	}

	Borrow<Button> createButton(const Borrow<my::Scene>& scene, StringView text, const int32& fontSize, const Vec2& pos, const ColorF& fontColor, const ColorF& boxColor)
	{
		auto button = scene->birth<Button>();
		button->transform->setPos({ pos, 0 });
		button->box->setting(text, fontSize, pos, fontColor, boxColor);
		return button;
	}

	Borrow<Button> createButton(const Borrow<my::Scene>& scene, StringView text, StringView assetName, const Vec2& pos, double w, double h, const ColorF& fontColor, const ColorF& boxColor)
	{
		auto button = scene->birth<Button>();
		button->transform->setPos({ pos, 0 });
		button->box->setting(text, assetName, pos, w, h, fontColor, boxColor);
		return button;
	}

	Borrow<Button> createButton(const Borrow<my::Scene>& scene, StringView text, StringView assetName, const Vec2& pos, const ColorF& fontColor, const ColorF& boxColor)
	{
		auto button = scene->birth<Button>();
		button->transform->setPos({ pos, 0 });
		button->box->setting(text, assetName, pos, fontColor, boxColor);
		return button;
	}

	void Selection::start()
	{
		Object::start();
		//初期化など
		ACreate(U"startDirection")
			.add([self = this->lend()] {
			self->determined = false;
			auto tmp = self->selecting;
			if (not self->selecting)self->selecting = 0;
			});

		ACreate(U"endDirection");
	}

	Actions& Selection::startDirection()
	{
		return actman.get(U"startDirection");
	}

	Actions& Selection::endDirection()
	{
		return actman.get(U"endDirection");
	}

	void Selection::addSelection(prg::IAction*&& eventHandler, prg::IAction*&& forcus, prg::IAction*&& release, prg::IAction*&& normal)
	{
		using namespace state;

		auto self = this->lend();

		SCreatorContainer dict;

		dict[U"State"] = [&](In info, A act)
			{
				act |= dict[U"Normal"](info);//選択していない
				act |= dict[U"Release"](info);//選択を外す
				act |= dict[U"Forcus"](info);//選択中
				act |= dict[U"Event"](info);//決定

				act.relate(U"Forcus", U"Event")
					.andIf([self] {return self->determined; });
				act.relate(U"Normal", U"Forcus")
					.andIf([self, n = num] {return self->selecting and (*self->selecting == n); });
				act.relate(U"Forcus", U"Release")
					.andIf([self, n = num] { return self->selecting and (*self->selecting != n); });

				return F(act);
			};

		dict[U"Normal"] = [=](In info, A act)
			{
				if (normal)act.addAct(normal);
				else act.add([](double) {});

				act.endIf([ac = act.lend()] {return ac->isAllFinished(); });

				return F(act);
			};

		dict[U"Forcus"] = [=](In info, A act)
			{
				act.addAct(forcus);

				act.endIf([ac = act.lend()] {return ac->isAllFinished(); });

				return F(act);
			};

		dict[U"Event"] = [=](In info, A act)
			{
				act.addAct(eventHandler);

				act.endIf([ac = act.lend()] {return ac->isAllFinished(); });

				return F(act);
			};

		dict[U"Release"] = [=](In info, A act)
			{
				act.addAct(release);

				act.endIf([ac = act.lend()] {return ac->isAllFinished(); });

				return F(act);
			};

		ACreate(U"Selection{}"_fmt(num), true) += dict[U"State"]();

		num++;
	}
}
