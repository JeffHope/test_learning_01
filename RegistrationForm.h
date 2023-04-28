#ifndef REGLOG_H
#define REGLOG_H
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QWidget>
#include <QDialog>
#include <QTcpSocket>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include "MessageHelper.h"

namespace Ui {
class RegistrationForm;
}

class RegistrationForm : public QDialog
{
    Q_OBJECT
signals:
    void registration();
public:
    explicit RegistrationForm(QWidget *parent = nullptr);
    ~RegistrationForm();
    void setWindow();
    const QString &currentLoginReg();
    const QString &currentPasswordReg();
    const QString &currentUserNameReg();


private slots:
    void on_pushButton_clicked();

private:
    Ui::RegistrationForm *ui;
    QString m_login;
    QString m_pass;
    QString m_username;
    QTcpSocket* m_socket;
};

#endif // REGLOG_H
