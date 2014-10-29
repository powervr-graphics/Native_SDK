package com.powervr.OGLES2TextureStreaming;

import com.powervr.CameraInterface.CameraInterface;
import com.powervr.CameraInterface.CameraInterface.CameraSource;

import android.app.NativeActivity;
import android.os.Bundle;
import android.widget.Toast;
import android.view.Gravity;
import android.graphics.Point;
import android.view.Display;

public class OGLES2TextureStreaming extends NativeActivity
{
    static 
	{
		System.loadLibrary("OGLES2TextureStreaming");
	}
	
	private CameraInterface mCameraInterface = null;

    @Override
    protected void onCreate (Bundle savedInstanceState)
    {
        super.onCreate(savedInstanceState);
		
		Display display = getWindowManager().getDefaultDisplay();
		Point size = new Point();
		display.getSize(size);
		
		mCameraInterface = new CameraInterface(size.x, size.y, CameraSource.REAR_FACING);
    }

    public void displayExitMessage(final String text) 
    {
        runOnUiThread(new Runnable() {
	    public void run() {
	    	Toast toast = Toast.makeText(getApplicationContext(), text, Toast.LENGTH_LONG);
	    	toast.setGravity(Gravity.CENTER, 0, 0);
	    	toast.show();
	    }
        });
    }
    
	@Override
    public void onPause() 
    {	
    	if (mCameraInterface != null) 
    	{
    		mCameraInterface.stop();
    		mCameraInterface = null;
    	}
    	super.onPause();
    }
}
