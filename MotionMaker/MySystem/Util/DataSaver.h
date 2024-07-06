#pragma once

class DataSaver
{
public:
	HashTable<String,DataSaver> children;
	String name, data;

	DataSaver() {};

	DataSaver(const String& name, const String& data)
		:name(name), data(Escape(data))
	{
		ConstractChild();
	}

	static String preProcess(const String& data);

	void ConstractChild()
	{
		children = LoadDataSaver(data);
	}

	template<class T>
	T get(int32 index,T default_,char spliter=U',') const
	{
		auto a=data.split(spliter);
		try
		{
			if (a.size() <= index)return default_;
			return Parse<T>(a[index]);
		}
		catch (std::out_of_range& oor)
		{
			return default_;
		}
	}

	Optional<DataSaver> getDataSaver(StringView name)const
	{
		if (children.contains(name))return children.at(name);
		else return none;
	}

	static String Escape(const String& text)
	{
		String result = text;

		result = text.replaced(U"\n", U"");
		result = result.replaced(U"\t", U"");
		result = result.replaced(U" ", U"");
		return preProcess(result.replaced(U"\\n", U"\n"));
	}

	static HashTable<String,DataSaver> LoadDataSaver(String text)
	{
		HashTable<String,DataSaver>result;
		int32 start = -1;
		String name{};
		bool comp=false;
		int32 depth=0;	
		String cont{};		//中身　子に渡す
		for(int32 i=0;i<text.size();++i)
		{
			if (comp)
			{
				if (text[i] == U'{')
				{
					depth += 1;
				}
				else if (text[i]==U'}')
				{
					depth -= 1;
				}
				
				if(not ((text[i] == U'{' and depth==1) or (text[i]==U'}' and depth==0)))cont << text[i];

				if (depth == 0)
				{
					result.emplace(name, DataSaver(name, cont));
					name={};
					cont = {};
					comp = false;
				}
			}
			else {
				//start==-1のときはデータセーバーの{...}の中に入ってない
				if (start == -1 and text[i] == U'[')
				{
					start = i;
				}

				if (start != -1 and text[i] != U'[')
				{
					if (text[i] == U']')
					{
						start = -1;
						comp = true;
						depth = 0;
					}
					else {
						name << text[i];
					}
				}
			}
		}
		return result;
	}

};

