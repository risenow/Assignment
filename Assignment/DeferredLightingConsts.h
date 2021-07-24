#ifndef DEFERRED_LIGHTING_CONSTANTS
#define DEFERRED_LIGHTING_CONSTANTS

#include "ConstantsPreprocessing.h"

DECLARE_CONSTANT_STRUCTURE DeferredLightingConsts
{
    DECLARE_FLOAT4X4 shadowview; // saving maximum precision using explicit matrixes instead of multiplication result but perf???
    DECLARE_FLOAT4X4 shadowproj;
    DECLARE_FLOAT4X4 unview;
    DECLARE_FLOAT4X4 unproj;
    DECLARE_FLOAT4X4 proj;
    DECLARE_FLOAT4 lightDir; //in view space
};

#endif