#include "ship/controller/controldevice/controller/mapping/sdl/SDLGyroMapping.h"
#include "ship/controller/controldevice/controller/mapping/ControllerGyroMapping.h"
#include <spdlog/spdlog.h>
#include "ship/Context.h"

#include "ship/config/ConsoleVariable.h"
#include "ship/utils/StringHelper.h"
#include "ship/controller/controldeck/ControlDeck.h"

#if defined(__IOS__)
extern "C" int TwoShipApple_GetNativeControllerGyro(float* gyroX, float* gyroY, float* gyroZ);
#endif

namespace Ship {
#ifdef __ANDROID__
static SDL_Sensor* sAndroidGyroSensor = nullptr;

static bool OpenAndroidGyroSensor() {
    if (sAndroidGyroSensor != nullptr) {
        return true;
    }

    if ((SDL_WasInit(SDL_INIT_SENSOR) & SDL_INIT_SENSOR) == 0) {
        SDL_InitSubSystem(SDL_INIT_SENSOR);
    }

    for (int32_t i = 0; i < SDL_NumSensors(); i++) {
        if (SDL_SensorGetDeviceType(i) == SDL_SENSOR_GYRO) {
            sAndroidGyroSensor = SDL_SensorOpen(i);
            break;
        }
    }

    return sAndroidGyroSensor != nullptr;
}
#endif

SDLGyroMapping::SDLGyroMapping(uint8_t portIndex, float sensitivity, float neutralPitch, float neutralYaw,
                               float neutralRoll)
    : ControllerInputMapping(PhysicalDeviceType::SDLGamepad),
      ControllerGyroMapping(PhysicalDeviceType::SDLGamepad, portIndex, sensitivity), mNeutralPitch(neutralPitch),
      mNeutralYaw(neutralYaw), mNeutralRoll(neutralRoll) {
}

void SDLGyroMapping::Recalibrate() {
#if defined(__IOS__)
    float appleGyroData[3];
    if (mPortIndex == 0 &&
        TwoShipApple_GetNativeControllerGyro(&appleGyroData[0], &appleGyroData[1], &appleGyroData[2])) {
        mNeutralPitch = appleGyroData[0];
        mNeutralYaw = appleGyroData[1];
        mNeutralRoll = appleGyroData[2];
        return;
    }
#endif

    for (const auto& [instanceId, gamepad] :
         Context::GetInstance()->GetControlDeck()->GetConnectedPhysicalDeviceManager()->GetConnectedSDLGamepadsForPort(
             mPortIndex)) {
        if (!SDL_GameControllerHasSensor(gamepad, SDL_SENSOR_GYRO)) {
            continue;
        }

        // just use gyro on the first gyro supported device we find
        float gyroData[3];
#ifdef __ANDROID__
        GetAndroidGyroData(gamepad, gyroData);
#else
        SDL_GameControllerSetSensorEnabled(gamepad, SDL_SENSOR_GYRO, SDL_TRUE);
        SDL_GameControllerGetSensorData(gamepad, SDL_SENSOR_GYRO, gyroData, 3);
#endif

        mNeutralPitch = gyroData[0];
        mNeutralYaw = gyroData[1];
        mNeutralRoll = gyroData[2];
        return;
    }

#ifdef __ANDROID__
    float gyroData[3];
    GetAndroidGyroData(nullptr, gyroData);
    mNeutralPitch = gyroData[0];
    mNeutralYaw = gyroData[1];
    mNeutralRoll = gyroData[2];
    return;
#endif

    // if we didn't find a gyro device zero everything out
    mNeutralPitch = 0;
    mNeutralYaw = 0;
    mNeutralRoll = 0;
}

void SDLGyroMapping::UpdatePad(float& x, float& y) {
    if (Context::GetInstance()->GetControlDeck()->GamepadGameInputBlocked()) {
        x = 0;
        y = 0;
        return;
    }

#if defined(__IOS__)
    float appleGyroData[3];
    if (mPortIndex == 0 &&
        TwoShipApple_GetNativeControllerGyro(&appleGyroData[0], &appleGyroData[1], &appleGyroData[2])) {
        x = (appleGyroData[0] - mNeutralPitch) * mSensitivity;
        y = (appleGyroData[1] - mNeutralYaw) * mSensitivity;
        return;
    }
#endif

    for (const auto& [instanceId, gamepad] :
         Context::GetInstance()->GetControlDeck()->GetConnectedPhysicalDeviceManager()->GetConnectedSDLGamepadsForPort(
             mPortIndex)) {
        if (!SDL_GameControllerHasSensor(gamepad, SDL_SENSOR_GYRO)) {
            continue;
        }

        // just use gyro on the first gyro supported device we find
        float gyroData[3];
#ifdef __ANDROID__
        GetAndroidGyroData(gamepad, gyroData);
#else
        SDL_GameControllerSetSensorEnabled(gamepad, SDL_SENSOR_GYRO, SDL_TRUE);
        SDL_GameControllerGetSensorData(gamepad, SDL_SENSOR_GYRO, gyroData, 3);
#endif

        x = (gyroData[0] - mNeutralPitch) * mSensitivity;
        y = (gyroData[1] - mNeutralYaw) * mSensitivity;
        return;
    }

#ifdef __ANDROID__
    float gyroData[3];
    GetAndroidGyroData(nullptr, gyroData);
    x = (gyroData[0] - mNeutralPitch) * mSensitivity;
    y = (gyroData[1] - mNeutralYaw) * mSensitivity;
    return;
#endif

    // if we didn't find a gyro device zero everything out
    x = 0;
    y = 0;
}

std::string SDLGyroMapping::GetGyroMappingId() {
    return StringHelper::Sprintf("P%d", mPortIndex);
}

void SDLGyroMapping::SaveToConfig() {
    const std::string mappingCvarKey = CVAR_PREFIX_CONTROLLERS ".GyroMappings." + GetGyroMappingId();

    Ship::Context::GetInstance()->GetConsoleVariables()->SetString(
        StringHelper::Sprintf("%s.GyroMappingClass", mappingCvarKey.c_str()).c_str(), "SDLGyroMapping");
    Ship::Context::GetInstance()->GetConsoleVariables()->SetFloat(
        StringHelper::Sprintf("%s.Sensitivity", mappingCvarKey.c_str()).c_str(), mSensitivity);
    Ship::Context::GetInstance()->GetConsoleVariables()->SetFloat(
        StringHelper::Sprintf("%s.NeutralPitch", mappingCvarKey.c_str()).c_str(), mNeutralPitch);
    Ship::Context::GetInstance()->GetConsoleVariables()->SetFloat(
        StringHelper::Sprintf("%s.NeutralYaw", mappingCvarKey.c_str()).c_str(), mNeutralYaw);
    Ship::Context::GetInstance()->GetConsoleVariables()->SetFloat(
        StringHelper::Sprintf("%s.NeutralRoll", mappingCvarKey.c_str()).c_str(), mNeutralRoll);

    Ship::Context::GetInstance()->GetConsoleVariables()->Save();
}

void SDLGyroMapping::EraseFromConfig() {
    const std::string mappingCvarKey = CVAR_PREFIX_CONTROLLERS ".GyroMappings." + GetGyroMappingId();

    Ship::Context::GetInstance()->GetConsoleVariables()->ClearVariable(
        StringHelper::Sprintf("%s.GyroMappingClass", mappingCvarKey.c_str()).c_str());
    Ship::Context::GetInstance()->GetConsoleVariables()->ClearVariable(
        StringHelper::Sprintf("%s.Sensitivity", mappingCvarKey.c_str()).c_str());
    Ship::Context::GetInstance()->GetConsoleVariables()->ClearVariable(
        StringHelper::Sprintf("%s.NeutralPitch", mappingCvarKey.c_str()).c_str());
    Ship::Context::GetInstance()->GetConsoleVariables()->ClearVariable(
        StringHelper::Sprintf("%s.NeutralYaw", mappingCvarKey.c_str()).c_str());
    Ship::Context::GetInstance()->GetConsoleVariables()->ClearVariable(
        StringHelper::Sprintf("%s.NeutralRoll", mappingCvarKey.c_str()).c_str());

    Ship::Context::GetInstance()->GetConsoleVariables()->Save();
}

#ifdef __ANDROID__
void SDLGyroMapping::GetAndroidGyroData(SDL_GameController* gamepad, float gyroData[3]) {
    if (gamepad != nullptr && SDL_GameControllerHasSensor(gamepad, SDL_SENSOR_GYRO)) {
        SDL_GameControllerSetSensorEnabled(gamepad, SDL_SENSOR_GYRO, SDL_TRUE);
        SDL_GameControllerGetSensorData(gamepad, SDL_SENSOR_GYRO, gyroData, 3);
        return;
    }

    if (!OpenAndroidGyroSensor()) {
        gyroData[0] = 0.0f;
        gyroData[1] = 0.0f;
        gyroData[2] = 0.0f;
        return;
    }

    SDL_SensorUpdate();
    SDL_SensorGetData(sAndroidGyroSensor, gyroData, 3);

    const float gyroX = gyroData[0];
    const float gyroY = gyroData[1];
    switch (SDL_GetDisplayOrientation(0)) {
        case SDL_ORIENTATION_PORTRAIT:
            break;
        case SDL_ORIENTATION_PORTRAIT_FLIPPED:
            gyroData[0] = -gyroX;
            gyroData[1] = -gyroY;
            break;
        case SDL_ORIENTATION_LANDSCAPE:
            gyroData[0] = -gyroY;
            gyroData[1] = gyroX;
            break;
        case SDL_ORIENTATION_LANDSCAPE_FLIPPED:
            gyroData[0] = gyroY;
            gyroData[1] = -gyroX;
            break;
        case SDL_ORIENTATION_UNKNOWN:
            break;
    }
}
#endif

std::string SDLGyroMapping::GetPhysicalDeviceName() {
    return "SDL Gamepad";
}
} // namespace Ship
