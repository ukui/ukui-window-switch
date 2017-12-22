#include "mylabel.h"

MyLabel::MyLabel(int index, const QString &text, QWidget *parent)
	: QLabel(parent)
{
    //setText(text);
    m_index = index;
    m_title = text;
}

MyLabel::~MyLabel()
{
}

void MyLabel::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
	emit myclicked(m_index);
	setBrightColor();
    }

    QLabel::mousePressEvent(event); //deliver the event to parent class.
}

void MyLabel::setBrightColor()
{
    setStyleSheet("background-color: transparent; border: 2px solid white");
}

void MyLabel::setHideColor()
{
    setStyleSheet("background-color: transparent; border: 2px solid transparent");
}

QString MyLabel::getTitle()
{
    return m_title;
}

void MyLabel::setTitle(QString title)
{
    m_title = title;
}

int MyLabel::getIndex()
{
    return m_index;
}

void MyLabel::setIndex(int index)
{
    m_index = index;
}
