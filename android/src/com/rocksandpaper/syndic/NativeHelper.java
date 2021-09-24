/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

package com.rocksandpaper.syndic;

import org.qtproject.qt5.android.QtNative;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;

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
} 
