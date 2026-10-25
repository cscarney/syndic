/**
 * SPDX-FileCopyrightText: 2026 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

"use strict";

globalThis.ext = globalThis.browser ?? globalThis.chrome;

const Syndic = (() => {
  const DEFAULT_PORT = 47913;
  const PAIR_POLL_TIMEOUT_MS = 30000;
  const DEFAULT_TIMEOUT_MS = 60000;

  /**
   * An API failure.
   */
  class SyndicError extends Error {
    constructor(code, status) {
      super(code);
      this.code = code;
      this.status = status;
    }
  }

  async function getSettings() {
    const settings =
        await ext.storage.local.get({port: DEFAULT_PORT, token: ""});
    settings.port = Number(settings.port) || DEFAULT_PORT;
    return settings;
  }

  async function setToken(token) { await ext.storage.local.set({token}); }

  async function apiOrigin() {
    const {port} = await getSettings();
    return `http://127.0.0.1:${port}`;
  }

  async function request(method, path, body,
                         {timeout = DEFAULT_TIMEOUT_MS, auth = true} = {}) {
    const {token} = await getSettings();
    const headers = {};
    if (body !== undefined) {
      headers["Content-Type"] = "application/json";
    }
    if (auth && token) {
      headers["Authorization"] = `Bearer ${token}`;
    }

    let response;
    try {
      response = await fetch(`${await apiOrigin()}/api/v1/${path}`, {
        method,
        headers,
        body: body === undefined ? undefined : JSON.stringify(body),
        signal: AbortSignal.timeout(timeout),
        cache: "no-store",
      });
    } catch (e) {
      if (e.name === "TimeoutError") {
        throw new SyndicError("timeout");
      }
      throw new SyndicError("unreachable");
    }

    const data = await response.json().catch(() => ({}));
    if (!response.ok) {
      throw new SyndicError(data.error || data.status ||
                                `http-${response.status}`,
                            response.status);
    }
    return data;
  }

  /**
   * Returns {app, apiVersion, paired}
   * Throws if server is unreachable
   */
  function ping() { return request("GET", "ping", undefined, {timeout: 3000}); }

  /**
   * Start pairing; returns {requestId, code}
   */
  function beginPairing(clientName) {
    return request("POST", "pair", {clientName}, {auth: false});
  }

  /**
   * Wait until the server approves or denies the pairing request
   */
  async function waitForPairing(requestId) {
    for (;;) {
      let result;
      try {
        result =
            await request("POST", "pair/wait", {requestId},
                          {auth: false, timeout: PAIR_POLL_TIMEOUT_MS + 5000});
      } catch (e) {
        if (e.code === "denied" || e.code === "unknown") {
          return false;
        }
        throw e;
      }
      if (result.status === "approved") {
        await setToken(result.token);
        return true;
      }
      // still pending; poll again
    }
  }

  function resolve(url) { return request("POST", "resolve", {url}); }

  /**
   * Save a page.

  `page` = {url, html, title, author}
  `target` = {feed: "<feed id>"} || {feed: null} || {subscribe: {url, name}}
   */
  function save(page, target) {
    return request("POST", "save", {...page, ...target}, {timeout: 120000});
  }

  return {
    SyndicError,
    DEFAULT_PORT,
    getSettings,
    setToken,
    apiOrigin,
    ping,
    beginPairing,
    waitForPairing,
    resolve,
    save
  };
})();

globalThis.Syndic = Syndic;
