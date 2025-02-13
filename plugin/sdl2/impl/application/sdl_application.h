#pragma once

#include "core/base/api.h"
#include "core/forwarding.h"
#include "core/inf/builder.h"

#include "app/base/application.h"
#include "app/base/application_manifest.h"

struct SDL_cond;
struct SDL_mutex;
struct SDL_Window;

namespace ark {

class SDLApplication final : public Application {
public:
    SDLApplication(sp<ApplicationDelegate> applicationDelegate, sp<ApplicationContext> applicationContext, uint32_t width, uint32_t height, const ApplicationManifest& manifest);

    int run() override;
    const sp<ApplicationController>& controller() override;

    void onSurfaceChanged();

//  [[plugin::builder]]
    class BUILDER final : public Builder<Application> {
    public:
        BUILDER() = default;

        sp<Application> build(const Scope& args) override;
    };

private:
    void initialize();

    void pollEvents(uint64_t timestamp);
    uint32_t toSDLWindowFlag(const sp<ApplicationContext>& applicationContext, ApplicationManifest::WindowFlags appWindowFlag);

private:
    SDL_Window* _main_window;
    SDL_cond* _cond;
    SDL_mutex* _lock;

    sp<ApplicationController> _controller;

    bool _use_open_gl;
    bool _vsync;
};

}
