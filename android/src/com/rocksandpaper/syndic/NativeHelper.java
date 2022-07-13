/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

package com.rocksandpaper.syndic;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.util.DisplayMetrics;
import android.util.TypedValue;
import com.rocksandpaper.syndic.BuildConfig;
import org.qtproject.qt5.android.QtNative;

public class NativeHelper {
    public static void sendUrl(String url) {
        if (QtNative.activity() == null)
            return;
        Intent sendIntent = new Intent();
        sendIntent.setAction(Intent.ACTION_SEND);
        sendIntent.putExtra(Intent.EXTRA_TEXT, url);
        sendIntent.setType("text/plain");
        QtNative.activity().startActivity(Intent.createChooser(sendIntent,""));
    }

    public static int getColor(int attribute) {
      TypedValue result = new TypedValue();
      QtNative.activity().getTheme().resolveAttribute(attribute, result, true);
      return 0xFFFFFF & result.data;
    }

    public static float getFontScale() {
      DisplayMetrics dm =
          QtNative.activity().getResources().getDisplayMetrics();
      return dm.scaledDensity / dm.density;
    }

    public static int getVersionCode() { return BuildConfig.VERSION_CODE; }
} 
