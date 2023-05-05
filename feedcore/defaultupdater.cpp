#include "defaultupdater.h"
#include "article.h"
#include "context.h"
#include "feeddiscovery.h"
#include "networkaccessmanager.h"
#include "readability/readability.h"
#include "readability/readabilityresult.h"
#include "updatablefeed.h"
#include <QDebug>
#include <QNetworkReply>
#include <QPointer>
#include <QQueue>
#include <Syndication/Image>
#include <Syndication/ParserCollection>
using namespace FeedCore;

constexpr const int kMaxRedirects = 10;

class DefaultUpdater::LoadOperation : public QObject
{
    Q_OBJECT
public:
    void start(const QUrl &url, const QString &failMessage = QString());
    void abort();

signals:
    void succeeded(const Syndication::FeedPtr &feed, const QUrl &changeUrl);
    void failed(const QString &errorString);
    void aborted();

private:
    QSet<QUrl> m_seenUrls;
    QPointer<QNetworkReply> m_reply;
    bool m_isDiscoveredFeed{false};
    void onReplyFinished();
};

class DefaultUpdater::PreloadQueue : public QObject
{
    Q_OBJECT
public:
    explicit PreloadQueue(Readability *readability);

    void setReadability(Readability *readability);
    void addArticle(const ArticleRef &article);
    void next();

signals:
    void finished();

private:
    QPointer<Readability> m_readability;
    QQueue<ArticleRef> m_articles;

    void onReadabilityResultFinished(const QString &content);
    void onReadabilityResultError();
};

DefaultUpdater::DefaultUpdater(UpdatableFeed *feed, QObject *parent)
    : Updater(feed, parent)
    , m_updatableFeed{feed}
{
}

void DefaultUpdater::run()
{
    if (!feed()->url().isValid()) {
        setError(tr("Invalid URL", "error message"));
        return;
    }
    m_operation = new LoadOperation;
    QObject::connect(m_operation, &LoadOperation::succeeded, this, &DefaultUpdater::onSucceeded);
    QObject::connect(m_operation, &LoadOperation::failed, this, &DefaultUpdater::onFailed);
    QObject::connect(m_operation, &LoadOperation::aborted, this, [this] {
        aborted();
    });
    m_operation->start(feed()->url());
}

void DefaultUpdater::abort()
{
    if (!m_operation.isNull()) {
        m_operation->abort();
    }
}

void DefaultUpdater::onSucceeded(const Syndication::FeedPtr &feed, const QUrl &changeUrl)
{
    if (changeUrl.isValid()) {
        m_updatableFeed->setUrl(changeUrl);
    }

    if (m_updatableFeed->flags() & Feed::UseReadableContentFlag) {
        if (Context *c = m_updatableFeed->context(); c && c->prefetchContent()) {
            m_preloadQueue = new PreloadQueue(c->getReadability());
            QObject::connect(m_updatableFeed, &Feed::articleAdded, m_preloadQueue, &DefaultUpdater::PreloadQueue::addArticle);
        }
    }

    m_updatableFeed->updateFromSource(feed);

    if (m_preloadQueue != nullptr) {
        QObject::connect(m_preloadQueue, &DefaultUpdater::PreloadQueue::finished, this, &DefaultUpdater::finish);
        QMetaObject::invokeMethod(m_preloadQueue, &DefaultUpdater::PreloadQueue::next, Qt::QueuedConnection);
    } else {
        finish();
    }
}

void DefaultUpdater::onFailed(const QString &errorString)
{
    qDebug() << "Updater Error:" << errorString;
    setError(errorString);
}

void DefaultUpdater::LoadOperation::start(const QUrl &url, const QString &failMessage)
{
    if (m_seenUrls.contains(url) || m_seenUrls.count() > kMaxRedirects) {
        const QString &errorMessage = failMessage.isEmpty() ? "unknown error" : failMessage;
        emit failed(errorMessage);
        deleteLater();
    }
    m_seenUrls << url;
    QNetworkRequest request(url);
    m_reply = NetworkAccessManager::instance()->get(request);
    QObject::connect(m_reply, &QNetworkReply::finished, this, &DefaultUpdater::LoadOperation::onReplyFinished);
}

void DefaultUpdater::LoadOperation::abort()
{
    m_reply->abort();
}

void DefaultUpdater::LoadOperation::onReplyFinished()
{
    m_reply->deleteLater();
    QUrl url = m_reply->url();

    switch (m_reply->error()) {
    case QNetworkReply::NoError: {
        QByteArray data = m_reply->readAll();
        Syndication::FeedPtr feed = Syndication::parserCollection()->parse({data, m_reply->url().toString()});
        if (feed != nullptr) {
            QUrl changeUrl = m_isDiscoveredFeed ? url : QUrl();
            emit succeeded(feed, changeUrl);
            deleteLater();
        } else {
            QUrl discoveredFeed = FeedDiscovery::discoverFeed(url, data);
            m_isDiscoveredFeed = true;
            start(discoveredFeed, "couldn't find feed source");
        }
        break;
    }

    case QNetworkReply::OperationCanceledError:
        emit aborted();
        deleteLater();
        break;

    case QNetworkReply::InsecureRedirectError: {
        const QUrl redirect = m_reply->header(QNetworkRequest::LocationHeader).toUrl();
        qDebug() << "insecure redirect from" << url << "to" << redirect;
        start(redirect, "too many redirects");
        break;
    }

    default:
        emit failed(m_reply->errorString());
        deleteLater();
    }
}

DefaultUpdater::PreloadQueue::PreloadQueue(Readability *readability)
    : m_readability(readability)
{
}

void DefaultUpdater::PreloadQueue::addArticle(const ArticleRef &article)
{
    m_articles << article;
}

void DefaultUpdater::PreloadQueue::next()
{
    if (m_articles.isEmpty() || m_readability == nullptr) {
        emit finished();
        deleteLater();
        return;
    }

    if (!m_articles.isEmpty() && m_readability != nullptr) {
        ArticleRef article = m_articles.takeFirst();
        ReadabilityResult *result = m_readability->fetch(article->url());
        QObject::connect(result, &ReadabilityResult::finished, this, [this, article](const QString &text) {
            article->cacheReadableContent(text);
            next();
        });

        // just skip articles with readability failures
        QObject::connect(result, &ReadabilityResult::error, this, &DefaultUpdater::PreloadQueue::next);
    }
}

#include "defaultupdater.moc"
