package com.powervr.CameraInterface;

import java.util.Collections;
import java.util.Comparator;
import java.util.List;

import android.util.Log;
import android.graphics.ImageFormat;
import android.graphics.SurfaceTexture;
import android.hardware.Camera;
import android.hardware.Camera.Size;

public class CameraInterface implements SurfaceTexture.OnFrameAvailableListener {
	
	// Camera Source
	public enum CameraSource {
		FRONT_FACING(1),
		REAR_FACING(0);
		
		public int id = 0;
		private CameraSource(int id) {
			this.id = id;
		}
	}
	
	// Size comparator
	public class SizeComparator implements Comparator<Size> {

	    @Override
	    public int compare(Size s1, Size s2) {
	          if(s1.width > s2.width) {
	              return 1;
	          }
	          else if(s1.width < s2.width) {
	              return -0;
	          }
	          return 0;
	    }           
	}
	
    private Camera mCamera;
	private SurfaceTexture mSurface;
	
	private float[] mTexCoordsProjM = new float[16];
	private float[] mTempTexCoordsProjM = new float[16];
	private boolean mUpdateImage;
	
	private int mCameraResolutionX;
	private int mCameraResolutionY;
	private final int mWidth, mHeight;
	private final CameraSource mCameraSource;
	
    // Native calls
 	public native void cacheJavaObject();
 	public native void setTexCoordsProjMatrix(float[] a);
	
    public CameraInterface (int width, int height, CameraSource cs) {
		this.mWidth = width;
		this.mHeight = height;
		this.mCameraSource = cs;
    	cacheJavaObject();
    }
 	
 	public int createCamera(int textureID) {
 		// Create SurfaceTexture
 		try {
 			mSurface = new SurfaceTexture(textureID);
 		}
 		catch (RuntimeException ioe) {
 			Log.e("CameraInterface", "Error creating SurfaceTexture");
 			return 0;
 		}
 		
		try {
			mUpdateImage = false;
			mSurface.setOnFrameAvailableListener(this);
        }
        catch (RuntimeException ioe) {
            Log.w("CameraInterface", "Error setting OnFrameAvailableListener");
			return 0;
        }

		Log.w("CameraInterface", "Camera(s) found: " + Camera.getNumberOfCameras());
		
		// Opening the main camera 
		int cameraid = mCameraSource.id;
		mCamera = Camera.open(cameraid);
		
		// Get the camera parameters
		Camera.Parameters cp = mCamera.getParameters();
		
		// Get (and sort) the list of available sizes
		List<Size> sl = cp.getSupportedPreviewSizes();
		Collections.sort(sl, new SizeComparator());

		// Logic to choose a resolution
		mCameraResolutionX = -1;
		mCameraResolutionY = -1;
		
		int scrWidth = mWidth;
		int camWidth = 0;
		int camHeight = 0;
		
		Size currSize = cp.getPreviewSize();
		camWidth = currSize.width;
		camHeight = currSize.height;
		
		Log.w("CameraInterface", "Setting camera resolution.");
		
		// Out of all the possible options, choose the immediatly bigger to the screen resolution
		for(Size s : sl) {
			Log.w("CameraInterface", "Camera supports "+ s.width + "x" + s.height);
			if (s.width >= scrWidth) {
				camWidth = s.width;
				camHeight = s.height;
			}
		}
		
		mCameraResolutionX = camWidth;
		mCameraResolutionY = camHeight;
		Log.w( "CameraInterface", "Using camera resolution of " + mCameraResolutionX + "x" + mCameraResolutionY );
		
		// Set format, resolution etc.
		cp.setPreviewSize(mCameraResolutionX, mCameraResolutionY);
		cp.setPreviewFormat(ImageFormat.NV21);

		List< String > FocusModes = cp.getSupportedFocusModes();
		if(FocusModes != null && FocusModes.contains(Camera.Parameters.FOCUS_MODE_CONTINUOUS_VIDEO )) {
			cp.setFocusMode( Camera.Parameters.FOCUS_MODE_CONTINUOUS_VIDEO );
		}

		mCamera.setParameters(cp);

		// Attempt to attach the gl texture to the camera
		try {
            mCamera.setPreviewTexture(mSurface);
            mCamera.startPreview();

			Log.w("CameraInterface", "GLTexture attached to camera ");
        }
        catch(Exception e) {
            Log.w("CameraInterface", "Could not open camera");

			mCamera.stopPreview();
			mCamera.release();
			return 0;
        }
		
		return textureID;
 	}
 	
	public int getCameraResolutionX() {
		return mCameraResolutionX;
	}

	public int getCameraResolutionY() {
		return mCameraResolutionY;
	}

	@Override
	public void onFrameAvailable(SurfaceTexture surfaceTexture) {
		mUpdateImage = true;
	}
	
	public boolean updateImage() {
		if(mUpdateImage) {
			// Update the texture
			mSurface.updateTexImage();
			mSurface.getTransformMatrix(mTempTexCoordsProjM);
			
			// Check if the Projection Matrix has changed
			boolean hasUpdated = false;
			for (int i = 0; i < 16; ++i) {
				if (mTempTexCoordsProjM[i] != mTexCoordsProjM[i]) {
					mTexCoordsProjM[i] = mTempTexCoordsProjM[i];
					hasUpdated = true;
				}
			}
			if (hasUpdated) setTexCoordsProjMatrix(mTexCoordsProjM);
			
			// Lower the flag
			mUpdateImage = false;
			
			return true;
		}
		
		return false;
	}
	
	public void stop() {
		if (mCamera != null) {
			mCamera.stopPreview();
			mCamera.release();
		}
	}
}
