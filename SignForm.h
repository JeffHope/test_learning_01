#ifndef SIGNIN_H
#define SIGNIN_H
#include <QFile>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QWidget>
#include <QDialog>
#include <QTcpSocket>
#include "RegistrationForm.h"

namespace Ui {
class SignForm;
}

class SignForm : public QDialog
{
    Q_OBJECT

public:
    explicit SignForm(QWidget *parent = nullptr);
    ~SignForm();

    const QString &currentLogin();
    const QString &currentPassword();

signals:
    void authorize();

private:
    Ui::SignForm *ui;
    RegistrationForm* regIn;
    QString m_login;
    QString m_pass;
};

#endif // SIGNIN_H
