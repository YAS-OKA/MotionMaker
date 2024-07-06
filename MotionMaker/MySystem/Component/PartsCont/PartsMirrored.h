#pragma once
#include "../../EC.hpp"

namespace mot
{
	class PartsManager;
}
class Transform;

class PartsMirrored:public Component
{
private:
	double scale = 1;
	int32 firstScale;
	bool active;
	Borrow<Transform> transform;//これの方向をもとにする
	bool mirrored;

	double mirroredTime = 0.2;
	double timer = 0;
public:
	Borrow<mot::PartsManager> pman;
	//パーツマネージャー　最初から反転するか
	PartsMirrored(const Borrow<mot::PartsManager>& pman,bool mirrorStart = false);

	bool getMirrored()const;

	void start()override;

	void update(double dt)override;
};
