#ifndef ARK_APP_INF_BROAD_PHRASE_H_
#define ARK_APP_INF_BROAD_PHRASE_H_

#include <map>
#include <iterator>
#include <unordered_set>
#include <vector>

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/v3.h"

#include "app/forwarding.h"

namespace ark {

class ARK_API BroadPhrase {
public:
    struct Candidate {
        Candidate() = default;
        Candidate(int32_t id, const V2& position, float rotation, int32_t shapeId, sp<CollisionFilter> collisionFilter, std::vector<Box> shapes)
            : id(id), position(position), rotation(rotation), shape_id(shapeId), _collision_filter(std::move(collisionFilter)), shapes(std::move(shapes)) {
        }

        int32_t id;
        V2 position;
        float rotation;
        int32_t shape_id;
        sp<CollisionFilter> _collision_filter;
        std::vector<Box> shapes;
    };

    struct Result {
        Result() = default;
        Result(std::unordered_set<int32_t> dynamicCandidates, std::vector<Candidate> staticCandidates)
            : dynamic_candidates(std::move(dynamicCandidates)), static_candidates(std::move(staticCandidates)) {
        }

        void merge(const Result& other) {
            std::copy(other.dynamic_candidates.begin(), other.dynamic_candidates.end(), std::inserter(dynamic_candidates, dynamic_candidates.begin()));
            std::copy(other.static_candidates.begin(), other.static_candidates.end(), std::back_inserter(static_candidates));
        }

        std::unordered_set<int32_t> dynamic_candidates;
        std::vector<Candidate> static_candidates;
    };

public:
    virtual ~BroadPhrase() = default;

    virtual void create(int32_t id, const V3& position, const V3& aabb) = 0;
    virtual void update(int32_t id, const V3& position, const V3& aabb) = 0;
    virtual void remove(int32_t id) = 0;

    virtual Result search(const V3& position, const V3& aabb) = 0;
    virtual Result rayCast(const V3& from, const V3& to) = 0;

};

}

#endif
