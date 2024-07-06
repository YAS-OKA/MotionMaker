#include "../stdafx.h"
#include "GameAction.h"
#include"../Game/Object.h"

namespace prg
{
	void EndAction::endAll()
	{
		endOtherIf([](IAction*) {return true; });
	}
	void EndAction::endOtherIf(const std::function<bool(IAction*)>& f)
	{
		this->f = f;
	}
	void EndAction::addException(StringView id)
	{
		exception << actions->getAction<IAction>(id)->lend();
	}
	void EndAction::addException(const Borrow<IAction>& act)
	{
		exception << act;
	}
	void EndAction::addTarget(StringView id)
	{
		targets << actions->getAction<IAction>(id)->lend();
	}

	void EndAction::start()
	{
		if (f)actions->getAll().each([&](IAction* act) {if (f(act))targets << act->lend(); });
	}

	void EndAction::update(double dt)
	{
		//null または 例外のアクションは除外
		targets.remove_if([&](const Borrow<IAction>& target) {return (not target) or exception.contains(target); });
		//終了する
		targets.each([&](Borrow<IAction>& target) {if (actions)actions->end(target); });
	}

	void ActCluster::update(double dt)
	{
		IAction::update(dt);
		dt *= timeScale;
		//start
		for (auto it = acts.begin(), end = acts.end(); it != end; ++it)
		{
			auto& act = (*it);
			if (not act->started)
			{
				if (act->startCondition.commonCheck())act->start();

				act->startCondition.countFrame();//カウント
			}
		}
		//update
		for (auto it = acts.begin(), en = acts.end(); it != en; ++it)
		{
			if ((*it)->isActive())(*it)->update(dt);
		}
		//end
		for (auto it = acts.begin(), end = acts.end(); it != end; ++it)
		{
			auto& act = (*it);
			if (act->isActive()) {
				if (act->endCondition.commonCheck())
				{
					act->end();
					act->reset();
				}
				else {
					act->endCondition.countFrame();//カウント
				}
			}
		}
	}

	void ActCluster::end()
	{
		IAction::end();
		for (auto it = acts.begin(), end = acts.end(); it != end; ++it)
		{
			auto& act = (*it);
			act->end();
			act->reset();
		}
	}


	MoveAct::MoveAct(const Borrow<Transform>& transform, const Vec3& initVel, const Optional<double>& t)
		:transform(transform), initVel(initVel), IAction(t)
	{
	}

	MoveAct::MoveAct(const Vec3& initVel, const Optional<double>& t)
		:initVel(initVel),IAction(t)
	{
	}

	void MoveAct::setTransform(const Borrow<Transform>& t)
	{
		transform = t;
	}

	Vec3 MoveAct::getVel() const
	{
		return vel;
	}

	void MoveAct::start()
	{
		IAction::start();
		vel = initVel;
	}

	void MoveAct::update(double dt)
	{
		IAction::update(dt);
		vel += acc * dt;
		if(transform)*transform += vel * dt;
	}

	MulMove::MulMove(const Borrow<Transform>& transform, const Optional<double>& constantSpeed, const Optional<double>& t)
		:IAction(t), transform(transform), constantSpeed(constantSpeed)
	{
	}

	void MulMove::setLimit(double limitingSpeed)
	{
		this->limitingSpeed = limitingSpeed;
	}

	void MulMove::addMove(MoveAct&& move)
	{
		moves << util::uPtr<MoveAct>(new MoveAct(std::forward<MoveAct>(move)));
	}

	void MulMove::update(double dt)
	{
		IAction::update(dt);
		dt *= timeScale;
		//start
		for (auto it = moves.begin(), end = moves.end(); it != end; ++it)
		{
			auto& act = (*it);
			if (not act->started)
			{
				if (act->startCondition.commonCheck())act->start();

				act->startCondition.countFrame();//カウント
			}
		}
		//update
		Vec3 vel = { 0,0,0 };
		for (auto it = moves.begin(), en = moves.end(); it != en; ++it)
		{
			if ((*it)->isActive())
			{
				(*it)->timer += dt;
				(*it)->vel += (*it)->acc * dt;
				vel += (*it)->vel;
			}
		}

		if (constantSpeed)vel.setLength(*constantSpeed);
		if (limitingSpeed)vel.setLength(Min(vel.length(), *limitingSpeed));
		this->vel = vel;
		*transform += vel * dt;

		//end
		for (auto it = moves.begin(), end = moves.end(); it != end; ++it)
		{
			auto& act = (*it);
			if (act->isActive()) {
				if (act->endCondition.commonCheck())
				{
					act->end();
					act->reset();
				}
				else {
					act->endCondition.countFrame();//カウント
				}
			}
		}
	}

	void MulMove::end()
	{
		IAction::end();
		for (auto it = moves.begin(), end = moves.end(); it != end; ++it)
		{
			auto& act = (*it);
			act->end();
			act->reset();
		}
	}

	FreeFall::FreeFall(const Borrow<Transform>& transform, const Vec3& initVel, double acc, const Optional<double>& t)
		:MoveAct(transform, initVel, t)
	{
		setAcc(acc);
	}

	void FreeFall::setAcc(double a)
	{
		acc = Vec3{ 0,a,0 };
	}

	LifeSpan::LifeSpan(const Borrow<Object>& target, const Optional<double>& time)
		:target(target), IAction(time)
	{}

	void LifeSpan::end()
	{
		if (target)target->die();
		target = nullptr;
	}

	void Hitbox::reset()
	{
		IAction::reset();
		collidedEntitys.clear();
	}

	void Hitbox::start()
	{
		IAction::start();
		collider->collidable = true;
		hitbox->getComponent<Draw3D>(U"hitbox")->visible = true;
	}

	void Hitbox::update(double dt)
	{
		IAction::update(dt);

		collidedEntitys.clear();

		for (const auto& tc : targetCategory)
		{
			for (const auto& entity : collider->intersects(tc))
			{
				collidedEntitys[tc].emplace(entity);
			}
		}
	}

	void Hitbox::end()
	{
		IAction::end();
		collidedEntitys.clear();
		collider->collidable = false;
		hitbox->getComponent<Draw3D>(U"hitbox")->visible = false;
	}
}
