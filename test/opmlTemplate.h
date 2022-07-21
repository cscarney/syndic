/**
 * SPDX-FileCopyrightText: 2022 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef OPMLTEMPLATE_H
#define OPMLTEMPLATE_H

static constexpr const char *testOpmlTemplate = R"(<?xml version="1.0" encoding="UTF-8"?>
<opml version="1.0">
    <head/>
    <body>
        <outline type="rss" text="Feed 1" xmlUrl="https://feed-1.example/" />
        <outline type="rss" text="Feed 2" xmlUrl="https://feed-2.example/" />
        <outline text="Category 1">
            <outline type="rss" text="Feed 3" xmlUrl="https://feed-3.example/" />
            <outline type="rss" text="Feed 4" xmlUrl="https://feed-4.example/" />
        </outline>
    </body>
</opml>
)";

#endif // OPMLTEMPLATE_H
