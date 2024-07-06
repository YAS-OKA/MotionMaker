#pragma once
#include"Util/Borrow.h"

namespace util {
	class Convert2DTransform;
}

struct PSFog
{
	Float3 fogColor;
	float fogCoefficient;
};

class IDraw3D;
class IDraw2D;
class IDrawing;
class Camera;

class DrawManager final
{
private:
	Array<Borrow<IDraw3D>> m_drawings3D;
	Array<Borrow<IDraw2D>> m_drawings2D;
	std::function<bool(const Borrow<IDrawing>&)> canDraw;
	Borrow<Camera> m_camera;
	PixelShader ps;
	double fogParam = 0.6;
	ConstantBuffer<PSFog> cb;
public:
	MSRenderTexture renderTexture;
	//2D
	ColorF backGroundColor;
	Vec2 scale{ 1,1 };
	Vec2 scalePos{ 0,0 };
	Vec2 translate{ 0,0 };
	double angle = 0;

	std::function<void()>debugDraw=nullptr;

	DrawManager(const ColorF& backGround = { 0,0,0,1 });
	DrawManager(const Borrow<Camera>& camera, const ColorF& backGround = { 0,0,0,1 });
	DrawManager(const Borrow<Camera>& camera, const MSRenderTexture& renderTexture, const ColorF& backGround = { 0,0,0,1 });

	~DrawManager();
	//カメラと描画条件をセットする
	void setting(const Borrow<Camera>& camera, std::function<bool(const Borrow<IDrawing>&)> f = [](const Borrow<IDrawing>&) {return true; });
	void set3D(const Borrow<IDraw3D>& drawing);
	void set2D(const Borrow<IDraw2D>& drawing);
	Borrow<IDraw2D> get2D(std::function<bool(const Borrow<IDraw2D>&)> filter = [](const Borrow<IDraw2D>&) {return true; });
	Array<Borrow<IDraw2D>> get2Ds(std::function<bool(const Borrow<IDraw2D>&)> filter = [](const Borrow<IDraw2D>&) {return true; });
	void drop(const Borrow<IDrawing>& drawing);
	void remove3D(const Borrow<IDraw3D>& drawing);
	void remove2D(const Borrow<IDraw2D>& drawing);
	MSRenderTexture getRenderTexture()const;
	Borrow<Camera> getCamera()const;
	virtual void update();
	virtual void draw(bool draw3D=true)const;
	void createPicture(FilePath path,Vec2 pos, Size size)const;
};
