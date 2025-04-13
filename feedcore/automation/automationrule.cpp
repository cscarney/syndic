#include "automationrule.h"
#include "article.h"
#include "feed.h"
#include <QJsonObject>
#include <QProcess>

using namespace FeedCore;

AutomationRule::AutomationRule(QObject *parent)
    : AbstractAutomationRule{parent}
{
}

AutomationRule::AutomationRule(const QJsonObject &obj, QObject *parent)
    : AbstractAutomationRule(parent)
    , m_matchField(static_cast<MatchField>(obj.value("matchField").toInt()))
    , m_matchValue(obj.value("matchValue").toString())
    , m_shouldMarkStarred(obj.value("shouldMarkStarred").toBool())
    , m_shouldMarkRead(obj.value("shouldMarkRead").toBool())
    , m_shouldRunShellScript(obj.value("shellScript").isString())
    , m_shellScript(obj.value("shellScript").toString())
{
}

FeedCore::AutomationRule::MatchField AutomationRule::matchField() const
{
    return m_matchField;
}

void AutomationRule::setMatchField(FeedCore::AutomationRule::MatchField newMatchField)
{
    if (m_matchField == newMatchField) {
        return;
    }
    m_matchField = newMatchField;
    emit matchFieldChanged();
}

const QString &AutomationRule::matchValue() const
{
    return m_matchValue;
}

void AutomationRule::setMatchValue(const QString &newMatchValue)
{
    if (m_matchValue == newMatchValue) {
        return;
    }
    m_matchValue = newMatchValue;
    emit matchValueChanged();
}

bool AutomationRule::shouldMarkStarred() const
{
    return m_shouldMarkStarred;
}

void AutomationRule::setShouldMarkStarred(bool newShouldMarkStarred)
{
    if (m_shouldMarkStarred == newShouldMarkStarred) {
        return;
    }
    m_shouldMarkStarred = newShouldMarkStarred;
    emit shouldMarkStarredChanged();
}

bool AutomationRule::shouldMarkRead() const
{
    return m_shouldMarkRead;
}

void AutomationRule::setShouldMarkRead(bool newShouldMarkRead)
{
    if (m_shouldMarkRead == newShouldMarkRead) {
        return;
    }
    m_shouldMarkRead = newShouldMarkRead;
    emit shouldMarkReadChanged();
}

bool AutomationRule::shouldRunShellScript() const
{
    return m_shouldRunShellScript;
}

void AutomationRule::setShouldRunShellScript(bool newShouldRunShellScript)
{
    if (m_shouldRunShellScript == newShouldRunShellScript) {
        return;
    }
    m_shouldRunShellScript = newShouldRunShellScript;
    emit shouldRunShellScriptChanged();
}

const QString &AutomationRule::shellScript() const
{
    return m_shellScript;
}

void AutomationRule::setShellScript(const QString &newShellScript)
{
    if (m_shellScript == newShellScript) {
        return;
    }
    m_shellScript = newShellScript;
    emit shellScriptChanged();
}

bool AutomationRule::matches(const ArticleRef &article)
{
    switch (m_matchField) {
    case MatchEverything:
        return true;
    case MatchFeed:
        return (article->feed()->name() == m_matchValue);
    case MatchAuthor:
        return article->author().contains(m_matchValue, Qt::CaseInsensitive);
    case MatchTitle:
        return article->title().contains(m_matchValue, Qt::CaseInsensitive);
    default:
        return false;
    }
}

QFuture<void> AutomationRule::beginPerformAction(const ArticleRef &article)
{
    if (m_shouldMarkRead) {
        article->setRead(true);
    }
    if (m_shouldMarkStarred) {
        article->setStarred(true);
    }

    if (m_shouldRunShellScript) {
        auto *process = new QProcess(this);
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        env.insert("SYNDIC_ARTICLE_TITLE", article->title());
        env.insert("SYNDIC_ARTICLE_AUTHOR", article->author());
        env.insert("SYNDIC_ARTICLE_FEED", article->feed()->name());
        env.insert("SYNDIC_ARTICLE_LINK", article->url().toString());
        process->setProcessEnvironment(env);
        QObject::connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), process, &QObject::deleteLater);
        process->start("/bin/sh", {"-c", m_shellScript});
        return QtFuture::connect(process, &QProcess::finished).then([](auto) {});
    }
    return QtFuture::makeReadyVoidFuture();
}
