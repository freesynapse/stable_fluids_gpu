#pragma once

#include <stdint.h>

class Config
{
public:
    // solver parameters
    static float velocityDissipation;
    static float densityDissipation;
    static float vorticityDissapation;
    static float vorticityConfinement;
    static uint32_t jacobiIterCount;

    // interaction
    static float forceRadius;
    static float forceMultiplier;
    static float splatRadius;

    static bool isRunning;
    static bool showQuivers;

    // renderer
    static uint32_t quiverSamplingRate;

};
