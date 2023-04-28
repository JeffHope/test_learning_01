#pragma once
#include <QTcpServer>
#include <QTcpSocket>
#include <QMap>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMetaEnum>
#include <QMetaObject>
#include <QTimer>
#include "MessageHelper.h"

class Server : public QTcpServer {
    Q_OBJECT

public:

    Server();

    void sendToClient(QTcpSocket *clientSocket, const QJsonDocument& json);
    bool checkAuth(const QString &login, const QString &password, QString &dest_username);
    void accountsBase(const QString &login, const QString &password, const QString &username);
    bool checkRegistration(const QString &login, const QString &username);

protected:
    void prepareStatus();

private:
    QTimer *m_timer = nullptr;
    QJsonDocument users_doc;
    QMap<QString, MessageHelper::STATUS> users_status;

    QString users_file_path = "/home/nikita/Documents/QTProjects/chat/build-SERVER_FOR_CHAT-Desktop_Qt_5_12_12_GCC_64bit-Debug/userdate.json";

public slots:
    void slotRead();
    void newClient();
    void broadcastStatus();

};
