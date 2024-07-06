#include "../stdafx.h"
#include "DataSaver.h"

String DataSaver::preProcess(const String& text)
{
	bool bracesFlag = false;//波かっこ
	size_t closeBracketsInd = 0;
	String res = text;
	size_t insertNum = 0;
	for (size_t i = 0; i < text.size(); i++)
	{
		if (text[i] == U']') {
			bracesFlag = true;
			closeBracketsInd = i+1;
		}
		else if (bracesFlag) {
			if (text[i] == U'{') {
				bracesFlag = false;
			}
			else if (text[i] == U'[') {
				//省略された波かっこを補充
				res.insert(closeBracketsInd + insertNum, U"{"); insertNum++;
				res.insert(i + insertNum, U"}"); insertNum++;
				bracesFlag = false;
			}
		}
	}
	//最後のパックで波かっこが省略されていたらここで補充
	if (bracesFlag)
	{
		res.insert(closeBracketsInd + insertNum, U"{");
		res.append(U"}");
	}

	return res;
}
