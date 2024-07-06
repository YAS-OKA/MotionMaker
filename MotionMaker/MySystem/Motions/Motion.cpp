#include "../stdafx.h"
#include "Motion.h"
#include"../Util/Util.h"

namespace
{
	HashSet<String> simpleMotion{U"SetAngle", U"SetZ",U"SetPos"};
	constexpr int32 SetRotateCenterMotionsUpdatePriority = 100;
	constexpr int32 SetVariablesMotionsUpdatePriority = 200;
}

prg::Actions* mot::MotionScript::CreateMotions(const Borrow<PartsManager>& pMan, const String& motionName, const String& text)
{
	prg::Actions* actions = new prg::Actions();
	DecoderSet(&deco).motionScriptCmd(pMan, actions->lend());
	String target = U"";
	String len = U"0";
	String addParallel = U"false";
	auto lines = text.split_lines();
	for (size_t i = 0; i < lines.size(); i++)
	{
		auto cLine = lines[i];
		//前処理 
		String line = U"";
		const auto& e = cLine.indexOf(U"//");
		if (e == 0)continue;											//先頭からコメントが始まってたら次の行へ
		for (int32 i = 0; i < e and i < cLine.size(); ++i)line << cLine[i];//文中のコメントの削除
		auto token = line.split(U' ');									//空白で文字列を分割 トークン化
		token.remove_if([](const String& str) {return str == U""; });	//なんも書いてないトークンを削除
		if (token.isEmpty()) {											//トークンがなければaddParallelをfalseにして
			addParallel = U"false";
			continue;
		}
		
		//主処理
		if (token[0] == U"JOINT")
		{
			target = token[1];
			continue;
		}
		else if (token[0] == U"LEN")
		{
			len = token[1];
			continue;
		}
		else if (token[0] == U"Add")//他のモーションをこのモーションに挿入する
		{
			if (auto _src = GetMotionScrOf(*AllText, token[1])) {
				auto _motionSrc = MotionScript();
				if (addParallel == U"true")
				{
					actions->addActParallel(_motionSrc.CreateMotions(pMan, token[1], *_src));
				}
				else {
					actions->addAct(_motionSrc.CreateMotions(pMan, token[1], *_src));
				}
				addParallel = U"true";
			}
			continue;
		}
		else if (token[0] == U"CONST")
		{
			//定数を登録
			constant.emplace(token[1], token[2]);
			continue;
		}

		if (token[0] == U"Start" or token[0] == U"SetRC" or token[0] == U"SetParent")
		{
			//LENは必要ない　ちょっと特殊な命令かも
		}
		else if (token[0] == U"Param")
		{
			//のちのち演算を行えるようにする(例えば2+angle)には2項めを変える
			pMan->addComponentNamed<Field<String>>(PartsParamsVariable::NameProc(motionName, token[1]), GetParamDefault(token[2]));
			//Lenは必要ない
			token.insert(token.begin() + 1, motionName);
		}
		else if (token[0] == U"Wait")
		{
			//lenは第2引数に入れる
			token.insert(token.begin() + 1, len);
		}
		else if (token[0] == U"MyPrint")
		{
			token.insert(token.begin() + 2, len);
		}
		else if (simpleMotion.contains(token[0]))
		{
			//lenは第3引数に入れる
			token.insert(token.begin() + 2, len);
		}
		else if (token[0] == U"Pause")
		{
			//lenは第7引数に入れる
			token.insert(token.begin() + 6, len);
		}
		else if (token[0] == U"PauseVec")
		{
			token.insert(token.begin() + 4, len);
		}
		else
		{
			//lenは第４引数に入れる
			token.insert(token.begin() + 3, len);
		}

		//3要素をセット
		token.insert(token.begin() + 1, { target,addParallel });
		//Paramを使用したら　それが何番目の引数で使用されたのかを保存
		Array<std::pair<String,int32>> usedParams{};
		int32 argNum=-3;
		for (auto& t : token)//変数を置き換え
		{
			if (t[0] == U'@')
			{
				const auto& varName = util::slice(t, 1, t.size());
				//定数が使われたら
				if (constant.contains(varName))
				{
					//定数で置き換え
					t = constant[varName];
				}
				//Paramが使われたら
				else if (auto comp = pMan->getComponent<Field<String>>(PartsParamsVariable::NameProc(motionName, varName)))
				{
					t = comp->value;
					usedParams << std::pair<String, size_t>(varName, argNum);
				}
			}
			argNum++;
		}

		deco.input(token)->decode()->execute();
		//変数セットアクション
		Borrow<SetVariables> sv;
		//Paramを使用した場合
		for (const auto& [name,idx] : usedParams)
		{
			if (not sv)
			{
				sv = actions->addParallel<SetVariables>(motionName);
				sv->pman = pMan;
			}
			auto backActions = actions->getAction(actions->getSepSize() - 2);
			sv->addVarSettingFunc(*backActions[backActions.size() - 2], name, idx);//ここで-2にするのはsetVariableが一番後ろだから
		}

		addParallel = U"true";
	}

	return actions;
}

