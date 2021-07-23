#ifndef DEFERRED_LIGHTING_CONSTANTS
#define DEFERRED_LIGHTING_CONSTANTS

#include "ConstantsPreprocessing.h"

DECLARE_CONSTANT_STRUCTURE DeferredLightingConsts
{
    DECLARE_FLOAT4X4 unproj;
    DECLARE_FLOAT4X4 proj;
    DECLARE_FLOAT4 lightPos; //in view space
};

#endif