#ifndef ARK_APP_IMPL_APPLICATION_SDL_APPLICATION_H_
#define ARK_APP_IMPL_APPLICATION_SDL_APPLICATION_H_

#include "core/base/api.h"
#include "core/forwarding.h"

#include "app/base/application.h"
#include "app/base/event.h"

struct SDL_cond;
struct SDL_mutex;
struct SDL_Window;

namespace ark {

class ARK_API SDLApplication : public Application {
public:
    SDLApplication(const sp<ApplicationDelegate>& applicationDelegate, const sp<ApplicationContext>& applicationContext, uint32_t width, uint32_t height, const ApplicationManifest& manifest);

    virtual int run() override;
    virtual const sp<ApplicationController>& controller() override;

    void onSurfaceChanged();

private:
    void pollEvents(uint64_t timestamp);
    uint32_t toSDLWindowFlag(const sp<ApplicationContext>& applicationContext, uint32_t appWindowFlag);

private:
    SDL_Window* _main_window;
    SDL_cond* _cond;
    SDL_mutex* _lock;

    sp<ApplicationController> _controller;

    uint32_t _window_flag;

    bool _use_open_gl;
};

}

#endif
