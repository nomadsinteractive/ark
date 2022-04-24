package com.nomadsinteractive.launchpad;

import java.io.IOException;
import java.io.InputStreamReader;
import java.lang.reflect.Field;
import java.util.HashMap;
import java.util.Map;
import javax.microedition.khronos.egl.EGLConfig;
import javax.microedition.khronos.opengles.GL10;

import android.app.Activity;
import android.content.pm.ActivityInfo;
import android.opengl.GLSurfaceView;
import android.os.Bundle;
import android.util.Log;
import android.view.KeyEvent;
import android.view.MotionEvent;
import android.view.SurfaceHolder;
import android.view.View;
import com.nomadsinteractive.ark.ApplicationContextAdapter;
import com.nomadsinteractive.ark.GL2JNIView;
import com.nomadsinteractive.ark.JNILib;

public class MainActivity extends Activity {
    private static final String TAG = "MainActivity";
    
    private GL2JNIView mView;
    private boolean mInitialized = false;

    @Override
    protected void onCreate(Bundle icicle) {
        super.onCreate(icicle);

        Map<String, String> infos = loadInfoMap();
        String screenOrientation = infos.get("screen_orientation");
        int requestedOrientation = ActivityInfo.SCREEN_ORIENTATION_UNSPECIFIED;
        if(screenOrientation != null) {
            try {
                Field field = ActivityInfo.class.getDeclaredField("SCREEN_ORIENTATION_" + screenOrientation.toUpperCase());
                requestedOrientation = field.getInt(null);
            } catch(NoSuchFieldException e) {
                Log.e(TAG, String.format("Invalid screen_orientation: %s", screenOrientation));
            } catch(IllegalAccessException e) {
                Log.e(TAG, e.getMessage());
            }
        }
        
        if(requestedOrientation != ActivityInfo.SCREEN_ORIENTATION_UNSPECIFIED && requestedOrientation != getRequestedOrientation()) {
            setRequestedOrientation(requestedOrientation);
            recreate();
            return;
        }

        String libraries = infos.get("libraries");
        if(libraries != null) {
            for(String j : libraries.split(";"))
                System.loadLibrary(j.trim());
        }
        
        mView = new GL2JNIView(getApplication(), false, 16, 0);
        JNILib.onCreate(new ApplicationContextAdapter(getApplicationContext()), getAssets());
        mView.setRenderer(new Renderer(mView.getHolder()));
        mView.setOnTouchListener(new View.OnTouchListener() {
            @Override
            public boolean onTouch(View v, MotionEvent event) {
                return JNILib.onEvent(event.getAction(), event.getX(), event.getY(), event.getEventTime());
            }
        });
        mView.setOnKeyListener(new View.OnKeyListener() {
            @Override
            public boolean onKey(View view, int i, KeyEvent keyEvent) {
                return JNILib.onKeyEvent(keyEvent.getAction(), keyEvent.getKeyCode(), keyEvent.getEventTime());
            }
        });
        setContentView(mView);
        mInitialized = true;
    }

    @Override
    protected void onPause() {
        super.onPause();
        if(mInitialized) {
            mView.onPause();
            JNILib.onPause();
        }
    }

    @Override
    protected void onResume() {
        super.onResume();
        if(mInitialized) {
            mView.onResume();
            JNILib.onResume();
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        if(mInitialized) {
            JNILib.onDestroy();
            mInitialized = false;
        }
    }

    @Override
    public void onBackPressed() {
        if(mInitialized) {
            JNILib.onEvent(10000, 0, 0, System.currentTimeMillis());
        }
    }

    private Map<String, String> loadInfoMap() {
        Map<String, String> infos = new HashMap<String, String>();
        try {
            InputStreamReader in = new InputStreamReader(getAssets().open("android.info"), "utf-8");
            char[] buf = new char[8192];
            StringBuilder sb = new StringBuilder();
            int len;
            while((len = in.read(buf)) > 0) {
                sb.append(buf, 0, len);
            }
            in.close();

            String s = sb.toString();
            for(String i : s.split("\n")) {
                final String line = i.trim();
                int pos = line.indexOf(':');
                if(pos != -1) {
                    String key = line.substring(0, pos).trim();
                    String value = line.substring(pos + 1).trim();
                    infos.put(key, value);
                }
            }
        } catch(IOException e) {
            Log.e(TAG, e.getMessage());
        }
        return infos;
    }

    public static class Renderer implements GLSurfaceView.Renderer {
        private SurfaceHolder mSurfaceHoder;
        
        public Renderer(SurfaceHolder surfaceHolder) {
            mSurfaceHoder = surfaceHolder;
        }
        
        public void onDrawFrame(GL10 gl) {
            JNILib.onDraw();
        }

        public void onSurfaceChanged(GL10 gl, int width, int height) {
            JNILib.onSurfaceChanged(width, height);
        }

        public void onSurfaceCreated(GL10 gl, EGLConfig config) {
            JNILib.onSurfaceCreated(mSurfaceHoder.getSurface());
        }
    }
}
