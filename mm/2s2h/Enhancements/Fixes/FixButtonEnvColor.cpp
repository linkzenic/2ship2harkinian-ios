#include <libultraship/bridge.h>
#include "2s2h/Enhancements/FrameInterpolation/FrameInterpolation.h"
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"

extern "C" {
#include "functions.h"
#include "gfx.h"
#include "variables.h"
}

#define CVAR_NAME "gFixes.FixButtonEnvColor"
#define CVAR CVarGetInteger(CVAR_NAME, 0)

void RegisterFixButtonEnvColor() {
    COND_VB_SHOULD(VB_SET_BUTTON_ENV_COLOR, CVAR, {
        OPEN_DISPS(gPlayState->state.gfxCtx);
        gDPSetEnvColor(OVERLAY_DISP++, 0, 0, 0, 255);
        CLOSE_DISPS(gPlayState->state.gfxCtx);
    });
}

static RegisterShipInitFunc initFunc(RegisterFixButtonEnvColor, { CVAR_NAME });
