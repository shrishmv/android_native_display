package com.example.ndkdisplay;

import java.util.ArrayList;

import android.app.Activity;
import android.content.Context;
import android.graphics.SurfaceTexture;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuItem;
import android.view.Surface;
import android.view.SurfaceHolder;
import android.view.SurfaceHolder.Callback;
import android.view.SurfaceView;
import android.view.TextureView;
import android.view.View;
import android.view.TextureView.SurfaceTextureListener;
import android.view.View.OnClickListener;
import android.view.ViewGroup.LayoutParams;
import android.view.animation.AnimationSet;
import android.view.animation.DecelerateInterpolator;
import android.view.animation.RotateAnimation;
import android.widget.Button;
import android.widget.FrameLayout;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;
import android.widget.Toast;

import com.example.ndkdisplay.R;
import com.example.ndkdisplay.R.id;

public class MainActivity extends Activity {

	public static String 		LOGTAG = "NDK_DISPLAY";
	private DisplayTest 		testTask = null;
	//private AnimationTest 		animationTask = null;
	private Surface 			mSSurface;
	private Surface 			mTSurface;

	private Button 						mButtonView;
	private ArrayList<TestVector> mtestVectors = new ArrayList<TestVector>();
	
	private int fps_sleep = 33 * 1;
	
	View TexViewParent; 
	View SurfViewParent; 

	
	SurfaceView			mSurfaceView;
	TextureView			mTextureView;
	
	private ZoomFrameLayoutView SzoomView;
	private ZoomFrameLayoutView TzoomView;
	
	private RelativeLayout 				surf_container;
	private RelativeLayout 				tex_container;
	
	//window id
	// 0 - surfaceview
	// 1 - textureview
	
	//0 - 720
	//1 - VGA
	//2 - 180p
	private int index = 2;
	
	//true = surfaceView
	//false - textureView
	private boolean 			mSurface_or_texture = false;
	
	private OnClickListener onStartTest = new OnClickListener() {
		
		@Override
		public void onClick(View v) {
			startTest();
		}
	};
	
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		
		mtestVectors.add(new TestVector("/sdcard/mixed720p.yuv", 720, 1280));
		mtestVectors.add(new TestVector("/sdcard/news_640x480.yuv", 480, 640));
		mtestVectors.add(new TestVector("/sdcard/180p.yuv", 180, 320));
		
		mButtonView = (Button) findViewById(R.id.button);
		
		findViewById(R.id.button2).setOnClickListener(new OnClickListener() {
			
			@Override
			public void onClick(View v) {
				
				AnimationSet animSet = new AnimationSet(true);
				animSet.setInterpolator(new DecelerateInterpolator());
				animSet.setFillAfter(true);
				animSet.setFillEnabled(true);

				final RotateAnimation animRotate = new RotateAnimation(0.0f, -360.0f,
				    RotateAnimation.RELATIVE_TO_SELF, 0.5f, 
				    RotateAnimation.RELATIVE_TO_SELF, 0.5f);

				animRotate.setDuration(4000);
				animRotate.setFillAfter(true);
				animSet.addAnimation(animRotate);

				tex_container.startAnimation(animSet);
				surf_container.startAnimation(animSet);
				
			}
		});
		
		mButtonView.setOnClickListener(new OnClickListener() {
			
			@Override
			public void onClick(View v) {
				// TODO Auto-generated method st			
				surf_container.setRotation(rotation);
				
				tex_container.setRotation(rotation);
				rotation += 30.0;
				
			}
		});
		

		//SURFACEVIEW - START
		SurfViewParent = ((LayoutInflater) getSystemService(Context.LAYOUT_INFLATER_SERVICE)).inflate(R.layout.surfview, null, false);

		mSurfaceView = (SurfaceView) SurfViewParent.findViewById(id.surf_view);
		mSurfaceView.getHolder().addCallback(new Callback() {
			
			@Override
			public void surfaceDestroyed(SurfaceHolder holder) {
				Log.d(LOGTAG, " SURFACE DESTROYED , STOPING RENDER");
				
				if(null != testTask){
					testTask.stopRenderThread();
				}
				
				return;
			}
			
			@Override
			public void surfaceCreated(SurfaceHolder holder) {
				mSSurface = holder.getSurface();
				int status = nativeDisplaySetWindowPtr(mSSurface, 0);
				if(0 != status){
					Log.d(LOGTAG, " NATIVE SET CALL FAILED");
					return;
				}
			
				status = nativeDisplayInit(mtestVectors.get(index).width, 
										   mtestVectors.get(index).height,
										   mtestVectors.get(index).path, 0);
				if(0 != status){
					Log.d(LOGTAG, " NATIVE INIT CALL FAILED");
				}
				return;
			}
			
			@Override
			public void surfaceChanged(SurfaceHolder holder, int format, int width,
					int height) {
				// TODO Auto-generated method stub
				
			}
		});
		//SURFACEVIEW - END
	
		//TEXTUREVIEW - START

	
		TexViewParent = ((LayoutInflater) getSystemService(Context.LAYOUT_INFLATER_SERVICE)).inflate(R.layout.texview, null, false);
	
		mTextureView = (TextureView) TexViewParent.findViewById(id.tex_view);
		mTextureView.setScaleX(1.0001f);
		
