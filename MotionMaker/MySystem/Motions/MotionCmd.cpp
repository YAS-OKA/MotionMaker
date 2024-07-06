#include"../stdafx.h"
#include"MotionCmd.h"
#include "../Util/Cmd.hpp"
#include"../Game/Scenes.h"
#include"Parts.h"
#include"Motion.h"

namespace mot
{
	void MakeParts::start()
	{
		IAction::start();
		String parentName = pmanager->partsArray.isEmpty() ? U"" : U"master";

		if (path)
		{
			loadPartsTexture(pmanager->scene, *path);

			createdParts = pmanager->addParts(PartsParams(name, *path, TextureAsset(*path), parentName, { x,y }));

			if (createHitbox)createdParts->createHitbox(TextureAsset(*path).size() / 2.0, Image{ localPath + *path }.alphaToPolygons());
		}
		else
		{
			createdParts = pmanager->addParts(PartsParams(name, none, fig, parentName, { x,y }));
			if (createHitbox)createdParts->createHitbox({ 0,0 }, { fig.asPolygon() });
		}
	}

	Borrow<Parts> MakeParts::getCreatedParts(bool keepParts)
	{
		auto ret = createdParts;
		if (not keepParts)createdParts = nullptr;
		return ret;
	}

	Borrow<Parts> FindParts::getFindParts(bool keepParts)
	{
		auto ret = foundParts;
		if (not keepParts)foundParts = nullptr;
		return ret;
	}

	void FindParts::start()
	{
		foundParts = pmanager->find(name);
	}

	HashSet<Parts*> KillParts::getKilledParts(bool keepParts)
	{
		HashSet<Parts*> ret = killedParts;
		if (not keepParts)killedParts = {};
		return ret;
	}

	void KillParts::start()
	{
		if (auto parts = pmanager->find(name))
		{
			if (killChildren or parts->getName() == U"master")
			{
				//子を殺す
				prg::Actions actions;
				Array<KillParts*>acts;
				for (const auto& child : parts->partsRelation.getChildren())
				{
					acts << actions.addParallel<KillParts>(child->name, true).build(pmanager);
				}
				actions.start(true);

				for (const auto& act : acts)
				{
					for (const auto& p : act->getKilledParts())
						killedParts.emplace(p);
				}
			}
			else {
				auto parent = parts->partsRelation.getParent();

				auto child = parts->partsRelation.getChild();
				//自分の親を自分の子の親にする
				if (child != nullptr and parent != nullptr)child->setParent(parent->getName());
			}
			pmanager->killParts(parts);

			killedParts.emplace(parts);
		}
	}
	StartMotion::StartMotion(String motionName, bool loop)
		:IAction(0), motionName(motionName), loopedMotion(loop)
	{
	}

	void StartMotion::start()
	{
		IAction::start();
		pMan->actman[motionName].loop = loopedMotion;
		pMan->startAction(motionName);
	}

	EraseMotion::EraseMotion(StringView motionName)
		:IAction(0),motionName(motionName)
	{
	}

	EraseMotion* EraseMotion::build(const Borrow<PartsManager>& pMan)
	{
		this->pMan = pMan;
		return this;
	}

	void EraseMotion::start()
	{
		prg::IAction::start();
		for (auto& parts : pMan->partsArray)
		{
			parts->actman.erase(motionName);
		}
	}

	LoadMotionScript::LoadMotionScript(FilePath path,String motionName)
		:path(path),motionName(motionName)
	{
	}

	LoadMotionScript* LoadMotionScript::build(const Borrow<PartsManager>& pman)
	{
		this->pman = pman;
		return this;
	}
	void LoadMotionScript::start()
	{
		if (not MotionScript().LoadFile(pman, path, motionName))
		{
			Console << U"モーションスクリプト読み込みに失敗\n\
					指定されたパス:{}"_fmt(path);
		}
	}
	WriteMotionScript::WriteMotionScript(FilePath path, String motionName, Optional<String> len, Optional<String> time)
		:path(path), motionName(motionName), time(time), len(len)
	{
	}

	WriteMotionScript* WriteMotionScript::build(const Borrow<PartsManager>& pmana)
	{
		this->pman = pmana;
		return this;
	}

	Array<String> WriteMotionScript::createMotionText() const
	{
		Array<String> res;
		if (time)res << U"TIME {}"_fmt(*time);
		if (len)res << U"LEN {}"_fmt(*len);

		HashSet<String> target_list = targets;
		//ターゲットが空（指定されていない）場合はすべてのパーツをターゲットにする。
		if (target_list.empty())pman->partsArray.each([&](const Borrow<Parts>& p)mutable { target_list.emplace(p->name); });

		for (const auto& _name : target_list)
		{
			auto parts = pman->find(_name);

			if ((not parts) or (_name == U"master"))continue;//パーツが存在しない場合とmasterパーツは除外

			res << U"JOINT {}"_fmt(parts->name);
			Vec2 pos = parts->getPos();
			if (parts->parent)
			{
				//親の回転を消した相対距離にする
				pos.rotate(-parts->parent->getAbsAngle() * 1_deg);
			}
			res << U"SetRC {} {}"_fmt(parts->getRotatePos().x, parts->getRotatePos().y);
			res << U"Pause {} {} {} {} {}"_fmt(pos.x, pos.y, parts->getScale().x, parts->getScale().y, parts->getAngle());
		}

		return res;
	}

	void WriteMotionScript::start()
	{
		String inputText = U"";

		size_t process = 0;

		Array<String> text;

		{
			auto reader = TextReader{ path };

			text = reader ? reader.readAll().split_lines() : Array<String>{};

			if (text.isEmpty())text << U"";
		}

		for (size_t lineNumber = 0; lineNumber < text.size(); lineNumber++)
		{
			auto& line = text[lineNumber];

			if (process == 0)//モーション名が一致するリージョンを探す
			{
				if (line[0] == '#' and util::slice(line, 1, line.size()) == motionName)
				{
					process++;//次の工程へ
				}
				else if (lineNumber + 1 >= text.size())//最後の行まで一致するモーション名が見つからなかった場合
				{
					//モーションを追加する
					text.push_back(U"#" + motionName);
					lineNumber++;//次の行へ移動
					process++;//次の工程へ
				}
			}

			if (process == 1)//モーションを挿入する
			{
				//次の行が　最後の行　または　別のリージョン　の場合
				if (lineNumber + 1 >= text.size() or text[lineNumber + 1][0] == U'#')
				{
					auto newTexts = createMotionText();
					text.insert(text.begin() + lineNumber + 1, newTexts.begin(), newTexts.end());//モーションを挿入する
					process++;
				}
			}

			if (process == 2)//編集したテキストを反映
			{
				TextWriter writer{ path };
				writer.clear();
				for (auto it = text.begin(), en = text.end(); it != en; ++it)
				{
					writer.writeln(*it);
				}

				break;//コンプリート
			}
		}

	}
}
