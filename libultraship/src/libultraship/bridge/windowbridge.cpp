#include "libultraship/bridge/windowbridge.h"
#include "ship/window/Window.h"
#include "ship/Context.h"

#include <chrono>
#include <thread>

extern "C" {

uint32_t WindowGetWidth() {
    return Ship::Context::GetInstance()->GetWindow()->GetWidth();
}

uint32_t WindowGetHeight() {
    return Ship::Context::GetInstance()->GetWindow()->GetHeight();
}

float WindowGetAspectRatio() {
    return Ship::Context::GetInstance()->GetWindow()->GetCurrentAspectRatio();
}

bool WindowIsRunning() {
    return Ship::Context::GetInstance()->GetWindow()->IsRunning();
}

bool WindowIsFrameReady() {
    auto window = Ship::Context::GetInstance()->GetWindow();
    window->HandleEvents();
    const bool ready = window->IsRunning() && window->IsFrameReady();
    if (!ready) {
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
    return ready;
}

int32_t WindowGetPosX() {
    return Ship::Context::GetInstance()->GetWindow()->GetPosX();
}

int32_t WindowGetPosY() {
    return Ship::Context::GetInstance()->GetWindow()->GetPosY();
}

bool WindowIsFullscreen() {
    return Ship::Context::GetInstance()->GetWindow()->IsFullscreen();
}
}
