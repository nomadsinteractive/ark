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
        Candidate(int32_t id, const V2& position, float rotation, int32_t shapeId, sp<CollisionFilter> collisionFilter, Box bodyDef)
            : _id(id), _position(position), _rotation(rotation), _shape_id(shapeId), _collision_filter(std::move(collisionFilter)), _body_def(std::move(bodyDef)) {
        }

        int32_t _id;
        V2 _position;
        float _rotation;
        int32_t _shape_id;
        sp<Tile> _tile;
        sp<CollisionFilter> _collision_filter;
        Box _body_def;
    };

    struct Result {
        Result() = default;
        Result(std::unordered_set<int32_t> dynamicCandidates, std::vector<Candidate> staticCandidates)
            : _dynamic_candidates(std::move(dynamicCandidates)), _static_candidates(std::move(staticCandidates)) {
        }

        void merge(const Result& other) {
            std::copy(other._dynamic_candidates.begin(), other._dynamic_candidates.end(), std::inserter(_dynamic_candidates, _dynamic_candidates.begin()));
            std::copy(other._static_candidates.begin(), other._static_candidates.end(), std::back_inserter(_static_candidates));
        }

        std::unordered_set<int32_t> _dynamic_candidates;
        std::vector<Candidate> _static_candidates;
    };

public:
    virtual ~BroadPhrase() = default;

    virtual void create(int32_t id, const V3& position, const V3& aabb) = 0;
    virtual void update(int32_t id, const V3& position, const V3& aabb) = 0;
    virtual void remove(int32_t id) = 0;

    virtual Result search(const V3& position, const V3& aabb) = 0;
    virtual Result rayCast(const V3& from, const V3& to, const sp<CollisionFilter>& collisionFilter = nullptr) = 0;

};

}

#endif
