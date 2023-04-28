#include "SignForm.h"
#include "ui_SignForm.h"
#include <QDebug>
#include <mainwindow.h>
#include <QMessageBox>
#include "MessageHelper.h"

SignForm::SignForm(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SignForm)
{
    ui->setupUi(this);
    regIn = new RegistrationForm(this);
    connect(ui->pb_signin, &QAbstractButton::clicked, this, &SignForm::authorize);
    connect(ui->pb_reg, &QAbstractButton::clicked, regIn, &QDialog::exec);
    ui->le_password->setEchoMode(QLineEdit::EchoMode::Password);

    // TODO: remove
    ui->le_login->setText("AAA");
    ui->le_password->setText("111");
}


const QString &SignForm::currentLogin() {
    m_login = ui->le_login->text();
    return m_login;
}

const QString &SignForm::currentPassword() {
    m_pass = ui->le_password->text();
    return m_pass;
}

SignForm::~SignForm() {
    delete ui;
}
