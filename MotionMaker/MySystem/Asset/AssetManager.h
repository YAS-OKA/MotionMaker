#pragma once

namespace resource
{
	/// @brief 各Assetのレジスタークラス　filenameを修飾する
	class font:public FontAsset
	{
	public:
		explicit font(AssetNameView name);

		static bool Load(AssetNameView name, const String& preloadText = U"");

		static bool Register(AssetNameView name, int32 fontSize, FilePathView path, FontStyle style = FontStyle::Default);

		static bool Register(AssetNameView name, int32 fontSize, FilePathView path, size_t faceIndex, FontStyle style = FontStyle::Default);

		static bool Register(AssetNameView name, int32 fontSize, Typeface typeface = Typeface::Regular, FontStyle style = FontStyle::Default);

		static bool Register(AssetNameView name, FontMethod fontMethod, int32 fontSize, FilePathView path, FontStyle style = FontStyle::Default);

		static bool Register(AssetNameView name, FontMethod fontMethod, int32 fontSize, FilePathView path, size_t faceIndex, FontStyle style = FontStyle::Default);

		static bool Register(AssetNameView name, FontMethod fontMethod, int32 fontSize, Typeface typeface = Typeface::Regular, FontStyle style = FontStyle::Default);

		static bool Register(AssetNameView name, std::unique_ptr<FontAssetData>&& data);


		static bool Register(AssetNameAndTags nameAndTags, int32 fontSize, FilePathView path, FontStyle style = FontStyle::Default);

		static bool Register(AssetNameAndTags nameAndTags, int32 fontSize, FilePathView path, size_t faceIndex, FontStyle style = FontStyle::Default);

		static bool Register(AssetNameAndTags nameAndTags, int32 fontSize, Typeface typeface = Typeface::Regular, FontStyle style = FontStyle::Default);

		static bool Register(AssetNameAndTags nameAndTags, FontMethod fontMethod, int32 fontSize, FilePathView path, FontStyle style = FontStyle::Default);

		static bool Register(AssetNameAndTags nameAndTags, FontMethod fontMethod, int32 fontSize, FilePathView path, size_t faceIndex, FontStyle style = FontStyle::Default);

		static bool Register(AssetNameAndTags nameAndTags, FontMethod fontMethod, int32 fontSize, Typeface typeface = Typeface::Regular, FontStyle style = FontStyle::Default);
	};
	class texture :public TextureAsset
	{
	public:
		explicit texture(AssetNameView name);
		/// @brief テクスチャアセットを登録します。
		/// @param name テクスチャアセットの登録名
		/// @param path 登録するテクスチャのファイルパス
		/// @param desc テクスチャの設定
		/// @return 登録に成功した場合 true, それ以外の場合は false

		static bool Load(AssetNameView name);
		
		static bool Register(AssetNameView name, FilePathView path, TextureDesc desc = TextureDesc::Unmipped);

		static bool Register(AssetNameView name, FilePathView rgb, FilePathView alpha, TextureDesc desc = TextureDesc::Unmipped);

		static bool Register(AssetNameView name, const Color& rgb, FilePathView alpha, TextureDesc desc = TextureDesc::Unmipped);

		/// @brief テクスチャアセットを登録します。
		/// @param name テクスチャアセットの登録名
		/// @param emoji 登録する絵文字
		/// @param desc テクスチャの設定
		/// @return 登録に成功した場合 true, それ以外の場合は false
		static bool Register(AssetNameView name, const Emoji& emoji, TextureDesc desc = TextureDesc::Mipped);

		static bool Register(AssetNameView name, const Icon& icon, int32 size, TextureDesc desc = TextureDesc::Unmipped);

		static bool Register(AssetNameView name, std::unique_ptr<TextureAssetData>&& data);


		static bool Register(AssetNameAndTags nameAndTags, FilePathView path, TextureDesc desc = TextureDesc::Unmipped);

		static bool Register(AssetNameAndTags nameAndTags, FilePathView rgb, FilePathView alpha, TextureDesc desc = TextureDesc::Unmipped);

		static bool Register(AssetNameAndTags nameAndTags, const Color& rgb, FilePathView alpha, TextureDesc desc = TextureDesc::Unmipped);

