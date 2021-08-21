#ifndef CONTENTIMAGEITEM_H
#define CONTENTIMAGEITEM_H
#include <QImage>
#include <QQuickItem>

class ContentImageItem : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(QUrl source READ source WRITE setSource NOTIFY sourceChanged)
public:
    explicit ContentImageItem(QQuickItem *parent=nullptr);
    QSGNode *updatePaintNode(QSGNode *node, UpdatePaintNodeData *data) override;
    QUrl source() const;
    void setSource(const QUrl& src);

signals:
    void sourceChanged(QUrl src);

protected:
    void geometryChanged(const QRectF &newGeometry, const QRectF &oldGeometry) override;
    void updatePolish() override;

private:
    QUrl m_src;
    QImage m_image;
    bool m_needsUpdate{false};
};

#endif // CONTENTIMAGEITEM_H
