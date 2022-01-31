#pragma once

namespace math
{

template<class T>
auto max(T a, T b)
{
	return b < a ? a : b;
}

template<class T>
auto min(T a, T b)
{
	return a < b ? a : b;
}

}