/**
 * SPDX-FileCopyrightText: 2021 Connor Carney <hello@connorcarney.com>
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "contentimageitem.h"

#include <QBuffer>
#include <QNetworkReply>
#include <QQmlEngine>
#include <QQuickWindow>
#include <QSGImageNode>
#include <QSGNode>
#include <QSGTexture>
#include <QThread>

ContentImageItem::ContentImageItem(QQuickItem *parent)
    : QQuickItem(parent)
{
}

ContentImageItem::~ContentImageItem()
{
    cancelImageLoad();
}

QSGNode *ContentImageItem::updatePaintNode(QSGNode *node, QQuickItem::UpdatePaintNodeData * /* data */)
{
    auto *imageNode = static_cast<QSGImageNode *>(node); // NOLINT dynamic_cast with Qt-provided type doesn't work on android
    if (m_needsUpdate) {
        imageNode = window()->createImageNode();
        auto *texture = window()->createTextureFromImage(m_image);
        imageNode->setFiltering(QSGTexture::Linear);
        imageNode->setOwnsTexture(true);
        imageNode->setTexture(texture);
    }
    imageNode->setRect(boundingRect());
    m_needsUpdate = false;
    return imageNode;
}

QUrl ContentImageItem::source() const
{
    return m_src;
}

void ContentImageItem::setSource(const QUrl &src)
{
    if (m_src == src) {
        return;
    }
    m_src = src;
    if (!src.isEmpty()) {
        m_image = QImage();
        beginImageLoad();
    }
    polish();
    emit sourceChanged(m_src);
}

ContentImageItem::LoadStatus ContentImageItem::loadStatus()
{
    return m_loadStatus;
}

void ContentImageItem::geometryChange(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    QQuickItem::geometryChange(newGeometry, oldGeometry);
}

void ContentImageItem::updatePolish()
{
    QQuickItem::updatePolish();
}

void ContentImageItem::beginImageLoad()
{
    if (m_reply != nullptr) {
        cancelImageLoad();
    }
    QNetworkAccessManager *nam{qmlEngine(this)->networkAccessManager()};
    QNetworkRequest req(m_src);
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    m_reply = nam->get(req);
    QObject::connect(m_reply, &QNetworkReply::finished, this, &ContentImageItem::onImageLoadFinished);
}

void ContentImageItem::cancelImageLoad()
{
    if (m_reply == nullptr) {
        return;
    }
    m_reply->deleteLater();
    m_reply->abort();
    m_reply = nullptr;
}

void ContentImageItem::onImageLoadFinished()
{
    QNetworkReply *reply = m_reply;
    m_reply = nullptr;
    reply->deleteLater();
    QNetworkReply::NetworkError error = reply->error();
    if (error == QNetworkReply::OperationCanceledError) {
        setLoadStatus(Cancelled);
        return;
    }

    if (error != QNetworkReply::NoError) {
        qDebug() << "image download failed:" << reply->url() << reply->errorString();
        setLoadStatus(Error);
        return;
    }

    QByteArray arr = reply->readAll();
    QBuffer buffer(&arr);
    if (!m_image.load(&buffer, nullptr)) {
        qDebug() << "invalid image:" << reply->url();
        setLoadStatus(Error);
        return;
    }
    setFlag(QQuickItem::ItemHasContents, true);
    setImplicitWidth(m_image.width());
    setImplicitHeight(m_image.height());
    setLoadStatus(Complete);
    m_needsUpdate = true;
    update();
}

void ContentImageItem::setLoadStatus(LoadStatus v)
{
    if (m_loadStatus == v) {
        return;
    }
    m_loadStatus = v;
    emit loadStatusChanged();
}
