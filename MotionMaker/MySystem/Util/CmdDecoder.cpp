#include "../stdafx.h"
#include"CmdDecoder.h"
//#include"../Prg/Action.h
#include"Util.h"

CmdDecoder* CmdDecoder::input(const String& input, char spliter)
{
	return this->input(input.split(spliter));
}

CmdDecoder* CmdDecoder::input(Array<String> tokens)
{
	token = tokens;
	return this;
}

std::shared_ptr<ICMD> CmdDecoder::decode()
{
	String head;
	Array<String> arg;

	if (token.isEmpty())goto Err;

	head = token[0];

	arg = util::slice(token, 1, token.size());

	if (table.contains(head) and table[head].contains(arg.size()))return table[head][arg.size()](arg);	
Err:
//例外処理
	return nullptr;	
}

#include"../Motions/MotionCmd.h"
#include"../Motions/Motion.h"

void DecoderSet::motionScriptCmd(const Borrow<mot::PartsManager>& pmanager,const Borrow<prg::Actions>& actions)
{
	using namespace mot;

	decoder->add<SetMotion<RotateTo>, String, bool, double, double, Optional<bool>, int32>(U"SetAngle", pmanager, actions);
	decoder->add<SetMotion<RotateTo>, String, bool, double, double, Optional<bool>>(U"SetAngle", pmanager, actions);
	decoder->add<SetMotion<RotateTo>, String, bool, double, double>(U"SetAngle", pmanager, actions);
	decoder->add<SetMotion<RotateTo>, String, bool, double>(U"SetAngle", pmanager, actions);
	decoder->add<SetMotion<MoveTo>, String, bool, Vec2, double>(U"SetPos", pmanager, actions);
	decoder->add<SetMotion<MoveTo>, String, bool, Vec2>(U"SetPos", pmanager, actions);
	decoder->add<SetMotion<PauseTo>, String, bool, double, double, double, double, double, double, Optional<bool>, int32>(U"Pause", pmanager, actions);
	decoder->add<SetMotion<PauseTo>, String, bool, double, double, double, double, double, double, Optional<bool>>(U"Pause", pmanager, actions);
	decoder->add<SetMotion<PauseTo>, String, bool, double, double, double, double, double, double>(U"Pause", pmanager, actions);
	decoder->add<SetMotion<PauseTo>, String, bool, Vec2, Vec2, double, double, Optional<bool>, int32>(U"PauseVec", pmanager, actions);
	decoder->add<SetMotion<PauseTo>, String, bool, Vec2, Vec2, double, double, Optional<bool>>(U"PauseVec", pmanager, actions);
	decoder->add<SetMotion<PauseTo>, String, bool, Vec2, Vec2, double, double>(U"PauseVec", pmanager, actions);
	decoder->add<SetMotion<StartMotion>, String, bool, String>(U"Start",pmanager, actions);
	decoder->add<SetMotion<StartMotion>, String, bool, String, bool>(U"Start",pmanager, actions);
	decoder->add<SetMotion<Wait>, String, bool, double>(U"Wait", pmanager, actions);
	decoder->add<SetMotion<MyPrint>, String, bool, String,double>(U"MyPrint", pmanager, actions);
	decoder->add<SetMotion<SetRotateCenter>, String, bool, double, double>(U"SetRC", pmanager, actions);
	decoder->add<SetMotion<SetParent>, String, bool, String>(U"SetParent", pmanager, actions);
	decoder->add<SetMotion<PartsParamsVariable>, String, bool, String, String, String>(U"Param", pmanager, actions);

	decoder->add<EraseMotion, StringView>(U"em", pmanager);
	decoder->add<LoadMotionScript, FilePath, String>(U"load", pmanager);
	decoder->add<StartMotion, String>(U"start", pmanager);
	decoder->add<StartMotion, String,bool>(U"start", pmanager);	
}

void DecoderSet::registerMakePartsCmd(const Borrow<mot::PartsManager>& pmana,bool createHitbox, const EventFunction<mot::MakeParts>& e)
{
	using namespace mot;

	decoder->add_event_cmd<MakeParts, String, String, double, double>(U"mkpar",e, pmana,createHitbox);
	decoder->add_event_cmd<MakeParts, String, String, double, double, double, double>(U"mkpar",e, pmana,createHitbox);
	decoder->add_event_cmd<MakeParts, String, String, double, double, double, double,double>(U"mkpar",e, pmana, createHitbox);
	decoder->add_event_cmd< MakeParts, String, String, double, double, double>(U"mkpar",e, pmana,createHitbox);
}

void DecoderSet::registerMakePartsCmd(const Borrow<mot::PartsManager>& pmanager, MakePartsPostProcessing processing, const EventFunction<mot::MakeParts>& e)
{
	using namespace mot;
	decoder->add_original<MakeParts, String, String, double, double>(U"mkpar",e, processing);
	decoder->add_original<MakeParts, String, String, double, double, double, double>(U"mkpar",e, processing);
	decoder->add_original<MakeParts, String, String, double, double, double, double, double>(U"mkpar",e, processing);
	decoder->add_original<MakeParts, String, String, double, double, double>(U"mkpar", e,processing);
}
