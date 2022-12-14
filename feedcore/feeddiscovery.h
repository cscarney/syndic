#pragma once

#include "gumbovisitor.h"
#include <QUrl>

namespace FeedCore
{

class FeedDiscovery : public GumboVisitor
{
public:
    static QUrl discoverFeed(const QUrl &url, const QByteArray &html);

private:
    explicit FeedDiscovery(const QUrl &url, const QByteArray &html);
    void visitLinkElement(const GumboElement &element);
    void visitAnchorElement(const GumboElement &element);
    void visitElementOpen(GumboNode *node) override;
    void discovered(int score, const QUrl &url);
    QUrl m_bestCandidate;
    int m_bestScore;
};

}
