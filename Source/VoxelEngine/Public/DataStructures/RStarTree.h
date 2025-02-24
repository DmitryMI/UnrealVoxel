#pragma once

#define BOOST_GEOMETRY_NO_ROBUSTNESS

THIRD_PARTY_INCLUDES_START
#pragma push_macro("CONSTEXPR")
#undef CONSTEXPR
#pragma push_macro("dynamic_cast")
#undef dynamic_cast
#pragma push_macro("check")
#undef check
#pragma push_macro("PI")
#undef PI

#include<boost/geometry.hpp>
#include<boost/geometry/geometries/point.hpp>
#include<boost/geometry/geometries/box.hpp>
#include<boost/geometry/index/rtree.hpp>

#pragma pop_macro("PI")
#pragma pop_macro("check")
#pragma pop_macro("dynamic_cast")
#pragma pop_macro("CONSTEXPR")
THIRD_PARTY_INCLUDES_END

namespace VoxelEngine::DataStructures
{
	template<typename T>
	class TRStarTree
	{

	};
}