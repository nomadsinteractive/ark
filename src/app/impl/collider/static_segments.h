#ifndef ARK_APP_IMPL_COLLIDER_STATIC_SEGMENTS_H_
#define ARK_APP_IMPL_COLLIDER_STATIC_SEGMENTS_H_

#include <set>
#include <vector>

namespace ark {

class StaticSegments {
public:
    StaticSegments();

    void insert(uint32_t id, float position, float radius);
    void remove(uint32_t id);

    std::set<uint32_t> findCandidates(float p1, float p2);

private:
    void updateSearchRadius();

    struct Segment {
        Segment(float position);
        Segment(uint32_t rigidBodyId, float position, float radius);
        Segment(const Segment& other);

        bool operator < (const Segment& other) const;

        uint32_t _id;
        float _position;
        float _radius;
    };

private:
    std::vector<Segment> _blocks;
    float _search_radius;
};

}

#endif