		mTextureView.setSurfaceTextureListener(new SurfaceTextureListener() {
			
			@Override
			public void onSurfaceTextureUpdated(SurfaceTexture surface) {
			}
			
			@Override
			public void onSurfaceTextureSizeChanged(SurfaceTexture surface, int width,
					int height) {
				
			}
			
			@Override
			public boolean onSurfaceTextureDestroyed(SurfaceTexture surface) {
				
				Log.d(LOGTAG, " SURFACE DESTROYED , STOPING RENDER");
				
				if(null != testTask){
					testTask.stopRenderThread();
				}
				
				return true;
			}
			
			@Override
			public void onSurfaceTextureAvailable(SurfaceTexture surface, int width,
					int height) {
				
				mTSurface = new Surface(surface); 
				int status = nativeDisplaySetWindowPtr(mTSurface, 1);
				if(0 != status){
					Log.d(LOGTAG, " NATIVE SET CALL FAILED");
					return;
				}
			
				status = nativeDisplayInit(mtestVectors.get(index).width, 
										   mtestVectors.get(index).height,
										   mtestVectors.get(index).path, 1);
				if(0 != status){
					Log.d(LOGTAG, " NATIVE INIT CALL FAILED");
				}
				return;
			}
		});
		//TEXTUREVIEW - END		

		LinearLayout.LayoutParams tvlparams = new LinearLayout.LayoutParams(LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT);
		LinearLayout.LayoutParams svlparams = new LinearLayout.LayoutParams(LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT) ;
		
		TexViewParent.setLayoutParams(tvlparams);
		SurfViewParent.setLayoutParams(svlparams);
		SzoomView = new ZoomFrameLayoutView(this);
		TzoomView = new ZoomFrameLayoutView(this);
		
		SzoomView.addView(SurfViewParent);
		
		TzoomView.addView(TexViewParent);

		surf_container = (RelativeLayout) findViewById(R.id.surf_container);
		tex_container = (RelativeLayout) findViewById(R.id.tex_container);
		
		surf_container.addView(SzoomView);  
		surf_container.invalidate();
		
		tex_container.addView(TzoomView);  
		tex_container.invalidate();
		
		
	}
	
	
	@Override
	protected void onResume() {
		super.onResume();
		
		/**
		 * Start the video render after 1 sec or activity bring up
		 */
		Handler handler = new Handler(getMainLooper());
		handler.postDelayed(new Runnable() {
			
			@Override
			public void run() {
				startTest();
				
			}
		}, 1000);
		
	}
	
	
	
	@Override
	protected void onPause() {
		super.onPause();
		
//		if(null != testTask){
//			testTask.stopRenderThread();
//		}
		
	}
	
	/**
	 * Method to start the test in a background tests
	 */
	public void startTest(){
		
		if(null == testTask){
			testTask = new DisplayTest();
			testTask.execute();
		}else{
			Toast.makeText(getApplicationContext(), "Already running in background", Toast.LENGTH_SHORT).show();
		}
		
	}
	
	/**
	 * native method to start the ndk tests
	 */
	public static native int nativeDisplayDeInit(int id);
	public static native int nativeDisplayInit(int width, int height, String path,int id);
	public static native int nativeDisplayRenderFrame(int id);
	public static native int nativeDisplaySetWindowPtr(Surface surface,int id);

	/**
	 * Static method to load the native library
	 */
    static {
        System.loadLibrary("native_display");
    }
    
    
    
//    class AnimationTest extends AsyncTask<Void, Void, Integer> {
//
//		@Override
//		protected Integer doInBackground(Void... params) {
//			
//			while(true){
//				try {
//					Thread.sleep(1000);
//				} catch (InterruptedException e) {
//					// TODO Auto-generated catch block
//					e.printStackTrace();
//				}
//				
//				Log.d(LOGTAG, "UI - ROTATE");
//				
//				mSurfaceView.setRotation(rotation);
//				mTextureView.setRotation(rotation);
//				rotation += 30.0;
//			}
//			
//		}
//		
//		@Override
//		protected void onPostExecute(Integer result) {
//			animationTask = null;
//			 rotation = 0;
//		}
//    	
//    }
    
    float rotation = 0;
    
    /**
     * Background thread to start the ndk render tests
     * 
     * @author Shrish
     *
     */
    class DisplayTest extends AsyncTask<Void, Void, Integer> {

    	boolean runTest = true;
    	
    	public void stopRenderThread(){
    		Log.d(LOGTAG, "UI - WWWWTTTTTTFFFFFFF");
    		runTest = false;
    	}
    	
    	@Override
    	protected void onPreExecute() {
    		Log.d(LOGTAG, "UI - TESTS STARTED");
    		Toast.makeText(getApplicationContext(), "Tests Started", Toast.LENGTH_SHORT).show();
    	}
    	
		@Override
		protected Integer doInBackground(Void... params) {
			
			while(runTest){
				try {
					Thread.sleep(fps_sleep);
				} catch (InterruptedException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
				
				nativeDisplayRenderFrame(0);
				nativeDisplayRenderFrame(1);

			}
			
			nativeDisplayDeInit(0);
			nativeDisplayDeInit(1);
			
			mSSurface = null;	
			mTSurface = null;
			Log.d(LOGTAG, "EXITING RENDER THREAD");
				
			return 0;

		}
		
		@Override
		protected void onPostExecute(Integer result) {
			Log.d(LOGTAG, "UI - TESTS DONE");
			Toast.makeText(getApplicationContext(), "Tests completed", Toast.LENGTH_SHORT).show();
			testTask = null;
		}
    	
    }
    
    class TestVector {
    	public int height;
    	public int width;
    	public String path;
    	
    	public TestVector(String p, int h , int w){
    		path = p;
    		width = w;
    		height = h;
    	}
    }
}
