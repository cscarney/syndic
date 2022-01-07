#ifndef FEEDCORE_OPMLREADER_H
#define FEEDCORE_OPMLREADER_H
#include <QObject>
#include <QXmlStreamReader>

namespace FeedCore
{
class OpmlReader : public QObject
{
    Q_OBJECT
public:
    OpmlReader(QIODevice *device);
    void readAll();

signals:
    void foundFeed(const QUrl &url, const QString &text, const QString &category);

private:
    QXmlStreamReader xml;
};
}

#endif // FEEDCORE_OPMLREADER_H
