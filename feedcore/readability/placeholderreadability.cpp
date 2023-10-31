#include "placeholderreadability.h"
#include "readabilityresult.h"

namespace FeedCore
{

PlaceholderReadability::PlaceholderReadability() = default;

ReadabilityResult *PlaceholderReadability::fetch(const QUrl &url)
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
