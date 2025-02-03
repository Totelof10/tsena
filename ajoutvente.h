#ifndef AJOUTVENTE_H
#define AJOUTVENTE_H

#include <QWidget>
#include <QListWidget>
#include <QDate>
#include <QPrinter>
#include <QPrintDialog>
#include <QTextDocument>
#include <QFile>
#include <QTextStream>
#include <QPrintPreviewDialog>
#include <QTextEdit>


namespace Ui {
class AjoutVente;
}

class AjoutVente : public QWidget
{
    Q_OBJECT

public:
    explicit AjoutVente(QWidget *parent = nullptr);
    ~AjoutVente();

private slots:
    void ajouterNouvelleVente();
    void afficherInformation();
    void ajouterPanier();
    void enleverPanier();
    void viderPanier();
    void mettreAJourPrix(int index);
    void mettreAJourTotal();
    void clearForm();
    void remettreAZero(int index);



signals:
    void ajouterVente();
    void CA();

private:
    Ui::AjoutVente *ui;
    QMap<int, double> mapPrixProduits;
    QMap<int, double> mapTotauxProduits; // Stocke les totaux par produit
    QTextEdit *texte;
    double totalCumulatif = 0.0; // Stocke le total cumulé
    QString convertirNombreEnLettres(long long nombre) {
        if (nombre == 0) return "zéro";

        static const QStringList unites = { "", "un", "deux", "trois", "quatre", "cinq", "six", "sept", "huit", "neuf" };
        static const QStringList dizaines = { "", "dix", "vingt", "trente", "quarante", "cinquante", "soixante", "soixante-dix", "quatre-vingt", "quatre-vingt-dix" };
        static const QStringList specialDizaines = { "dix", "onze", "douze", "treize", "quatorze", "quinze", "seize", "dix-sept", "dix-huit", "dix-neuf" };

        QString result;

        // Fonction pour convertir un nombre entre 1 et 999
        auto convertirCentaines = [&](int n) -> QString {
            QString res;
            if (n >= 100) {
                int centaine = n / 100;
                res += (centaine == 1) ? "cent" : unites[centaine] + " cent";
                n %= 100;
                if (n > 0) res += " ";
            }

            if (n >= 10 && n <= 19) {
                res += specialDizaines[n - 10];
            } else {
                int dizaine = n / 10;
                int unite = n % 10;

                if (dizaine > 0) {
                    res += dizaines[dizaine];
                    if (unite == 1 && (dizaine == 1 || dizaine == 7 || dizaine == 9)) {
                        res += "-et-";
                    } else if (unite > 0) {
                        res += "-";
                    }
                }

                if (unite > 0) {
                    res += unites[unite];
                }
            }
            return res;
        };

        // Gestion des millions
        if (nombre >= 1000000) {
            int million = nombre / 1000000;
            result += (million == 1) ? "un million" : convertirCentaines(million) + " millions";
            nombre %= 1000000;
            if (nombre > 0) result += " ";
        }

        // Gestion des milliers
        if (nombre >= 1000) {
            int millier = nombre / 1000;
            if (millier == 1) {
                result += "mille";
            } else {
                result += convertirCentaines(millier) + " mille";
            }
            nombre %= 1000;
            if (nombre > 0) result += " ";
        }

        // Gestion des centaines, dizaines et unités
        if (nombre > 0) {
            result += convertirCentaines(nombre);
        }
        result += " Ariary";

        return result;
    }



};

#endif // AJOUTVENTE_H
