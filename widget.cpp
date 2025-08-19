#include "widget.h"
#include "ui_widget.h"
#include <QDebug>
#include <QProcess>

#include <QCoreApplication>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>
#include <QDebug>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    this->setWindowTitle("本地离线AI聊天工具");
    ui->comboBox->addItem("DeepSeek-R1-Distill-Qwen-1.5B-UD-IQ2_M");

    newThread = new QThread(this);
    worker = new Worker();
    worker->moveToThread(newThread);

    newThread->start();

//    connect(newThread, &QThread::started, worker, &Worker::sendPromptToServer);
    connect(newThread, &QThread::destroyed, newThread, &QThread::deleteLater);
    connect(newThread, &QThread::destroyed, worker, &QThread::deleteLater);
}

Widget::~Widget()
{
    delete ui;

    newThread->quit();
    newThread->wait();
}




void Widget::on_pushButton_clicked()
{
    if(worker)
    {
        worker->sendPromptToServer(ui->lineEdit->text());
    }
}
