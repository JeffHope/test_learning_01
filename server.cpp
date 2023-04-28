#include "server.h"
#include <QDataStream>
#include <QThread>
#include <QTime>
#include <QMetaEnum>
#include <QMetaObject>
#include <QApplication>
#include <QMessageBox>

Server::Server() {
    m_timer = new QTimer(this);
    m_timer->setInterval(1000);
    m_timer->start();

    QFile file(users_file_path);
    if (!file.exists()) {
        qCritical() << "File of users is not exist";
        QApplication::exit(-1);
    }

    if (!file.open(QIODevice::ReadOnly)) {
        qCritical() << "Cannot open file";
        QApplication::exit(-1);
    }

    users_doc = QJsonDocument::fromJson(file.readAll());
    file.close();

    this->setMaxPendingConnections(10);

    connect(this, &Server::newConnection, this, &Server::newClient);
    connect(m_timer, &QTimer::timeout, this, &Server::broadcastStatus);

    prepareStatus();
}

void Server::prepareStatus() {
    QJsonObject users_json = users_doc.object();
    QJsonArray src_accounts = users_json["accounts"].toArray();
    for (const auto &account : src_accounts) {
        QJsonObject src_obj = account.toObject();
        QString username = src_obj["username"].toString();
        users_status[username] = MessageHelper::STATUS::Offline;
    }
}

void Server::broadcastStatus() {
    foreach (QTcpSocket* socket, this->findChildren<QTcpSocket*>()) {
        QJsonArray accounts;

        for (const auto &username : users_status.keys()) {
            QJsonObject user_obj;
            user_obj["username"] = username;
            user_obj["status"] = MessageHelper::enumStatusToString(users_status[username]);
            accounts.append(user_obj);
        }
        QJsonObject dest_obj;
        dest_obj["accounts"] = accounts;
        qDebug() << dest_obj;

        socket->write(MessageHelper::make(MessageHelper::TYPE::Broadcast, dest_obj));
    }
    prepareStatus();
}

bool Server::checkAuth(const QString &login, const QString &password, QString &username) {

    QJsonObject users_json = users_doc.object();
    QJsonArray arr = users_json["accounts"].toArray();
    for (const auto &account : arr) {
        QJsonObject obj = account.toObject();
        if (obj["login"] == login) {
            if (obj["password"] == password) {
                username = obj["username"].toString();
                return true;
            }
            continue;
        }
    }

    return false;
}

void Server::newClient() {
    QTcpSocket *mSocket = this->nextPendingConnection();
    connect(mSocket, &QTcpSocket::readyRead, this, &Server::slotRead);
    qDebug() << "Client connected ";
}

void Server::slotRead() {
    QTcpSocket *sender_socket = (QTcpSocket*) sender();
    QByteArray data = sender_socket->readAll();
    QJsonDocument recv_doc = QJsonDocument::fromBinaryData(data);
    QJsonObject recv_json = recv_doc.object();

    if(recv_json.contains("type")) {
        if (recv_json["type"] == MessageHelper::enumTypeToString(MessageHelper::TYPE::Message)) {
            foreach (QTcpSocket *socket, this->findChildren<QTcpSocket *>()) {
                sendToClient(socket, recv_doc);
            }
        }

        if (recv_json["type"] == MessageHelper::enumTypeToString(MessageHelper::TYPE::Status)) {
            QString username = recv_json["username"].toString();
            QString status = recv_json["status"].toString();
            users_status[username] = MessageHelper::statusStringToEnum(status);
        }

        if (recv_json["type"] == MessageHelper::enumTypeToString(MessageHelper::TYPE::Auth)) {
            QString auth_username = "";
            if (!checkAuth(recv_json["login"].toString(), recv_json["password"].toString(), auth_username)) {
                sender_socket->write(MessageHelper::makeError());
                return;
            }
            //
            if(users_status[auth_username] != MessageHelper::STATUS::Offline) {
                sender_socket->write(MessageHelper::makeDoubleSignError());
                return;
            }
            //
            sender_socket->write(MessageHelper::makeAccess(auth_username));
        }
        if(recv_json["type"] == MessageHelper::enumTypeToString(MessageHelper::TYPE::Registration)) {
            if(!checkRegistration(recv_json["login"].toString(), recv_json["username"].toString())) {
                sender_socket->write(MessageHelper::makeError());
            } else {
                accountsBase(recv_json["login"].toString(), recv_json["password"].toString(), recv_json["username"].toString());
                sender_socket->write(MessageHelper::makeAccess(recv_json["username"].toString()));
            }
        }
    }
}

void Server::accountsBase(const QString &login, const QString &password, const QString &username) {
    QJsonObject json = users_doc.object();

    QJsonObject account;
    account["login"] = login;
    account["password"] = password;
    account["username"] = username;

    QJsonArray arr = json["accounts"].toArray();
    arr.append(account);

    users_doc.setObject(json);

    QFile file(users_file_path);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(users_doc.toJson(QJsonDocument::JsonFormat::Indented));
        file.close();
    }

    qDebug() << "данные получены";
}

bool Server::checkRegistration(const QString &login, const QString &username) {
    QJsonObject json = users_doc.object();
    QJsonArray arr = json["accounts"].toArray();
    for(int i = 0; i < arr.size(); ++i) {
        QJsonObject temp_value = arr[i].toObject();
        if(temp_value.value("login") == login) {
            qDebug() << "Данный логин уже зарегестрирован.";
            return false;
        }
    }
    return true;
}
void Server::sendToClient(QTcpSocket *clientSocket, const QJsonDocument& json) {
    qDebug() << " send data...";
    clientSocket->write(json.toBinaryData());
}
