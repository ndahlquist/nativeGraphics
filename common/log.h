// log.h
// nativeGraphics
// Crossplatform log utilities

#ifndef LOG_H
#define LOG_H

#include <cstdlib>

#ifdef ANDROID_NDK
    #include <android/log.h>
    #define  LOGI(...)  __android_log_print(ANDROID_LOG_INFO,LOG_TAG,__VA_ARGS__)
    #define  LOGE(...)  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG,__VA_ARGS__)
#else // linux & ios
    // TODO: does this work for ios?
    // TODO: Rewrite
    #define  LOGI(...);  printf("INFO: ");printf(__VA_ARGS__);printf("\n");
    #define  LOGE(...);  printf("ERROR: ");printf(__VA_ARGS__);printf("\n");
#endif

static void printGLString(const char *name, GLenum s) {
    const char *v = (const char *) glGetString(s);
    LOGI("GL %s = %s\n", name, v);
}

static void checkGlError(const char* op) {
    for (GLint error = glGetError(); error; error = glGetError()) {
        LOGI("after %s() glError (0x%x)\n", op, error);
#ifndef BUILD_RELEASE
        exit(-1); // Die fast and early
#endif
    }
}

#endif // LOG_H