void mot::MotionScript::Load(const Borrow<PartsManager>& pMan, const String& motionName, const String& text)
{
	pMan->actman.create(motionName) += std::move(*CreateMotions(pMan, motionName, text));
}

bool mot::MotionScript::LoadFile(const Borrow<PartsManager>& pMan, const String& path, const String& motionName)
{
	//モーションスクリプトを取得
	if (auto reader = TextReader{ path })
	{
		//全文を保管
		AllText = reader.readAll();
		if (auto inputText = GetMotionScrOf(*AllText, motionName))
		{
			Load(pMan, motionName, *inputText);
			AllText = none;
			return true;
		}
	}
	//取得できなかった場合
	return false;
}

Optional<String> mot::MotionScript::GetMotionScrOf(const String& allText, StringView motionName, bool removeBackBlank)
{
	Optional<String> result = none;

	result = U"";
	bool find = false;
	for (const auto& line : allText.split_lines())
	{
		if (find)
		{
			if (line[0] == U'#')break;
			*result += line;
			*result += U"\n";
		}

		if (line[0] == U'#')
		{
			if (motionName == util::slice(line, 1, line.size()))
			{
				find = true;
			}
			else
			{
				find = false;
			}
		}
	}
	//文末に改行があれば削除
	if (removeBackBlank)
	{
		bool comple= false;
		while(not comple)
		{
			if (result->back() != U'\n') {
				comple = true;
			} else {
				result->pop_back();
			}
		}
	}

	return result;
}

Array<String> mot::MotionScript::GetMotionNames(FilePathView scriptPath)
{
	Array<String> res;

	for (const auto& line : TextReader{ scriptPath }.readAll().split_lines())
	{
		if (line[0] == U'#')
		{
			res << util::slice(line, 1, line.size());//モーション名を取得
		}
	}

	return res;
}

mot::PartsMotion::PartsMotion(const Borrow<Parts>& target, double time)
	:TimeAction(time), target(target)
{
}

mot::Rotate::Rotate(const Borrow<Parts>& target, double angle, double time)
	:PartsMotion(target, time), ang(angle)
{
}

void mot::Rotate::update(double dt)
{
	PartsMotion::update(dt);

	target->setAngle(target->getAngle() + ang * dtPerTime(dt));
}

mot::RotateTo::RotateTo(const Borrow<Parts>& target, double angle, double time, Optional<bool> clockwizeRotation, int32 rotation)
	:PartsMotion(target, time), angle(angle), clockwizeRotation(clockwizeRotation), rotation(rotation)
{
	firstRotateDirectionIsDesignated = (bool)clockwizeRotation;//noneでなければtrue
}
//0~360ｓ
double seikika(double theta) {
	if (0 <= theta)return Fmod(theta, 360.0);//正規化
	else return 360 + Fmod(theta, 360.0);
}

