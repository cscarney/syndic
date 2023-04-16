/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#pragma once
#include <QImage>
#include <QQuickItem>
class QNetworkReply;

/**
 * A QQuickItem that displays block-level images.
 *
 * This component is used in the article view to display fluid-width images
 * since the built-in text renderer doesn't support that.  Small images that
 * are rendered inline with the text do not use this component.
 */
class ContentImageItem : public QQuickItem
{
    Q_OBJECT

    /**
     * The url to load the image from
     */
    Q_PROPERTY(QUrl source READ source WRITE setSource NOTIFY sourceChanged)

    /**
     * Whether the image has loaded sucessfully
     */
    Q_PROPERTY(ContentImageItem::LoadStatus loadStatus READ loadStatus NOTIFY loadStatusChanged)
public:
    enum LoadStatus { Loading, Complete, Cancelled, Error };
    Q_ENUM(LoadStatus);

    explicit ContentImageItem(QQuickItem *parent = nullptr);
    ~ContentImageItem();
    QSGNode *updatePaintNode(QSGNode *node, UpdatePaintNodeData *data) override;
    QUrl source() const;
    void setSource(const QUrl &src);
    LoadStatus loadStatus();

signals:
    void sourceChanged(QUrl src);
    void loadStatusChanged();

protected:
    void geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry) override;
    void updatePolish() override;

private:
    QUrl m_src;
    QImage m_image;
    bool m_needsUpdate{false};
    QNetworkReply *m_reply{nullptr};
    LoadStatus m_loadStatus{Loading};

    enum ImageLoadFlags { None = 0, UseHttp2Flag = 1 };

    void beginImageLoad(ImageLoadFlags flags = None);
    void cancelImageLoad();
    void onImageLoadFinished();
    void setLoadStatus(LoadStatus v);
};
