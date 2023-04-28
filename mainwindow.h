#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QTcpSocket>
#include <QMainWindow>
#include <QString>
#include <QMetaEnum>
#include <QMetaObject>
#include "SignForm.h"
#include "RegistrationForm.h"
#include <QDialog>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "MessageHelper.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    enum ClientState{
        onAuth = 0x00,
        onReg = 0x01
    };
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    
    void updateStatus(const QJsonArray &json_array);
    void updateMessages(const QString &username, const QString &msg);
    void updateUsername(const QString &username);
    QString selectedStatus();


protected:
    void pushMessage();
    void sendMessage(const QString &username, const QString &msg);

    void slotReadyRead();
    void authorize();
    void registration();

    void stateChanged(QAbstractSocket::SocketState state);
    void connectError(QAbstractSocket::SocketError error);

private:
    Ui::MainWindow *ui;
    SignForm *sign;
    RegistrationForm *reg;

    QString m_current_username;
    int m_current_status_idx = 0;

    QTcpSocket *m_socket = nullptr;
    QByteArray mData;
    ClientState m_client_state = ClientState::onAuth;

};
#endif // MAINWINDOW_H
