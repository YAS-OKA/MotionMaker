#pragma once
#include"../MySystem/GameIncludes.h"



namespace mot
{
	class PartsEditor :public my::Scene
	{
	public:
		void start()override;
		class Impl;
		Impl* impl;
	};

	class StateViewScene :public my::Scene
	{
	public:
		void start()override;
	};
}
