#include "parser.h"

QString Parser::parseUtter(QString rawUtter)
{
    QString parsedUtter;

    // For Second Language Learner protocol,
    // Keep only the first part (english phonetic representation)
    // if the utter contains translation and original language
    if (rawUtter.contains(";")) {
        int idxSemiCol = rawUtter.indexOf(";");
        parsedUtter = (idxSemiCol != 0) ? rawUtter.left(idxSemiCol) : "";
    }
    else {
        parsedUtter = rawUtter;
    }

    // Remove special characters that cannot be used as folder/file name
    QStringList specialChar;
    specialChar << "?" << ":" << "/" << "\\" << "*" << "<" << ">" << "|";

    foreach (QString charac, specialChar) {
        parsedUtter.remove(charac);
    }

    // Format the utterance by removing empty spaces and reduce length
    parsedUtter.truncate(40);
    parsedUtter = parsedUtter.trimmed();

    return parsedUtter;
}
