#include "clickablelabel.h"
#include <QMouseEvent>

ClickableLabel::ClickableLabel(QWidget *parent, Qt::WindowFlags f)
    : QLabel(parent, f)
{
    setStyleSheet(QString("QLabel { border: %1 %2; }").arg(SELECTION_BORDER_STYLE).arg(UNSELECTED_BORDER_COLOR));
}

ClickableLabel::~ClickableLabel()
{
}

void ClickableLabel::setSelected(bool value)
{
    if (selected != value)
    {
        selected = value;
        updateStyle();
        emit selectionChanged(selected);
    }
}

void ClickableLabel::updateStyle()
{
    QString color = selected ? SELECTED_BORDER_COLOR : UNSELECTED_BORDER_COLOR;
    setStyleSheet(QString("QLabel { border: %1 %2; }").arg(SELECTION_BORDER_STYLE).arg(color));
}

void ClickableLabel::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        emit clicked();
    }
    QLabel::mousePressEvent(event);
}