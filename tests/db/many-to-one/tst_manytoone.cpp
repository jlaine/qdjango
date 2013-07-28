#include "QDjango.h"
#include "QDjangoQuerySet.h"
#include "QDjangoWhere.h"

#include "models.h"
#include "util.h"

class tst_ManyToOne : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

private:
    QDjangoMetaModel reporterModel;
    QDjangoMetaModel articleModel;

};

void tst_ManyToOne::initTestCase()
{
    QVERIFY(initialiseDatabase());

    reporterModel = QDjango::registerModel<Reporter>();
    QCOMPARE(reporterModel.createTable(), true);

    articleModel = QDjango::registerModel<Article>();
    QCOMPARE(articleModel.createTable(), true);
}

void tst_ManyToOne::cleanupTestCase()
{
    QCOMPARE(reporterModel.dropTable(), true);
    QCOMPARE(articleModel.dropTable(), true);
}

QTEST_MAIN(tst_ManyToOne);
#include "tst_manytoone.moc"
