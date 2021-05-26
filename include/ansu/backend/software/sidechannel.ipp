
#pragma once

#include <queue>

namespace polyfill
{
	template <typename T>
	struct sidechannel
	{
		T data;
		bool last = false;

		sidechannel() {}
		sidechannel(const T& dt) : data(dt) {}
		sidechannel(T&& dt) : data(std::move(dt)) {}
		//		sidechannel(T t) : data(t) {}
		//		sidechannel(const T& t) : data(t) {}
		//		sidechannel(T&& t) : data(std::move(t)) {}
		operator T() const { return data; }
	};

} // namespace polyfill

namespace ANS
{
	namespace backend
	{
		template <typename T>
		using side = polyfill::sidechannel<T>;
	}
} // namespace ANS
