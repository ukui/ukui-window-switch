#ifndef UKWS_WINDOW_EXTRA_LABEL_H
#define UKWS_WINDOW_EXTRA_LABEL_H

#include <QLabel>

class UkwsWindowExtraLabel : public QLabel
{
    Q_OBJECT
public:
    explicit UkwsWindowExtraLabel();
    void paintEvent(QPaintEvent *);

    QPixmap originalQPixmap;
};

#endif // UKWS_WINDOW_EXTRA_LABEL_H
