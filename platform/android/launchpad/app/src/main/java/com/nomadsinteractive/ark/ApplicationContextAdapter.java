package com.nomadsinteractive.ark;

import java.io.File;

import android.content.Context;

public class ApplicationContextAdapter {
    
    private Context mContext;
    
    public ApplicationContextAdapter(Context context) {
        mContext = context;
    }
    
    public File getDir(String a1, int a2) {
        return mContext.getDir(a1, a2);
    }
    
    public File getExternalFilesDir(String s) {
        return mContext.getExternalFilesDir(s);
    }
    
    public void sysCall(int id, String value) {
        // Handle sys calls from native codes
    }
}
