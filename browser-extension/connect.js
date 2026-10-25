/**
 * SPDX-FileCopyrightText: 2026 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

"use strict";

const PING_INTERVAL_MS = 1000;
const HOST_PERMISSION = {
  origins: ["http://127.0.0.1/*"]
};

function showOnly(id) { showSections(["permission", "launch", "pairing"], id); }

const sleep = (ms) => new Promise((resolve) => setTimeout(resolve, ms));

async function ensurePermission() {
  if (await ext.permissions.contains(HOST_PERMISSION)) {
    return;
  }
  showOnly("permission");
  setStatus(t("statusWaitingForPermission"), {busy: false});
  await new Promise((resolve) => {
    $("grant").onclick = async () => {
      if (await ext.permissions.request(HOST_PERMISSION)) {
        resolve();
      }
    };
  });
}

async function waitForSyndic() {
  for (let attempt = 0;; ++attempt) {
    try {
      return await Syndic.ping();
    } catch (e) {
      if (attempt === 0) {
        showOnly("launch");
        setStatus(t("statusWaitingForStart"));
      }
      await sleep(PING_INTERVAL_MS);
    }
  }
}

async function main() {
  await ensurePermission();
  showOnly(null);
  setStatus(t("statusLookingForSyndic"));
  const status = await waitForSyndic();
  if (status.paired) {
    showOnly(null);
    setStatus(t("statusAlreadyConnected"), {busy: false});
    return;
  }

  showOnly(null);
  setStatus(t("statusRequestingAccess"));
  const {requestId, code} = await Syndic.beginPairing(t("pairingClientName"));
  showOnly("pairing");
  $("code").textContent = code;
  setStatus(t("statusWaitingForApproval"));

  if (await Syndic.waitForPairing(requestId)) {
    showOnly(null);
    setStatus(t("statusConnected"), {busy: false});
    setTimeout(() => window.close(), 3000);
  } else {
    showOnly(null);
    setStatus(t("statusDenied"), {busy: false, error: true});
  }
}

localizeDocument();
main().catch(
    (e) => setStatus(t("statusFailed", e.code ? errorText(e.code) : e.message),
                     {busy: false, error: true}));
