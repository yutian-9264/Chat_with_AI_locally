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
#include <QFileDialog>
#include <QVBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QFileInfo>


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
    process = new QProcess(this);

    connect(newThread, &QThread::destroyed, newThread, &QThread::deleteLater);
    connect(newThread, &QThread::destroyed, worker, &QThread::deleteLater);
    connect(worker, &Worker::receiveText, this, &Widget::receiveTextFromThread);
    connect(worker, &Worker::streamFinished, [=](){
        ui->pushButton->setDisabled(false);
    });
    connect(process, &QProcess::readyReadStandardError,  [=]()
    {
        QByteArray output = process->readAllStandardError();
        QString text = QString::fromUtf8(output);
        qDebug() << "Server Output:" << text;

        if (text.contains("listening on"))
        {
            qDebug() << "llama服务器已连接成功！";
            msgDialog->accept();
            ui->label->setText("当前模型：" + modelFileName);
            ui->buttonSelect->setText("更换模型");
        }
    });
    connect(ui->radioButton_2, &QRadioButton::toggled, ui->comboBox, &QComboBox::setEnabled);

    QStringList cpuMultiThreadNums;
    cpuMultiThreadNums << "4线程"<< "8线程" << "12线程" << "16线程" << "20线程" << "32线程";

    ui->pushButton->setDefault(true);
    ui->radioButton->setAutoExclusive(false);
    ui->radioButton_2->setAutoExclusive(false);
    ui->comboBox->setDisabled(true);
    ui->comboBox->addItems(cpuMultiThreadNums);

    if (QSqlDatabase::contains("qt_sql_default_connection")) {
        database = QSqlDatabase::database("qt_sql_default_connection");
    }else{
        database = QSqlDatabase::addDatabase("QSQLITE");
        database.setDatabaseName("MyDataBase.db");
        database.setUserName("yt");
        database.setPassword("123");
    }

    if(!database.open()) {
        qDebug() << "Error: failed to connect database." << database.lastError();
    }else{
        qDebug()<<"database connected";

        QSqlQuery sql_query(database);
        QString create_sql = "CREATE TABLE IF NOT EXISTS student (id int primary key, name varchar(30), age int)";
        sql_query.prepare(create_sql);
        if(!sql_query.exec()) {
            qDebug() << "Error: Fail to create table." << sql_query.lastError();
        } else {
            qDebug() << "Table created!";
        }

        QString update_sql = "update student set name = :name where id = :id";
        sql_query.prepare(update_sql);
        sql_query.bindValue(":name", "Qt");
        sql_query.bindValue(":id", 1);
        if(!sql_query.exec()) {
            qDebug()<<sql_query.lastError();
        }else{
            qDebug()<<"updated!";
        }

        QString select_sql = "select id, name from student";
        if(!sql_query.exec(select_sql)) {
            qDebug() << sql_query.lastError();
        } else {
            while(sql_query.next()) {
                int id = sql_query.value(0).toInt();
                QString name = sql_query.value(1).toString();
                qDebug()<<QString("id:%1 name:%2").arg(id).arg(name);
            }
        }

        QString select_all_sql = "select * from student";
        if(!sql_query.exec(select_all_sql)) {
            qDebug() << sql_query.lastError();
        } else {
            while(sql_query.next()) {
                int id = sql_query.value(0).toInt();
                QString name = sql_query.value(1).toString();
                int age = sql_query.value(2).toInt();
                qDebug()<<QString("id:%1 name:%2 age:%3").arg(id).arg(name).arg(age);
            }
        }

        QString select_max_sql = "select max(id) from student";
        int max_id = 0;
        if(!sql_query.exec(select_max_sql)) {
            qDebug() << sql_query.lastError();
        } else {
            while(sql_query.next()) {
                max_id = sql_query.value(0).toInt();
                qDebug()<<QString("max id:%1").arg(max_id);
            }
        }

//        QString delete_sql = "delete from student where id = ?";
//        sql_query.prepare(delete_sql);
//        sql_query.addBindValue(1);
//        if(!sql_query.exec()) {
//            qDebug() << sql_query.lastError();
//        } else {
//            qDebug() << "deleted!";
//        }

        QString clear_sql = "delete from student";
        sql_query.prepare(clear_sql);
        if(!sql_query.exec()) {
            qDebug() << sql_query.lastError();
        } else {
            qDebug() << "table cleared!";
        }

        if(!sql_query.exec(select_all_sql)) {
            qDebug() << sql_query.lastError();
        } else {
            while(sql_query.next()) {
                int id = sql_query.value(0).toInt();
                QString name = sql_query.value(1).toString();
                int age = sql_query.value(2).toInt();
                qDebug()<<QString("id:%1 name:%2 age:%3").arg(id).arg(name).arg(age);
            }
        }

//        QString insert_sql = "insert into student values (?, ?, ?)";
//        sql_query.prepare(insert_sql);
//        sql_query.addBindValue(1);
//        sql_query.addBindValue("wang");
//        sql_query.addBindValue(25);
//        if(!sql_query.exec()) {
//            qDebug()<<sql_query.lastError();
//        }else{
//            qDebug()<<"inserted wang!";
//        }

//        if(!sql_query.exec("INSERT INTO student VALUES(3, \"Li\", 23)")) {
//            qDebug() << sql_query.lastError();
//        }else{
//            qDebug() << "inserted Li!";
//        }


    }


}


