#pragma once
#include "range/v3/all.hpp"

#include <optional>
#include <queue>

template <typename... Ts> struct overloaded : Ts... { using Ts::operator()...;  };
template <typename... Ts> overloaded(Ts...)->overloaded<Ts...>;

namespace yc
{
	template <typename T>
	static std::optional<T> val_pop(std::queue<std::optional<T>>& o)
	{
		auto r = o.empty() ? std::nullopt : o.back();
		if(r.has_value()) o.pop();
		return r;
	}

	struct time_val_t
	{
		float t = 0;
		float rate;
		bool once_f;

		void update(const float _t) { t += _t; }
		void reset() { t = 0; }

		time_val_t(float r_) :
			t(0), rate(r_), once_f(true)
		{}

		template <typename F>
		void timer_end(F f, float ut)
		{
			update(ut);
			if (t >= rate)
			{
				reset();
				f();
			}
		}
		template <typename F>
		void timer_start(F f)
		{
			if (once_f)
			{
				f();
				once_f = false;
			}
		}
	};

	template< class, class = std::void_t<> > 
	struct needs_unapply : std::true_type { };

	template< class T > 
	struct needs_unapply<T, std::void_t<decltype(std::declval<T>()())>> : std::false_type { };

	template <typename F> auto
	curry(F&& f) {
		/// Check if f() is a valid function call. If not we need 
		/// to curry at least one argument:
		if constexpr (needs_unapply<decltype(f)>::value) {
			return [=](auto&& x) {
				return curry(
					[=](auto&&...xs) -> decltype(f(x, xs...)) {
						return f(x, xs...);
					}
				);
			};
		}
		else {
			/// If 'f()' is a valid call, just call it, we are done.
			return f();
		}
	}

	/*
	int main()
	{
		auto f = [](auto a, auto b, auto c, auto d) {
			return a * b * c * d;
		};

		return curry(f)(1)(2)(3)(4);
	}
	*/


}

namespace yc_range
{
	template<typename T>
	concept is_stl_range = requires { typename T::iterator; };

	template <is_stl_range T1, typename T2>
	static void remove_if(T1& container, T2 condition)
	{
		container.erase(std::remove_if(container.begin(), container.end(), condition), container.end());
	}

	template <is_stl_range T1, typename T2>
	static void for_each(T1& container, T2 item)
	{
		std::for_each(container.begin(), container.end(), item);
	}

	template <is_stl_range T1, typename T2>
	static auto find_if(T1& container, T2 item)
	{
		return std::find_if(container.begin(),
					        container.end(),
					        item);
	}

	template <is_stl_range T1, typename T2>
	static bool contains_key(T1& container, T2& key)
	{
		return container.find(key) != container.end();
	}

	template <is_stl_range T1, typename T2>
	static auto map(T1& container, T2 item)
	{
		return container | ::ranges::views::transform(item) | ::ranges::to_vector;
	}

	template <is_stl_range T1, typename T2>
	static auto fliter(T1& container, T2 item)
	{
		return container | ::ranges::views::filter(item) | ::ranges::to_vector;
	}
}

#define lamda(val, condition) [&](const auto& val) { return condition; }