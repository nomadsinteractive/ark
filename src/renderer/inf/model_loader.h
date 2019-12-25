#ifndef ARK_RENDERER_INF_MODEL_LOADER_H_
#define ARK_RENDERER_INF_MODEL_LOADER_H_

#include "core/base/api.h"
#include "core/forwarding.h"
#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"
#include "renderer/inf/render_model.h"

namespace ark {

class ARK_API ModelLoader {
public:
    ModelLoader(RenderModel::Mode renderMode, const sp<Model>& unitModel)
        : _render_mode(renderMode), _unit_model(unitModel) {
    }
    virtual ~ModelLoader() = default;

    RenderModel::Mode renderMode() const {
        return _render_mode;
    }

    const sp<Model>& unitModel() const {
        return _unit_model;
    }

    virtual sp<Model> load(int32_t type) = 0;

private:
    RenderModel::Mode _render_mode;
    sp<Model> _unit_model;
};

}

#endif
