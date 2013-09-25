#include "QDjango.h"
#include "QDjangoModel.h"
#include "QDjangoQuerySet.h"
#include "QDjangoWhere.h"
#include "QDjangoValidator.h"

#include "util.h"

class Publication : public QDjangoModel
{
    Q_OBJECT
    Q_PROPERTY(QString title READ title WRITE setTitle)
    Q_PROPERTY(QString content READ content WRITE setContent)
    Q_PROPERTY(QString website READ website WRITE setWebsite)

public:
    Publication(QObject *parent = 0)
        : QDjangoModel(parent)
    {
        addValidator("title",
            new QDjangoRegExpValidator("[^&<>]*", tr("invalid title")));
        addValidator("title",
             new QDjangoRegExpValidator("[^\"]*", tr("no quotation marks!")));
        addValidator("content",
            new QDjangoRegExpValidator("[^&<>]*", tr("invalid content")));
        addValidator("website", new QDjangoUrlValidator(tr("invalid url")));
    }

    QString title() const { return m_title; }
    void setTitle(const QString &title) { m_title = title; }

    QString content() const { return m_content; }
    void setContent(const QString &content) { m_content = content; }

    QString website() const { return m_website; }
    void setWebsite(const QString &site) { m_website = site; }

private:
    QString m_title;
    QString m_content;
    QString m_website;

};

/** Test QDjangoModel class.
 */
class tst_Validators : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void init();

    void testRegExpValidator();

    void testUrlValidator_data();
    void testUrlValidator();

    void cleanup();
    void cleanupTestCase();
};

/** Create database tables before running tests.
 */
void tst_Validators::initTestCase()
{
    QVERIFY(initialiseDatabase());
    QCOMPARE(QDjango::registerModel<Publication>().createTable(), true);
}

/** Load fixtures.
 */
void tst_Validators::init()
{
}

/** Clear database tables after each test.
 */
void tst_Validators::cleanup()
{
    QCOMPARE(QDjangoQuerySet<Publication>().remove(), true);
}

/** Drop database tables after running tests.
 */
void tst_Validators::cleanupTestCase()
{
    QCOMPARE(QDjango::registerModel<Publication>().dropTable(), true);
}

void tst_Validators::testRegExpValidator()
{
    Publication p;
    p.setTitle("<Zombie&Attack!>");
    p.setContent("t&e&s&t&i&n&g");
    QHash<QByteArray, QString> result = p.cleanFields(QStringList() << "title" << "content");
    QCOMPARE(result.size(), 2);

    result = p.cleanFields(QStringList() << "content");
    QCOMPARE(result.size(), 1);

    p.setContent("testing");
    result = p.cleanFields(QStringList() << "title" << "content");
    QCOMPARE(result.size(), 1);

    p.setTitle("Zombie Attack!");
    result = p.cleanFields(QStringList() << "title" << "content");
    QVERIFY(result.isEmpty());

    p.setTitle("Zombie \" Attack!");
    result = p.cleanFields(QStringList() << "title" << "content");
    QCOMPARE(result.size(), 1);
}

void tst_Validators::testUrlValidator_data()
{
    QTest::addColumn<QString>("string");
    QTest::addColumn<int>("errors");

    QTest::newRow("valid1") << "http://www.djangoproject.com/" << 0;
    QTest::newRow("valid2") << "http://localhost/" << 0;
    QTest::newRow("valid3") << "http://example.com/" << 0;
    QTest::newRow("valid4") << "http://www.example.com/" << 0;
    QTest::newRow("valid5") << "http://www.example.com:8000/test" << 0;
    QTest::newRow("valid6") << "http://valid-with-hyphens.com/" << 0;
    QTest::newRow("valid7") << "http://subdomain.example.com/" << 0;
    QTest::newRow("valid8") << "http://200.8.9.10/" << 0;
    QTest::newRow("valid9") << "http://200.8.9.10:8000/test" << 0;
    QTest::newRow("valid10") << "http://valid-----hyphens.com/" << 0;
    QTest::newRow("valid11") << "http://example.com?something=value" << 0;
    QTest::newRow("valid12") << "http://example.com/index.php?something=value&another=value2" << 0;

    QTest::newRow("invalid1") << "foo" << 1;
    QTest::newRow("invalid2") << "http://" << 1;
    QTest::newRow("invalid3") << "http://example" << 1;
    QTest::newRow("invalid4") << "http://example." << 1;
    QTest::newRow("invalid5") << "http://.com" << 1;
    QTest::newRow("invalid6") << "http://invalid-.com" << 1;
    QTest::newRow("invalid7") << "http://-invalid.com" << 1;
    QTest::newRow("invalid8") << "http://inv-.alid-.com" << 1;
    QTest::newRow("invalid9") << "http://inv-.-alid.com" << 1;
}

void tst_Validators::testUrlValidator()
{
    QFETCH(QString, string);
    QFETCH(int, errors);

    Publication p;
    p.setWebsite(string);
    QHash<QByteArray, QString> result = p.cleanFields();
    QCOMPARE(result.size(), errors);
}

QTEST_MAIN(tst_Validators);
#include "tst_validators.moc"
