

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <jni.h>
#include <android/log.h>
#include <android/window.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>

#include "native_video_display.h"

#define FORMAT_YUV_420_P 0x32315659

native_display_params displayHandle;
char * dummy;

jint Java_com_example_ndkdisplay_MainActivity_nativeDisplayInit(JNIEnv* env, jobject thiz, jint width, jint height, jstring args)
{
	const char *commandArgs = NULL;
	commandArgs = (*env)->GetStringUTFChars(env, args, NULL);

	strcpy(displayHandle.file_name,commandArgs);

    if(NULL == displayHandle.window){
        LOG(" NATIVE WINDOW IS NULL");
        return -1;
    }
    int i;
    int status;
    displayHandle.frame_index = 0;
    displayHandle.height = height;
    displayHandle.width = width;
    displayHandle.format = FORMAT_YUV_420_P;
    displayHandle.frame_count = 0;

    LOG("..................DISP HEIGHT - %d",displayHandle.height);
    LOG("..................DISP WIDTH - %d",displayHandle.width);
    LOG("..................DISP FMT - %x",displayHandle.format);

	status = ANativeWindow_setBuffersGeometry(displayHandle.window,  displayHandle.width,  displayHandle.height, displayHandle.format);
	if(0 != status){
		LOG("...........ERROR.........1");
		//return -1;
	}

	int w = ANativeWindow_getWidth(displayHandle.window);
	if(w != displayHandle.width){
		LOG("...........ERROR.........2 , w - %d",w);
		//return -1;
		//continue;
	}

	int h = ANativeWindow_getHeight(displayHandle.window);
	if(h != displayHandle.height){
		LOG("...........ERROR.........3 , h - %d",h);
		//return -1;
		//continue;
	}

	int f = ANativeWindow_getFormat(displayHandle.window);
	if(f != displayHandle.format){
		LOG("...........ERROR.........4 , f - %d",f);
		//return -1;
		//continue;
	}

    displayHandle.fp = fopen(commandArgs, "rb");
    if(NULL == displayHandle.fp){
        LOG("...........ERROR.........5");
        return -1;
    }

    for(i = 0;  ; i++){

    	LOG("FRAME COUNT - %d",i);

        int size_read;
        char *y,*u,*v;
        displayHandle.yuv_buffer[i] = (char *)malloc(displayHandle.height * displayHandle.width * 2);
        if(NULL == displayHandle.yuv_buffer[i]){
            LOG("...........MALLOC ERROR.........6");
            break;
        }

        y = displayHandle.yuv_buffer[i];
        v = y + displayHandle.height * displayHandle.width;
        u = v + (displayHandle.height * displayHandle.width)/4;

        /* copying y */
        size_read = fread(y, 1, displayHandle.height * displayHandle.width, displayHandle.fp);
        if(size_read != (displayHandle.height * displayHandle.width)){
            LOG("...........ERROR.........7, size read - %d",size_read);
            break;
        }

        /* copying u */
        size_read = fread(u, 1, (displayHandle.height * displayHandle.width)/4, displayHandle.fp);
        if(size_read != (displayHandle.height * displayHandle.width)/4){
            LOG("...........ERROR........8, size read - %d",size_read);
            break;
        }

        /* copying v */
        size_read = fread(v, 1, (displayHandle.height * displayHandle.width)/4, displayHandle.fp);
        if(size_read != (displayHandle.height * displayHandle.width)/4){
            LOG("...........ERROR.........9, size read - %d",size_read);
            break;
        }
    }

    dummy = (char *)malloc(displayHandle.height * displayHandle.width * 2);

    displayHandle.frame_count = i;

    fclose(displayHandle.fp);

    displayHandle.fp = NULL;

    if(0 == displayHandle.frame_count){
    	LOG("...........ERROR........9.5, FILE READ FAILED - %d",displayHandle.frame_count);
    	return -1;
    }

    LOG("HERE3 - %d",displayHandle.frame_count);
    (*env)->ReleaseStringUTFChars(env, args, commandArgs);

    LOG("NATIVE DISPLAY INIT PASSED - %d",displayHandle.frame_count);
    return 0;
}

