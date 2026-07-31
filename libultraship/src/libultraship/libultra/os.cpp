#include "libultraship/libultraship.h"
#include <SDL2/SDL.h>
#include <ratio>

#if defined(__IOS__) && !defined(__TVOS__)
extern "C" void TwoShipIOS_GetTouchPadState(uint16_t* buttons, int8_t* leftX, int8_t* leftY,
                                             int8_t* rightX, int8_t* rightY);
#endif

// Establish a chrono duration for the N64 46.875MHz clock rate
typedef std::ratio<3000, 64> n64ClockRatio;
typedef std::ratio_divide<std::micro, n64ClockRatio> n64CycleRate;
typedef std::chrono::duration<long long, n64CycleRate> n64CycleRateDuration;

extern "C" {
uint8_t __osMaxControllers = MAXCONTROLLERS;
uint64_t __osCurrentTime = 0;

int32_t osContInit(OSMesgQueue* mq, uint8_t* controllerBits, OSContStatus* status) {
    *controllerBits = 0;
    status->status |= 1;

    std::string controllerDb = Ship::Context::LocateFileAcrossAppDirs("gamecontrollerdb.txt");
    int mappingsAdded = SDL_GameControllerAddMappingsFromFile(controllerDb.c_str());
    if (mappingsAdded >= 0) {
        SPDLOG_INFO("Added SDL game controllers from \"{}\" ({})", controllerDb, mappingsAdded);
    } else {
        SPDLOG_ERROR("Failed add SDL game controller mappings from \"{}\" ({})", controllerDb, SDL_GetError());
    }

    SDL_SetHint(SDL_HINT_JOYSTICK_THREAD, "1");
    uint32_t controllerSubsystems = SDL_INIT_GAMECONTROLLER;
#if !defined(__TVOS__)
    controllerSubsystems |= SDL_INIT_SENSOR;
#endif
    if (SDL_Init(controllerSubsystems) != 0) {
        SPDLOG_ERROR("Failed to initialize SDL game controllers ({})", SDL_GetError());
        exit(EXIT_FAILURE);
    }

    Ship::Context::GetInstance()->GetControlDeck()->Init(controllerBits);

    return 0;
}

int32_t osContStartReadData(OSMesgQueue* mesg) {
    return 0;
}

void osContGetReadData(OSContPad* pad) {
    memset(pad, 0, sizeof(OSContPad) * __osMaxControllers);

    Ship::Context::GetInstance()->GetControlDeck()->WriteToPad(pad);
#if defined(__IOS__) && !defined(__TVOS__)
    uint16_t touchButtons = 0;
    int8_t touchLeftX = 0;
    int8_t touchLeftY = 0;
    int8_t touchRightX = 0;
    int8_t touchRightY = 0;
    TwoShipIOS_GetTouchPadState(&touchButtons, &touchLeftX, &touchLeftY, &touchRightX, &touchRightY);
    pad[0].button |= touchButtons;
    if (touchLeftX != 0 || touchLeftY != 0) {
        pad[0].stick_x = touchLeftX;
        pad[0].stick_y = touchLeftY;
    }
    if (touchRightX != 0 || touchRightY != 0) {
        pad[0].right_stick_x = touchRightX;
        pad[0].right_stick_y = touchRightY;
    }
#endif
}

void osSetTime(OSTime time) {
    __osCurrentTime =
        std::chrono::duration_cast<n64CycleRateDuration>(std::chrono::steady_clock::now().time_since_epoch()).count() +
        time;
}

// Returns the OS time matching the N64 46.875MHz cycle rate
uint64_t osGetTime() {
    return std::chrono::duration_cast<n64CycleRateDuration>(std::chrono::steady_clock::now().time_since_epoch())
               .count() -
           __osCurrentTime;
}

// Returns the CPU clock count matching the N64 46.875Mhz cycle rate
uint32_t osGetCount() {
    return std::chrono::duration_cast<n64CycleRateDuration>(std::chrono::steady_clock::now().time_since_epoch())
        .count();
}

OSPiHandle* osCartRomInit() {
    return NULL;
}

int osSetTimer(OSTimer* t, OSTime countdown, OSTime interval, OSMesgQueue* mq, OSMesg msg) {
    return 0;
}

int32_t osEPiStartDma(OSPiHandle* pihandle, OSIoMesg* mb, int32_t direction) {
    return 0;
}

uint32_t osAiGetLength() {
    // TODO: Implement
    return 0;
}

int32_t osAiSetNextBuffer(void* buff, size_t len) {
    // TODO: Implement
    return 0;
}

int32_t __osMotorAccess(OSPfs* pfs, uint32_t vibrate) {
    auto io = Ship::Context::GetInstance()->GetControlDeck()->GetControllerByPort(pfs->channel)->GetRumble();
    if (vibrate) {
        io->StartRumble();
    } else {
        io->StopRumble();
    }

    return 0;
}

int32_t osMotorInit(OSMesgQueue* ctrlrqueue, OSPfs* pfs, int32_t channel) {
    pfs->channel = channel;
    return 0;
}
}
