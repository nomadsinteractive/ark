#ifndef ARK_APP_IMPL_PARTITION_KDTREE_PARTITION_H_
#define ARK_APP_IMPL_PARTITION_KDTREE_PARTITION_H_

#include <vector>
#include <unordered_map>

#include <nanoflann.hpp>

#include "core/forwarding.h"
#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"
#include "core/types/owned_ptr.h"

#include "app/inf/partition.h"

namespace ark {

class KDTreePartition : public Partition {
public:
    KDTreePartition();

    virtual uint32_t addPoint(const sp<VV>& pos) override;
    virtual void updatePoint(uint32_t id) override;
    virtual void removePoint(uint32_t id) override;
    virtual List<uint32_t> within(float x, float y, float radius) override;

public:
//  [[plugin::builder("kdtree")]]
    class BUILDER : public Builder<Partition> {
    public:
        BUILDER();

        virtual sp<Partition> build(const sp<Scope>& args) override;

    };

private:
    struct Adapter {

        struct Point {
            Point(uint32_t id, const sp<VV>& position);
            Point(const Point& other) = default;

            void update();
            void dispose();

            uint32_t _id;
            sp<VV> _position;
            float _v[2];
            bool _disposed;
        };

        std::vector<Point> pts;

        inline size_t kdtree_get_point_count() const { return pts.size(); }
        inline float kdtree_get_pt(const size_t idx, int dim) const { return pts[idx]._v[dim]; }
        template <class BBOX> bool kdtree_get_bbox(BBOX& /* bb */) const { return false; }

    };

	typedef nanoflann::KDTreeSingleIndexDynamicAdaptor<
		nanoflann::L2_Simple_Adaptor<float, Adapter>,
		Adapter, 2, uint32_t> kd_tree_t;

private:
	Adapter::Point& getPoint(uint32_t id);

	void buildIndex();
	bool rebuildIfNeeded();

	Adapter _adapter;
	op<kd_tree_t> _tree;

	uint32_t _id_generator;
	uint32_t _updated_count;
	std::unordered_map<uint32_t, uint32_t> _id_map;
};

}

#endif
