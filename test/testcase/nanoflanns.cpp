#include "test/base/test_case.h"

#include <nanoflann.hpp>
#include "nanoflann_utils.h"
#include <ctime>
#include <cstdlib>
#include <iostream>

#include "core/collection/list.h"
#include "core/inf/variable.h"
#include "core/util/math.h"

#include "graphics/base/v2.h"

#include "app/base/kdtree.h"

using namespace std;
using namespace nanoflann;

template <typename num_t>
void kdtree_demo(const size_t N)
{
	PointCloud<num_t> cloud;

	// construct a kd-tree index:
	typedef KDTreeSingleIndexDynamicAdaptor<
		L2_Simple_Adaptor<num_t, PointCloud<num_t> > ,
		PointCloud<num_t>,
		3 /* dim */
		> my_kd_tree_t;

	dump_mem_usage();

	my_kd_tree_t   index(3 /*dim*/, cloud, KDTreeSingleIndexAdaptorParams(10 /* max leaf */) );

	// Generate points:
	generateRandomPointCloud(cloud, N);

	num_t query_pt[3] = { 0.5, 0.5, 0.5 };

	// add points in chunks at a time
	int chunk_size = 100;
	for(int i = 0; i < N; i = i + chunk_size)
	{
		size_t end = min(size_t(i + chunk_size), N - 1);
		// Inserts all points from [i, end]
		index.addPoints(i, end);
	}

	// remove a point
	size_t removePointIndex = N - 1;
	index.removePoint(removePointIndex);

	dump_mem_usage();
	{
		// do a knn search
		const size_t num_results = 1;
		size_t ret_index;
		num_t out_dist_sqr;
		nanoflann::KNNResultSet<num_t> resultSet(num_results);
		resultSet.init(&ret_index, &out_dist_sqr );
		index.findNeighbors(resultSet, query_pt, nanoflann::SearchParams(10));

		std::cout << "knnSearch(nn="<<num_results<<"): \n";
		std::cout << "ret_index=" << ret_index << " out_dist_sqr=" << out_dist_sqr << endl;
	}
	{
		// Unsorted radius search:
		const num_t radius = 1;
		std::vector<std::pair<size_t, num_t> > indices_dists;
		RadiusResultSet<num_t, size_t> resultSet(radius, indices_dists);

		index.findNeighbors(resultSet, query_pt, nanoflann::SearchParams());

		// Get worst (furthest) point, without sorting:
		std::pair<size_t,num_t> worst_pair = resultSet.worst_item();
		cout << "Worst pair: idx=" << worst_pair.first << " dist=" << worst_pair.second << endl;
	}
}

namespace ark {
namespace unittest {

class NanoFlannsTestCase : public TestCase {
public:
    virtual int launch() {
        srand(time(NULL));
        kdtree_demo<float>(10000);

        KDTree kdtree;
        for(int i = 0; i < 100; ++i)
        {
            uint32_t id = kdtree.addPoint(sp<VV2::Impl>::make(V2(Math::randf(), Math::randf())));
            kdtree.updatePoint(id);
        }

        for(int i = 0; i < 100; ++i)
        {
            uint32_t id = kdtree.addPoint(sp<VV2::Impl>::make(V2(Math::randf(), Math::randf())));
            kdtree.updatePoint(id);
            kdtree.removePoint(id);
        }
        uint32_t id = kdtree.addPoint(sp<VV2::Impl>::make(V2(-0.1f, 0.0f)));
        std::unordered_set<uint32_t> result = kdtree.within(-1.0f, 0.0f, 1.0f);

        if(result.size() != 1)
            return 1;

        return 0;
    }

};

}
}


ark::unittest::TestCase* nanoflanns_create() {
    return new ark::unittest::NanoFlannsTestCase();
}
