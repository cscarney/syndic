#ifndef GUMBOVISITOR_H
#define GUMBOVISITOR_H
#include <QString>
#include <gumbo.h>

class GumboVisitor
{
public:
    explicit GumboVisitor(const QString &input);
    ~GumboVisitor();
    GumboVisitor(GumboVisitor &other) = delete;
    void operator=(GumboVisitor &other) = delete;

    void walk();

    virtual void visitElementOpen(GumboNode *node) {  }
    virtual void visitText(GumboNode *node) { }
    virtual void visitElementClose(GumboNode *node) { };
    virtual void finished() {}

protected:
    void supress();
    void stop();
    GumboNode *const&root() { return m_root; }

private:
    GumboOutput *m_gumbo;
    QByteArray m_data;
    GumboNode *m_root;
    GumboNode *m_node;
    bool m_supress { false };
    bool m_stop { false };
    void moveNext();
};

#endif // GUMBOVISITOR_H
