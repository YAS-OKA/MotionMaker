#pragma once
#include"Object.h"
#include"../EC.hpp"
#include"../Component/Draw.h"
#include"Utilities.h"
#include"../Prg/Init.h"
#include"StateMachine.h"

namespace my {
	class Scene;
}

namespace ui
{
	class ProgressBar :public Object
	{
		double m_rate = 0;
		double m_w = 0;
		std::variant<Borrow<DrawRectF>, Borrow<Draw2D<RoundRect>>> rect;
		Borrow<IDraw2D> back;
	public:
		Array<std::pair<double, ColorF>> m_barColors = {
			{ 1.0, ColorF(0.1, 0.8, 0.2) }
		};

		ProgressBar() = default;

		void setting(const Vec3& pos, double w, double h, double round = 0.0);

		void setting(const Vec3& pos, double w, double h, const ColorF& backgroundColor, const ColorF& barColor, double round = 0.0);

		void setting(const Vec3& pos, double w, double h, const ColorF& backgroundColor, const Array<std::pair<double, ColorF>>& barColors, double round = 0.0);

		double rate(double r);

		double rate()const;

		bool visible(bool visible);

		bool visible()const;
	};

	template <class Draw>
	Borrow<Draw> makeUiLike(const Borrow<Draw> draw_class, bool drawSurface = true)
	{
		draw_class->dManagerInfluence->value.SetInfluence(0, 0, 0);
		if (drawSurface)draw_class->shallow->layer = 10;
		return draw_class;
	}
	//左上から右下まで
	enum class Arrangement
	{
		tl,
		top,
		tr,
		left,
		center,
		right,
		bl,
		bottom,
		br,
	};

	class Text :public Object
	{
	public:
		Borrow<DrawFont> font;

		Borrow<Text> setting(StringView text, StringView assetName, const Vec2& pos, const ColorF& fontColor = Palette::Black);
		Borrow<Text> setting(StringView text, const int32& fontSize, const Vec2& pos, const ColorF& fontColor = Palette::Black);

		void setText(StringView text);

		void start() override;
	};

	class TextBox :public Object
	{
	public:
		using DrawFrame = Draw2D<Frame<RectF>>;

		Borrow<Object> textObject;
		Borrow<DrawRectF> box;
		Borrow<DrawFont> font;
		Borrow<DrawFrame> frame;
		double x_margin = 2;
		double y_margin = 2;

		Borrow<TextBox> setting(StringView text, const int32& fontSize, const Vec2& pos, double w, double h, const ColorF& fontColor = Palette::Black, const ColorF& boxColor = Palette::White);

		Borrow<TextBox> setting(StringView text, const int32& fontSize, const Vec2& pos, const ColorF& fontColor = Palette::Black, const ColorF& boxColor = Palette::White);

		Borrow<TextBox> setting(StringView text, StringView assetName, const Vec2& pos, double w, double h, const ColorF& fontColor = Palette::Black, const ColorF& boxColor = Palette::White);

		Borrow<TextBox> setting(StringView text, StringView assetName, const Vec2& pos, const ColorF& fontColor = Palette::Black, const ColorF& boxColor = Palette::White);

		Arrangement arrange = Arrangement::center;

		void setText(StringView text);

		void fitSize();

		void start()override;

		void update(double dt)override;
	};

	Borrow<TextBox> createTextBox(const Borrow<my::Scene>& scene, StringView text, const int32& fontSize, const Vec2& pos, double w, double h, const ColorF& fontColor = Palette::Black, const ColorF& boxColor = Palette::White);

	Borrow<TextBox> createTextBox(const Borrow<my::Scene>& scene, StringView text, const int32& fontSize, const Vec2& pos, const ColorF& fontColor = Palette::Black, const ColorF& boxColor = Palette::White);

	Borrow<TextBox> createTextBox(const Borrow<my::Scene>& scene, StringView text, StringView assetName, const Vec2& pos, double w, double h, const ColorF& fontColor = Palette::Black, const ColorF& boxColor = Palette::White);

	Borrow<TextBox> createTextBox(const Borrow<my::Scene>& scene, StringView text, StringView assetName, const Vec2& pos, const ColorF& fontColor = Palette::Black, const ColorF& boxColor = Palette::White);

