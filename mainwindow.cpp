#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTime>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_socket = new QTcpSocket(this);
    sign = new SignForm(this);
    reg = new RegistrationForm(this);

    ui->te_chat->setReadOnly(true);

    connect(m_socket, &QTcpSocket::readyRead, this, &MainWindow::slotReadyRead);
    connect(m_socket, &QTcpSocket::stateChanged, this, &MainWindow::stateChanged);
    connect(m_socket, static_cast<void (QAbstractSocket::*)(QAbstractSocket::SocketError)>(&QTcpSocket::error), this, &MainWindow::connectError);

    connect(ui->pb_sign, &QAbstractButton::clicked, sign, &QDialog::exec);
    connect(ui->pb_send, &QAbstractButton::clicked, this, &MainWindow::pushMessage);
    connect(ui->le_message, &QLineEdit::returnPressed, this, &MainWindow::pushMessage);

    connect(sign, &SignForm::authorize, this, &MainWindow::authorize);
    connect(reg, &RegistrationForm::registration, this, &MainWindow::registration);

    ui->cb_status->addItem(MessageHelper::enumStatusToString(MessageHelper::STATUS::Online));
    ui->cb_status->addItem(MessageHelper::enumStatusToString(MessageHelper::STATUS::Idle));
    ui->cb_status->setCurrentIndex(0);

    show();
    sign->exec();
}

MainWindow::~MainWindow() {
    delete ui;
}

QString MainWindow::selectedStatus() {
    m_current_status_idx = ui->cb_status->currentIndex();
    return ui->cb_status->itemText(m_current_status_idx);
}

void MainWindow::updateStatus(const QJsonArray &json_array) {
    QStringList account_list;
    for (const auto &account : json_array) {
        QJsonObject obj = account.toObject();
        QString username_status = QString("%1: %2").arg(obj["username"].toString()).arg(obj["status"].toString());
        account_list.append(username_status);
    }
    ui->lw_status->clear();
    ui->lw_status->addItems(account_list);
    // ui->lw_status->updateItems(account_list);
}

void MainWindow::updateMessages(const QString &username, const QString &msg) {
    QString chat_message = QString("%1: %2").arg(username).arg(msg);
    ui->te_chat->append(chat_message);
}

void MainWindow::updateUsername(const QString &username) {
    m_current_username = username;
}

void MainWindow::pushMessage() {
    QString msg = ui->le_message->text();
    if (!msg.isEmpty()) {
        sendMessage(m_current_username, msg);
        ui->le_message->clear();
    }
}

void MainWindow::authorize() {

    if(sign->currentLogin().isEmpty() || sign->currentPassword().isEmpty()) {
        m_client_state = ClientState::onAuth;
        QMessageBox msg;
        msg.setText("Одно из полей ввода данных не заполнено!");
        msg.exec();
        return;
    }

    m_socket->connectToHost("127.0.0.1", 65000);
}
void MainWindow::registration(){
    m_client_state = ClientState::onReg;
    if(reg->currentLoginReg().isEmpty() || reg->currentPasswordReg().isEmpty() || reg->currentUserNameReg().isEmpty()){
        QMessageBox msg;
        msg.setText("Одно из полей ввода данных не заполнено!");
        msg.exec();
        return;
    }
    m_socket->connectToHost("127.0.01", 65000);
}

void MainWindow::connectError(QAbstractSocket::SocketError error) {
    QMessageBox msg;
    msg.setText("Не удалось установить подключение: " + m_socket->errorString());
    msg.exec();
}

void MainWindow::stateChanged(QAbstractSocket::SocketState state) {

    switch (state) {
    case QAbstractSocket::SocketState::UnconnectedState:

        break;
    case QAbstractSocket::SocketState::ConnectedState:
        // продумать условие при регистрации пользователя
        switch(m_client_state){
            case 0:
                m_socket->write(MessageHelper::makeAuth(sign->currentLogin(), sign->currentPassword()));
                break;
            case 1:
                m_socket->write(MessageHelper::makeRegistration(reg->currentLoginReg(),reg->currentPasswordReg(),reg->currentUserNameReg()));
                break;
        }
    default:
        break;
    }
}

void MainWindow::slotReadyRead() {
    QByteArray data = m_socket->readAll();
    QJsonDocument recv_doc = QJsonDocument::fromBinaryData(data);
    QJsonObject recv_json = recv_doc.object();

    QMessageBox msg;
    if(recv_json.contains("type")) {
        if (recv_json["type"] == MessageHelper::enumTypeToString(MessageHelper::TYPE::Message)) {
            updateMessages(recv_json["username"].toString(), recv_json["message"].toString());
        }

        if (recv_json["type"] == MessageHelper::enumTypeToString(MessageHelper::TYPE::Broadcast)) {
            updateStatus(recv_json["accounts"].toArray());
            // m_socket->write(MessageHelper::makeStatus(m_current_username, MessageHelper::STATUS::Online));
            QJsonObject status_obj;
            status_obj["username"] = m_current_username;
            status_obj["status"] = selectedStatus();
            m_socket->write(MessageHelper::make(MessageHelper::TYPE::Status, status_obj));
        }

        if (recv_json["type"] == MessageHelper::enumTypeToString(MessageHelper::TYPE::Access)) {
            updateUsername(recv_json["username"].toString());
            sign->close();
        }

        if (recv_json["type"] == MessageHelper::enumTypeToString(MessageHelper::TYPE::Error)) {
            m_socket->disconnectFromHost();
            msg.setText("Неверный логин/пароль");
            msg.exec();
        }

        if(recv_json["type"] == MessageHelper::enumTypeToString(MessageHelper::TYPE::DoubleSigned)){
            m_socket->disconnectFromHost();
            msg.setText("Пользователь уже авторизован");
            msg.exec();
        }
    }
}

void MainWindow::sendMessage(const QString &username, const QString &msg) {
    m_socket->write(MessageHelper::makeMessage(username, msg));
}
