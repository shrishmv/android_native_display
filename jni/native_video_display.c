

#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <jni.h>
#include <android/log.h>
#include <android/window.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "libyuv.h"
#include "convert_from.h"

#include "native_video_display.h"

#define RGB_DISPALY 1

#define FORMAT_YUV_420_P 0x32315659

#if (1 == RGB_DISPALY)

#define IMAGE_FORMAT WINDOW_FORMAT_RGBA_8888
#define MULTIPLIER 4

#elif(2 == RGB_DISPALY)

#define IMAGE_FORMAT WINDOW_FORMAT_RGB_565
#define MULTIPLIER 2

#else

#define IMAGE_FORMAT FORMAT_YUV_420_P

#endif

native_display_params displayHandle[2];


jint Java_com_example_ndkdisplay_MainActivity_nativeDisplayInit(JNIEnv* env, jobject thiz, jint width, jint height, jstring args, jint id)
{
	if((id < 0) || (id >= 2)){
		 LOG("NATIVE WINDOW INDEX IS INVALID - %d",id);
		return -1;
	}

	const char *commandArgs = NULL;
	commandArgs = (*env)->GetStringUTFChars(env, args, NULL);

	strcpy(displayHandle[id].file_name,commandArgs);

    if(NULL == displayHandle[id].window){
        LOG(" NATIVE WINDOW IS NULL - %d",id);
        return -1;
    }
    int i;
    int status;
    displayHandle[id].frame_index = 0;
    displayHandle[id].height = height;
    displayHandle[id].width = width;
    displayHandle[id].format = IMAGE_FORMAT;
    displayHandle[id].frame_count = 0;

    displayHandle[id].fp_rgb = NULL;

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
        LOG("...........ERROR.........5 - %d",id);
        return -1;
    }

    for(i = 0;  ; i++){

    	LOG("FRAME COUNT - %d",i);

        int size_read;
        char *y,*u,*v;
        displayHandle[id].yuv_buffer[i] = (char *)malloc(displayHandle[id].height * displayHandle[id].width * 2);
        if(NULL == displayHandle[id].yuv_buffer[i]){
            LOG("...........MALLOC ERROR.........6 - %d",id);
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



    displayHandle[id].frame_count = i - 1;

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

jint Java_com_example_ndkdisplay_MainActivity_nativeDisplayDeInit(JNIEnv* env, jobject thiz,jint id)
{
	if((id < 0) || (id >= 2)){
		 LOG(" NATIVE WINDOW INDEX IS INVALID - %d",id);;
		return -1;
	}

    if(NULL == displayHandle[id].window){
        LOG(" NATIVE WINDOW IS NULL - %d",id);
        return -1;
    }

    ANativeWindow_release(displayHandle[id].window);

    int i;
    for(i = 0; i < displayHandle[id].frame_count; i++){
    	if(NULL != displayHandle[id].yuv_buffer[i]){
    		free(displayHandle[id].yuv_buffer[i]);
    	}
    }

    LOG(" NATIVE DEINIT PASSED - %d",id);

}

jint Java_com_example_ndkdisplay_MainActivity_nativeDisplayRenderFrame(JNIEnv* env, jobject thiz,jint id)
{
	if((id < 0) || (id >= 2)){
		 LOG(" NATIVE WINDOW INDEX IS INVALID - %d",id);
		return -1;
	}

    if(NULL == displayHandle[id].window){
        LOG(" NATIVE WINDOW IS NULL  - %d",id);
        return -1;
    }

    int status,size_read;
    ANativeWindow_Buffer dispBuff;
    ARect* inOutDirtyBounds;
    char *y, *u, *v;
    char *y_dest, *u_dest, *v_dest;

    int ndk_buffer_height;

    status = ANativeWindow_lock(displayHandle[id].window, &dispBuff,NULL);
    if(0 != status){
        LOG("...........ERROR.........10 - %d",id);
        return -1;
    }

    //displayHandle[id].frame_index = 0;

    //LOG("...........THIS SHOULD COME ");

    //copy buffer
    y = displayHandle[id].yuv_buffer[displayHandle[id].frame_index];
    v = y + displayHandle[id].height * displayHandle[id].width;
    u = v + (displayHandle[id].height * displayHandle[id].width)/4;

    static int first_time = 1;

#if RGB_DISPALY //HERE 123

    /* convert yuv to rgb */
    int rgb_buffer_size;
    rgb_buffer_size = displayHandle[id].height * displayHandle[id].width * MULTIPLIER;

#endif //HERE 123


    if(first_time){
    	LOG("...........NDK A NATIVE WINDOW - HEIGHT - %d",dispBuff.height);
    	LOG("...........NDK A NATIVE WINDOW - WIDTH - %d",dispBuff.width);
    	LOG("...........NDK A NATIVE WINDOW - STRIDE - %d",dispBuff.stride);
    	first_time = 0;
    }


#if RGB_DISPALY

    char *rgb_dest;

    rgb_dest = (char *)dispBuff.bits;

   status = I420ToABGR(y, displayHandle[id].width,
				   	   u, displayHandle[id].width/2,
					   v, displayHandle[id].width/2,
					   rgb_dest, dispBuff.stride * MULTIPLIER,
					   displayHandle[id].width, displayHandle[id].height);
   if(0 != status){
	   LOG("<<<<<<<<<<<<<ERROR IN YUV TO RGB CONVERT");
   }


#else
    int hieght_multiple = 16;

    if(displayHandle[id].height % hieght_multiple != 0){

    	ndk_buffer_height = hieght_multiple * ((displayHandle[id].height/hieght_multiple) + 1);

    	if(first_time){
    		LOG("...........HEIGHT NOT MULTIPLE OF %d ",hieght_multiple);
    	}
    }else{
    	ndk_buffer_height = displayHandle[id].height;

    	if(first_time){
    		LOG("...........HEIGHT IS	 MULTIPLE OF %d",hieght_multiple);
    	}
    }

    ndk_buffer_height = displayHandle[id].height;

    if(first_time){
    	LOG("........... new height.........- %d",ndk_buffer_height);
    }

    if(dispBuff.width == dispBuff.stride){

		if(first_time){

			LOG("...........width and stride are same - Y IS STRIDE.........- %d", dispBuff.stride);

			first_time = 0;
		}

		y_dest = (char *)dispBuff.bits;
		v_dest = y_dest + ndk_buffer_height * displayHandle[id].width;
		u_dest = v_dest + (ndk_buffer_height * displayHandle[id].width)/4;

		memcpy(y_dest, y, displayHandle[id].height * displayHandle[id].width);
		memcpy(v_dest, v, (displayHandle[id].height * displayHandle[id].width)/4);
		memcpy(u_dest, u, (displayHandle[id].height * displayHandle[id].width)/4);

	}else {
		int i = 0;
		int y_stride = dispBuff.stride;
		int uv_stride;
		//uv_stride = 16 * (((dispBuff.width/2)/16) + 1);
		//uv_stride = 16 * (((dispBuff.stride/2)/16) + 1);

		if((dispBuff.stride/2)%16 == 0){
			uv_stride = dispBuff.stride/2;
		}else{
			uv_stride = 16 * (((dispBuff.stride/2)/16) + 1);
		}

		if(first_time){

			LOG("...........width and stride are different - Y STRIDE.........- %d", y_stride);
			LOG("...........UV STRIDE.........- %d", uv_stride);

			first_time = 0;
		}

		 //LOG(" UV STRIDE - %d",uv_stride);

		y_dest = (char *)dispBuff.bits;
		v_dest = y_dest + (ndk_buffer_height * dispBuff.stride);
		u_dest = v_dest + ((ndk_buffer_height/4) * uv_stride * 2);

		/* copy y */
		for(i = 0; i < dispBuff.height; i++){
			memcpy(y_dest, y, displayHandle[id].width);
			y_dest += dispBuff.stride;
			y += displayHandle[id].width;
		}

		/* copy v */
		for(i = 0; i < dispBuff.height/2; i++){
			memcpy(v_dest, v, displayHandle[id].width/2);
			//memset(v_dest, 0x00, displayHandle[id].width/2);
			v_dest += uv_stride;
			v += displayHandle[id].width/2;
		}

		/* copy u */
		for(i = 0; i < dispBuff.height/2; i++){
			memcpy(u_dest, u, displayHandle[id].width/2);
			//memset(u_dest, 0x00, displayHandle[id].width/2);
			u_dest += uv_stride;
			u += displayHandle[id].width/2;
		}
	}
#endif

    displayHandle[id].frame_index++;
    displayHandle[id].frame_index %= (displayHandle[id].frame_count);

    status = ANativeWindow_unlockAndPost(displayHandle[id].window);
    if(0 != status){
        LOG("...........ERROR.........11 - %d",id);
        return -1;
    }

    //LOG("NATIVE RENDER FRAME PASSED");
    return 0;
}

jint Java_com_example_ndkdisplay_MainActivity_nativeDisplaySetWindowPtr(JNIEnv* env, jobject thiz, jobject surface,jint id)
{
	if((id < 0) || (id >= 2)){
		 LOG(" NATIVE WINDOW INDEX IS INVALID - %d",id);
		return -1;
	}

    if(NULL == surface){
        LOG("SURFACE IS NULL - %d",id);
        return -1;
    }

    displayHandle[id].window = ANativeWindow_fromSurface(env, surface);
    if(NULL == displayHandle[id].window){
        LOG(" NATIVE WINDOW IS NULL - %d",id);
        return -1;
    }

    ANativeWindow_acquire(displayHandle[id].window);

    LOG(" SET WINDOW PTR PASSED - %d",id);
    return 0;
}

#ifdef __cplusplus
}  // extern "C"
#endif

