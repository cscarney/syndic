#ifndef DATARETRIEVER_H
#define DATARETRIEVER_H

#include <Syndication/DataRetriever>
#include <QNetworkAccessManager>

namespace FeedCore {

class DataRetriever : public Syndication::DataRetriever
{
public:
    DataRetriever();

    void retrieveData(const QUrl &url) override final;
    int errorCode() const override final;
    void abort() override final;

private:
    QNetworkAccessManager m_nam;
    QNetworkReply *m_reply;

private slots:
    void slotRedirect(const QUrl &url);
    void slotFinished();
};

}

#endif // DATARETRIEVER_H
