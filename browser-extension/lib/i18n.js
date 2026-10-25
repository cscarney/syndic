/**
 * SPDX-FileCopyrightText: 2026 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

/*
 * Localization helpers. Strings live in _locales/<lang>/messages.json.
 *
 * Mark translatable elements with data-i18n="messageName" (text content)
 * or data-i18n-<attribute>="messageName" (attribute value).
 */

"use strict";

function t(name, ...substitutions) {
  const message = ext.i18n.getMessage(name, substitutions.map(String));
  if (!message) {
    console.warn(`Missing translation: ${name}`);
    return name;
  }
  return message;
}

function localizeDocument(root = document) {
  document.documentElement.lang =
      ext.i18n.getMessage("@@ui_locale").replace("_", "-");
  document.documentElement.dir = ext.i18n.getMessage("@@bidi_dir") || "ltr";

  for (const element of root.querySelectorAll("*")) {
    for (const attribute of element.getAttributeNames()) {
      if (attribute === "data-i18n") {
        element.textContent = t(element.dataset.i18n);
      } else if (attribute.startsWith("data-i18n-")) {
        element.setAttribute(attribute.slice("data-i18n-".length),
                             t(element.getAttribute(attribute)));
      }
    }
  }
}

const ERROR_MESSAGES = {
  "unreachable": "errorUnreachable",
  "timeout": "errorTimeout",
  "unauthorized": "errorUnauthorized",
  "forbidden-host": "errorForbidden",
  "forbidden-origin": "errorForbidden",
  "invalid-url": "errorInvalidUrl",
  "invalid-feed": "errorInvalidFeed",
  "too-large": "errorTooLarge",
  "extraction-failed": "errorExtractionFailed",
  "subscribe-failed": "errorSubscribeFailed",
  "save-failed": "errorSaveFailed",
};

/** A user-facing message for a Syndic API error code. */
function errorText(code) {
  const name = ERROR_MESSAGES[code];
  return name ? t(name) : t("errorUnknown", code ?? "?");
}

globalThis.t = t;
globalThis.localizeDocument = localizeDocument;
globalThis.errorText = errorText;