		static bool Register(AssetNameAndTags nameAndTags, const Emoji& emoji, TextureDesc desc = TextureDesc::Mipped);

		static bool Register(AssetNameAndTags nameAndTags, const Icon& icon, int32 size, TextureDesc desc = TextureDesc::Unmipped);
	};
	class audio:public AudioAsset
	{
	public:
		explicit audio(AssetNameView name);

		static bool Load(AssetNameView name);

		static bool Register(AssetNameView name, FilePathView path);

		static bool Register(AssetNameView name, FilePathView path, const Loop loop);

		static bool Register(AssetNameView name, FilePathView path, Arg::loopBegin_<uint64> loopBegin);

		static bool Register(AssetNameView name, FilePathView path, Arg::loopBegin_<uint64> loopBegin, Arg::loopEnd_<uint64> loopEnd);

		static bool Register(AssetNameView name, FilePathView path, const Optional<AudioLoopTiming>& loop);

		static bool Register(AssetNameView name, Audio::FileStreaming st, FilePathView path);

		static bool Register(AssetNameView name, Audio::FileStreaming st, FilePathView path, Loop loop);

		static bool Register(AssetNameView name, Audio::FileStreaming st, FilePathView path, Arg::loopBegin_<uint64> loopBegin);

		static bool Register(AssetNameView name, GMInstrument instrument, uint8 key, const Duration& duration, double velocity = 1.0, Arg::sampleRate_<uint32> sampleRate = Wave::DefaultSampleRate);

		static bool Register(AssetNameView name, GMInstrument instrument, uint8 key, const Duration& noteOn, const Duration& noteOff, double velocity = 1.0, Arg::sampleRate_<uint32> sampleRate = Wave::DefaultSampleRate);

		static bool Register(AssetNameView name, std::unique_ptr<AudioAssetData>&& data);


		static bool Register(AssetNameAndTags nameAndTag, FilePathView path);

		static bool Register(AssetNameAndTags nameAndTag, FilePathView path, const Loop loop);

		static bool Register(AssetNameAndTags nameAndTag, FilePathView path, Arg::loopBegin_<uint64> loopBegin);

		static bool Register(AssetNameAndTags nameAndTag, FilePathView path, Arg::loopBegin_<uint64> loopBegin, Arg::loopEnd_<uint64> loopEnd);

		static bool Register(AssetNameAndTags nameAndTag, FilePathView path, const Optional<AudioLoopTiming>& loop);

		static bool Register(AssetNameAndTags nameAndTag, Audio::FileStreaming st, FilePathView path);

		static bool Register(AssetNameAndTags nameAndTag, Audio::FileStreaming st, FilePathView path, Loop loop);

		static bool Register(AssetNameAndTags nameAndTag, Audio::FileStreaming st, FilePathView path, Arg::loopBegin_<uint64> loopBegin);

		static bool Register(AssetNameAndTags nameAndTag, GMInstrument instrument, uint8 key, const Duration& duration, double velocity = 1.0, Arg::sampleRate_<uint32> sampleRate = Wave::DefaultSampleRate);

		static bool Register(AssetNameAndTags nameAndTag, GMInstrument instrument, uint8 key, const Duration& noteOn, const Duration& noteOff, double velocity = 1.0, Arg::sampleRate_<uint32> sampleRate = Wave::DefaultSampleRate);
	};

	enum Mode {
		debug, relase
	};
}

/// @brief リソース管理を行う 
class AssetManager
{
public:
	/// @brief デバッグかリリースか　開発中はデバッグにしておく
	static resource::Mode mode;

private:
	AssetManager() {};
	~AssetManager() {};
public:
	/// @brief パスをリリース用に修飾する
	static FilePath myAsset(FilePathView path);
};

//アセットの名前を記憶しておくクラス
class RegisterAssets
{
public:
	RegisterAssets()
		:type(AssetType::none)
	{

	}

	//使い方-> resource::texture(r(U"ex"),U"example/ex.png");
	String operator ()(const String& name);

	enum class AssetType {
		texture,
		audio,
		font,
		none
	}type;

	HashSet<String> textures;
	HashSet<String> audios;
	HashSet<String> fonts;
};
