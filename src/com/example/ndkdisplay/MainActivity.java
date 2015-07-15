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
import android.widget.FrameLayout;
import android.widget.LinearLayout;
import android.widget.RelativeLayout;
import android.widget.Toast;

import com.example.ndkdisplay.R;
import com.example.ndkdisplay.R.id;

public class MainActivity extends Activity {

	public static String 		LOGTAG = "NDK_DISPLAY";
	private DisplayTest 		testTask = null;
	private Surface 			mSurface;
	private ZoomFrameLayoutView zoomView;
	private RelativeLayout 		main_container;
	
	private ArrayList<TestVector> mtestVectors = new ArrayList<TestVector>();
	
	private int fps_sleep = 33 * 1;
	
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
		
		View v;
		
		if(mSurface_or_texture){
			
			//SURFACEVIEW - START
			SurfaceView			mSurfaceView;
			
			v = ((LayoutInflater) getSystemService(Context.LAYOUT_INFLATER_SERVICE)).inflate(R.layout.surfview, null, false);
	
			mSurfaceView = (SurfaceView) v.findViewById(id.surf_view);
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
					mSurface = holder.getSurface();
					int status = nativeDisplaySetWindowPtr(mSurface);
					if(0 != status){
						Log.d(LOGTAG, " NATIVE SET CALL FAILED");
						return;
					}
				
					status = nativeDisplayInit(mtestVectors.get(index).width, 
											   mtestVectors.get(index).height,
											   mtestVectors.get(index).path);
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
		
		}else{
		
			//TEXTUREVIEW - START
			TextureView			mTextureView;
		
			v = ((LayoutInflater) getSystemService(Context.LAYOUT_INFLATER_SERVICE)).inflate(R.layout.texview, null, false);
		
			mTextureView = (TextureView) v.findViewById(id.tex_view);
			//mTextureView.setScaleX(1.0001f);
			
			mTextureView.setSurfaceTextureListener(new SurfaceTextureListener() {
				
				@Override
				public void onSurfaceTextureUpdated(SurfaceTexture surface) {
					Log.d(LOGTAG, ">>>>>>>> onSurfaceTextureUpdated");
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
					
					mSurface = new Surface(surface); 
					int status = nativeDisplaySetWindowPtr(mSurface);
					if(0 != status){
						Log.d(LOGTAG, " NATIVE SET CALL FAILED");
						return;
					}
				
					status = nativeDisplayInit(mtestVectors.get(index).width, 
											   mtestVectors.get(index).height,
											   mtestVectors.get(index).path);
					if(0 != status){
						Log.d(LOGTAG, " NATIVE INIT CALL FAILED");
					}
					return;
					
				}
			});
			//TEXTUREVIEW - END		
			
		}
		
		v.setLayoutParams(new LinearLayout.LayoutParams(LayoutParams.WRAP_CONTENT, LayoutParams.WRAP_CONTENT));
		zoomView = new ZoomFrameLayoutView(this);
		zoomView.addView(v);

		main_container = (RelativeLayout) findViewById(R.id.main_container);
		main_container.addView(zoomView); 
		main_container.invalidate();
		
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
		
		if(null != testTask){
			testTask.stopRenderThread();
		}
		
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
	public static native int nativeDisplayDeInit();
	public static native int nativeDisplayInit(int width, int height, String path);
	public static native int nativeDisplayRenderFrame();
	public static native int nativeDisplaySetWindowPtr(Surface surface);

	/**
	 * Static method to load the native library
	 */
    static {
        System.loadLibrary("native_display");
    }
    
    /**
     * Background thread to start the ndk render tests
     * 
     * @author Shrish
     *
     */
    class DisplayTest extends AsyncTask<Void, Void, Integer> {

    	boolean runTest = true;
    	
    	public void stopRenderThread(){
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
				
				nativeDisplayRenderFrame();

			}
			
			nativeDisplayDeInit();
			
			mSurface = null;			
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
