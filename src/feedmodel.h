#pragma once

#include "articlelistmodel.h"

class FeedModel : public ArticleListModel
{
    Q_OBJECT

    /**
     * The feed whose items are displayed in the list
     */
    Q_PROPERTY(FeedCore::Feed *feed READ feed WRITE setFeed NOTIFY feedChanged);

public:
    explicit FeedModel(QObject *parent = nullptr);
    ~FeedModel();

    FeedCore::Feed *feed() const;
    void setFeed(FeedCore::Feed *feed);
    void requestUpdate() override;

signals:
    void feedChanged();

protected:
    void init() override;
    QFuture<FeedCore::ArticleRef> getArticles() override;
    void setStatusFromUpstream() override;

private:
    struct PrivData;
    std::unique_ptr<PrivData> d;
    void onStatusChanged();
};
