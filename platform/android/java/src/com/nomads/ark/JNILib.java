/*
 * Copyright (C) 2007 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.nomads.ark;

import android.content.res.AssetManager;
import android.view.Surface;

public class JNILib {

    public static native void onCreate(Object applicationContext, AssetManager assetManager);

    public static native void onSurfaceCreated(Surface surface);
    /**
     * @param width  the current view width
     * @param height the current view height
     */
    public static native void onSurfaceChanged(int width, int height);

    public static native void onDraw();
    public static native void onPause();
    public static native void onResume();
    public static native void onDestroy();

    public static native boolean onEvent(int action, float x, float y, long timestamp);
}
