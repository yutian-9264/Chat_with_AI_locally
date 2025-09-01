#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QThread>
#include "worker.h"
#include <QKeyEvent>
#include <QProcess>
#include <QMessageBox>
#include <QDialog>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void on_pushButton_clicked();
    void receiveTextFromThread(QString str, bool isReasoning);
    void on_buttonSelect_clicked();

protected:
    void keyPressEvent(QKeyEvent *event) override; // 重写键盘事件函数
private:
    Ui::Widget *ui;

    QThread* newThread;
    Worker* worker;
    bool newReasoning;
    bool newAnswer;
    QProcess* process;
    QDialog* msgDialog;
    QString modelFileName;
    QSqlDatabase database;

};
#endif // WIDGET_H
