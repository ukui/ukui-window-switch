#ifndef UKWS_NEW_WORKSPACE_BUTTON_H
#define UKWS_NEW_WORKSPACE_BUTTON_H

#include "ukws_window_extra_label.h"

#include <QObject>
#include <QWidget>
#include <QPushButton>
#include <QLayout>
#include <QEvent>
#include <QDebug>

class UkwsNewWorkspaceButton : public QWidget
{
    Q_OBJECT
public:
    explicit UkwsNewWorkspaceButton(QWidget *parent = nullptr);
//    void paintEvent(QPaintEvent *);
    bool eventFilter(QObject *object, QEvent *event);

public slots:
    void setSize(int width, int height);
    void setSizeByButtonSize(int width, int height);
//    void updateContent();

signals:
    void addWorkspace();

private:
    QPainterPath drawBackgroundPath();
    QPixmap drawBackground(QPainterPath path, QColor color);
    UkwsWindowExtraLabel *title;
    UkwsWindowExtraLabel *button;

    QVBoxLayout *mainLayout;

    QPixmap normalPixmap;
    QPixmap hoverPixmap;
    QPixmap pressPixmap;
    QPixmap disablePixmap;
};

#endif // UKWS_NEW_WORKSPACE_BUTTON_H
