#include "worker.h"
#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QUrl>
#include <QJsonObject>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QJsonParseError>
#include <QCoreApplication>
#include <QJsonArray>

Worker::Worker(QObject *parent) : QObject(parent)
{

}


void Worker::sendPromptToServer(const QString& promt)
{
    qDebug()<<"222"<<promt;

    QNetworkAccessManager* manager = new QNetworkAccessManager();
    QUrl url("http://127.0.0.1:8080/v1/chat/completions");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject jsonPayload;
    QJsonArray messages;

    // system role
    QJsonObject sysMsg;
    sysMsg["role"] = "system";
    sysMsg["content"] = "You are a helpful assistant.";
    messages.append(sysMsg);

    // user role
    QJsonObject userMsg;
    userMsg["role"] = "user";
    userMsg["content"] = promt;   // 这里放用户输入
    messages.append(userMsg);

    jsonPayload["messages"] = messages;
    jsonPayload["stream"] = true;
    jsonPayload["temperature"] = 0.6;
    jsonPayload["top_p"] = 0.95;
    jsonPayload["top_k"] = 20;
//    jsonPayload["n_predict"] = 256;   // 生成的最大 token 数


    QByteArray postData = QJsonDocument(jsonPayload).toJson();
    QNetworkReply* reply = manager->post(request, postData);
    connect(reply, &QNetworkReply::finished, [reply]() {
        // 删除对象
        reply->deleteLater();
    });
    connect(reply, &QNetworkReply::readyRead,[=](){
        QByteArray chunk = reply->readAll();
        qDebug() << chunk;
        if(!chunk.isEmpty())
        {
//            qDebug()<<"1";
            QList<QByteArray> lines = chunk.split('\n');
            for(const QByteArray& line : lines)
            {
                if(line.startsWith("data: "))
                {
                    QString dataStr = QString::fromUtf8(line.mid(6)).trimmed();
                    if(dataStr == "[DONE]")
                    {
                        qDebug() << "Stream finished";
                        emit streamFinished();
                        return;
                    }
                    QJsonParseError err;
                    QJsonDocument jsonDoc = QJsonDocument::fromJson(dataStr.toUtf8(), &err);

                    if(err.error == QJsonParseError::NoError && jsonDoc.isObject()) {
                        QJsonObject obj = jsonDoc.object();

                        if(obj.contains("choices"))
                        {
                            QJsonArray choices = obj["choices"].toArray();
                            if(!choices.isEmpty())
                            {
                                QJsonObject delta = choices[0].toObject()["delta"].toObject();

                                if(delta.contains("reasoning_content"))
                                {
                                    QString textReasoning = delta["reasoning_content"].toString();
                                    if(!textReasoning.isEmpty())
                                    {
//                                        qDebug() << "[reasoning]" <<textReasoning;
                                        receiveText(textReasoning, true);
                                    }
                                }

                                if(delta.contains("content"))
                                {
                                    QString text = delta["content"].toString();
                                    if(!text.isEmpty())
                                    {
//                                        qDebug() << "[content]"<< text;
                                        receiveText(text, false);
                                    }
                                }
                            }
                        }
                    }

                }
            }
        }
    });
}
