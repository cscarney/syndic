<!--
SPDX-FileCopyrightText: 2026 Connor Carney <hello@connorcarney.com>
SPDX-License-Identifier: GPL-3.0-or-later
-->

# Save to Syndic API

Syndic can save web pages that are supplied from the browser extension in `browser-extension/`. 
This document describes the local HTTP interface that the extension uses.

The server is only available when Syndic is built with Qt6HttpServer, and it only
listens when **Allow saving pages from the browser** is turned on in Syndic's settings.
It is off by default.

Endpoint is located at `http://127.0.0.1:<port>/api/v1/`. Request and response
bodies are JSON. Errors have the form `{"error": "<code>"}` with an appropriate HTTP
status. Error codes are machine-readable; clients are responsible for showing localized
text. `save` can fail with `invalid-url`, `invalid-feed`, `extraction-failed`,
`subscribe-failed` or `save-failed`.

## `GET ping`

No authentication needed. Returns:

```json
{"app": "syndic", "apiVersion": 1, "paired": true}
```

`paired` reports whether the request carried a valid token.

## `POST pair`

No authentication needed. Starts a pairing request.

```json
→ {"clientName": "Save to Syndic browser extension"}
← {"requestId": "…", "code": "4821"}
```

Show `code` to the user, then call `pair/wait`.

## `POST pair/wait`

Waits up to about 25 seconds for the user's decision.

```json
→ {"requestId": "…"}
← {"status": "approved", "token": "…"}   200; the token is only returned once
← {"status": "pending"}                  200; call again
← {"status": "denied"}                   403
← {"status": "unknown"}                  404; no such request
```

## `POST resolve`

Determines where a page should be saved. May require a network round trip for feed discovery.

```json
→ {"url": "https://example.com/2026/10/some-post"}
← {
    "article": {"title": "…", "url": "…", "feed": "<feed id>" | null, "feedName": "…" | null},
    "suggestedFeed": "<feed id>",
    "discovered": {"url": "https://example.com/feed.xml", "name": "Example", "isWebPage": false},
    "feeds": [{"id": "<feed id>", "name": "…", "category": "…"}, …]
  }
```

`article`, `suggestedFeed` and `discovered` are present only when they apply.
`feeds` lists every subscription, sorted by category and name. A feed's ID is its
feed URL.

## `POST save`

```json
→ {
    "url": "https://example.com/2026/10/some-post",
    "html": "<!DOCTYPE html><html>…",
    "title": "Some post",
    "author": "Someone",

    "feed": "<feed id>" | null,
    "subscribe": {"url": "<discovered feed url>", "name": "Example"}
  }
← {"title": "…", "url": "…", "feed": "<feed id>" | null, "feedName": "…" | null, "updatedExisting": false}
```

* `html` is optional. If it's missing, Syndic downloads the page itself.
* `title` and `author` are optional
* If `subscribe` is present, Syndic subscribes to that feed and saves into it.
  Otherwise the article goes into the feed named by `feed`
* Articles can be saved without a feed.

## Example

```sh
B=http://127.0.0.1:47913/api/v1
curl -s $B/ping
curl -s -d '{"clientName":"curl"}' $B/pair            # approve in Syndic
curl -s -d '{"requestId":"<id>"}' $B/pair/wait        # → token
curl -s -H "Authorization: Bearer $TOKEN" -d '{"url":"https://example.com/post"}' $B/save
```
