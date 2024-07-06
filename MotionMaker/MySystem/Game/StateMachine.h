#pragma once
#include"../Prg/Actions.h"

#define F(x) std::forward<StateActions>(x)

namespace state
{
	class IV {};

	template<class T>
	class Value :public IV
	{
	public:
		Value(const T& v)
			:v(v)
		{
		}
		T v;

		operator T ()
		{
			return v;
		}
	};

	class Info {
		std::shared_ptr<IV> v;
	public:
		bool valid = true;

		Info() {};

		template<class V>
		Info(const V& v)
			: v(std::shared_ptr<IV>(new Value<V>(v)))
		{
		}

		template<class V, std::size_t N, std::enable_if_t<!std::is_same_v<V, const char32_t>>* = nullptr>
		Info(V(&v)[N])
		{
			Array<V> vv{};
			for (size_t i = 0; i < N; i++)
			{
				vv.push_back(v[i]);
			}
			this->v = std::shared_ptr<IV>(new Value<Array<V>>(vv));
		}

		template<std::size_t N>
		Info(const char32_t(&v)[N])
		{
			String str;
			for (size_t i = 0; i < N; i++)
			{
				if (v[i] != U'\0')str += v[i];
			}
			this->v = std::shared_ptr<IV>(new Value<String>(str));
		}

		template<class V>
		Info& operator =(const V& v)
		{
			if (not this->v)
			{
				this->v = std::shared_ptr<IV>(new Value<V>(v));
			}
			else {
				static_cast<Value<V>*>(this->v.get())->v = v;
			}
			return *this;
		}

		template<class V>
		V getValue()
		{
			return *static_cast<Value<V>*>(v.get());
		}

		template<class V>
		operator V& ()
		{
			return *static_cast<Value<V>*>(v.get());
		};

		template<class V>
		Info& setValue(const V& v)
		{
			if (not this->v)
			{
				this->v = std::shared_ptr<IV>(new Value<V>(v));
			}
			else {
				static_cast<Value<V>*>(this->v.get())->v = v;
			}
			return *this;
		}
	};

	class Inform
	{
		HashTable<String, Info> info;
	public:
		Inform() = default;

		bool contains(StringView name);

		void set(StringView name, Info value);

		Info get(StringView name, Info&& default_value);
		Info& get(StringView name);
	};


	using In = Inform&;
	using A = StateActions&&;

	class StateCreator
	{
	public:
		StateCreator() = default;
		StateCreator(StringView name,
			const std::function<StateActions(In, StateActions&&)>& method = [](In, StateActions&& a) {return std::forward<StateActions>(a); })
			:method(method), name(name)
		{};

		StateCreator& operator = (const std::function<StateActions(In, StateActions&&)>& method)
		{
			this->method = method;
			return *this;
		}

		String name;

		std::function<StateActions(In, StateActions&&)> method;

		StateActions create(StringView name)const;

		StateActions operator()(In info) const
		{
			return F(method(info, std::move(create(name))));
		}

		StateActions operator()() const
		{
			auto info = Inform();
			return F(method(info, std::move(create(name))));
		}
	};

	class SCreatorContainer
	{
	public:
		HashTable<String, StateCreator> creators;

		bool contains(StringView name) { return creators.contains(name); }

		StateCreator& operator[](StringView name)
		{
			if (not creators.contains(name))creators.emplace(name, name);
			return creators[name];
		}

		const StateCreator& operator[](StringView name)const
		{
			return creators.at(name);
		}
	};

}
