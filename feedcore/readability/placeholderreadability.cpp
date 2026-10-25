#include "placeholderreadability.h"
#include "readabilityresult.h"

namespace FeedCore
{

PlaceholderReadability::PlaceholderReadability() = default;

ReadabilityResult *PlaceholderReadability::parse(const QString & /*html*/, const QUrl &url)
{
    return fetch(url);
}

ReadabilityResult *PlaceholderReadability::fetch(const QUrl & /*url*/)
{
    auto *result = new ReadabilityResult;
    QMetaObject::invokeMethod(
        result,
        [result] {
            emit result->error();
            result->deleteLater();
        },
        Qt::QueuedConnection);
    return result;
}

} // namespace FeedCore
