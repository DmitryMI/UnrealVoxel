#pragma once

#define BOOST_GEOMETRY_NO_ROBUSTNESS
#define _SECURE_SCL 0
#define _HAS_ITERATOR_DEBUGGING 0

#include <cstdint>

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
	public:
		constexpr static size_t NodeMaxElements = 16;
		using RealType = double;
		using PointType = boost::geometry::model::point<RealType, 3, boost::geometry::cs::cartesian>;
		using BoxType = boost::geometry::model::box<PointType>;
		using InternalValueType = std::pair<BoxType, T>;
		using ExternalValueType = TPair<FBox, T>;
		using RTreeType = boost::geometry::index::rtree<InternalValueType, boost::geometry::index::rstar<NodeMaxElements>>;

		void Insert(const FBox& BoundingBox, const T& Item)
		{
			RTree.insert(std::make_pair(ToBoost(BoundingBox), Item));
		}

		void Remove(const FBox& BoundingBox, const T& Item)
		{
			RTree.remove(std::make_pair(ToBoost(BoundingBox), Item));
		}

		TArray<ExternalValueType> Query(const FVector& Point)
		{
			std::vector<InternalValueType> ResultInternal;
			PointType PointInternal = ToBoost(Point);
			RTree.query(boost::geometry::index::intersects(PointInternal), std::back_inserter(ResultInternal));

			TArray<ExternalValueType> ResultExternal;
			ResultExternal.Reserve(ResultInternal.size());
			for (const auto& Pair : ResultInternal)
			{
				ExternalValueType PairExternal(ToUnreal(Pair.first), Pair.second);
				ResultExternal.Add(PairExternal);
			}

			return ResultExternal;
		}

	private:
		RTreeType RTree;

		static PointType ToBoost(const FVector& UnrealVector)
		{
			return PointType(UnrealVector.X, UnrealVector.Y, UnrealVector.Z);
		}

		static BoxType ToBoost(const FBox& UnrealBox)
		{
			return BoxType(ToBoost(UnrealBox.Min), ToBoost(UnrealBox.Max));
		}

		static FVector ToUnreal(const PointType& BoostPoint)
		{
			return FVector(BoostPoint.get<0>(), BoostPoint.get<1>(), BoostPoint.get<2>());
		}

		static FBox ToUnreal(const BoxType& BoostBox)
		{
			FVector Min = ToUnreal(BoostBox.min_corner());
			FVector Max = ToUnreal(BoostBox.max_corner());
			return FBox(Min, Max);
		}
	};
}