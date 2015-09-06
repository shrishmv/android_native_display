# 
# Android makefile for creating the ndk display test bench library.
#
#
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libyuv
LOCAL_SRC_FILES := $(LOCAL_PATH)/../libs/imports/libyuv.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE    := native_display

LOCAL_SRC_FILES := native_video_display.c

LOCAL_C_INCLUDES := ../\
					../libs/imports/include\
					../libs/imports/include/libyuv

LOCAL_LDLIBS := \
    -llog \
    -lGLESv2 \
    -landroid\
    -lOpenSLES

LOCAL_WHOLE_STATIC_LIBRARIES := \
					libyuv 
    
LOCAL_SHARED_LIBRARIES := \
    libutils \
    libandroid \
    libGLESv2

include $(BUILD_SHARED_LIBRARY)
