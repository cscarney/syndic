#include "contentimageitem.h"

#include <QNetworkReply>
#include <QSGNode>
#include <QSGTexture>
#include <QSGImageNode>
#include <QQuickWindow>
#include <QThread>
#include "networkaccessmanager.h"

ContentImageItem::ContentImageItem(QQuickItem *parent) :
    QQuickItem(parent)
{

}

QSGNode *ContentImageItem::updatePaintNode(QSGNode *node, QQuickItem::UpdatePaintNodeData *data)
{
    if (!m_needsUpdate) {
        return node;
    }
    auto *imageNode = static_cast<QSGImageNode*>(node);
    if (imageNode == nullptr) {
        imageNode = window()->createImageNode();
        auto *texture = window()->createTextureFromImage(m_image);
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

void ContentImageItem::setSource(QUrl src)
{
    if (m_src == src)
        return;

    m_src = src;
    if (!src.isEmpty()) {
        m_image = QImage();
        QNetworkAccessManager *nam{FeedCore::NetworkAccessManager::instance()};
        QNetworkRequest req(src);
        req.setHeader(QNetworkRequest::UserAgentHeader, "curl/4.6");
        req.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
        QNetworkReply *reply{nam->get(req)};
        QObject::connect(reply, &QNetworkReply::finished, this, [this, reply]{
            if (reply->error() != QNetworkReply::NoError) {
                qWarning() << "image load error: " << reply->errorString();
                return;
            }
            if (!m_image.load(reply, nullptr)) {
                qWarning() << "invalid image!" << reply->url();
                return;
            }
            setFlag(QQuickItem::ItemHasContents, true);
            setImplicitWidth(m_image.width());
            setImplicitHeight(m_image.height());
            m_needsUpdate = true;
            polish();
            reply->deleteLater();
        });
    }
    polish();

    emit sourceChanged(m_src);
}

void ContentImageItem::geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry)
{
    QQuickItem::geometryChanged(newGeometry, oldGeometry);
    m_needsUpdate = true;
}

void ContentImageItem::updatePolish()
{
    QQuickItem::updatePolish();
}
