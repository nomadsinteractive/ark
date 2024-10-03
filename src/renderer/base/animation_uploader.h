#pragma once

#include "core/base/api.h"
#include "core/inf/uploader.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

typedef std::vector<M4> AnimationFrame;

//[[script::bindings::extends(Uploader)]]
class ARK_API AnimationUploader final : public Uploader {
public:
    AnimationUploader(sp<Numeric> duration, uint32_t durationInTicks, const sp<Table<String, uint32_t>>& nodeIds, const sp<std::vector<AnimationFrame>>& animationFrames);

//  [[script::bindings::auto]]
    sp<Mat4> getNodeMatrix(const String& name);
//  [[script::bindings::auto]]
    std::vector<V3> getNodeDisplacements(const String& nodeName, const V3& origin) const;

    bool update(uint64_t timestamp) override;
    void upload(Writable& buf) override;

public:
    struct Stub {
        Stub(sp<Numeric> tick, uint32_t durationInTicks, const sp<Table<String, uint32_t>>& nodes, const sp<std::vector<AnimationFrame>>& animationFrames);

        bool update(uint64_t timestamp);
        void flat(void* buf);

        const M4* getFrameInput() const;

        uint32_t _duration_in_ticks;

        sp<Numeric> _tick;

        sp<Table<String, uint32_t>> _nodes;
        sp<std::vector<AnimationFrame>> _animation_frames;

        uint32_t _frame_index;
    };

private:
    sp<Stub> _stub;
};

}
