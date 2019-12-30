#include <QStyleOption>
#include <QPainter>

#include "ukws_window_extra_label.h"

UkwsWindowExtraLabel::UkwsWindowExtraLabel()
{
}


void UkwsWindowExtraLabel::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QStyleOption styleOpt;
    styleOpt.init(this);
    QPainter painter(this);
    style()->drawPrimitive(QStyle::PE_Widget, &styleOpt, &painter, this);

    QLabel::paintEvent(event);
}