jint Java_com_example_ndkdisplay_MainActivity_nativeDisplayDeInit(JNIEnv* env, jobject thiz)
{
    if(NULL == displayHandle.window){
        LOG(" NATIVE WINDOW IS NULL");
        return -1;
    }

    ANativeWindow_release(displayHandle.window);

    int i;
    for(i = 0; i < displayHandle.frame_count; i++){
    	if(NULL != displayHandle.yuv_buffer[i]){
    		free(displayHandle.yuv_buffer[i]);
    	}
    }

    LOG(" NATIVE DEINIT PASSED");

}

jint Java_com_example_ndkdisplay_MainActivity_nativeDisplayRenderFrame(JNIEnv* env, jobject thiz)
{
    if(NULL == displayHandle.window){
        LOG(" NATIVE WINDOW IS NULL");
        return -1;
    }

//    if(NULL == displayHandle.fp){
//		displayHandle.fp = fopen(displayHandle.file_name, "rb");
//		if(NULL == displayHandle.fp){
//			LOG("...........ERROR.........5.5");
//			return -1;
//		}
//    }

    int status,size_read;
    ANativeWindow_Buffer dispBuff;
    ARect* inOutDirtyBounds;
    char *y, *u, *v;
    char *y_dest, *u_dest, *v_dest;

    status = ANativeWindow_lock(displayHandle.window, &dispBuff,NULL);
    if(0 != status){
        LOG("...........ERROR.........10");
        return -1;
    }

    //copy buffer
    y = displayHandle.yuv_buffer[displayHandle.frame_index];
    v = y + displayHandle.height * displayHandle.width;
    u = v + (displayHandle.height * displayHandle.width)/4;

    y_dest = (char *)dispBuff.bits;
    //y_dest = dummy;

    v_dest = y_dest + displayHandle.height * displayHandle.width;
    u_dest = v_dest + (displayHandle.height * displayHandle.width)/4;

    memcpy(y_dest, y, displayHandle.height * displayHandle.width);
    memcpy(v_dest, u, (displayHandle.height * displayHandle.width)/4);
    memcpy(u_dest, v, (displayHandle.height * displayHandle.width)/4);

//    do{
//		/* copying y */
//		size_read = fread(y_dest, 1, displayHandle.height * displayHandle.width, displayHandle.fp);
//		if(size_read != (displayHandle.height * displayHandle.width)){
//			LOG("...........ERROR.........7, size read - %d",size_read);
//			fclose(displayHandle.fp);
//			displayHandle.fp = NULL;
//			break;
//		}
//
//		/* copying u */
//		size_read = fread(u_dest, 1, (displayHandle.height * displayHandle.width)/4, displayHandle.fp);
//		if(size_read != (displayHandle.height * displayHandle.width)/4){
//			LOG("...........ERROR........8, size read - %d",size_read);
//			fclose(displayHandle.fp);
//			displayHandle.fp = NULL;
//			break;
//		}
//
//		/* copying v */
//		size_read = fread(v_dest, 1, (displayHandle.height * displayHandle.width)/4, displayHandle.fp);
//		if(size_read != (displayHandle.height * displayHandle.width)/4){
//			LOG("...........ERROR.........9, size read - %d",size_read);
//			fclose(displayHandle.fp);
//			displayHandle.fp = NULL;
//			break;
//		}
//    }while(0);



    displayHandle.frame_index++;
    displayHandle.frame_index %= (displayHandle.frame_count);

    status = ANativeWindow_unlockAndPost(displayHandle.window);
    if(0 != status){
        LOG("...........ERROR.........11");
        return -1;
    }

    //LOG("NATIVE RENDER FRAME PASSED");
    return 0;
}

jint Java_com_example_ndkdisplay_MainActivity_nativeDisplaySetWindowPtr(JNIEnv* env, jobject thiz, jobject surface)
{
    if(NULL == surface){
        LOG("SURFACE IS NULL");
        return -1;
    }

    displayHandle.window = ANativeWindow_fromSurface(env, surface);
    if(NULL == displayHandle.window){
        LOG(" NATIVE WINDOW IS NULL");
        return -1;
    }

    ANativeWindow_acquire(displayHandle.window);

    LOG(" SET WINDOW PTR PASSED");
    return 0;
}

