#pragma once

#include <vector>

#include "core/inf/wirable.h"

#include "graphics/forwarding.h"

namespace ark {

class WithRenderable final : public Wirable {
private:
    struct Manifest {
        sp<Renderable> _renderable;
        sp<LayerContext> _layer_context;
        String _transform_node;
    };

public:
    WithRenderable(std::vector<Manifest> manifests);

    TypeId onPoll(WiringContext& context) override;
    void onWire(const WiringContext& context) override;

    struct ManifestFactory {
        ManifestFactory(BeanFactory& factory, const document& manifest);

        sp<Builder<Renderable>> _renderable;
        sp<Builder<LayerContext>> _layer_context;

        String _transform_node;
    };

//  [[plugin::builder("with_renderable")]]
    class BUILDER final : public Builder<Wirable> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        sp<Wirable> build(const Scope& args) override;

    private:
        std::vector<ManifestFactory> _manifests;
    };

private:
    std::vector<Manifest> _manifests;
};

}
