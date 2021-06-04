#pragma once

#include "ints.hpp"

#include <algorithm>
#include <queue>

namespace polyfill
{
	template <typename T>
	struct binstream
	{
		std::queue<T> backend;

		bool empty() { return backend.empty(); }

		T read()
		{
			T val;
			*this >> val;
			return val;
		}
	};

	template <typename T>
	binstream<T>& operator>>(binstream<T>& str, T& val)
	{
		val = str.backend.front();
		str.backend.pop();
		return str;
	}

	template <typename T>
	binstream<T>& operator<<(binstream<T>& str, T val)
	{
		str.backend.push(val);
		return str;
	}
} // namespace polyfill

namespace ANS
{
	namespace backend
	{
		template <typename T>
		using stream = polyfill::binstream<T>;

		template <typename T>
		stream<T> streamFromBuffer(T* buf, u64 count)
		{
			auto ret	= stream<T>();
			ret.backend = std::queue<T>(std::deque<T>(buf, buf + count));
			return ret;
		}

		template <typename T>
		stream<T> streamFromBufferReverse(T* buf, u64 count)
		{
			auto ret		= stream<T>();
			auto collection = std::deque<T>(buf, buf + count);
			std::reverse(collection.begin(), collection.end());
			ret.backend = std::queue<T>(collection);
			return ret;
		}
	} // namespace backend

} // namespace ANS
