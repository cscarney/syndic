#ifndef CONTENTIMAGEITEM_H
#define CONTENTIMAGEITEM_H
#include <QImage>
#include <QQuickItem>

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
public:
    explicit ContentImageItem(QQuickItem *parent = nullptr);
    QSGNode *updatePaintNode(QSGNode *node, UpdatePaintNodeData *data) override;
    QUrl source() const;
    void setSource(const QUrl &src);

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
