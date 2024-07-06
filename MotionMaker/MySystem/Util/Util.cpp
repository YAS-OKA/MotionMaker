#include"../../stdafx.h"
#include"Util.h"
#include<Windows.h>

namespace util
{
	StopMax::StopMax(double max, double value)
		:value(value), max(max) {}

	void StopMax::add(double additional)
	{
		value += additional;
		value = value > max ? max : value;
	}

	bool StopMax::additionable()const
	{
		return value < max;
	}

	std::string toStr(StringView s)
	{
		return std::string(s.begin(), s.end());
	}

	String toUStr(const std::string& s)
	{
		return String(s.begin(), s.end());
	}

	EditFile::EditFile(FilePathView path)
	{
		//auto sPath = str(path);
		////存在しなかったら作成
		//if (not FileSystem::IsFile(path)) {
		//	std::ofstream(sPath);
		//}
		//auto f = std::fstream(sPath);
		////オープンできたらfileにセット
		//if (f)
		//{
		//	file = uPtr<std::fstream>(new std::fstream(std::move(f)));
		//}
	}

	void EditFile::overwrite(StringView text)
	{
		if (not file)return;

		file->clear();
	}

	String EditFile::readAll()const
	{
		if (not file)return String{};//fileがセットされてなかったら

		std::string contents;
		std::string line;
		while (std::getline(*file, line)) {
			contents += line + "\n";
		}

		return toUStr(contents);
	}

	EditFile createFile(FilePath path)
	{
		return EditFile(path);
	}

}