Widget::~Widget()
{
    delete ui;

    newThread->quit();
    newThread->wait();
    if(process->state() == QProcess::Running)
    {
        process->terminate();
        if(!process->waitForFinished(1000))
        {
            process->kill();
            process->waitForFinished();
        }
    }

    database.close();
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
            cursor.insertText("\n", formatText);
            newAnswer = 0;
        }
        cursor.insertText(str, formatText);
    }
    ui->textBrowser->setTextCursor(cursor);
}

void Widget::keyPressEvent(QKeyEvent *event) {
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        ui->pushButton->click();
    } else {
        QWidget::keyPressEvent(event);
    }
}


void Widget::on_buttonSelect_clicked()
{
    if(ui->buttonSelect->text() == "更换模型")
    {
        if (process->state() == QProcess::Running) {
            process->terminate();
            if (!process->waitForFinished(1000)) {
                process->kill();
                process->waitForFinished();
            }
        }
    }

    QString filePath = QFileDialog::getOpenFileName(
                this,
                "选择模型",
                "../",
                "模型文件(*.gguf)");
    if(filePath.isEmpty())
    {
        return;
    }else{
        QFileInfo fileInfo(filePath);
        modelFileName = fileInfo.completeBaseName();
    }

    msgDialog = new QDialog (this);
    msgDialog->setWindowTitle("提示");
    msgDialog->setModal(true);
    QVBoxLayout* layout = new QVBoxLayout(msgDialog);
    QLabel* label = new QLabel("模型加载中...", msgDialog);
    layout->addWidget(label);
    QProgressBar *progressBar = new QProgressBar(msgDialog);
    progressBar->setRange(0, 0);
    layout->addWidget(progressBar);
    msgDialog->show();

    QString program = QCoreApplication::applicationDirPath() + "/llama_server/llama-server";
    QStringList arguments;
    arguments << "-m" << filePath
              << "--jinja"
              << "--reasoning-format" << "deepseek"
              << "-ngl" << "99"
              << "-fa" << "auto"
              << "-sm" << "row"
              << "--temp" << "0.6"
              << "--top-k" << "20"
              << "--top-p" << "0.95"
              << "--min-p" << "0"
              << "-c" << "40960"
              << "-n" << "32768"
              << "--no-context-shift";
    if(ui->radioButton_2->isChecked())
    {
        qint32 threadsNum;
        qDebug()<<"currentIndex"<<ui->comboBox->currentIndex();
        switch(ui->comboBox->currentIndex())
        {
        case 0: arguments << "-t" << "4";
            break;
        case 1: arguments << "-t" << "8";
            break;
        case 2: arguments << "-t" << "12";
            break;
        case 3: arguments << "-t" << "16";
            break;
        case 4: arguments << "-t" << "20";
            break;
        case 5: arguments << "-t" << "32";
            break;
        default: threadsNum = 1;
        }
    }
    process->start(program, arguments);
    qDebug() << "Start command:" << program << arguments;
}

void Widget::on_buttonSelect_2_clicked()
{
    Register m_register(this);
    int result = m_register.exec();

    if(result == QDialog::Accepted) {
        qDebug () << "QDialog accepted!";
    } else {
        qDebug () << "QDialog rejected!";
    }
}
