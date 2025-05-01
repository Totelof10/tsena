#ifndef RETOUR_H
#define RETOUR_H

#include <QWidget>

namespace Ui {
class Retour;
}

class Retour : public QWidget
{
    Q_OBJECT

public:
    explicit Retour(QWidget *parent = nullptr);
    ~Retour();

private:
    Ui::Retour *ui;
};

#endif // RETOUR_H
