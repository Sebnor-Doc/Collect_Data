#ifndef VFBMANAGER_H
#define VFBMANAGER_H

#include <QObject>
#include <QProcess>
#include <QVector>
#include <typedef.h>

class VfbManager : public QObject
{
    Q_OBJECT

public:
    VfbManager(QObject *parent = 0);

    void setVfbFilePath(QString filePath);
    void setRefRootPath(QString rootPath);
    void setRefOutPath(QString relativePath);

    void setSubId(int id);
    void setNumTrials(int numTrials);

    void setCurrentUtter(QString utter);
    void setCurrentTrial(int trial);

    void updateXML(int selection);
    void startVFBProgram();
    void playAudio();
    QVector<LocaData> getRefLocaData();

private:
    QString vfbFilePath;
    QString refRootPath;
    QString refOutPath;
    QString subOutPath;

    int currentTrial;
    QString currentUtter;

    int subId;
    int numTrials;

    QProcess *scoreGenProg;

public slots:
    void setSubOutPath(QString path);
};

#endif // VFBMANAGER_H
