#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


private slots:
    void inscription();
    void acceder();
    void nettoyerChamps();
    void inscrireUtilisateur();
    void fermerFenetre();
    void afficher();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
