#include "../stdafx.h"
#include "AssetManager.h"

resource::Mode AssetManager::mode = resource::debug;

FilePath AssetManager::myAsset(FilePathView path)
{
	if (mode == resource::relase)return Resource(path);
	else return FilePath{ path };
}

// FontAsset

resource::font::font(AssetNameView name)
	:FontAsset(name)
{

}

bool resource::font::Load(AssetNameView name, const String& preloadText)
{
	return FontAsset::Load(name, preloadText);

}

bool resource::font::Register(AssetNameView name, int32 fontSize, FilePathView path, FontStyle style)
{	
	
	return FontAsset::Register(name, fontSize,AssetManager::myAsset(path), style);
}

bool resource::font::Register(AssetNameView name, int32 fontSize, FilePathView path, size_t faceIndex, FontStyle style)
{
	
	
	return FontAsset::Register(name,fontSize,AssetManager::myAsset(path),faceIndex,style);
}

bool resource::font::Register(AssetNameView name, int32 fontSize, Typeface typeface, FontStyle style)
{
	
	return FontAsset::Register(name,fontSize,typeface,style);
}

bool resource::font::Register(AssetNameView name, FontMethod fontMethod, int32 fontSize, FilePathView path, FontStyle style)
{
	
	
	return FontAsset::Register(name,fontMethod,fontSize,AssetManager::myAsset(path),style);
}

bool resource::font::Register(AssetNameView name, FontMethod fontMethod, int32 fontSize, FilePathView path, size_t faceIndex, FontStyle style)
{
	
	
	return FontAsset::Register(name,fontMethod,fontSize,AssetManager::myAsset(path),faceIndex,style);
}

bool resource::font::Register(AssetNameView name, FontMethod fontMethod, int32 fontSize, Typeface typeface, FontStyle style)
{
	return FontAsset::Register(name,fontMethod,fontSize,typeface,style);
}

bool resource::font::Register(AssetNameView name, std::unique_ptr<FontAssetData>&& data)
{
	
	return FontAsset::Register(name,std::move(data));
}


bool resource::font::Register(AssetNameAndTags nameAndTags, int32 fontSize, FilePathView path, FontStyle style)
{
	
	return FontAsset::Register(nameAndTags,fontSize,AssetManager::myAsset(path),style);
}

bool resource::font::Register(AssetNameAndTags nameAndTags, int32 fontSize, FilePathView path, size_t faceIndex, FontStyle style)
{
	
	
	return FontAsset::Register(nameAndTags,fontSize,AssetManager::myAsset(path),faceIndex,style);
}

bool resource::font::Register(AssetNameAndTags nameAndTags, int32 fontSize, Typeface typeface, FontStyle style)
{
	
	return FontAsset::Register(nameAndTags,fontSize,typeface,style);
}

bool resource::font::Register(AssetNameAndTags nameAndTags, FontMethod fontMethod, int32 fontSize, FilePathView path, FontStyle style)
{
	
	
	return FontAsset::Register(nameAndTags,fontMethod,fontSize,AssetManager::myAsset(path),style);
}

bool resource::font::Register(AssetNameAndTags nameAndTags, FontMethod fontMethod, int32 fontSize, FilePathView path, size_t faceIndex, FontStyle style)
{
	
	
	return FontAsset::Register(nameAndTags,fontMethod,fontSize,AssetManager::myAsset(path),faceIndex,style);
}

bool resource::font::Register(AssetNameAndTags nameAndTags, FontMethod fontMethod, int32 fontSize, Typeface typeface, FontStyle style)
{
	
	return FontAsset::Register(nameAndTags,fontMethod,fontSize,typeface,style);
}

// TextureAsset

resource::texture::texture(AssetNameView name)
	:TextureAsset(name)
{

}

bool resource::texture::Load(AssetNameView name)
{
	return FontAsset::Load(name);
}

