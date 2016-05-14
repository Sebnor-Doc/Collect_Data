#ifndef DATACHECKER_H
#define DATACHECKER_H

#include <QObject>
#include <typedef.h>

class DataChecker
{

private:
    QString rootDir;
    int numTrials;

    QList<QString> categoryList;
    QList<QStringList*> utterList;

public:
    void setNumTrials(int numTrials);
    void setRootDir(QString rootDir);
    void setLists(QList<QString> catList, QList<QStringList*> utterList);

    QVector<int> checkUtter(QString cat, QString utter);
    QVector<BadTrial> checkAll();
};

#endif // DATACHECKER_H
