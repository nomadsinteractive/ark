#ifndef ARK_APP_BASE_KDTREE_H_
#define ARK_APP_BASE_KDTREE_H_

#include <vector>
#include <unordered_set>

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/types/shared_ptr.h"
#include "core/types/owned_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

class ARK_API KDTree {
public:
    KDTree();

//  [[script::bindings::auto]]
    uint32_t addPoint(const sp<Vec>& pos);
//  [[script::bindings::auto]]
    void updatePoint(uint32_t id);
//  [[script::bindings::auto]]
    void removePoint(uint32_t id);
//  [[script::bindings::auto]]
    std::unordered_set<uint32_t> within(float x, float y, float radius);

private:
	void buildIndex();
	bool rebuildIfNeeded();

	struct Stub;
	op<Stub> _stub;

	uint32_t _updated_count;
    std::unordered_set<uint32_t> _id_recycler;
};

}

#endif