bool resource::texture::Register(AssetNameView name, FilePathView path, TextureDesc desc)
{
	
	return TextureAsset::Register(name,AssetManager::myAsset(path),desc);
}

bool resource::texture::Register(AssetNameView name, FilePathView rgb, FilePathView alpha, TextureDesc desc)
{
	
	return TextureAsset::Register(name,rgb,alpha,desc);
}

bool resource::texture::Register(AssetNameView name, const Color& rgb, FilePathView alpha, TextureDesc desc)
{
	
	return TextureAsset::Register(name,rgb,alpha);
}

bool resource::texture::Register(AssetNameView name, const Emoji& emoji, TextureDesc desc)
{
	
	return TextureAsset::Register(name,emoji,desc);
}

bool resource::texture::Register(AssetNameView name, const Icon& icon, int32 size, TextureDesc desc)
{
	return TextureAsset::Register(name,icon,size,desc);
}

bool resource::texture::Register(AssetNameView name, std::unique_ptr<TextureAssetData>&& data)
{
	return TextureAsset::Register(name,std::move(data));
}


bool resource::texture::Register(AssetNameAndTags nameAndTags, FilePathView path, TextureDesc desc)
{	
	return TextureAsset::Register(nameAndTags,AssetManager::myAsset(path),desc);
}

bool resource::texture::Register(AssetNameAndTags nameAndTags, FilePathView rgb, FilePathView alpha, TextureDesc desc)
{
	
	return TextureAsset::Register(nameAndTags,rgb,alpha,desc);
}

bool resource::texture::Register(AssetNameAndTags nameAndTags, const Color& rgb, FilePathView alpha, TextureDesc desc)
{
	
	return TextureAsset::Register(nameAndTags,rgb,alpha,desc);
}

bool resource::texture::Register(AssetNameAndTags nameAndTags, const Emoji& emoji, TextureDesc desc)
{
	
	return TextureAsset::Register(nameAndTags,emoji,desc);
}

bool resource::texture::Register(AssetNameAndTags nameAndTags, const Icon& icon, int32 size, TextureDesc desc)
{
	
	return TextureAsset::Register(nameAndTags,icon,size,desc);
}

// AudioAsset

resource::audio::audio(AssetNameView name)
	:AudioAsset(name)
{

}

bool resource::audio::Load(AssetNameView name)
{
	return AudioAsset::Load(name);
}

bool resource::audio::Register(AssetNameView name, FilePathView path)
{
	
	
	return AudioAsset::Register(name, AssetManager::myAsset(path));
}

bool resource::audio::Register(AssetNameView name, FilePathView path, const Loop loop)
{
	
	
	return AudioAsset::Register(name,AssetManager::myAsset(path),loop);
}

bool resource::audio::Register(AssetNameView name, FilePathView path, Arg::loopBegin_<uint64> loopBegin)
{
	
	
	return AudioAsset::Register(name,AssetManager::myAsset(path),loopBegin);
}

bool resource::audio::Register(AssetNameView name, FilePathView path, Arg::loopBegin_<uint64> loopBegin, Arg::loopEnd_<uint64> loopEnd)
{
	
	
	return AudioAsset::Register(name,AssetManager::myAsset(path),loopBegin,loopEnd);
}

bool resource::audio::Register(AssetNameView name, FilePathView path, const Optional<AudioLoopTiming>& loop)
{
	
	
	return AudioAsset::Register(name,AssetManager::myAsset(path),loop);
}

bool resource::audio::Register(AssetNameView name, Audio::FileStreaming st, FilePathView path)
{
	
	
	return AudioAsset::Register(name, AssetManager::myAsset(path));
}

bool resource::audio::Register(AssetNameView name, Audio::FileStreaming st, FilePathView path, Loop loop)
{
	
	
	return AudioAsset::Register(name,st,AssetManager::myAsset(path),loop);
}

bool resource::audio::Register(AssetNameView name, Audio::FileStreaming st, FilePathView path, Arg::loopBegin_<uint64> loopBegin)
{
	
	
	return AudioAsset::Register(name,st,AssetManager::myAsset(path),loopBegin);
}

