#include "QDjango.h"
#include "QDjangoQuerySet.h"
#include "QDjangoWhere.h"

#include "models.h"
#include "util.h"

struct QObjectDeleter
{
    static inline void cleanup(QObject *pointer) {
        if (pointer)
            pointer->deleteLater();
    }
};

class tst_ManyToOne : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void testOne();

private:
    QDjangoMetaModel reporterModel;
    QDjangoMetaModel articleModel;

    QScopedPointer<Reporter, QObjectDeleter> r;
    QScopedPointer<Reporter, QObjectDeleter> r2;
    QScopedPointer<Article, QObjectDeleter> a;
    QScopedPointer<Article, QObjectDeleter> a2;

};

void tst_ManyToOne::initTestCase()
{
    // setup database
    QVERIFY(initialiseDatabase());
    reporterModel = QDjango::registerModel<Reporter>();
    articleModel = QDjango::registerModel<Article>();
    QVERIFY(reporterModel.createTable());
    QVERIFY(articleModel.createTable());

    // create some reporters and an article
    r.reset(new Reporter);
    r->setFirstName("John");
    r->setLastName("Smith");
    r->setEmail("john@example.com");
    QVERIFY(r->save());

    r2.reset(new Reporter);
    r2->setFirstName("Paul");
    r2->setLastName("Jones");
    r2->setEmail("paul@example.com");
    QVERIFY(r2->save());

    a.reset(new Article);
    a->setHeadline("This is a test");
    a->setPubDate(QDateTime(QDate(2005, 7, 27)));
    a->setReporter(r.data());
    QVERIFY(a->save());

    a2.reset(new Article);
    a2->setHeadline("Just another article");
    a2->setPubDate(QDateTime(QDate(2008, 1, 24)));
    a2->setReporter(r.data());
    QVERIFY(a2->save());
}

void tst_ManyToOne::cleanupTestCase()
{
    QVERIFY(articleModel.dropTable());
    QVERIFY(reporterModel.dropTable());
}

void tst_ManyToOne::testOne()
{
    QDjangoQuerySet<Reporter> reporters;
    int articleCount =
        reporters.filter(QDjangoWhere("article__headline", QDjangoWhere::Contains, "another"))
                 .count();
    QCOMPARE(articleCount, 1);
}

QTEST_MAIN(tst_ManyToOne);
#include "tst_manytoone.moc"
