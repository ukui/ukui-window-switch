#ifndef UKWS_WINDOW_ICON_H
#define UKWS_WINDOW_ICON_H

#include <QLabel>
//#include <Q>

class UkwsWindowIcon : public QLabel
{
    Q_OBJECT
public:
    explicit UkwsWindowIcon(QFrame *parent = nullptr);

    int w;
    int h;
    QPixmap originalQPixmap;
};

#endif // UKWS_WINDOW_ICON_H
