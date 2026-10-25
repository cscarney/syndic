/**
 * SPDX-FileCopyrightText: 2026 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

"use strict";

const BADGE_CLEAR_DELAY_MS = 4000;

function showBadge(text, color) {
  ext.action.setBadgeText({text});
  ext.action.setBadgeBackgroundColor({color});
  setTimeout(() => ext.action.setBadgeText({text: ""}), BADGE_CLEAR_DELAY_MS);
}

ext.runtime.onMessage.addListener((message) => {
  if (message?.type !== "save") {
    return undefined;
  }
  return Syndic.save(message.page, message.target)
      .then(
          (result) => {
            showBadge("✓", "#2e7d32");
            return {ok: true, result};
          },
          (error) => {
            showBadge("!", "#c62828");
            return {ok: false, code: error.code};
          },
      );
});
