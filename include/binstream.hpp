#include <queue>

namespace polyfill
{
	template <typename T>
	struct binstream
	{
		std::queue<T> backend;

		bool empty()
		{
			return backend.empty();
		}

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
}

namespace hls
{
	template <typename T>
	using stream = polyfill::binstream<T>;
}
