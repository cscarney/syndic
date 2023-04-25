/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "htmlsplitter.h"
#include <QRegularExpression>
#include <cstdlib>
#include <utility>

/* don't create image blocks for images that are smaller than this; just display them inline with the text */
static constexpr const int kHeightLimit = 36;

/* parse numeric attributes in base 10 */
static constexpr const int kNumericAttributeBase = 10;

HtmlSplitter::HtmlSplitter(const QString &input, QObject *blockParent)
    : GumboVisitor(input)
    , m_blockParent(blockParent)
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
    for (unsigned int i = 0; i < attributes.length; i++) {
        const GumboAttribute *attribute = static_cast<GumboAttribute *>(attributes.data[i]);
        const char *attributeName = attribute->name;
        const GumboStringPiece &originalValue = attribute->original_value;
        const QString attributeValue = QString::fromUtf8(originalValue.data, int(originalValue.length));
        tag.append(" ");
        tag.append(attributeName);
        tag.append("=");
        tag.append(attributeValue);
    }
    if (element.children.length == 0) {
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

static void pushAnchor(QStringList &anchors, const GumboElement &element)
{
    assert(element.tag == GUMBO_TAG_A);
    GumboAttribute *attr = gumbo_get_attribute(&element.attributes, "href");
    if (attr == nullptr) {
        anchors.push_back(QString());
        return;
    }
    anchors.push_back(QString::fromUtf8(attr->value));
}

static void popAnchor(QStringList &anchors)
{
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
        if (node->type != GUMBO_NODE_WHITESPACE) {
            m_haveTextContent = true;
        }
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
    if (m_currentTextBlock == nullptr) {
        return;
    }
    if (!m_haveTextContent) {
        // remove text blocks that don't have any text
        m_blocks.pop_back();
        delete m_currentTextBlock;
        m_currentTextBlock = nullptr;
        return;
    }

    // close out all open tags
    const auto &rootNode = root();
    for (;;) {
        assert(currentNode->type == GUMBO_NODE_ELEMENT);
        m_currentTextBlock->appendText(buildCloseTag(currentNode->v.element));
        if (currentNode == rootNode) {
            break;
        }
        currentNode = currentNode->parent;
    }
    m_currentTextBlock = nullptr;
}

static QSize getImageSize(GumboElement &el)
{
    QSize result{0, 0};
    if (GumboAttribute *heightAttr = gumbo_get_attribute(&el.attributes, "height")) {
        result.setHeight(strtol(heightAttr->value, nullptr, kNumericAttributeBase));
    }
    if (GumboAttribute *widthAttr = gumbo_get_attribute(&el.attributes, "width")) {
        result.setWidth(strtol(widthAttr->value, nullptr, kNumericAttributeBase));
    }
    return result;
}

void HtmlSplitter::createImageBlock(GumboNode *node, const QString &tag)
{
    assert(node->type == GUMBO_NODE_ELEMENT);
    GumboElement &element = node->v.element;

    // TODO maybe scan the attribute list once instead of searching for each attr separately?
    GumboAttribute *srcAttr = gumbo_get_attribute(&element.attributes, "src");
    if (srcAttr == nullptr) {
        return;
    }

    QSize size = getImageSize(element);
    if (auto h = size.height(); h && h < kHeightLimit) {
        ensureTextBlock();
        m_currentTextBlock->appendText(tag);
        return;
    }

    closeTextBlock(node->parent);
    auto *image = new ImageBlock(srcAttr->value, m_blockParent);
    if (!m_anchors.isEmpty()) {
        image->m_href = m_anchors.last();
    }
    GumboAttribute *titleAttr = gumbo_get_attribute(&element.attributes, "title");
    if (titleAttr != nullptr) {
        image->m_title = titleAttr->value;
    }

    image->m_size = size;
    m_blocks.push_back(image);
}

ImageBlock::ImageBlock(QString src, QObject *parent)
    : ContentBlock(parent)
    , m_src(std::move(src))
{
}

const QString &ImageBlock::delegateName() const
{
    static QString name{"ImageBlock"};
    return name;
}

QString ImageBlock::resolvedSrc(const QUrl &base)
{
    return base.resolved(m_src).toString();
}

QString ImageBlock::resolvedHref(const QUrl &base)
{
    if (m_href.isEmpty()) {
        return QString();
    }
    return base.resolved(m_href).toString();
}

float ImageBlock::aspectRatio()
{
    if (m_size.height() && m_size.width()) {
        return m_size.width() / float(m_size.height());
    }

    if (m_sizeGuess.height() && m_sizeGuess.width()) {
        return m_sizeGuess.width() / float(m_sizeGuess.height());
    }

    constexpr const float kDefaultAspectRatio = 5 / 3.0F;
    return kDefaultAspectRatio;
}

QSize ImageBlock::sizeGuess()
{
    if (m_sizeGuess.isValid()) {
        return m_sizeGuess;
    }

    static QRegularExpression extractSize("([1-9]\\d{1,3})x([1-9]\\d{1,3})");
    QRegularExpressionMatch match;
    if (m_src.lastIndexOf(extractSize, -1, &match) < 0) {
        m_sizeGuess = {0, 0};
    } else {
        m_sizeGuess = {static_cast<int>(std::stol(match.captured(1).toLatin1().data(), nullptr, kNumericAttributeBase)),
                       static_cast<int>(std::stol(match.captured(2).toLatin1().data(), nullptr, kNumericAttributeBase))};
    }
    return m_sizeGuess;
}

const QString &TextBlock::delegateName() const
{
    static QString name{"TextBlock"};
    return name;
}

void TextBlock::appendText(const QString &text)
{
    m_text.append(text);
}
