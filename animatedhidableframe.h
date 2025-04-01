#ifndef ANIMATEDHIDABLEFRAME_H
#define ANIMATEDHIDABLEFRAME_H

#include <QFrame>
#include <QWidget>
#include <QPushButton>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QMap>

class AnimatedHidableFrame : public QFrame {
    Q_OBJECT

public:
    explicit AnimatedHidableFrame(QWidget *parent = nullptr);
    void setupButtons(QPushButton* venteButton,
                      QPushButton* stockButton,
                      QPushButton* bonButton,
                      QPushButton* tiersButton);

private slots:
    void toggleFrame();

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    QPushButton* toggleButton;
    QPropertyAnimation* widthAnimation;
    QMap<QPushButton*, QString> originalButtonTexts;
    QMap<QPushButton*, QIcon> originalButtonIcons;
    QPushButton* btnPageVente;
    QPushButton* btnPageStock;
    QPushButton* btnPageBon;
    QPushButton* btnTiers;
};

#endif // ANIMATEDHIDABLEFRAME_H
