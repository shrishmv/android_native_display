
#include <assert.h>
#include <string.h>

#include <jni.h>
#include <android/log.h>
#include <android/window.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>


#ifndef _NATIVE_VIDEO_DISPLAY_H_
#define _NATIVE_VIDEO_DISPLAY_H_

#define LOGTAG "NDK_DISPLAY"

#define MAX_RAW_FRAME_COUNT 500

#define LOG(...) __android_log_print(ANDROID_LOG_DEBUG, LOGTAG, __VA_ARGS__)

typedef struct _native_display_params_
{
    ANativeWindow* window;

    int height;
    int width;
    int format;
    int frame_index;

    FILE *fp;
    char *yuv_buffer[MAX_RAW_FRAME_COUNT];

    int frame_count;
    char file_name[100];
}native_display_params;


jint Java_com_example_ndkdisplay_MainActivity_nativeDisplayInit(JNIEnv* env, jobject thiz, jint width, jint height, jstring args,jint id);

jint Java_com_example_ndkdisplay_MainActivity_nativeDisplayDeInit(JNIEnv* env, jobject thiz,jint id);

jint Java_com_example_ndkdisplay_MainActivity_nativeDisplayRenderFrame(JNIEnv* env, jobject thiz,jint id);

jint Java_com_example_ndkdisplay_MainActivity_nativeDisplaySetWindowPtr(JNIEnv* env, jobject thiz, jobject surface,jint id);

#endif //_NATIVE_VIDEO_DISPLAY_H_
