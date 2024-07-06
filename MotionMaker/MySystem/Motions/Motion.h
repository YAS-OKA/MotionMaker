#pragma once
#include"Parts.h"
#include"../Prg/Action.h"
#include"../Util/CmdDecoder.h"
#include"../Component/Field.h"

namespace mot
{
	class MotionScript
	{
		//LoadのときINS命令でファイルの全文を要求するときこいつを参照する
		Optional<String> AllText;
	public:
		CmdDecoder deco;

		HashTable<String, String> constant;

		prg::Actions* CreateMotions(const Borrow<PartsManager>& pMan, const String& motionName, const String& text);

		void Load(const Borrow<PartsManager>& pMan, const String& motionName, const String& text);

		bool LoadFile(const Borrow<PartsManager>& pMan, const String& path,const String& motionName);

		static Optional<String> GetMotionScrOf(const String& text, StringView motionName, bool removeBackBlank = false);

		static Array<String> GetMotionNames(FilePathView scriptPath);
	};

	class PartsMotion :public prg::TimeAction
	{
	public:
		Borrow<Parts> target;

		PartsMotion(const Borrow<Parts>& target, double time = 0);
	};

	class Rotate :public PartsMotion
	{
	public:
		double ang;

		Rotate(const Borrow<Parts>& target, double angle, double time = 0);

		void update(double dt)override;
	};

	class RotateTo :public PartsMotion
	{
		bool firstRotateDirectionIsDesignated;
	public:
		double angle;

		int32 rotation;

		Optional<bool> clockwizeRotation;

		Borrow<Rotate> impl;

		Actions acts;
		//clockwizeRotationは時計回りか反時計回りか。rotationは何回転するか。
		RotateTo(const Borrow<Parts>& target, double angle, double time = 0,Optional<bool> clockwizeRotation = none, int32 rotation = 0);

		void start()override;

		void update(double dt)override;

		void reset()override;
	};

	class Move :public PartsMotion
	{
	public:
		Vec2 move;

		Move(const Borrow<Parts>& target, double moveX, double moveY, double time = 0);

		Move(const Borrow<Parts>& target, const Vec2& move, double time = 0);

		void update(double dt)override;
	};

	class MoveTo :public PartsMotion
	{
	public:
		Vec2 dest;

		Borrow<Move> impl;

		Actions acts;

		MoveTo(const Borrow<Parts>& target, const Vec2& pos, double time = 0);

		void start()override;

		void update(double dt)override;
	};

	class AddZ :public PartsMotion
	{
	public :
		double z;

		AddZ(const Borrow<Parts>& target, double z, double time = 0);

		void update(double dt)override;
	};

	class SetZ :public PartsMotion
	{
	public:
		double z;

		SetZ(const Borrow<Parts>& target, double z, double time = 0);

		void update(double dt)override;
	};

	class AddScale :public PartsMotion
	{
	public:
		Vec2 scale;

		AddScale(const Borrow<Parts>& target, const Vec2& scale, double time = 0);

		void update(double dt)override;
	};

	class SetScale :public PartsMotion
	{
	public:
		Vec2 scale;

		SetScale(const Borrow<Parts>& target, double sX, double sY, double time = 0);

		SetScale(const Borrow<Parts>& target, const Vec2& scale, double time = 0);

		void update(double dt)override;
	};

	class SetRotateCenter :public PartsMotion
	{
	public:
		Vec2 pos;

		SetRotateCenter(const Borrow<Parts>& target, double x, double y);

		SetRotateCenter(const Borrow<Parts>& target, const Vec2& pos);

		void start()override;
	};

	class SetParent :public PartsMotion
	{
	public:
		String parentName;

		SetParent(const Borrow<Parts>& target, String parentName);

		void start()override;
	};

	class PauseTo :public PartsMotion
	{
	private:
		Optional<Vec2> parentDirectionInit;
		Optional<double>parentAngleInit;
		Vec2 directionInit;
		Vec2 moveInit;
		Borrow<Move> move;
		Borrow<RotateTo> rotateTo;
		Vec2 calDestination()const;
	public:
		Vec2 scale;
		Vec2 dest;
		double ang;
		const int32 rotation;
		Optional<bool> clockwizeRotation;

		Actions motions;

		PauseTo(const Borrow<Parts>& target, double destX, double destY, double sX, double sY, double angle, double time = 0, Optional<bool> clockwizeRotation = none, int32 rotation = 0);

		PauseTo(const Borrow<Parts>& target, const Vec2& pos, double sX, double sY, double angle, double time = 0, Optional<bool> clockwizeRotation = none, int32 rotation = 0);

		PauseTo(const Borrow<Parts>& target, double destX, double destY, const Vec2& scale, double angle, double time = 0, Optional<bool> clockwizeRotation = none, int32 rotation = 0);

		PauseTo(const Borrow<Parts>& target, const Vec2& pos, const Vec2& scale, double angle, double time = 0, Optional<bool> clockwizeRotation = none, int32 rotation = 0);

		void start()override;

		void update(double dt)override;
	};
	//パーツのパラメータ名をもとに変数の値を決定する
	class PartsParamsVariable :public PartsMotion
	{
		String _value;
	public:
		String variableName;

		String motionName;

		Borrow<PartsManager> pMan;

		PartsParamsVariable(const Borrow<Parts>& target, String motionName, String variableName, String value);

		static String NameProc(StringView motionName, StringView variableName);

		String decodeValue(String value);

		void start()override;
	};

	class SetVariables :public prg::IAction
	{
	private:
		Array<std::function<void()>> setVar;
	public:
		Borrow<PartsManager> pman;

		String motionName;

		SetVariables(String motionName);

		void start()override;

		//変数名から値を取得
		template<class VarType = double>
		Optional<VarType> var(const String& name)
		{
			return ParseOpt<VarType>(pman->getComponent<Field<String>>(name)->value);
		}
		//argsNumはパーツモーションのコンストラクタの何番目の変数をセットするかを指定する
		void addVarSettingFunc(const Borrow<IAction>& action, StringView varName,const int32& argsNum)
		{
			setVar << _createVarSettingFunc(action, varName, argsNum);
		}

		std::function<void()> _createVarSettingFunc(const Borrow<IAction>& action, StringView varName,const int32& argsNum = 0)
		{
			String name{ PartsParamsVariable::NameProc(motionName,varName) };

			if (auto ca = action.cast<Rotate>())
			{
				switch (argsNum)
				{
				case 0: return [=] { ca->ang = *var(name); };
				case 1: return [=] { ca->time = *var(name); };
				}
			}
			else if (auto ca = action.cast<RotateTo>())
			{
				switch (argsNum)
				{
				case 0: return [=] { ca->angle = *var(name); };
				case 1: return [=] { ca->time = *var(name); };
				case 2: return [=] { ca->clockwizeRotation = *var<bool>(name); };
				case 3: return [=] { ca->rotation = *var<int32>(name); };
				}
			}
			else if (auto ca = action.cast<MoveTo>())
			{
				switch (argsNum)
				{
				case 0: return [=] { ca->dest = *var<Vec2>(name); };
				case 1: return [=] { ca->time = *var(name); };
				}
			}
			else if (auto ca = action.cast<SetParent>())
			{
				switch (argsNum)
				{
				case 0: return [=] { ca->parentName = *var<String>(name); };
				}
			}

			throw Error{ U"まだ変数の使用が対応していないアクションです。この関数内に追加してください" };
		}
	};
}
