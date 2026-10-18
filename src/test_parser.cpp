#include <QCoreApplication>
#include <QDebug>
#include <QString>
#include <QVector>
#include "core/CanMessage.h"
#include "core/CanLogParser.h"

int main(int argc, char* argv[]) {
    QCoreApplication a(argc, argv);
    QVector<CanMessage> msgs;
    bool ok = CanLogParser::parseCanDump("/home/jayachandran/Downloads/candump.txt", msgs);
    qDebug() << "Success:" << ok;
    qDebug() << "Parsed messages:" << msgs.size();
    return 0;
}
