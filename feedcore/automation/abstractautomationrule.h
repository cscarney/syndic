#pragma once
#include "articleref.h"
#include <QFuture>
#include <QObject>

namespace FeedCore
{
class AbstractAutomationRule : public QObject
{
    Q_OBJECT
public:
    explicit AbstractAutomationRule(QObject *parent = nullptr)
        : QObject(parent){};
    ;
    virtual bool matches(const ArticleRef &article) = 0;
    virtual QFuture<void> beginPerformAction(const ArticleRef &article) = 0;
};
}
