/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "htmlsplitter.h"

#include <utility>

HtmlSplitter::HtmlSplitter(const QString &input, QObject *blockParent) :
    GumboVisitor(input),
    m_blockParent(blockParent)
{
    walk();
}

QVector<ContentBlock *> HtmlSplitter::cleanHtml(const QString &input, QObject *blockParent)
{
    return HtmlSplitter(input, blockParent).m_blocks;
}

static QString buildTag(const GumboElement &element)
{
    QString tag = "<";
    tag.append(gumbo_normalized_tagname(element.tag));
    const GumboVector &attributes = element.attributes;
    for(unsigned int i=0; i<attributes.length; i++) {
        const GumboAttribute *attribute = static_cast<GumboAttribute *>(attributes.data[i]);
        const char *attributeName = attribute->name;
        const GumboStringPiece &originalValue = attribute->original_value;
        const QString attributeValue = QString::fromUtf8(originalValue.data, originalValue.length);
        tag.append(" ");
        tag.append(attributeName);
        tag.append("=");
        tag.append(attributeValue);
    }
    if (element.children.length==0) {
        tag.append(" /");
    }
    tag.append(">");
    return tag;
}

static QString buildCloseTag(const GumboElement &element)
{
    QString tag = "</";
    tag.append(gumbo_normalized_tagname(element.tag));
    tag += ">";
    return tag;
}

void HtmlSplitter::visitElementOpen(GumboNode *node)
{
    GumboElement &element = node->v.element;
    QString tag = buildTag(element);
    switch (element.tag) {
    case GUMBO_TAG_IMG:
        createImageBlock(node);
        break;

    default:
        ensureTextBlock();
        m_currentTextBlock->appendText(tag);
    }
    m_openElements << tag;
}

void HtmlSplitter::visitText(GumboNode *node)
{
    GumboText &text = node->v.text;
    ensureTextBlock();
    m_currentTextBlock->appendText(QString::fromUtf8(text.original_text.data, text.original_text.length));
}

void HtmlSplitter::visitElementClose(GumboNode *node)
{
    m_openElements.removeLast();
    if (m_currentTextBlock != nullptr) {
        m_currentTextBlock->appendText(buildCloseTag(node->v.element));
    }
}

void HtmlSplitter::openTextBlock()
{
    m_currentTextBlock = new TextBlock(m_blockParent);
    m_blocks.push_back(m_currentTextBlock);
    const QStringList &openElements = m_openElements;
    for (const QString &element : openElements) {
        m_currentTextBlock->appendText(element);
    }
}

void HtmlSplitter::splitTextBlock(GumboNode *currentNode)
{
    if (m_currentTextBlock != nullptr) {
        closeTextBlock(currentNode);
    }
    openTextBlock();
}

void HtmlSplitter::ensureTextBlock()
{
    if (m_currentTextBlock == nullptr) {
        openTextBlock();
    }
}

void HtmlSplitter::closeTextBlock(GumboNode *currentNode)
{
    if (m_currentTextBlock == nullptr){return;}
    const auto &rootNode = root();
    for(;;) {
        assert(currentNode->type == GUMBO_NODE_ELEMENT);
        m_currentTextBlock->appendText(buildCloseTag(currentNode->v.element));
        if (currentNode==rootNode) {
            break;
        }
        currentNode = currentNode->parent;
    }
    m_currentTextBlock = nullptr;
}

void HtmlSplitter::createImageBlock(GumboNode *node)
{
    assert(node->type == GUMBO_NODE_ELEMENT);
    closeTextBlock(node->parent);
    GumboElement &element = node->v.element;
    GumboAttribute *attr = gumbo_get_attribute(&element.attributes, "src");
    if (attr == nullptr){return;}
    QUrl src = QString(attr->value);
    auto *image = new ImageBlock(src, m_blockParent);
    m_blocks.push_back(image);
}

ImageBlock::ImageBlock(QUrl src, QObject *parent):
    ContentBlock(parent),
    m_src(std::move(src))
{}

const QString &ImageBlock::delegateName() const
{
    static QString name { "ImageBlock" };
    return name;
}

const QString &TextBlock::delegateName() const
{
    static QString name { "TextBlock" };
    return name;
}

void TextBlock::appendText(const QString &text)
{
    m_text.append(text);
}
