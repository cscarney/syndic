#include "opmlreader.h"
#include <QUrl>

using namespace FeedCore;

namespace
{
enum ElementType { Feed, Category, Other };
}

OpmlReader::OpmlReader(QIODevice *device)
    : xml(device)
{
}

void OpmlReader::readAll()
{
    QList<ElementType> openElements;
    QStringList categories;
    while (!xml.atEnd()) {
        xml.readNext();
        if (xml.isStartElement()) {
            QXmlStreamAttributes attrs{xml.attributes()};
            if (xml.name() == "outline") {
                if (!attrs.hasAttribute("xmlUrl")) {
                    openElements.append(ElementType::Category);
                    categories.append(attrs.value("text").toString());
                } else {
                    openElements.append(ElementType::Feed);
                    QUrl xmlUrl(attrs.value("xmlUrl").toString());
                    QString text{attrs.value("text").toString()};
                    QString category = categories.join('/');
                    emit foundFeed(xmlUrl, text, category);
                }
            } else {
                openElements.append(ElementType::Other);
            }
        } else if (xml.isEndElement()) {
            ElementType elementType = openElements.takeLast();
            if (elementType == ElementType::Category) {
                categories.removeLast();
            }
        }
    }
}
