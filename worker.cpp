#include "worker.h"
#include <QDebug>

Worker::Worker(QObject *parent) : QObject(parent)
{

}


void Worker::sendPromptToServer(const QString& promt)
{
    qDebug()<<"222"<<promt;
}
