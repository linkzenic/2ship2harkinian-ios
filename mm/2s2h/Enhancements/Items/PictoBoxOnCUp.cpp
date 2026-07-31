#include <libultraship/bridge/consolevariablebridge.h>
#include "2s2h/GameInteractor/GameInteractor.h"
#include "2s2h/ShipInit.hpp"

extern "C" {
#include "variables.h"
#include "z64player.h"
s32 func_80831814(Player* player, PlayState* play, PlayerUnkAA5 arg2);
}

#define CVAR_NAME "gEnhancements.Items.PictoBoxOnCUp"
#define CVAR CVarGetInteger(CVAR_NAME, 0)

static void TryPictoBoxOnCUp(Input* input) {
    Player* player = GET_PLAYER(gPlayState);
    PlayerUnkAA5 firstPersonMode = PLAYER_UNKAA5_1;

    if (gSaveContext.save.saveInfo.inventory.items[SLOT_PICTOGRAPH_BOX] == ITEM_PICTOGRAPH_BOX) {
        firstPersonMode = PLAYER_UNKAA5_2;
    }

    if (player->tatlTextId == 0 && !Player_CheckHostileLockOn(player) &&
        ((input->press.button & BTN_CUP) == BTN_CUP) && !func_80831814(player, gPlayState, firstPersonMode)) {
        Audio_PlaySfx(NA_SE_SY_ERROR);
    }
}

void RegisterPictoBoxOnCUp() {
    COND_HOOK(OnPassPlayerInputs, CVAR, [](Input* input) { TryPictoBoxOnCUp(input); });
}

static RegisterShipInitFunc initFunc(RegisterPictoBoxOnCUp, { CVAR_NAME });
