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
	private DisplayTest 		testTask1 = null;
	
	private RelativeLayout 		main_container;
	
	private ArrayList<TestVector> mtestVectors = new ArrayList<TestVector>();
	
	private int fps_sleep = 33 * 1;
	
	//0 - 720
	//1 - VGA
	//2 - 180p
	//3 - random
	private int index = 3;
	
	
	private OnClickListener onStartTest = new OnClickListener() {
		
		@Override
		public void onClick(View v) {
			startTest();
		}
	};
	
	private SurfaceTextureListener onSurfaceTextChanged1 = new SurfaceTextureListener() {
		
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
			
			if(null != testTask1){
				testTask1.stopRenderThread();
			}
			
			return true;
		}
		
		@Override
		public void onSurfaceTextureAvailable(SurfaceTexture surface, int width,
				int height) {
			
			Surface mSurface = new Surface(surface); 
			int status = nativeDisplaySetWindowPtr(mSurface,0);
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
	};
	
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);
		
		mtestVectors.add(new TestVector("/sdcard/mixed720p.yuv", 720, 1280));
		mtestVectors.add(new TestVector("/sdcard/news_640x480.yuv", 480, 640));
		mtestVectors.add(new TestVector("/sdcard/180p.yuv", 180, 320));
		mtestVectors.add(new TestVector("/sdcard/frame_cap.yuv", 768, 486));
		
		View viewToAdd1;

		//TEXTUREVIEW - START
		TextureView			mTextureView1;
	
		viewToAdd1 = ((LayoutInflater) getSystemService(Context.LAYOUT_INFLATER_SERVICE)).inflate(R.layout.texview, null, false);
	
		mTextureView1 = (TextureView) viewToAdd1.findViewById(id.tex_view1);
		
		mTextureView1.setSurfaceTextureListener(onSurfaceTextChanged1);

		Log.d(LOGTAG, " Adding view to main layout");
		
		viewToAdd1.setLayoutParams(new LinearLayout.LayoutParams(800, 800));
	    
		main_container = (RelativeLayout) findViewById(R.id.main_container);
		
		main_container.addView(viewToAdd1);
		
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
		
		if(null != testTask1){
			testTask1.stopRenderThread();
		}
		
	}
	
	/**
	 * Method to start the test in a background tests
	 */
	public void startTest(){
		
		if(null == testTask1){
			testTask1 = new DisplayTest();
			testTask1.execute(0);
		}else{
			Toast.makeText(getApplicationContext(), "Already running in background", Toast.LENGTH_SHORT).show();
		}
		
	}
	
	/**
	 * native method to start the ndk tests
	 */
	public static native int nativeDisplayDeInit(int id);
	public static native int nativeDisplayInit(int width, int height, String path, int id);
	public static native int nativeDisplayRenderFrame(int id);
	public static native int nativeDisplaySetWindowPtr(Surface surface, int id);

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
    class DisplayTest extends AsyncTask<Integer, Void, Integer> {

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
		protected Integer doInBackground(Integer... params) {
			
			while(runTest){
				try {
					Thread.sleep(fps_sleep);
				} catch (InterruptedException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
				
				nativeDisplayRenderFrame(0);

			}
			
			nativeDisplayDeInit(0);
			
			Log.d(LOGTAG, "EXITING RENDER THREAD");
				
			return 0;

		}
		
		@Override
		protected void onPostExecute(Integer result) {
			Log.d(LOGTAG, "UI - TESTS DONE");
			Toast.makeText(getApplicationContext(), "Tests completed", Toast.LENGTH_SHORT).show();
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
