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
    : QObject{parent}
    , m_monitorFeed{parent->allItemsFeed()}
{
    QObject::connect(m_monitorFeed.get(), &Feed::articleAdded, this, &AutomationEngine::processArticle);
}

void AutomationEngine::processArticle(const ArticleRef &article)
{
    for (auto *r : std::as_const(m_rules)) {
        r->apply(article);
    }
}

const QList<AutomationRule *> &AutomationEngine::automationRules()
{
    return m_rules;
}

AutomationRule *AutomationEngine::addAutomationRule()
{
    auto *rule = new AutomationRule(this);
    m_rules << rule;
    return rule;
}

bool AutomationEngine::removeAutomationRule(AutomationRule *rule)
{
    if (m_rules.removeOne(rule)) {
        delete rule;
        return true;
    };
    return false;
}

AutomationEngine *AutomationEngine::fromDefaultConfigFile(Context *parent)
{
    constexpr const char *kConfigFileName = "automation.json";
    QString configPath = QStandardPaths::locate(QStandardPaths::AppDataLocation, kConfigFileName);
    if (configPath.isEmpty()) {
        return nullptr;
    }

    auto *result = new AutomationEngine(parent);
    if (!result->loadJson(configPath)) {
        delete result;
        return nullptr;
    }
    return result;
}

bool AutomationEngine::loadJson(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "failed to initialize automation: couldn't open config file";
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

    return m_rules.length() > 0;
}
