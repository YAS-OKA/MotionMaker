#pragma once
#include"../EC.hpp"
//フィールド 汎用的
template<class T>
class Field :public Component
{
public:
	Field() = default;
	Field(const T& init)
	{
		value = init;
	}
	
	operator T()
	{
		return value;
	}

	T value;
};

#include<any>

class AnyParams :public Component
{
public:
	HashTable<String, std::any> params;

	void emplace(StringView name, const std::any& param)
	{
		params.emplace(name, param);
	}

	template<class T>
	T& get(StringView param_name)
	{
		if (params.contains(param_name))
		{
			try {
				return std::any_cast<T&>(params[param_name]);
			}
			catch (std::bad_any_cast& e){
				throw Error{ U"キャスト失敗.\n{}は{}型にキャストできません"_fmt(param_name,Unicode::Widen(std::string(typeid(T).name()))) };
			}
		}
		else
		{
			throw Error{ U"{}はありません"_fmt(param_name) };
		}
	}

	
};
