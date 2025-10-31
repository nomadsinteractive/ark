#pragma once

#include <SDL3/SDL.h>

#include "core/base/api.h"
#include "core/forwarding.h"
#include "core/inf/builder.h"

#include "app/base/application.h"

namespace ark::plugin::sdl3 {

class ApplicationSDL3 final : public Application {
public:
    ApplicationSDL3(sp<ApplicationContext> applicationContext, const ApplicationManifest& manifest, sp<ApplicationDelegate> applicationDelegate = nullptr);

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

    void pollEvents(uint32_t timestamp);

private:
    SDL_Window* _main_window;
    SDL_Condition* _cond;
    SDL_Mutex* _lock;

    sp<ApplicationController> _controller;

    bool _use_open_gl;
    bool _vsync;
};

}
