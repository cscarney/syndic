#include "articleimpl.h"
#include "feedimpl.h"
#include "storageimpl.h"
#include <QJsonArray>
using namespace TTRSS;
using namespace FeedCore;

ArticleImpl::ArticleImpl(int articleId, FeedImpl *feed, StorageImpl *parent)
    : Article(feed, parent)
    , m_articleId{articleId}
{
}

void ArticleImpl::requestContent()
{
    if (auto *storage = qobject_cast<StorageImpl *>(parent())) {
        Client &client = storage->client();
        Client::ApiCall *call = client.getArticle(m_articleId);
        call->onSuccess(this, [this](const QJsonDocument &body) {
            qDebug() << "got article" << body;
            QJsonArray jArticles = body.object()["content"].toArray();
            if (jArticles.empty()) {
                gotContent("");
            } else {
                QJsonObject jArticle = jArticles[0].toObject();
                QString text = jArticle["content"].toString();
                gotContent(text);
            }
        });
        call->onError(this, [this](auto msg) {
            qDebug() << "error" << msg;
            gotContent("");
        });
    } else {
        Q_UNREACHABLE();
    }
}

void ArticleImpl::updateFromJson(const QJsonObject &apiObject)
{
    setTitle(apiObject["title"].toString());
    Article::setRead(!apiObject["unread"].toBool());
    setAuthor(apiObject["author"].toString());
    setUrl(apiObject["link"].toString());
    setDate(QDateTime::fromSecsSinceEpoch(apiObject["updated"].toInt()));
}
