#pragma once
#include<fstream>
#include<string>

namespace util
{
	//名前簡略化
	template<class T> using uPtr = std::unique_ptr<T>;
	template<class T> using sPtr = std::shared_ptr<T>;
	template<class T> using wPtr = std::weak_ptr<T>;

	constexpr auto sc = s3d::Scene::Center;
	constexpr auto sw = s3d::Scene::Width;
	constexpr auto sh = s3d::Scene::Height;

	//極座標
	inline Vec2 polar(double rad, double len = 1)
	{
		return len * Vec2{ Cos(rad),Sin(rad) };
	}
	//ベクトル内積
	inline double dotProduct(Vec2 vl, Vec2 vr) {
		return vl.x * vr.x + vl.y * vr.y;
	}

	inline double angleOf2Vec(Vec2 A, Vec2 B)
	{
		//内積とベクトル長さを使ってcosθを求める
		double cos_sita = dotProduct(A, B) / (A.length() * B.length());

		//cosθからθを求める
		return Acos(cos_sita);
	}
	//x,yをひっくり返す
	inline Vec2 invXY(const Vec2& p)
	{
		return { p.y,p.x };
	}

	inline double getRad(const Vec2& p)
	{
		return p.getAngle() - 90_deg;
	}

	template<class T>
	inline Array<T> slice(Array<T> arr, int32 a, int32 b)
	{
		Array<T> ret{};
		for (auto itr = arr.begin() + a, en = arr.begin() + b; itr != en; ++itr)ret << *itr;
		return ret;
	}


	inline String slice(StringView arr, size_t a, size_t b)
	{
		String ret = U"";
		for (auto itr = arr.begin() + a, en = arr.begin() + b; itr != en; ++itr)ret << *itr;
		return ret;
	}
	//配列に配列を挿入する(i番目に挿入する)
	template<class A>
	inline void append(Array<A>& arr, Array<A> _ins, size_t i)
	{
		for (auto itr = _ins.rbegin(), en = _ins.rend(); itr != en; ++itr)
		{
			arr.insert(arr.begin() + i, *itr);
		}
	}

	inline bool strEqual(StringView str, size_t from, size_t to, StringView cstr)
	{
		auto s = util::slice(str, from, to);

		if (not s.isEmpty() and s == cstr)return true;
		return false;
	}

	template<class T, class Fty, std::enable_if_t<std::is_invocable_r_v<double, Fty, T>>* = nullptr>
	inline T GetMax(Array<T> arr, Fty f, bool frontSidePrior = true)
	{
		if (arr.isEmpty())throw Error{ U"arrayが空でした。" };
		const auto comparing = frontSidePrior ? [](double v1, double v2) {return v1 < v2; } : [](double v1, double v2) {return v1 <= v2; };
		T* res = &arr[0];
		double value = f(*res);
		for (auto& elm : arr)
		{
			const auto elmValue = f(elm);
			if (comparing(value, elmValue))
			{
				res = &elm;
				value = elmValue;
			}
		}
		return *res;
	}

	//配列をもとに別の配列を生成
	template<class T, class U, class Fty, std::enable_if_t<std::is_invocable_r_v<U, Fty, T>>* = nullptr>
	inline Array<T> Generate(Array<U> arr, Fty f)
	{
		Array<T> result;
		std::transform(arr.begin(), arr.end(), std::back_inserter(result), f);
		return result;
	}


	//上限で止まる
	struct StopMax
	{
		double value;
		double max;

		StopMax() {};

		StopMax(double max, double value = 0);

		void add(double value);

		bool additionable()const;
	};

	std::string toStr(StringView s);

	String toUStr(const std::string& s);

	class EditFile
	{
	private:
		uPtr<std::fstream> file;		
	public:

		EditFile() = default;

		EditFile(FilePathView path);
		//上書き
		void overwrite(StringView text);
		//fileの中身をすべて読みだす
		String readAll()const;
	};

	EditFile createFile(FilePath path);

}
