/**
 * SPDX-FileCopyrightText: 2026 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

"use strict";

const $ = (id) => document.getElementById(id);

/**
 * Show the sections in `visible` and hide the rest of `all`.
 */
function showSections(all, ...visible) {
  for (const id of all) {
    $(id).hidden = !visible.includes(id);
  }
}

/**
 * Update the #status line.
 */
function setStatus(text, {busy = true, error = false} = {}) {
  $("status").classList.toggle("idle", !busy);
  $("status-text").textContent = text;
  $("status-text").classList.toggle("error", error);
}
