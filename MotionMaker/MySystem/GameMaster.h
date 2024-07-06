#pragma once
#include"EC.hpp"

namespace my {
	class Scene;
}

class GameMaster final :public Entity
{
public:
	void start()override;
	void update(double dt)override;

	void addScene(my::Scene* scene);

	EntityManager& getManager();
private:
	struct Impl;
	std::shared_ptr<Impl> p_impl;
};
