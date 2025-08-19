#include "widget.h"
#include "ui_widget.h"
#include <QDebug>
#include <QProcess>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    this->setWindowTitle("本地离线AI聊天工具");
    ui->comboBox->addItem("DeepSeek-R1-Distill-Qwen-1.5B-UD-IQ2_M");



    QString prompt;
    prompt = "hi, how are you?";
    QString exePath = QCoreApplication::applicationDirPath() + "/llama-cli.exe";
    QString modelPath = QCoreApplication::applicationDirPath() + "/DeepSeek-R1-Distill-Qwen-1.5B-UD-IQ2_M.gguf";

    QStringList arguments;
    arguments << "-m" << modelPath << "-p" << prompt;

    QProcess* process = new QProcess(this);
    process->start(exePath, arguments);
    process->waitForFinished();

    qDebug() << "stderr:" << process->readAllStandardError();

    int exitCode = process->exitCode();
    if(exitCode != 0)
    {
        qDebug() << "llama-cli运行错误，错误码" << exitCode;
        qDebug() << "错误输出：" <<process->readAllStandardError();
    }

    QString data = process->readAllStandardOutput();
    qDebug() << data;
}

Widget::~Widget()
{
    delete ui;
}

