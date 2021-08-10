#ifndef HTMLSPLITTER_H
#define HTMLSPLITTER_H
#include <QObject>
#include <QUrl>
#include "gumbovisitor.h"

class HtmlSplitter;
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
    Q_PROPERTY(QUrl src MEMBER m_src CONSTANT)
public:
    ImageBlock(QUrl src, QObject *parent=nullptr);
    const QString &delegateName() const override;
private:
    QUrl m_src;
};

class HtmlSplitter : public GumboVisitor
{
public:
    HtmlSplitter(const QString &input, QObject *blockParent=nullptr);
    static QVector<ContentBlock*> cleanHtml(const QString &input, QObject *blockParent=nullptr);

    void visitElementOpen(GumboNode *node) override;
    void visitText(GumboNode *node) override;
    void visitElementClose(GumboNode *node) override;
private:
    QVector<ContentBlock*> m_blocks;
    TextBlock *m_currentTextBlock=nullptr;
    QStringList m_openElements;
    QObject *m_blockParent;
    void openTextBlock();
    void splitTextBlock(GumboNode *currentNode);
    void ensureTextBlock();
    void closeTextBlock(GumboNode *currentNode);
    void createImageBlock(GumboNode *node);
};

#endif // HTMLSPLITTER_H
