#pragma once

#include <iterator>
#include <unordered_set>

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/v4.h"

#include "app/forwarding.h"

namespace ark {

class ARK_API BroadPhrase {
public:
    typedef uintptr_t CandidateIdType;

    struct Candidate {
        Candidate() = default;
        Candidate(CandidateIdType id, const V2& position, const V4& quaternion, const HashId shapeId, sp<CollisionFilter> collisionFilter, Box bodyDef)
            : _id(id), _position(position), _quaternion(quaternion), _shape_id(shapeId), _collision_filter(std::move(collisionFilter)), _body_def(std::move(bodyDef)) {
        }

        CandidateIdType _id;
        V2 _position;
        V4 _quaternion;
        HashId _shape_id;
        sp<CollisionFilter> _collision_filter;
        Box _body_def;
    };

    struct Result {
        Result() = default;
        Result(std::unordered_set<CandidateIdType> dynamicCandidates, std::vector<Candidate> staticCandidates)
            : _dynamic_candidates(std::move(dynamicCandidates)), _static_candidates(std::move(staticCandidates)) {
        }

        void merge(const Result& other) {
            std::copy(other._dynamic_candidates.begin(), other._dynamic_candidates.end(), std::inserter(_dynamic_candidates, _dynamic_candidates.begin()));
            std::copy(other._static_candidates.begin(), other._static_candidates.end(), std::back_inserter(_static_candidates));
        }

        std::unordered_set<CandidateIdType> _dynamic_candidates;
        Vector<Candidate> _static_candidates;
    };

public:
    virtual ~BroadPhrase() = default;

    virtual void create(CandidateIdType id, const V3& position, const V3& aabb) = 0;
    virtual void update(CandidateIdType id, const V3& position, const V3& aabb) = 0;
    virtual void remove(CandidateIdType id) = 0;

    virtual Result search(const V3& position, const V3& aabb) = 0;
    virtual Result rayCast(const V3& from, const V3& to, const sp<CollisionFilter>& collisionFilter = nullptr) = 0;

};

}
