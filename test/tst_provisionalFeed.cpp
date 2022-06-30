#include "provisionalfeed.h"
#include <QCoreApplication>
#include <QSignalSpy>
#include <QtTest>
using namespace FeedCore;

constexpr const char *kValidUrl = "https://runningincircles.com/feed";
constexpr const char *kUrlMissingSchema = "runningincircles.com/feed";
constexpr const char *kInvalidUrl = "!!!";

class testProvisionalFeed : public QObject
{
    Q_OBJECT
    QScopedPointer<ProvisionalFeed> m_testFeed;
    QScopedPointer<QSignalSpy> m_spyUrlChanged;
    QScopedPointer<QSignalSpy> m_spyUrlStringChanged;

private slots:
    void initTestCase()
    {
        qRegisterMetaType<FeedCore::Feed *>();
        qRegisterMetaType<FeedCore::ProvisionalFeed *>();
    }

    void init()
    {
        m_testFeed.reset(new ProvisionalFeed());
        m_spyUrlChanged.reset(new QSignalSpy(m_testFeed.get(), &ProvisionalFeed::urlChanged));
        m_spyUrlStringChanged.reset(new QSignalSpy(m_testFeed.get(), &ProvisionalFeed::urlStringChanged));
    }

    void cleanup()
    {
        m_testFeed.reset();
        m_spyUrlChanged.reset();
        m_spyUrlStringChanged.reset();
    }

    void testSetUrlStringToValidUrl()
    {
        m_testFeed->setUrlString(kValidUrl);
        QVERIFY(m_testFeed->url() == QUrl(kValidUrl));
        QVERIFY(m_spyUrlChanged->count() == 1);
        QVERIFY(m_spyUrlStringChanged->count() == 1);
    }

    void testSetUrlStringWithoutSchema()
    {
        m_testFeed->setUrlString(kUrlMissingSchema);
        QVERIFY(m_testFeed->url() == QUrl(kValidUrl));
        QVERIFY(m_testFeed->urlString() == kUrlMissingSchema);
        QVERIFY(m_spyUrlChanged->count() == 1);
        QVERIFY(m_spyUrlStringChanged->count() == 1);
    }

    void testUrlStringUpdatedWhenUrlChanged()
    {
        m_testFeed->setUrl(QUrl(kValidUrl));
        QVERIFY(m_testFeed->urlString() == kValidUrl);
        QVERIFY(m_spyUrlChanged->count() == 1);
        QVERIFY(m_spyUrlStringChanged->count() == 1);
    }

    void testUrlStringInvalidIgnored()
    {
        m_testFeed->setUrl(QUrl(kValidUrl));
        m_testFeed->setUrlString(kInvalidUrl);
        QVERIFY(m_testFeed->url() == QUrl(kValidUrl));
        QVERIFY(m_testFeed->urlString() == kInvalidUrl);
        QVERIFY(m_spyUrlChanged->count() == 1);
        QVERIFY(m_spyUrlStringChanged->count() == 2);
    }

    void testUrlStringEmptyIgnored()
    {
        m_testFeed->setUrl(QUrl(kValidUrl));
        m_testFeed->setUrlString("");
        QVERIFY(m_testFeed->url() == QUrl(kValidUrl));
        QVERIFY(m_testFeed->urlString() == "");
        QVERIFY(m_spyUrlChanged->count() == 1);
        QVERIFY(m_spyUrlStringChanged->count() == 2);
    }
};

QTEST_MAIN(testProvisionalFeed)
#include "tst_provisionalFeed.moc"
