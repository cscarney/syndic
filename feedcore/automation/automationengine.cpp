#include "automationengine.h"
#include "article.h"
#include "automationrule.h"
#include "context.h"
#include "feed.h"
#include <QDebug>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>

using namespace FeedCore;

AutomationEngine::AutomationEngine(Context *parent)
    : m_monitorFeed{parent->allItemsFeed()}
{
    QObject::connect(m_monitorFeed.get(), &Feed::articleAdded, this, &AutomationEngine::processArticle);
}

void AutomationEngine::processArticle(const ArticleRef &article)
{
    for (auto *r : std::as_const(m_rules)) {
        if (r->matches(article)) {
            m_actionQueue.enqueue({r, article});
        }
    }
    processPendingActions();
}

void AutomationEngine::addAutomationRule(AbstractAutomationRule *rule)
{
    if (m_rules.contains(rule)) {
        return;
    }
    rule->setParent(this);
    m_rules << rule;
}

const QList<AbstractAutomationRule *> &AutomationEngine::automationRules()
{
    return m_rules;
}

AbstractAutomationRule *AutomationEngine::addAutomationRule()
{
    auto *rule = new AutomationRule(this);
    m_rules << rule;
    return rule;
}

bool AutomationEngine::removeAutomationRule(AbstractAutomationRule *rule)
{
    if (m_rules.removeOne(rule)) {
        // If the rule is in the queue, remove it
        m_actionQueue.removeIf([rule](const PendingAction &op) {
            return op.rule == rule;
        });
        delete rule;
        return true;
    };
    return false;
}

AutomationEngine *AutomationEngine::fromDefaultConfigFile(Context *parent)
{
    constexpr const char *kConfigFileName = "automation.json";
    QString configPath = QStandardPaths::locate(QStandardPaths::AppDataLocation, kConfigFileName);
    if (configPath.isEmpty() || !QFile::exists(configPath)) {
        return nullptr;
    }

    auto *result = new AutomationEngine(parent);
    if (!result->loadJson(configPath)) {
        delete result;
        return nullptr;
    }
    return result;
}

void AutomationEngine::beginNextAction()
{
    m_activeActions++;
    PendingAction op = m_actionQueue.dequeue();
    auto action = op.rule->beginPerformAction(op.article);
    Future::safeThen(action, this, [this](auto) {
        onActionFinished();
    });
}

void AutomationEngine::processPendingActions()
{
    while (m_activeActions < maxConcurrentActions() && !m_actionQueue.isEmpty()) {
        beginNextAction();
    }
}

void AutomationEngine::onActionFinished()
{
    m_activeActions--;
    processPendingActions();
}

int AutomationEngine::maxConcurrentActions()
{
    return QThread::idealThreadCount();
}

bool AutomationEngine::loadJson(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return false;
    }
    QJsonDocument json = QJsonDocument::fromJson(file.readAll());
    if (!json.isArray()) {
        qDebug() << "failed to initialize automation: invalid config file";
    }

    const QJsonArray arr = json.array();
    for (const auto &val : arr) {
        if (!val.isObject()) {
            qDebug() << "skipping invalid automation rule" << val;
        }
        m_rules << new AutomationRule(val.toObject(), this);
    }

    return !m_rules.empty();
}
