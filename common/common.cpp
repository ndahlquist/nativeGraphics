//  common.cpp
//  nativeGraphics

#include "common.h"

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

#include "Eigen/Core"
#include "Eigen/Eigenvalues"

#include "transform.h"
#include "RenderObject.h"
#include "RenderLight.h"
#include "glsl_helper.h"

#define  LOG_TAG    "libnativegraphics"
#include "log.h"

using namespace std;

int width = 0;
int height = 0;

// Callback function to load resources.
void*(*resourceCallback)(const char *) = NULL;

void SetResourceCallback(void*(*cb)(const char *)) {
    resourceCallback = cb;
}

GLuint gFrameBuffer;
GLuint gDepthBuffer;
GLuint gPositionTexture;
GLuint gAlbedoTexture;

GLuint positionShader;
GLuint albedoShader;
GLuint brownShader;

RenderObject *cave;
RenderObject *character;
RenderLight *square;

GLuint framebuffer;

void setFrameBuffer(int handle) {
    framebuffer = handle;
}

// Initialize the application, loading all of the settings that
// we will be accessing later in our fragment shaders.
void Setup(int w, int h) {
    
    if(!resourceCallback) {
        LOGE("Resource callback not set.");
        exit(-1);
    }
    
    cave = new RenderObject("cave0.obj", "standard_v.glsl", "tex_diffuse_f.glsl");
    character = new RenderObject("raptor.obj", "standard_v.glsl", "tex_diffuse_f.glsl");
    character->AddTexture("raptor_albedo.jpg");
    square = new RenderLight("square.obj", "dr_standard_v.glsl", "dr_pointlight_f.glsl");
    
    positionShader = createShaderProgram((char *)resourceCallback("standard_v.glsl"), (char *)resourceCallback("position_f.glsl"));
    albedoShader = createShaderProgram((char *)resourceCallback("standard_v.glsl"), (char *)resourceCallback("tex_diffuse_f.glsl"));
    brownShader = createShaderProgram((char *)resourceCallback("standard_v.glsl"), (char *)resourceCallback("solid_color_f.glsl"));
    
    width = w;
    height = h;
    glViewport(0, 0, w, h);
    checkGlError("glViewport");
    
    // Allocate frame buffer
	glGenFramebuffers(1, &gFrameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gFrameBuffer);
    
    // Allocate position texture to render to.
    glGenTextures(1, &gPositionTexture);
    glBindTexture(GL_TEXTURE_2D, gPositionTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    checkGlError("AddTexture");
    
    square->textures.push_back(gPositionTexture);
    
    // Allocate albedo texture to render to.
    glGenTextures(1, &gAlbedoTexture);
    glBindTexture(GL_TEXTURE_2D, gAlbedoTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    checkGlError("AddTexture");
    
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPositionTexture, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    glGenRenderbuffers(1, &gDepthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, gDepthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, gDepthBuffer);
    
    //GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    //if(status != GL_FRAMEBUFFER_COMPLETE) TODO: figure out why this doesn't work.
    //    LOGE("Failed to allocate framebuffer object %x", status);
        
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    checkGlError("Common::setup");
}

float cameraPos[4] = {0,0,0.9,1};
float pan[3] = {0,0,0}, up[3] = {0,1,0};
float rot[2] = {0,0};

unsigned int frameNum = 0;

void RenderFrame() {

    //////////////////////////////////
    // Render to frame buffer
    
    glBindFramebuffer(GL_FRAMEBUFFER, gFrameBuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPositionTexture, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, gDepthBuffer);
    
    //GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    //if(status != GL_FRAMEBUFFER_COMPLETE)
    //    LOGE("Failed to allocate framebuffer object %x", status);
    
    glViewport(0, 0, width, height);
    
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0., 0., 0., 0.);
    checkGlError("glClearColor");
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    checkGlError("glClear");

    pLoadIdentity();
    perspective(20, (float) width / (float) height, 80, 180);
    
    mvLoadIdentity();
    lookAt(cameraPos[0]+pan[0], cameraPos[1]+pan[1], cameraPos[2]+pan[2], pan[0], pan[1], pan[2], up[0], up[1], up[2]);
    
    scalef(.2, .2, .2);
    translatef(0.0f, 0.0f, -600.0f);
    rotate(rot[1],rot[0],0);
    
    mvPushMatrix();
    translatef(0.0f, -40.0f, 0.0f);
    cave->SetShader(positionShader);
    cave->RenderFrame();
    mvPopMatrix();
    
    mvPushMatrix();
    translatef(68.0f, -40.0f, -20.0f);
    character->SetShader(positionShader);    
    character->RenderFrame();
    mvPopMatrix();

    ////////////////////////////////////////////////////
    // Render from frame buffer

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    
    glEnable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT);
    
    pLoadIdentity();
    mvLoadIdentity();
    
    /*pLoadIdentity();
    perspective(20, (float) width / (float) height, 10, 1000);
    
    mvLoadIdentity();
    lookAt(cameraPos[0]+pan[0], cameraPos[1]+pan[1], cameraPos[2]+pan[2], pan[0], pan[1], pan[2], up[0], up[1], up[2]);
    
    scalef(1.0, 1.0, 1.0);
    translatef(0.0f, 0.0f, -600.0f);
    rotate(rot[1],rot[0],0);
    translatef(0.0f, -40.0f, 0.0f);*/
    
    glUseProgram(square->gProgram);
    GLuint u_LightPosHandle = glGetUniformLocation(square->gProgram, "u_LightPos");
    glUniform3f(u_LightPosHandle, 10.0f * cos(frameNum++ / 50.0f), 0.0, -100 + 100.0f * sin(frameNum++ / 200.0f));
    
    GLuint u_FragWidth = glGetUniformLocation(square->gProgram, "u_FragWidth");
    glUniform1i(u_FragWidth, width);
    
    GLuint u_FragHeight = glGetUniformLocation(square->gProgram, "u_FragHeight");
    glUniform1i(u_FragHeight, height);
    
    square->RenderFrame();
}

float lastPointer[2] = {0,0};

void PointerDown(float x, float y, int pointerIndex) {
    lastPointer[0] = x;
    lastPointer[1] = y;
}

void PointerMove(float x, float y, int pointerIndex) {
     float deltaX = x - lastPointer[0];
	 float deltaY = y - lastPointer[1];
	 
	 rot[0] +=  8.0 * deltaX;
	 rot[1] += -1.0 * deltaY;
	 
	 lastPointer[0] = x;
     lastPointer[1] = y;
}

void PointerUp(float x, float y, int pointerIndex) {
    lastPointer[0] = x;
    lastPointer[1] = y;
}

#undef LOG_TAG
