//  RenderObject.h
//  nativeGraphics

#ifndef __nativeGraphics__RenderObject__
#define __nativeGraphics__RenderObject__

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

#include <vector>

using namespace std;

class RenderObject {
public:
    RenderObject(const char *objFile, const char *vertexShaderFile, const char *fragmentShaderFile);

    void AddTexture(const char *textureFilename, bool normalmap = false);
    
    // Render color and geometry to g buffer.
    void Render();
    
    // Render only a portion of the geometry.
    // (Used for screen-space intersections)
    void HalfRender();
     
    GLuint colorShader;
    
protected:
    void SetShader(const GLuint shaderProgram);
    void RenderPass();
    
    GLuint gvPositionHandle;
    GLuint gmvMatrixHandle;
    GLuint gmvpMatrixHandle;
    GLuint textureUniform;
    GLuint gvTexCoords;
    GLuint gvNormals;
    GLuint gVertexBuffer;
    GLuint normalMapUniform;
    
    GLuint texture;
    GLuint normalTexture;
    int numVertices;
};


#endif // __nativeGraphics__RenderObject__
