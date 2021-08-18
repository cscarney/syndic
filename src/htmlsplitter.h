/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef HTMLSPLITTER_H
#define HTMLSPLITTER_H
#include <QObject>
#include <QUrl>
#include "gumbovisitor.h"

class ContentBlock;
class TextBlock;
/**
 * Separate text and images into blocks that can be rendered separately.
 */
class HtmlSplitter : public GumboVisitor
{
public:
    /**
     * Accept an HTML string & return a list of alternating text and image blocks.  The document is split
     * wherever an image is found (excluding small images).  The text blocks contain HTML that can be
     * rendered in a QML text object.
     *
     * The resulting block objects are owned by /blockParent/.
     */
    static QVector<ContentBlock*> cleanHtml(const QString &input, QObject *blockParent=nullptr);
private:
    HtmlSplitter(const QString &input, QObject *blockParent=nullptr);
    void visitElementOpen(GumboNode *node) override;
    void visitText(GumboNode *node) override;
    void visitElementClose(GumboNode *node) override;
    QVector<ContentBlock*> m_blocks;
    TextBlock *m_currentTextBlock{nullptr};
    bool m_haveTextContent{false};
    QStringList m_openElements;
    QStringList m_anchors;
    QObject *m_blockParent;
    void openTextBlock();
    void splitTextBlock(GumboNode *currentNode);
    void ensureTextBlock();
    void closeTextBlock(GumboNode *currentNode);
    void createImageBlock(GumboNode *node, const QString &tag);
};


class ContentBlock : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString delegateName READ delegateName CONSTANT)
public:
    ContentBlock(QObject *parent=nullptr): QObject(nullptr){}
    virtual const QString &delegateName() const = 0;
};

class TextBlock : public ContentBlock {
    Q_OBJECT
    Q_PROPERTY(QString text MEMBER m_text CONSTANT)
public:
    TextBlock(QObject *parent=nullptr) : ContentBlock(parent){}
    const QString &delegateName() const override;
private:
    QString m_text { "" };
    void appendText(const QString &text);
    friend HtmlSplitter;
};

class ImageBlock : public ContentBlock {
    Q_OBJECT
public:
    ImageBlock(QString src, QObject *parent=nullptr);
    const QString &delegateName() const override;
    Q_INVOKABLE QString resolvedSrc(QUrl base);
    Q_INVOKABLE QString resolvedHref(QUrl base);
private:
    QString m_src;
    QString m_href;
    friend HtmlSplitter;
};

#endif // HTMLSPLITTER_H
