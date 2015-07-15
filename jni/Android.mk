# 
# Android makefile for creating the ndk display test bench library.
#
#
LOCAL_PATH := $(call my-dir)


include $(CLEAR_VARS)

LOCAL_MODULE    := native_display

LOCAL_SRC_FILES := native_video_display.c

LOCAL_C_INCLUDES := ../

LOCAL_LDLIBS := \
    -llog \
    -lGLESv2 \
    -landroid\
    -lOpenSLES
    
LOCAL_SHARED_LIBRARIES := \
    libutils \
    libandroid \
    libGLESv2

include $(BUILD_SHARED_LIBRARY)
