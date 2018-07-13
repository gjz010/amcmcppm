/*
 * Everything that a raytrace worker required to perform a raytrace.
 * RenderScene is the form generated using scene file, while BakedRenderScene is the pre-processed form of RenderScene.
 * Raytracing is done using BakedRenderScene.
 * Note that BakedRenderScene should be read-only to workers. Moreover, BakedRenderScene should be a self-contained object so that it can be easily serialized and carried to other machines.
*/
#include "defs.h"
#include "Path.h"
#include "BakedRenderScene.h"

class RenderScene{
    virtual BakedRenderScene* bake()=0;

};


