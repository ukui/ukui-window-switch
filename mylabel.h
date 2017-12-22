#ifndef MY_LABEL_H
#define MY_LABEL_H

#include <QLabel>
#include <QWidget>
#include <QMouseEvent>

class MyLabel : public QLabel
{
    Q_OBJECT
public:
    explicit MyLabel(int index, const QString &text = "", QWidget *parent = 0);
    ~MyLabel();
signals:
    void myclicked(int);

protected:
    void mousePressEvent(QMouseEvent *event);

public:
    void setBrightColor();
    void setHideColor();
    QString getTitle();
    void setTitle(QString title);
    int getIndex();
    void setIndex(int index);

private:
    int m_index;
    QString m_title;
};

#endif //MY_LABEL_H
