#ifndef DATARETRIEVER_H
#define DATARETRIEVER_H
#include <Syndication/DataRetriever>
#include <QNetworkAccessManager>

namespace FeedCore {
class DataRetriever : public Syndication::DataRetriever
{
public:
    void retrieveData(const QUrl &url) final;
    int errorCode() const final;
    void abort() final;
private:
    QNetworkAccessManager m_nam;
    QNetworkReply *m_reply { nullptr };
    void onRedirect(const QUrl &url);
    void onFinished();
};
}
#endif // DATARETRIEVER_H
