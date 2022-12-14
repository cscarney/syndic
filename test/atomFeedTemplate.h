/**
 * SPDX-FileCopyrightText: 2022 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once

static constexpr const char *testAtomFeedTemplate = R"(<?xml version="1.0" encoding="utf-8"?>
<feed xmlns="http://www.w3.org/2005/Atom">

  <title>Example Feed</title>
  <link href="http://example.org/"/>
  <updated>2003-12-13T18:30:02Z</updated>
  <author>
    <name>John Doe</name>
  </author>
  <id>urn:uuid:60a76c80-d399-11d9-b93C-0003939e0af6</id>

  <entry>
    <title>Title 1</title>
    <link href="http://example.com/title-1"/>
    <id>title-1-id</id>
    <updated>%1</updated>
    <summary>Title 1 Text</summary>
  </entry>

  <entry>
    <title>Title 2</title>
    <link href="http://example.net/title-2"/>
    <id>title-2-id</id>
    <updated>%2</updated>
    <summary>Title 1 Text</summary>
  </entry>

</feed>
)";
