#ifndef CUSTOMMESSAGEBOX_H
#define CUSTOMMESSAGEBOX_H

#include <QMessageBox>

class CustomMessageBox : public QMessageBox
{
    Q_OBJECT

public:
    explicit CustomMessageBox(QWidget *parent = nullptr);

    void showInformation(const QString &title, const QString &message);
    void showWarning(const QString &title, const QString &message);
    void showError(const QString &title, const QString &message);
};

#endif // CUSTOMMESSAGEBOX_H
