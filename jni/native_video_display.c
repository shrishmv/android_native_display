

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

native_display_params displayHandle[2];
char * dummy;

jint Java_com_example_ndkdisplay_MainActivity_nativeDisplayInit(JNIEnv* env, jobject thiz, jint width, jint height, jstring args, jint id)
{
	if((id >= 2)||(id < 0)){
		LOG(" INVALID WINDOW ID");
		return -1;
	}

	const char *commandArgs = NULL;
	commandArgs = (*env)->GetStringUTFChars(env, args, NULL);

	strcpy(displayHandle[id].file_name,commandArgs);

    if(NULL == displayHandle[id].window){
        LOG(" NATIVE WINDOW IS NULL");
        return -1;
    }
    int i;
    int status;
    displayHandle[id].frame_index = 0;
    displayHandle[id].height = height;
    displayHandle[id].width = width;
    displayHandle[id].format = FORMAT_YUV_420_P;
    displayHandle[id].frame_count = 0;

    LOG("..................DISP HEIGHT - %d",displayHandle[id].height);
    LOG("..................DISP WIDTH - %d",displayHandle[id].width);
    LOG("..................DISP FMT - %x",displayHandle[id].format);

	status = ANativeWindow_setBuffersGeometry(displayHandle[id].window,  displayHandle[id].width,  displayHandle[id].height, displayHandle[id].format);
	if(0 != status){
		LOG("...........ERROR.........1");
		//return -1;
	}

	int w = ANativeWindow_getWidth(displayHandle[id].window);
	if(w != displayHandle[id].width){
		LOG("...........ERROR.........2 , w - %d",w);
		//return -1;
		//continue;
	}

	int h = ANativeWindow_getHeight(displayHandle[id].window);
	if(h != displayHandle[id].height){
		LOG("...........ERROR.........3 , h - %d",h);
		//return -1;
		//continue;
	}

	int f = ANativeWindow_getFormat(displayHandle[id].window);
	if(f != displayHandle[id].format){
		LOG("...........ERROR.........4 , f - %d",f);
		//return -1;
		//continue;
	}

    displayHandle[id].fp = fopen(commandArgs, "rb");
    if(NULL == displayHandle[id].fp){
        LOG("...........ERROR.........5");
        return -1;
    }

    for(i = 0;  ; i++){

    	LOG("FRAME COUNT - %d",i);

        int size_read;
        char *y,*u,*v;
        displayHandle[id].yuv_buffer[i] = (char *)malloc(displayHandle[id].height * displayHandle[id].width * 2);
        if(NULL == displayHandle[id].yuv_buffer[i]){
            LOG("...........MALLOC ERROR.........6");
            break;
        }

        y = displayHandle[id].yuv_buffer[i];
        v = y + displayHandle[id].height * displayHandle[id].width;
        u = v + (displayHandle[id].height * displayHandle[id].width)/4;

        /* copying y */
        size_read = fread(y, 1, displayHandle[id].height * displayHandle[id].width, displayHandle[id].fp);
        if(size_read != (displayHandle[id].height * displayHandle[id].width)){
            LOG("...........ERROR.........7, size read - %d",size_read);
            break;
        }

        /* copying u */
        size_read = fread(u, 1, (displayHandle[id].height * displayHandle[id].width)/4, displayHandle[id].fp);
        if(size_read != (displayHandle[id].height * displayHandle[id].width)/4){
            LOG("...........ERROR........8, size read - %d",size_read);
            break;
        }

        /* copying v */
        size_read = fread(v, 1, (displayHandle[id].height * displayHandle[id].width)/4, displayHandle[id].fp);
        if(size_read != (displayHandle[id].height * displayHandle[id].width)/4){
            LOG("...........ERROR.........9, size read - %d",size_read);
            break;
        }
    }

    dummy = (char *)malloc(displayHandle[id].height * displayHandle[id].width * 2);

    displayHandle[id].frame_count = i;

    fclose(displayHandle[id].fp);

    displayHandle[id].fp = NULL;

    if(0 == displayHandle[id].frame_count){
    	LOG("...........ERROR........9.5, FILE READ FAILED - %d",displayHandle[id].frame_count);
    	return -1;
    }

    LOG("HERE3 - %d",displayHandle[id].frame_count);
    (*env)->ReleaseStringUTFChars(env, args, commandArgs);

    LOG("NATIVE DISPLAY INIT PASSED - %d",displayHandle[id].frame_count);
    return 0;
}

