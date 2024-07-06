#pragma once
#include "Action.h"
#include"Condition.h"

namespace prg
{
	//こいつは子アクションの所有権持ってるので気を付けて
	class Actions :public IAction
	{
	public:
		Actions(StringView id=U"");

		template<class Act, std::enable_if_t<std::is_base_of_v<IAction,Act>>* = nullptr >
		Actions(Act&& other)
			:IAction()
		{
			endCondition.addIn(U"isAllFinished",[borrowed = lend()] {return borrowed->isAllFinished(); });
			addAct(new Act(std::move(other)));
		}

		Actions(Actions&&) = default;

		Actions& operator=(Actions&&) = default;

		~Actions();

		void clear();
		
		void swapOne(size_t index,IAction* act);

		bool empty()const;

		template<class T = FuncAction, int32 Index = -1, class... Args>
		T& add(Args&&... args)
		{
			auto action = new T(args...);

			return addAct<T, Index>(action);
		}
		template<class T, int32 Index = -1>
		T& addAct(T* action, const String& id = U"")
		{
			//nを0~separate.size()内に収める
			int32 n = Index >= int32(separate.size()) ? separate.size() - 1 : Index;

			if (n < 0)n = separate.size() + n;

			_insert(n, action);

			separate << 0;

			for (auto it = separate.rbegin(), en = separate.rbegin() + (separate.size() - n - 1); it != en; ++it)
			{
				//一つ前の値+1して後ろにシフト
				*it = *(it + 1) + 1;
			}

			if(id!=U"")action->id = id;

			return *action;
		}
		template<class T = FuncAction, int32 Index = -1, class... Args>
		T& addParallel(Args&&... args)
		{
			auto action = new T(args...);

			return addActParallel<T, Index>(action);
		}
		template<class T, int32 Index = -1>
		T& addActParallel(T* action, const String& id = U"")
		{
			if (update_list.isEmpty() and Index == -1)return addAct<T, Index>(action);

			int32 n = Index >= int32(separate.size()) ? separate.size() - 1 : Index;

			if (n < 0)n = separate.size() + n;

			_insert(n, action);

			for (auto itr = separate.begin() + n, en = separate.end(); itr != en; ++itr)
			{
				++(*itr);
			}

			if (id != U"")action->id = id;

			return *action;
		}

		template<class T = IAction>
		T* getAction(StringView id)
		{
			//キャストに失敗したらnullptr
			for (auto& action : update_list)if (action->id == id)return dynamic_cast<T*>(action);

			//見つからなければ
			return nullptr;
		}

		Array<IAction*> getActions(StringView id);

		int32 getIndex(IAction* action);

		int32 getActiveIndex()const;

		int32 getActiveNum()const;

		Array<IAction*> getAll()const;

		Array<IAction*> getAction(const int32& index);

		IAction* getAction(const int32& index1, const int32& index2);

		IAction* operator[](const String& id);

		Array<IAction*> operator [](const int32& index);

		int32 getSepSize()const;

		int32 getLoopCount()const;
		/// @brief アクションをリセットしてから開始する
		void restart();

		virtual void start()override;
		/// @brief アクションを開始 startFirstActionsをtrueにすると、一番最初のアクションがこのstartの中で実行される
		void start(bool startFirstActions);
		/// @brief 指定したインデックスからアクションを開始
		void start(const int32& startIndex, bool startFirstAction);
		/// @brief 引数に渡したアクションがアクティブじゃなければ開始させる。
		void start(IAction* act);
		/// @brief アクションをリセット
		void reset()override;
		/// @brief アクションを終了する
		void end()override;
		/// @brief 引数に渡したアクションがアクティブならそのアクションを終了する。
		void end(IAction* act);

		bool isAllFinished();

		virtual void update(double dt)override;
		
		//終了したときに再開する
		bool loop = false;
		//停止する
		bool stopped = false;
		//ループカウントの上限　別にカンストしてもループは続く
		const int32 maxLoopCount = 10000;

	protected:
		void _insert(int32 septIndex, IAction* act);

		void _sort();

		virtual void _startCheck();

		void _update(double dt);

		virtual void _endCheck();

		bool _isEnded(std::tuple<int32, int32> se)const;

