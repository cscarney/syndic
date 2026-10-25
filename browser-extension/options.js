/**
 * SPDX-FileCopyrightText: 2026 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

"use strict";

async function refresh() {
  const {token} = await Syndic.getSettings();
  $("forget").disabled = !token;
  $("status").textContent =
      t(token ? "optionsConnected" : "optionsNotConnected");
}

$("forget").onclick = async () => {
  await Syndic.setToken("");
  refresh();
};

$("connect").onclick = () =>
    ext.tabs.create({url: ext.runtime.getURL("connect.html")});

localizeDocument();
refresh();