void mot::RotateTo::start()
{
	PartsMotion::start();

	if (acts.empty())impl = acts.add<Rotate>(target, 0, time);

	double delta = seikika(angle) - seikika(target->getAngle());

	if (not clockwizeRotation)//回転が短い方向を求める
	{
		clockwizeRotation = (delta < 0) ^ (abs(delta) < 180);
	}

	if (*clockwizeRotation)
	{
		//時計回り
		impl->ang = delta < 0 ? delta + 360 : delta;
		impl->ang += 360 * rotation;
	}
	else
	{
		//反時計回り
		impl->ang = delta > 0 ? delta - 360 : delta;
		impl->ang -= 360 * rotation;
	}

	acts.start(true);
}

void mot::RotateTo::update(double dt)
{
	PartsMotion::update(dt);

	acts.update(dt);
}

void mot::RotateTo::reset()
{
	PartsMotion::reset();

	if (not firstRotateDirectionIsDesignated)clockwizeRotation = none;
}

mot::Move::Move(const Borrow<Parts>& target, double moveX, double moveY, double time)
	:Move(target, {moveX,moveY},time)
{
}

mot::Move::Move(const Borrow<Parts>& target, const Vec2& move, double time)
	:PartsMotion(target, time), move(move)
{
}

void mot::Move::update(double dt)
{
	PartsMotion::update(dt);

	target->setPos(target->getPos() + move * dtPerTime(dt));
}

mot::MoveTo::MoveTo(const Borrow<Parts>& target, const Vec2& pos, double time)
	:PartsMotion(target, time), dest(pos)
{
}

void mot::MoveTo::start()
{
	PartsMotion::start();

	if (acts.empty())impl = acts.add<Move>(target, 0, 0, time);

	impl->move = dest - target->getPos();

	acts.start(true);
}

void mot::MoveTo::update(double dt)
{
	PartsMotion::update(dt);

	acts.update(dt);
}

mot::AddZ::AddZ(const Borrow<Parts>& target, double z, double time)
	:PartsMotion(target,time),z(z)
{
}

void mot::AddZ::update(double dt)
{
	PartsMotion::update(dt);

	target->setZ(target->getZ() + z * dtPerTime(dt));
}

mot::SetZ::SetZ(const Borrow<Parts>& target, double z, double time)
	:PartsMotion(target,time),z(z)
{
}

void mot::SetZ::update(double dt)
{
	PartsMotion::update(dt);

}

mot::AddScale::AddScale(const Borrow<Parts>& target, const Vec2& scale, double time)
	:PartsMotion(target,time),scale(scale)
{
}

void mot::AddScale::update(double dt)
{
	PartsMotion::update(dt);

}

mot::SetScale::SetScale(const Borrow<Parts>& target, double sX, double sY, double time)
	:SetScale(target, { sX,sY }, time)
{
}

mot::SetScale::SetScale(const Borrow<Parts>& target, const Vec2& scale, double time)
	:PartsMotion(target, time), scale(scale)
{
}

void mot::SetScale::update(double dt)
{
	PartsMotion::update(dt);

}

Vec2 mot::PauseTo::calDestination()const
{
	//目標から回転後の座標を引く
	if (target->parent)
	{
		const auto& parentAspect = target->parent->transform->getAspect().xy();
		const auto& d = dest * parentAspect;
		/*
		const auto& nowRotatePos = target->getRotatePos().rotate(target->getAbsAngle() * 1_deg);

		return d + nowRotatePos.rotated((ang - target->getAngle())*1_deg)- target->getPos().rotated(-target->parent->getAbsAngle() * 1_deg)-nowRotatePos;
		*/
		return d.rotatedAt(d + (target->getRotatePos() * parentAspect).rotate(ang * 1_deg), (target->getAngle() - ang) * 1_deg)//目標座標を初期角度まで回転させる
			- target->getRelativePos();//親の回転を消した相対座標
	}
	//たぶん親がいないなんてことはない(masterpartsに命令を出さない限り)
	return dest;
}

