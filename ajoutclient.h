#ifndef AJOUTCLIENT_H
#define AJOUTCLIENT_H

#include <QWidget>

namespace Ui {
class AjoutClient;
}

class AjoutClient : public QWidget
{
    Q_OBJECT

public:
    explicit AjoutClient(QWidget *parent = nullptr);
    ~AjoutClient();

private slots:
    void ajouterNouveauClient();
    void annuler();

signals:
    void ajouteClient();

private:
    Ui::AjoutClient *ui;
};

#endif // AJOUTCLIENT_H
