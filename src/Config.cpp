
#include "Config.h"

// static decls
float       Config::velocityDissipation     = 0.997f;
float       Config::densityDissipation      = 0.997f;
float       Config::vorticityDissapation    = 0.7f;
float       Config::vorticityConfinement    = 0.1f;
uint32_t    Config::jacobiIterCount         = 50;

float       Config::forceRadius             = 0.003f;
float       Config::forceMultiplier         = 50.0f;
float       Config::splatRadius             = 0.001f;

bool        Config::isRunning               = true;
bool        Config::showQuivers             = false;

uint32_t    Config::quiverSamplingRate      = 8;



