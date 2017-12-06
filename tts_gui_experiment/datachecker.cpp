#include "datachecker.h"
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include "parser.h"

void DataChecker::setNumTrials(int numTrials)
{
    this->numTrials = numTrials;
}

void DataChecker::setRootDir(QString rootDir)
{
    this->rootDir = rootDir;
}

void DataChecker::setLists(QList<QString> catList, QList<QStringList *> utterList)
{
    this->categoryList  = catList;
    this->utterList     = utterList;
}

QVector<int> DataChecker::checkUtter(QString cat, QString utter) {

    QVector<int> badTrials;        // Output vector containing bad trial indexes

    QString utterDir = QString("%1/%2/%3").arg(rootDir).arg(cat).arg(utter);
    QDir dir(utterDir);
    dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);   // Only check for directories at first
    dir.setSorting(QDir::Name);
    QFileInfoList dirList = dir.entryInfoList();        // Generate a list of all sub-folders

    // Check data integroty for each sub-folder and associated files
    for (int i = 1; i <= numTrials; i++) {

        bool dirFound = false;
        QFileInfo trialDirInfo;

        foreach (QFileInfo dirElt, dirList) {
            QString dirName = dirElt.baseName();     // Extract sub-folder name

            if (dirName.contains(QString::number(i))){
                trialDirInfo = dirElt;
                dirFound = true;
                break;
            }
        }

        // Add missing trial index if trial number not found in sub-folder names
        if (!dirFound) {
            badTrials.append(i);
            continue;
        }

        // Get list of files in this sub-folder
        QString trialDirPath = trialDirInfo.filePath();
        QDir trialDir(trialDirPath);
        trialDir.setFilter(QDir::Files);                // Only check for files at this step
        QFileInfoList fileList = trialDir.entryInfoList();

        // Add index to bad trials if 5 files are not found
        if ( fileList.isEmpty() || ( fileList.size() < 5 ) ){

            badTrials.append(i);
            continue;
        }
    }

//    return badTrials;

    // NOTE: Remove this line for actual datachecking
    QVector<int> bogus;
    return bogus;
}

QVector<BadTrial> DataChecker::checkAll(){

    QVector<BadTrial> allBadTrials;

    for (int catIdx = 0; catIdx < categoryList.size(); catIdx++) {

        for (int utterIdx = 0; utterIdx < utterList.at(catIdx)->size(); utterIdx++) {

            QString category = categoryList.at(catIdx);
            QString utter    = Parser::parseUtter(utterList.at(catIdx)->at(utterIdx));

            QVector<int> badTrials = checkUtter(category, utter);

            if (!badTrials.isEmpty()) {
                foreach (int trial, badTrials) {
                    allBadTrials.append( BadTrial {category, utter, trial} );
                }
            }
        }
    }

    return allBadTrials;
}