jint Java_com_example_ndkdisplay_MainActivity_nativeDisplayDeInit(JNIEnv* env, jobject thiz, jint id)
{
	if((id >= 2)||(id < 0)){
		LOG(" INVALID WINDOW ID");
		return -1;
	}

    if(NULL == displayHandle[id].window){
        LOG(" NATIVE WINDOW IS NULL");
        return -1;
    }

    ANativeWindow_release(displayHandle[id].window);

    int i;
    for(i = 0; i < displayHandle[id].frame_count; i++){
    	if(NULL != displayHandle[id].yuv_buffer[i]){
    		free(displayHandle[id].yuv_buffer[i]);
    	}
    }

    LOG(" NATIVE DEINIT PASSED");

}

jint Java_com_example_ndkdisplay_MainActivity_nativeDisplayRenderFrame(JNIEnv* env, jobject thiz, jint id)
{
	if((id >= 2)||(id < 0)){
		LOG(" INVALID WINDOW ID");
		return -1;
	}

    if(NULL == displayHandle[id].window){
        LOG(" NATIVE WINDOW IS NULL");
        return -1;
    }

//    if(NULL == displayHandle[id].fp){
//		displayHandle[id].fp = fopen(displayHandle[id].file_name, "rb");
//		if(NULL == displayHandle[id].fp){
//			LOG("...........ERROR.........5.5");
//			return -1;
//		}
//    }

    int status,size_read;
    ANativeWindow_Buffer dispBuff;
    ARect* inOutDirtyBounds;
    char *y, *u, *v;
    char *y_dest, *u_dest, *v_dest;

    status = ANativeWindow_lock(displayHandle[id].window, &dispBuff,NULL);
    if(0 != status){
        LOG("...........ERROR.........10");
        return -1;
    }

    //copy buffer
    y = displayHandle[id].yuv_buffer[displayHandle[id].frame_index];
    v = y + displayHandle[id].height * displayHandle[id].width;
    u = v + (displayHandle[id].height * displayHandle[id].width)/4;

    y_dest = (char *)dispBuff.bits;
    //y_dest = dummy;

    v_dest = y_dest + displayHandle[id].height * displayHandle[id].width;
    u_dest = v_dest + (displayHandle[id].height * displayHandle[id].width)/4;

    memcpy(y_dest, y, displayHandle[id].height * displayHandle[id].width);
    memcpy(v_dest, v, (displayHandle[id].height * displayHandle[id].width)/4);
    memcpy(u_dest, u, (displayHandle[id].height * displayHandle[id].width)/4);

//    do{
//		/* copying y */
//		size_read = fread(y_dest, 1, displayHandle[id].height * displayHandle[id].width, displayHandle[id].fp);
//		if(size_read != (displayHandle[id].height * displayHandle[id].width)){
//			LOG("...........ERROR.........7, size read - %d",size_read);
//			fclose(displayHandle[id].fp);
//			displayHandle[id].fp = NULL;
//			break;
//		}
//
//		/* copying u */
//		size_read = fread(u_dest, 1, (displayHandle[id].height * displayHandle[id].width)/4, displayHandle[id].fp);
//		if(size_read != (displayHandle[id].height * displayHandle[id].width)/4){
//			LOG("...........ERROR........8, size read - %d",size_read);
//			fclose(displayHandle[id].fp);
//			displayHandle[id].fp = NULL;
//			break;
//		}
//
//		/* copying v */
//		size_read = fread(v_dest, 1, (displayHandle[id].height * displayHandle[id].width)/4, displayHandle[id].fp);
//		if(size_read != (displayHandle[id].height * displayHandle[id].width)/4){
//			LOG("...........ERROR.........9, size read - %d",size_read);
//			fclose(displayHandle[id].fp);
//			displayHandle[id].fp = NULL;
//			break;
//		}
//    }while(0);



    displayHandle[id].frame_index++;
    displayHandle[id].frame_index %= (displayHandle[id].frame_count);

    status = ANativeWindow_unlockAndPost(displayHandle[id].window);
    if(0 != status){
        LOG("...........ERROR.........11");
        return -1;
    }

    //LOG("NATIVE RENDER FRAME PASSED");
    return 0;
}

jint Java_com_example_ndkdisplay_MainActivity_nativeDisplaySetWindowPtr(JNIEnv* env, jobject thiz, jobject surface, jint id)
{
	if((id >= 2)||(id < 0)){
		LOG(" INVALID WINDOW ID");
		return -1;
	}

    if(NULL == surface){
        LOG("SURFACE IS NULL");
        return -1;
    }

    displayHandle[id].window = ANativeWindow_fromSurface(env, surface);
    if(NULL == displayHandle[id].window){
        LOG(" NATIVE WINDOW IS NULL");
        return -1;
    }

    ANativeWindow_acquire(displayHandle[id].window);

    LOG(" SET WINDOW PTR PASSED");
    return 0;
}

