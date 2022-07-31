/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "gumbovisitor.h"
using namespace FeedCore;

GumboVisitor::GumboVisitor(const QString &input)
    : GumboVisitor(input.toUtf8())
{
}

GumboVisitor::GumboVisitor(const QByteArray &utf8Data)
    : m_data{utf8Data}
    , m_gumbo{gumbo_parse(m_data)}
    , m_root{m_gumbo->root}
    , m_node{m_root}
{
}

void GumboVisitor::walk()
{
    while (m_node != nullptr) {
        switch (m_node->type) {
        case GUMBO_NODE_TEXT:
        case GUMBO_NODE_CDATA:
        case GUMBO_NODE_WHITESPACE:
            visitText(m_node);
            break;
        case GUMBO_NODE_ELEMENT: {
            visitElementOpen(m_node);
            GumboElement &element = m_node->v.element;
            if (element.children.length > 0) {
                m_node = static_cast<GumboNode *>(element.children.data[0]);
                continue;
            }
            visitElementClose(m_node);
            break;
        }
        default:
            break;
        }
        moveNext();
    };
    finished();
}

void GumboVisitor::moveNext()
{
    do {
        unsigned int nextIndex = m_node->index_within_parent + 1;
        GumboElement &parent = m_node->parent->v.element;
        if (parent.children.length > nextIndex) {
            m_node = static_cast<GumboNode *>(parent.children.data[nextIndex]);
            return;
        }
        visitElementClose(m_node->parent);
        m_node = m_node->parent;
    } while (m_node != m_root);
    m_node = nullptr;
}

GumboVisitor::~GumboVisitor()
{
    gumbo_destroy_output(m_gumbo);
}
