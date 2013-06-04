#ifndef __nativeGraphics__common__
#define __nativeGraphics__common__

#include "RenderPipeline.h"


/** This part of the interface is called by the "upper" level of the program.
    (Android Java, iOS Obj-C, Linux C++.                                  **/
void SetResourceCallback(void*(*callbackfunc)(const char *));
void Setup(int w, int h);
void setFrameBuffer(int handle);
void RenderFrame();

// Note that these may be called asynchronously with RenderFrame
void PointerDown(float x, float y, int pointerIndex = -1);
void PointerMove(float x, float y, int pointerIndex = -1);
void PointerUp(float x, float y, int pointerIndex = -1);
void UpdateOrientation(float roll, float pitch, float yaw);


/** This part of the interface is uesd by the "lower" level of the program. **/

// Callback function to load resources.
extern void*(*resourceCallback)(const char *);
extern int displayWidth;
extern int displayHeight;
extern GLuint defaultFrameBuffer;
extern RenderPipeline * pipeline;

#endif // __nativeGraphics__common__
