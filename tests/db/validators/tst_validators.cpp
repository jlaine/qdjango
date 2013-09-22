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
    }

    QString title() const { return m_title; }
    void setTitle(const QString &title) { m_title = title; }

    QString content() const { return m_content; }
    void setContent(const QString &content) { m_content = content; }

private:
    QString m_title;
    QString m_content;

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
    QHash<QByteArray, QString> result = p.cleanFields();
    QCOMPARE(result.size(), 2);

    result = p.cleanFields(QStringList() << "content");
    QCOMPARE(result.size(), 1);

    p.setContent("testing");
    result = p.cleanFields();
    QCOMPARE(result.size(), 1);

    p.setTitle("Zombie Attack!");
    result = p.cleanFields();
    QVERIFY(result.isEmpty());

    p.setTitle("Zombie \" Attack!");
    result = p.cleanFields();
    QCOMPARE(result.size(), 1);
}

QTEST_MAIN(tst_Validators);
#include "tst_validators.moc"
