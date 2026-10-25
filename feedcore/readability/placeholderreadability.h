#pragma once

#include "readability.h"

namespace FeedCore
{

class PlaceholderReadability : public FeedCore::Readability
{
public:
    PlaceholderReadability();
    ReadabilityResult *fetch(const QUrl &url) override;
    ReadabilityResult *parse(const QString &html, const QUrl &url) override;
};

} // namespace FeedCore
