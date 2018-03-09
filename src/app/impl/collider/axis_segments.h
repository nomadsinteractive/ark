#ifndef ARK_APP_IMPL_COLLIDER_AXIS_SEGMENTS_H_
#define ARK_APP_IMPL_COLLIDER_AXIS_SEGMENTS_H_

#include <set>
#include <unordered_map>
#include <vector>

namespace ark {

class AxisSegments {
public:
    AxisSegments();

    void insert(uint32_t id, float position, float radius);
    void update(uint32_t id, float position, float radius);
    void remove(uint32_t id);

    std::set<uint32_t> findCandidates(float p1, float p2) const;

private:
    struct Segment {
        Segment(float position);
        Segment(uint32_t rigidBodyId, float position, float radius);
        Segment(const Segment& other);

        bool operator < (const Segment& other) const;

        void update(float position, float radius);

        uint32_t _id;
        float _position;
        float _radius;
    };

    void updateSearchRadius();
    std::vector<Segment>::iterator findSegmentById(uint32_t id);

private:
    std::vector<Segment> _blocks;
    std::unordered_map<uint32_t, float> _positions;
    float _search_radius;
};

}

#endif
