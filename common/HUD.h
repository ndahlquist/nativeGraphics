//  HUD.h
//  nativeGraphics

#ifndef __nativeGraphics__HUD__
#define __nativeGraphics__HUD__

#include <iostream>
#include <string>

#ifdef ANDROID_NDK
#include "importgl.h"
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <jni.h>
#elif __APPLE__
#include <stdlib.h>
#include <OpenGLES/ES2/gl.h>
#else // linux
#include <GL/glew.h>
#include <stdio.h>
#endif

#include "RenderObject.h"

using namespace std;

class HUD : public RenderObject {
public:
    HUD();
    void Render(float health);
    void RenderElement(GLuint textureHandle, float xdisp, float ydisp, float xscale, float yscale);
    GLuint AddTexture(const char *textureFilename);
    
    GLuint healthbarTex;
    GLuint healthbarBorderTex;
    GLuint radarTex;
};


#endif // __nativeGraphics__HUD__