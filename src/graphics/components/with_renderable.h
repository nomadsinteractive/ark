#pragma once

#include "core/base/string.h"
#include "core/inf/builder.h"
#include "core/inf/wirable.h"

#include "graphics/forwarding.h"

namespace ark {

class WithRenderable final : public Wirable {
private:
    struct Manifest {
        sp<Layer> _layer;
        sp<Renderable> _renderable;
        sp<RenderObject> _render_object;
        String _transform_node;
    };

public:
    WithRenderable(Vector<Manifest> manifests);

    void onWire(const WiringContext& context, const Box& self) override;

    struct ManifestFactory {
        ManifestFactory(BeanFactory& factory, const document& manifest);

        builder<Layer> _layer;
        builder<Renderable> _renderable;
        builder<RenderObject> _render_object;

        String _transform_node;
    };

//  [[plugin::builder("with_renderable")]]
    class BUILDER final : public Builder<Wirable> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        sp<Wirable> build(const Scope& args) override;

    private:
        Vector<ManifestFactory> _manifests;
    };

private:
    Vector<Manifest> _manifests;
};

}
