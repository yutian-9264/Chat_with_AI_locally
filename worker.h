#ifndef WORKER_H
#define WORKER_H

#include <QObject>

class Worker : public QObject
{
    Q_OBJECT
public:
    explicit Worker(QObject *parent = nullptr);
    void sendPromptToServer(const QString& promt);

signals:

};

#endif // WORKER_H
