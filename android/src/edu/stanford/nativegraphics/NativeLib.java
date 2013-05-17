package edu.stanford.nativegraphics;

import android.annotation.SuppressLint;
import android.content.Context;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;

public class NativeLib {
	private static final String TAG = "NativeLib";
	private Context mContext;
	
	public NativeLib(Context context, int width, int height) {
		mContext = context;
		init(width, height);
	}
	
    static {
        System.loadLibrary("nativegraphics");
    }

    // Called from native
    public String stringCallback(String fileName) {
    	String splitName = fileName.split("\\.")[0];
    	int resID = mContext.getResources().getIdentifier(splitName, "raw", "edu.stanford.nativegraphics");
    	if(resID == 0)
    		return null;
        return RawResourceReader.readTextFileFromRawResource(mContext, resID);
    }
    
    // Called from native
	public Bitmap drawableCallback(String fileName) {
    	String splitName = fileName.split("\\.")[0];
    	int resID = mContext.getResources().getIdentifier(splitName, "drawable", "edu.stanford.nativegraphics");
    	if(resID == 0)
    		return null;
		final BitmapFactory.Options options = new BitmapFactory.Options();
		options.inScaled = false;	// No pre-scaling
		System.gc();
    	return BitmapFactory.decodeResource(mContext.getResources(), resID, options);
    }

    // Native Functions
    private native void init(int width, int height);
    public native void renderFrame();
    public static native void pointerDown(float x, float y);
    public static native void pointerMove(float x, float y);
    public static native void pointerUp(float x, float y);
}
