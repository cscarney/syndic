/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "htmlsplitter.h"
#include <cstdlib>
#include <utility>

static constexpr const int heightLimit = 36;

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
        const QString attributeValue = QString::fromUtf8(originalValue.data, int(originalValue.length));
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

static void pushAnchor(QStringList &anchors, const GumboElement &element) {
    assert(element.tag == GUMBO_TAG_A);
    GumboAttribute *attr = gumbo_get_attribute(&element.attributes, "href");
    if (attr == nullptr) {
        anchors.push_back(QString());
        return;
    }
    anchors.push_back(QString::fromUtf8(attr->value));
}

static void popAnchor(QStringList &anchors) {
    anchors.pop_back();
}

void HtmlSplitter::visitElementOpen(GumboNode *node)
{
    GumboElement &element = node->v.element;
    QString tag = buildTag(element);
    switch (element.tag) {
    case GUMBO_TAG_IMG:
        createImageBlock(node, tag);
        break;

    case GUMBO_TAG_A:
        pushAnchor(m_anchors, element);
        ensureTextBlock();
        m_currentTextBlock->appendText(tag);
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
    QString textContent = QString::fromUtf8(text.original_text.data, int(text.original_text.length));
    if (!textContent.isEmpty()) {
        m_haveTextContent = true;
        m_currentTextBlock->appendText(textContent);
    }
}

void HtmlSplitter::visitElementClose(GumboNode *node)
{
    GumboElement &element = node->v.element;
    if (element.tag == GUMBO_TAG_A) {
        popAnchor(m_anchors);
    }
    m_openElements.removeLast();
    if (m_currentTextBlock != nullptr) {
        m_currentTextBlock->appendText(buildCloseTag(element));
    }
}

void HtmlSplitter::openTextBlock()
{
    m_currentTextBlock = new TextBlock(m_blockParent);
    m_haveTextContent = false;
    m_blocks.push_back(m_currentTextBlock);

    // re-open any tags that were open at the end of the last block
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
    if (!m_haveTextContent) {
        // remove text blocks that don't have any text
        m_blocks.pop_back();
        delete m_currentTextBlock;
        m_currentTextBlock = nullptr;
        return;
    }

    // close out all open tags
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

void HtmlSplitter::createImageBlock(GumboNode *node, const QString &tag)
{
    assert(node->type == GUMBO_NODE_ELEMENT);
    GumboElement &element = node->v.element;
    GumboAttribute *srcAttr = gumbo_get_attribute(&element.attributes, "src");
    if (srcAttr == nullptr){return;}
    GumboAttribute *heightAttr = gumbo_get_attribute(&element.attributes, "height");
    if (heightAttr != nullptr) {
        long int height = strtol(heightAttr->value, nullptr, 10);
        if (height < heightLimit) {
            ensureTextBlock();
            m_currentTextBlock->appendText(tag);
            return;
        }
    }
    closeTextBlock(node->parent);
    auto *image = new ImageBlock(srcAttr->value, m_blockParent);
    if (!m_anchors.isEmpty()) {
        image->m_href = m_anchors.last();
    }
    m_blocks.push_back(image);
}

ImageBlock::ImageBlock(QString src, QObject *parent):
    ContentBlock(parent),
    m_src(std::move(src))
{}

const QString &ImageBlock::delegateName() const
{
    static QString name { "ImageBlock" };
    return name;
}

QString ImageBlock::resolvedSrc(const QUrl& base)
{
    return base.resolved(m_src).toString();
}

QString ImageBlock::resolvedHref(const QUrl& base)
{
    if (m_href.isEmpty()){return QString();}
    return base.resolved(m_href).toString();
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
