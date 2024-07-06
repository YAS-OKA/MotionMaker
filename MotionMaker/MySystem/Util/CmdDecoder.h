#pragma once
#include"Cmd.hpp"

namespace {

	template<typename Type>
	inline Type convert(const String& value)
	{
		if (auto result = EvalOpt(value))
		{
			return static_cast<Type>(*result);
		}
		// 例外処理
	}

	template<>
	inline Optional<String> convert<Optional<String>>(const String& value)
	{
		return static_cast<Optional<String>>(value);
	}

	template<>
	inline bool convert<bool>(const String& value)
	{
		if (value == U"true") return true;
		else if (value == U"false") return false;
		// 例外処理
	}

	template<>
	inline StringView convert<StringView>(const String& value) { return value; }

	template<>
	inline String convert<String>(const String& value) { return value; }

	template<>
	inline Optional<bool> convert<Optional<bool>>(const String& value)
	{
		if (value == U"none")
		{
			return none;
		}
		else
		{
			return convert<bool>(value);
		}
		// 例外処理
	}

	template<>
	inline Vec2 convert<Vec2>(const String& value)
	{
		const auto& xy = value.replaced(U"(", U"").replaced(U")", U"").split(U',');
		return { convert<double>(xy[0]),convert<double>(xy[1]) };
	}

	// 配列に対してパック展開を行う関数
	template <typename T, class... Args, std::size_t... Indices>
	T* arrayPackHelper(Array<String> arr, std::index_sequence<Indices...>) {
		// パック展開を行って関数fに渡す
		return new T(convert<Args>(arr[Indices])...);
	}
}
//String配列をパラメータパックにしてTのコンストラクタに渡し、T*を生成して返す。
template <typename T, class... Args>
T* arrayPack(Array<String> arr) {
	return arrayPackHelper<T, Args...>(arr, std::make_index_sequence<sizeof...(Args)>{});
}

using Delegate = std::function<std::shared_ptr<ICMD> (Array<String>)>;
using ArgProcessing = std::function<Array<String>(Array<String>)>;

class CmdDecoder:public Borrowable
{
public:
	Array<String> token;
	//命令テーブル
	HashTable<String, HashTable<size_t, Delegate>> table;

	//入力を受け取る 自分を返す
	virtual CmdDecoder* input(const String& input, char spliter = U' ');
	virtual CmdDecoder* input(Array<String> tokens);
	//コマンドを登録する
	template<class Act, class... Args>
	void add_original(
		const String& cmd,
		const EventFunction<Act>& ef = Eventa<Act>::NonEvent,
		const PostProcessing<Act>& postProcessing = [](Act* act) {return act; },			//作られたアクションに対しての処理
		const ArgProcessing& argProcessing = [](Array<String> args) {return args; })	//アクションに渡す引数に対しての処理
	{
		if constexpr (sizeof...(Args) == 0) {
			table[cmd][sizeof...(Args)] = [=](const Array<String> arr)
				{
					return std::shared_ptr<ICMD>(new CMD<Act>(postProcessing(new Act()), Eventa<Act>(ef)));
				};
		}
		else
		{
			table[cmd][sizeof...(Args)] = [=](const Array<String> arr)
				{
					return std::shared_ptr<ICMD>(new CMD<Act>(postProcessing(arrayPack<Act, Args...>(argProcessing(arr))), Eventa<Act>(ef)));
				};
		}
	}

	//コマンドを登録する コマンド実行後にイベントが呼ばれる
	template<class Act, class... Args, class... Args2>
	void add_event_cmd(const String& cmd,const EventFunction<Act>& ef, Args2 ...postProcessBuildsArgs)
	{
		add_original<Act, Args...>(cmd, ef,buildProcess<Act>(postProcessBuildsArgs...));
	}
	//コマンドを登録する
	template<class Act, class... Args, class... Args2>
	void add(const String& cmd, Args2 ...postProcessBuildsArgs)
	{
		add_original<Act, Args...>(cmd, Eventa<Act>::NonEvent, buildProcess<Act>(postProcessBuildsArgs...));
	}

	std::shared_ptr<ICMD> decode();
};

namespace my
{
	class Scene;
}

namespace mot
{
	class PartsManager;
	class MakeParts;
}

class DecoderSet
{
public:
	CmdDecoder* decoder;
	DecoderSet(CmdDecoder* decoder)
		:decoder(decoder) {}

	void setTargetDecoder(CmdDecoder* target)
	{
		decoder = target;
	}
	//loadを使うんだったら2つめの引数はnullでいいよ
	void motionScriptCmd(const Borrow<mot::PartsManager>& pmanager,const Borrow<prg::Actions>& actions);

	void objScriptCmd(const Borrow<Object>& obj);

	// mkpar(name,path or figuretype,x,y (and w,h case fig)) MakeParts
	void registerMakePartsCmd(const Borrow<mot::PartsManager>& pmanager, bool createHitbox = false, const EventFunction<mot::MakeParts>& ef = Eventa<mot::MakeParts>::NonEvent);
	using MakePartsPostProcessing = PostProcessing<mot::MakeParts>;
	void registerMakePartsCmd(const Borrow<mot::PartsManager>& pmanager, MakePartsPostProcessing processing, const EventFunction<mot::MakeParts>& ef = Eventa<mot::MakeParts>::NonEvent);
};