mot::PauseTo::PauseTo(const Borrow<Parts>& target, double destX, double destY, double sX, double sY, double angle, double time, Optional<bool> clockwizeRotation, int32 rotation)
	:PartsMotion(target, time), ang(angle), clockwizeRotation(clockwizeRotation), rotation(rotation), dest({ destX,destY }), scale({ sX,sY }),parentDirectionInit(none)
{
}

mot::PauseTo::PauseTo(const Borrow<Parts>& target, const Vec2& pos, double sX, double sY, double angle, double time, Optional<bool> clockwizeRotation, int32 rotation)
	:PauseTo(target,pos,{sX,sY},angle,time,clockwizeRotation,rotation)
{
}

mot::PauseTo::PauseTo(const Borrow<Parts>& target, double destX, double destY, const Vec2& scale, double angle, double time, Optional<bool> clockwizeRotation, int32 rotation)
	:PauseTo(target, { destX,destY }, scale, angle, time, clockwizeRotation, rotation)
{
}

mot::PauseTo::PauseTo(const Borrow<Parts>& target, const Vec2& pos, const Vec2& scale, double angle, double time, Optional<bool> clockwizeRotation, int32 rotation)
	:PartsMotion(target, time), ang(angle), clockwizeRotation(clockwizeRotation), rotation(rotation), dest(pos), scale(scale), parentDirectionInit(none)
{
}

void mot::PauseTo::start()
{
	PartsMotion::start();

	if (motions.empty())
	{
		rotateTo=motions.addParallel<RotateTo>(target, ang, time, clockwizeRotation, rotation);
		motions.addParallel<SetScale>(target, scale.x, scale.y, time);
		move = motions.addParallel<Move>(target, 0, 0, time);
	}
	constexpr double _floor = 100;
	move->move = (calDestination()*_floor).asPoint()/_floor;
	
	moveInit = move->move;

	if (target->parent)parentAngleInit = target->parent->getAbsAngle();//target->parent->transform->getDirection().xy();

	motions.start(true);
}

void mot::PauseTo::update(double dt)
{
	if (parentAngleInit)
	{
		move->move = moveInit.rotated(target->parent->getAbsAngle() * 1_deg);
	}

	PartsMotion::update(dt);
	motions.update(dt);
}

mot::SetRotateCenter::SetRotateCenter(const Borrow<Parts>& target, double x, double y)
	:SetRotateCenter(target, { x,y })
{
}

mot::SetRotateCenter::SetRotateCenter(const Borrow<Parts>& target, const Vec2& pos)
	:PartsMotion(target), pos(pos)
{
	this->updatePriority = SetRotateCenterMotionsUpdatePriority;
}

void mot::SetRotateCenter::start()
{
	PartsMotion::start();
	target->setRotatePos(pos);
}

mot::SetParent::SetParent(const Borrow<Parts>& target, String parentName)
	:PartsMotion(target), parentName(parentName)
{
}

void mot::SetParent::start()
{
	PartsMotion::start();
	target->setParent(parentName);
}

mot::PartsParamsVariable::PartsParamsVariable(const Borrow<Parts>& target, String motionName, String variableName, String value)
	:PartsMotion(target), motionName(motionName), variableName(variableName), _value(value)
{
}

String mot::PartsParamsVariable::NameProc(StringView motionName, StringView variableName)
{
	return U"{}/{}"_fmt(motionName, variableName);
}

String mot::PartsParamsVariable::decodeValue(String value)
{
	for (const auto& name : PartsParams::Names)
	{
		//変数名を置き換える
		value.replace(name, GetParamStr(target->params, name));
	}
	return value;
}

void mot::PartsParamsVariable::start()
{
	PartsMotion::start();
	//ここで値を更新する。
	pMan->getComponent<Field<String>>(PartsParamsVariable::NameProc(motionName, variableName))
		->value = decodeValue(_value);
}

mot::SetVariables::SetVariables(String motionName)
	:IAction(0), motionName(motionName)
{
	this->updatePriority = SetVariablesMotionsUpdatePriority;
}

void mot::SetVariables::start()
{
	IAction::start();
	//実行
	for (auto& pro : setVar)
	{
		pro();
	}
}
