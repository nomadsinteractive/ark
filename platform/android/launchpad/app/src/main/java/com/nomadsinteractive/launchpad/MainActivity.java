package com.nomadsinteractive.launchpad;

import java.io.IOException;
import java.io.InputStreamReader;
import java.lang.reflect.Field;
import java.util.HashMap;
import java.util.Map;

import android.content.pm.ActivityInfo;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import com.nomadsinteractive.ark.GL2JNIView;
import com.nomadsinteractive.ark.JNILib;

public class MainActivity extends AppCompatActivity {
    private static String TAG = "MainActivity";
    
        private GL2JNIView mView;
    
        @Override
        protected void onCreate(Bundle icicle) {
            Map<String, String> infos = loadInfoMap();
            String libraries = infos.get("libraries");
            if(libraries != null) {
                for(String j : libraries.split(";"))
                    System.loadLibrary(j.trim());
            }
    
            String screenOrientation = infos.get("screen_orientation");
            if(screenOrientation != null) {
                try {
                    Field field = ActivityInfo.class.getDeclaredField("SCREEN_ORIENTATION_" + screenOrientation.toUpperCase());
                    setRequestedOrientation(field.getInt(null));
                } catch(NoSuchFieldException e) {
                    Log.e(TAG, String.format("Invalid screen_orientation: %s", screenOrientation));
                } catch(IllegalAccessException e) {
                    Log.e(TAG, e.getMessage());
                }
            }
    
            super.onCreate(icicle);
            mView = new GL2JNIView(getApplication(), false, 16, 0);
            setContentView(mView);
        }
    
        @Override
        protected void onPause() {
            super.onPause();
            mView.onPause();
            JNILib.onPause();
        }
    
        @Override
        protected void onResume() {
            super.onResume();
            mView.onResume();
            JNILib.onResume();
        }
    
        @Override
        protected void onDestroy() {
            super.onDestroy();
            JNILib.onDestroy();
        }
    
        @Override
        public void onBackPressed() {
            JNILib.onEvent(10000, 0, 0, System.currentTimeMillis());
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
}
