<!--
SPDX-FileCopyrightText: 2026 Connor Carney <hello@connorcarney.com>
SPDX-License-Identifier: GPL-3.0-or-later
-->

# Save to Syndic (Firefox extension)

Adds a **Save to Syndic** button (shortcut: <kbd>Alt</kbd>+<kbd>Shift</kbd>+<kbd>S</kbd>).
It sends the page as it currently appears in the browser to Syndic, including content
that only exists after scripts have run or after you've logged in. Syndic will match the
content to an existing feed or article, or give you the option to subscribe to a feed
from the subscribed article.

## Requirements

Syndic must be built with Qt6HttpServer, and **Allow saving pages from the browser**
must be turned on in Syndic's settings (it is off by default). The extension talks to
Syndic at `http://127.0.0.1:47913` via a [`JSON API`](../doc/save-api.md).

## Testing

```sh
cd browser-extension
npx web-ext lint
npx web-ext run                                   # temporary install in a new profile
npx web-ext build                                 # creates an unsigned .zip in web-ext-artifacts/
```



If Syndic is configured to listen on a port other than 47913, set the extension's
`port` storage key from the extension's console (`about:debugging` → *Inspect*):
`browser.storage.local.set({port: 12345})`.