bool resource::audio::Register(AssetNameView name, GMInstrument instrument, uint8 key, const Duration& duration, double velocity, Arg::sampleRate_<uint32> sampleRate)
{
	
	return AudioAsset::Register(name,instrument,key,duration,velocity,sampleRate);
}

bool resource::audio::Register(AssetNameView name, GMInstrument instrument, uint8 key, const Duration& noteOn, const Duration& noteOff, double velocity, Arg::sampleRate_<uint32> sampleRate)
{
	
	return AudioAsset::Register(name,instrument,key,noteOn,noteOff,velocity,sampleRate);
}

bool resource::audio::Register(AssetNameView name, std::unique_ptr<AudioAssetData>&& data)
{
	
	return AudioAsset::Register(name, std::move(data));
}


bool resource::audio::Register(AssetNameAndTags nameAndTag, FilePathView path)
{
	
	
	return AudioAsset::Register(nameAndTag, AssetManager::myAsset(path));
}

bool resource::audio::Register(AssetNameAndTags nameAndTag, FilePathView path, const Loop loop)
{
	
	
	return AudioAsset::Register(nameAndTag,AssetManager::myAsset(path),loop);
}

bool resource::audio::Register(AssetNameAndTags nameAndTag, FilePathView path, Arg::loopBegin_<uint64> loopBegin)
{
	return AudioAsset::Register(nameAndTag,AssetManager::myAsset(path),loopBegin);
}

bool resource::audio::Register(AssetNameAndTags nameAndTag, FilePathView path, Arg::loopBegin_<uint64> loopBegin, Arg::loopEnd_<uint64> loopEnd)
{
	
	
	return AudioAsset::Register(nameAndTag,AssetManager::myAsset(path),loopBegin,loopEnd);
}

bool resource::audio::Register(AssetNameAndTags nameAndTag, FilePathView path, const Optional<AudioLoopTiming>& loop)
{
	
	
	return AudioAsset::Register(nameAndTag,AssetManager::myAsset(path),loop);
}

bool resource::audio::Register(AssetNameAndTags nameAndTag, Audio::FileStreaming st, FilePathView path)
{
	
	
	return AudioAsset::Register(nameAndTag,st, AssetManager::myAsset(path));
}

bool resource::audio::Register(AssetNameAndTags nameAndTag, Audio::FileStreaming st, FilePathView path, Loop loop)
{
	
	
	return AudioAsset::Register(nameAndTag,st,AssetManager::myAsset(path),loop);
}

bool resource::audio::Register(AssetNameAndTags nameAndTag, Audio::FileStreaming st, FilePathView path, Arg::loopBegin_<uint64> loopBegin)
{
	
	
	return AudioAsset::Register(nameAndTag,st,AssetManager::myAsset(path),loopBegin);
}

bool resource::audio::Register(AssetNameAndTags nameAndTag, GMInstrument instrument, uint8 key, const Duration& duration, double velocity, Arg::sampleRate_<uint32> sampleRate)
{
	
	return AudioAsset::Register(nameAndTag,instrument,key,duration,velocity,sampleRate);
}

bool resource::audio::Register(AssetNameAndTags nameAndTag, GMInstrument instrument, uint8 key, const Duration& noteOn, const Duration& noteOff, double velocity, Arg::sampleRate_<uint32> sampleRate)
{
	
	return AudioAsset::Register(nameAndTag,instrument,key,noteOn,noteOff,velocity,sampleRate);
}

String RegisterAssets::operator()(const String& name)
{
	switch (type)
	{
	case AssetType::none:
		throw Error{ U"typeを設定してください" };
	case AssetType::texture:
		textures.emplace(name);
		break;
	case AssetType::font:
		fonts.emplace(name);
		break;
	case AssetType::audio:
		audios.emplace(name);
		break;
	}
	return name;
}
