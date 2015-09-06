package com.example.ndkdisplay;

import java.io.IOException;

import android.annotation.SuppressLint;
import android.graphics.SurfaceTexture;
import android.hardware.Camera;
import android.hardware.Camera.PreviewCallback;
import android.view.Gravity;
import android.view.TextureView;
import android.view.TextureView.SurfaceTextureListener;
import android.widget.FrameLayout;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;

public class CameraUtils implements SurfaceTextureListener {

	private Camera mCamera;
	private TextureView mTexture;
	
	public CameraUtils(TextureView tv){
		mTexture = tv;
		mTexture.setSurfaceTextureListener(this);
	}
	
	@SuppressWarnings("deprecation")
	@SuppressLint("NewApi")
	@Override
	public void onSurfaceTextureAvailable(SurfaceTexture arg0, int arg1,
			int arg2) {
		mCamera = Camera.open(0);
		Camera.Size previewSize = mCamera.getParameters().getPreviewSize();
		mTexture.setLayoutParams(new LinearLayout.LayoutParams(
				previewSize.width, previewSize.height));
		try {
			mCamera.setPreviewTexture(arg0);
		} catch (IOException t) {
		}
		mCamera.startPreview();
		mTexture.setAlpha(1.0f);
		mTexture.setRotation(90.0f);
		
		mCamera.setPreviewCallbackWithBuffer(new PreviewCallback() {
			
			@Override
			public void onPreviewFrame(byte[] data, Camera camera) {
				// TODO Auto-generated method stub
				
			}
		});
	}

	@Override
	public boolean onSurfaceTextureDestroyed(SurfaceTexture arg0) {
		mCamera.stopPreview();
		mCamera.release();
		return true;
	}

	@Override
	public void onSurfaceTextureSizeChanged(SurfaceTexture arg0, int arg1,
			int arg2) {
		
	}

	@Override
	public void onSurfaceTextureUpdated(SurfaceTexture arg0) {
		// TODO Auto-generated method stub
	}
	
}
