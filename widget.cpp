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
#include <QTextCursor>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    newReasoning = 1;
    newAnswer = 1;

    ui->setupUi(this);
    this->setWindowTitle("本地离线AI聊天工具");
//    ui->comboBox->addItem("DeepSeek-R1-Distill-Qwen-1.5B-UD-IQ2_M");

    newThread = new QThread(this);
    worker = new Worker();
    worker->moveToThread(newThread);

    newThread->start();

//    connect(newThread, &QThread::started, worker, &Worker::sendPromptToServer);
    connect(newThread, &QThread::destroyed, newThread, &QThread::deleteLater);
    connect(newThread, &QThread::destroyed, worker, &QThread::deleteLater);

    connect(worker, &Worker::receiveText, this, &Widget::receiveTextFromThread);  
    connect(worker, &Worker::streamFinished, [=](){
        ui->pushButton->setDisabled(false);
    });

    ui->pushButton->setDefault(true);
}

Widget::~Widget()
{
    delete ui;

    newThread->quit();
    newThread->wait();
}




void Widget::on_pushButton_clicked()
{
    ui->pushButton->setDisabled(true);
    QTextCursor cursor = ui->textBrowser->textCursor();
    QTextBlockFormat userFormat;
    userFormat.setAlignment(Qt::AlignRight);
    cursor.insertBlock(userFormat);
    QTextCharFormat userCharFormat;
    userCharFormat.setForeground(Qt::blue);

    cursor.insertText("\n\n\n\n", userCharFormat);
    cursor.insertText(ui->lineEdit->text(), userCharFormat);


    newReasoning = 1;
    newAnswer = 1;
    QString sendMsg;
    if(ui->radioButton->isChecked())
    {
        sendMsg = ui->lineEdit->text();
    }else{
        sendMsg = ui->lineEdit->text() + " /no_think";
    }
    if(worker)
    {
        worker->sendPromptToServer(sendMsg);
    }
    ui->lineEdit->clear();
}

void Widget::receiveTextFromThread(QString str, bool isReasoning)
{
    QTextCursor cursor = ui->textBrowser->textCursor();
    QTextCharFormat formatTextReasoning;
    QTextCharFormat formatText;
    formatTextReasoning.setForeground(Qt::gray);
    formatText.setForeground(Qt::black);

    QTextBlockFormat leftFormat;
    leftFormat.setAlignment(Qt::AlignLeft);
//    cursor.insertBlock(leftFormat);




    if(isReasoning)
    {
        if(newReasoning)
        {
            cursor.insertBlock(leftFormat);
            cursor.insertText("\n", formatText);
            newReasoning = 0;
        }
        cursor.insertText(str, formatTextReasoning);
    }else{
        if(newAnswer)
        {
            cursor.insertBlock(leftFormat);
            newAnswer = 0;
        }
        cursor.insertText(str, formatText);
    }
    ui->textBrowser->setTextCursor(cursor);
}

void Widget::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        ui->pushButton->click(); // 触发按钮点击
    } else {
        QWidget::keyPressEvent(event); // 其他按键默认处理
    }
}

