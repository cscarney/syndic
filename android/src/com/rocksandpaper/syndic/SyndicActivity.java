/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

package com.rocksandpaper.syndic;

import android.content.res.Configuration;
import org.qtproject.qt5.android.bindings.QtActivity;

public class SyndicActivity extends QtActivity {
  public native void refreshStyle();

  public SyndicActivity() {
    // use the theme from the manifest...
    super.QT_ANDROID_DEFAULT_THEME = null;
  }

  @Override
  public void onConfigurationChanged(Configuration newConfig) {
    super.onConfigurationChanged(newConfig);
    refreshStyle();
  }
}
