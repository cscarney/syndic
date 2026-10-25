/**
 * SPDX-FileCopyrightText: 2026 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

static constexpr const char *kFeedTemplate = R"(<?xml version="1.0" encoding="utf-8"?>
<feed xmlns="http://www.w3.org/2005/Atom">
  <title>Example Feed</title>
  <link href="http://example.com/"/>
  <updated>2024-01-01T00:00:00Z</updated>
  <id>urn:example-feed</id>
  %1
</feed>
)";

static constexpr const char *kEntryTemplate = R"(
  <entry>
    <title>Post %1</title>
    <link href="http://example.com/posts/%1"/>
    <id>post-%1</id>
    <updated>2024-01-0%1T00:00:00Z</updated>
    <summary>Feed summary %1</summary>
  </entry>
)";

static constexpr const char *kDiscoveryPage = R"(<html><head><link rel="alternate" type="application/atom+xml" href="feed.atom"></head><body></body></html>)";
