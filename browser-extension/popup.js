/**
 * SPDX-FileCopyrightText: 2026 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

"use strict";

const SUBSCRIBE = "subscribe";
const NO_FEED = "none";
const FEED_PREFIX = "feed:";

function show(...ids) {
  showSections(["status", "connect", "save-form", "done"], ...ids);
}

function showStatus(text, options) {
  show("status");
  setStatus(text, options);
}

function showConnect(text, buttonText) {
  show("connect");
  $("connect-text").textContent = text;
  $("connect-button").textContent = buttonText;
  $("connect-button").onclick = async () => {
    await ext.tabs.create({url: ext.runtime.getURL("connect.html")});
    window.close();
  };
}

// Runs in the page: get the current DOM state as HTML
function capturePage() {
  const meta = (name) =>
      document.querySelector(`meta[name="${name}"], meta[property="${name}"]`)
          ?.content?.trim() ||
      "";
  const doctype =
      document.doctype
          ? new XMLSerializer().serializeToString(document.doctype) + "\n"
          : "";
  return {
    url: location.href,
    html: doctype + document.documentElement.outerHTML,
    title: meta("og:title") || document.title.trim(),
    author: meta("author") || meta("article:author"),
  };
}

async function getPage() {
  const [tab] = await ext.tabs.query({active: true, currentWindow: true});
  if (!tab || !/^https?:/.test(tab.url ?? "")) {
    return null;
  }
  const [injection] = await ext.scripting.executeScript(
      {target: {tabId: tab.id}, func: capturePage});
  return injection?.result ?? null;
}

function option(value, text) {
  const element = document.createElement("option");
  element.value = value;
  element.textContent = text;
  return element;
}

// The last choice the user made between subscribing to a discovered feed and
// saving without a feed.
const DISCOVERED_CHOICE_KEY = "discoveredFeedChoice";

async function populateFeeds(resolution) {
  const select = $("feed");
  select.replaceChildren();

  if (resolution.discovered) {
    select.append(option(SUBSCRIBE,
                         t("feedOptionSubscribe", resolution.discovered.name)));
  }
  select.append(option(NO_FEED, t("feedOptionNone")));

  const categories = new Map();
  for (const feed of resolution.feeds) {
    if (!categories.has(feed.category)) {
      categories.set(feed.category, []);
    }
    categories.get(feed.category).push(feed);
  }
  for (const [category, feeds] of categories) {
    let parent = select;
    if (category) {
      parent = document.createElement("optgroup");
      parent.label = category;
      select.append(parent);
    }
    for (const feed of feeds) {
      parent.append(option(FEED_PREFIX + feed.id, feed.name || feed.id));
    }
  }

  const existing = resolution.article;
  if (existing) {
    // the page is merged into the existing article, wherever that is
    select.value = existing.feed ? FEED_PREFIX + existing.feed : NO_FEED;
    select.disabled = true;
  } else if (resolution.discovered) {
    const stored =
        await ext.storage.local.get({[DISCOVERED_CHOICE_KEY]: SUBSCRIBE});
    select.value =
        stored[DISCOVERED_CHOICE_KEY] === NO_FEED ? NO_FEED : SUBSCRIBE;
  } else if (resolution.suggestedFeed) {
    select.value = FEED_PREFIX + resolution.suggestedFeed;
  } else {
    select.value = NO_FEED;
  }
}

async function rememberDiscoveredChoice(resolution) {
  const value = $("feed").value;
  if (resolution.discovered && !resolution.article &&
      (value === SUBSCRIBE || value === NO_FEED)) {
    await ext.storage.local.set({[DISCOVERED_CHOICE_KEY]: value});
  }
}

function targetFor(resolution) {
  const value = $("feed").value;
  if (value === SUBSCRIBE) {
    return {
      subscribe:
          {url: resolution.discovered.url, name: resolution.discovered.name}
    };
  }
  if (value.startsWith(FEED_PREFIX)) {
    return {feed: value.slice(FEED_PREFIX.length)};
  }
  return {feed: null};
}

async function showForm(page, resolution) {
  const existing = resolution.article;
  $("title").value = existing?.title || page.title;
  $("title").disabled = !!existing;
  await populateFeeds(resolution);
  show("save-form");

  $("cancel").onclick = () => window.close();
  $("save-form").onsubmit = async (event) => {
    event.preventDefault();
    $("save").disabled = true;
    const target = existing ? {} : targetFor(resolution);
    await rememberDiscoveredChoice(resolution);
    showStatus(t(target.subscribe ? "statusSubscribing" : "statusSaving"));
    const response = await ext.runtime.sendMessage({
      type: "save",
      page: {...page, title: $("title").value.trim()},
      target,
    });
    if (response?.ok) {
      show("done");
      const {updatedExisting, feedName} = response.result;
      $("done-text").textContent = updatedExisting ? t("doneUpdated")
                                   : feedName ? t("doneSavedToFeed", feedName)
                                              : t("doneSaved");
      setTimeout(() => window.close(), 1200);
    } else {
      showStatus(errorText(response?.code), {busy: false, error: true});
    }
  };
  $("save").focus();
}

async function main() {
  let page;
  try {
    page = await getPage();
  } catch (e) {
    page = null;
  }
  if (!page) {
    showStatus(t("pageNotSavable"), {busy: false});
    return;
  }

  let status;
  try {
    status = await Syndic.ping();
  } catch (e) {
    showConnect(t("notRunning"), t("buttonStartSyndicEllipsis"));
    return;
  }
  if (!status.paired) {
    showConnect(t("notConnected"), t("buttonConnectEllipsis"));
    return;
  }

  showStatus(t("statusCheckingFeed"));
  let resolution;
  try {
    resolution = await Syndic.resolve(page.url);
  } catch (e) {
    if (e.code === "unauthorized") {
      showConnect(t("tokenRejected"), t("buttonReconnectEllipsis"));
    } else {
      showStatus(errorText(e.code), {busy: false, error: true});
    }
    return;
  }
  showForm(page, resolution);
}

localizeDocument();
main();
