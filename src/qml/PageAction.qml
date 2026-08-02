/**
 * SPDX-FileCopyrightText: 2026 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

import org.kde.kirigami as Kirigami

/**
 * A Kirigami.Action carrying a stable identifier.
 *
 * Pages are pushed by URL and are therefore transient, so anything that
 * outlives a page (such as a platform menu bar) cannot hold a reference to
 * one of its actions. Instead it holds an id and resolves it against the
 * live pageStack. The id must be stable, e.g. "feed.markAllRead".
 */
Kirigami.Action {
    property string actionId
}
