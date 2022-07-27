/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef GUMBOVISITOR_H
#define GUMBOVISITOR_H
#include "gumbo/gumbo.h"
#include <QString>

/**
 * base class for walking an HTML document tree.
 *
 * This is a relatively thin wrapper around the gumbo parser.  Derived
 * classes override some or all of the visit* methods, which are then called
 * for each node by walk().
 */
class GumboVisitor
{
public:
    explicit GumboVisitor(const QString &input);
    virtual ~GumboVisitor();
    GumboVisitor(GumboVisitor &other) = delete;
    void operator=(GumboVisitor &other) = delete;

    /**
     * Walk the element tree.
     *
     * This calls the appropriate visit* methods for each node in the parse tree
     */
    void walk();

    /**
     * The root node of the parse tree.
     */
    GumboNode *const &root()
    {
        return m_root;
    }

private:
    virtual void visitElementOpen(GumboNode *node)
    {
    }
    virtual void visitText(GumboNode *node)
    {
    }
    virtual void visitElementClose(GumboNode *node){};
    virtual void finished()
    {
    }

    const QByteArray m_data;
    GumboOutput *m_gumbo;
    GumboNode *m_root;
    GumboNode *m_node;
    void moveNext();
};

#endif // GUMBOVISITOR_H