		std::tuple<int32, int32> _getArea(const int32& Index)const;

		int32 activeIndex = 0;
		//実行中のアクションの個数
		int32 activeNum = 0;
		//終了していないIActionがいくつあるか addしたときに++ updateでendのとき-- resetのとき=
		int32 notFinishedActNum = 0;

		Array<IAction*> update_list;
		//仕切り
		Array<int32> separate{ 0 };

		int32 loopCount = 0;
	};

	/*注意　基本的に親クラスのスタートは使わないほうが良いっぽい*/
	class StateActions :public Actions
	{
	public:
		StateActions(StringView id);

		void start()override;//親クラスのstartとはちょっと仕組みが違う

		void update(double dt)override;
		//from->toの条件
		IAction& relate(StringView from, StringView to);

		IAction& relate(Array<String> from, StringView to);

		IAction& relate(IAction* from_act, IAction* to_act);
		
		//重複可能
		void duplicatable(String a, String b);

		IAction& operator[](StringView name);

		Array<String> getAllState()const;

		HashSet<String> getState()const;

	protected:
		void _startCheck()override;

		HashSet<String> m_states;

		HashTable<String,HashSet<String>> canDuplicate;
	};
}

/******      以下はActionの演算子オーバーロード     *****/
using namespace prg;
//aとbを同時に開始するアクション　addParallel
template<class Act1, class Act2>
typename std::enable_if_t<!std::is_base_of_v<Actions, Act1>  && std::is_base_of_v<IAction, Act1>&& std::is_base_of_v<IAction, Act2>, Actions>
operator + (Act1&& a, Act2&& b)
{
	Actions actions;
	actions.addAct(new Act1(std::forward<Act1>(a)));
	actions.addActParallel(new Act2(std::forward<Act2>(b)));
	return actions;
}
template<class Act1,class Act2>
typename std::enable_if_t<std::is_base_of_v<Actions, Act1> && std::is_base_of_v<IAction, Act2>, Act1&&>
operator + (Act1& a, Act2&& b)
{
	a.addActParallel(new Act2(std::forward<Act2>(b)));
	return std::forward<Act1>(a);
}
template<class Act1, class Act2>
typename std::enable_if_t<std::is_base_of_v<Actions, Act1>&& std::is_base_of_v<IAction, Act2>, Act1&&>
operator + (Act1&& a, Act2&& b)
{
	a.addActParallel(new Act2(std::forward<Act2>(b)));
	return std::forward<Act1>(a);
}


//aからbに移り変わるアクション　add
template<class Act1, class Act2>
typename std::enable_if_t<!std::is_base_of_v<Actions,Act1> && std::is_base_of_v<IAction, Act1>&& std::is_base_of_v<IAction, Act2>, Actions>
operator >> (Act1&& a, Act2&& b)
{
	Actions actions;
	actions.addAct(new Act1(std::forward<Act1>(a)));
	actions.addAct(new Act2(std::forward<Act2>(b)));
	return actions;
}
template<class Act1, class Act2>
typename std::enable_if_t<std::is_base_of_v<Actions, Act1>&& std::is_base_of_v<IAction, Act2>, Act1>
operator >> (Act1&& a, Act2&& b)
{
	a.addAct(new Act2(std::forward<Act2>(b)));
	return std::forward<Act1>(a);
}

//template<class Act2>
//typename std::enable_if_t<std::is_base_of_v<IAction, Act2>, Actions&&>
//operator >> (Actions&& a, Act2&& b)
//{
//	a.addAct(new Act2(std::forward<Act2>(b)));
//	return std::forward<Actions>(a);
//}
template<class Act1, class Act2>
typename std::enable_if_t<std::is_base_of_v<Actions,Act1> && std::is_base_of_v<IAction, Act2>, void>
operator += (Act1& a, Act2&& b)
{
	a.addAct(new Act2(std::forward<Act2>(b)));
}
template<class Act1,class Act2>
typename std::enable_if_t<std::is_base_of_v<Actions, Act1>&& std::is_base_of_v<IAction, Act2>, void>
operator |= (Act1& a, Act2&& b)
{
	a.addActParallel(new Act2(std::forward<Act2>(b)));
}
