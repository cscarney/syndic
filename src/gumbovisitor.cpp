#include "gumbovisitor.h"
#include <QDebug>

GumboVisitor::GumboVisitor(const QString &input)
{
    m_data = input.toUtf8();
    m_gumbo = gumbo_parse(m_data);
}

void GumboVisitor::walk()
{
    m_root = m_gumbo->root;
    m_node = m_root;
    while(m_node) {
        switch (m_node->type) {
        case GUMBO_NODE_TEXT:
        case GUMBO_NODE_CDATA:
            visitText(m_node);
            break;
        case GUMBO_NODE_ELEMENT: {
            m_supress = false;
            visitElementOpen(m_node);
            if (m_supress || m_stop) {
                break;
            }
            GumboElement &element = m_node->v.element;
            if (element.children.length > 0) {
                m_node = static_cast<GumboNode *>(element.children.data[0]);
                continue;
            } else {
                visitElementClose(m_node);
            }
            break;
        }
        default:
            break;
        }
        moveNext();
    };
    finished();
}

void GumboVisitor::supress()
{
    m_supress = true;
}

void GumboVisitor::stop()
{
    m_stop = true;
}

void GumboVisitor::moveNext()
{
    if (m_stop) {
        m_node = nullptr;
        return;
    }
    do {
        unsigned int nextIndex = m_node->index_within_parent + 1;
        GumboElement &parent = m_node->parent->v.element;
        if (parent.children.length > nextIndex) {
            m_node = static_cast<GumboNode *>(parent.children.data[nextIndex]);
            return;
        }
        visitElementClose(m_node->parent);
        if (m_stop) {
            m_node = nullptr;
            return;
        }
        m_node = m_node->parent;
    } while (m_node != m_root);
    m_node = nullptr;
}

GumboVisitor::~GumboVisitor()
{
    gumbo_destroy_output(m_gumbo);
}