	class InputBox :public Object
	{
	private:
		bool active = false;
		//棒の場所
		Vec2 barPos;
		//棒が指す行と列の場所
		Point barPoint;
		//カーソルの場所
		int32 cursorPos = 0;
		//範囲入力 通常はs=e=cursorPos 必ずs<=e
		int32 editS;
		int32 editE;
		//begin~endまでの文字をtextに置き換える
		void input(const int32& b, const int32& e, StringView text);
		//begin~endまでの文字を取得
		String getText(int32 b, int32 e)const;

		int32 getCharaNum(Point pos)const;

		void keyInput(StringView text);

		void setBarPosition();
	public:
		bool canInput = true;

		Borrow<TextBox> box;

		bool isActive()const;

		void start()override;

		void update(double dt)override;
	};

	Borrow<InputBox> createInputBox(const Borrow<my::Scene>& scene, const int32& fontSize, const Vec2& pos, double w, double h, StringView text = U"", const ColorF& fontColor = Palette::Black, const ColorF& boxColor = Palette::White);

	Borrow<InputBox> createInputBox(const Borrow<my::Scene>& scene, StringView assetName, const Vec2& pos, double w, double h, StringView text = U"", const ColorF& fontColor = Palette::Black, const ColorF& boxColor = Palette::White);

	class SimpleInputArea :public IDraw2D
	{
	public:
		bool canInput = true;
		double h, w;

		mutable TextAreaEditState tex;

		SimpleInputArea(DrawManager* manager, double w, double h, StringView text = U"");

		bool isActive();

		String getText()const;

		void draw()const override;
	};

	Borrow<SimpleInputArea> createSimpleInputArea(const Borrow<my::Scene>& scene, const Vec2& pos, double w, double h, StringView text = U"");

	class SimpleInputBox :public IDraw2D
	{
	public:
		bool canInput = true;

		double w;

		mutable TextEditState tex;

		Borrow<util::MouseObject> mouse;

		SimpleInputBox(DrawManager* manager, double w, StringView text = U"");

		bool isEditing()const;

		String getText()const;

		void draw()const override;
	};

	Borrow<SimpleInputBox> createSimpleInputBox(const Borrow<my::Scene>& scene, const Vec2& pos, double w, StringView text = U"");

	class Button :public Object
	{
	private:
		bool flag;
	public:
		Borrow<Collider> collider;

		bool active = true;

		Borrow<TextBox> box;

		void setText(StringView text);

		void start()override;

		void update(double dt)override;

		bool pushed();
	};

	Borrow<Button> createButton(const Borrow<my::Scene>& scene, StringView text, const int32& fontSize, const Vec2& pos, double w, double h, const ColorF& fontColor = Palette::Black, const ColorF& boxColor = Palette::White);

	Borrow<Button> createButton(const Borrow<my::Scene>& scene, StringView text, const int32& fontSize, const Vec2& pos, const ColorF& fontColor = Palette::Black, const ColorF& boxColor = Palette::White);

	Borrow<Button> createButton(const Borrow<my::Scene>& scene, StringView text, StringView assetName, const Vec2& pos, double w, double h, const ColorF& fontColor = Palette::Black, const ColorF& boxColor = Palette::White);

	Borrow<Button> createButton(const Borrow<my::Scene>& scene, StringView text, StringView assetName, const Vec2& pos, const ColorF& fontColor = Palette::Black, const ColorF& boxColor = Palette::White);

	//using Event = std::function<void()>;

	//template<class... Args>	Borrow<ui::Button> ButtonEvent(Array<Event> events, const Borrow<my::Scene>& scene, StringView text, Args&& ...args)
	//{
	//	auto button = createButton(scene, text, args...);
	//	auto& act = button->box->ACreate(U"onClicked");
	//	for (const auto& e : events)
	//	{
	//		act.add(e).startIf<ButtonChecker>(button);
	//	}
	//	return button;
	//}

	//template<class... Args>	Borrow<ui::Button> ButtonEvent(const Event& e, const Borrow<my::Scene>& scene, StringView text, Args&& ...args)
	//{
	//	return ButtonEvent(Array<Event>{ e }, scene, text, args...);
	//}

	class Selection :public Object
	{
		size_t num = 0;
	public:
		//決定ならこれをtrueに
		bool determined = false;
		//選択中の選択肢
		Optional<size_t> selecting = none;

		void start()override;

		Actions& startDirection();

		Actions& endDirection();

		void addSelection(prg::IAction*&& eventHandler, prg::IAction*&& forcus, prg::IAction*&& release, prg::IAction*&& normal = nullptr);

		void select(size_t i);
	};
}
