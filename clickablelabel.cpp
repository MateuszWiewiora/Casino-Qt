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

void ClickableLabel::setSelected(bool selected)
{
    m_selected = selected;

    // Get current stylesheet and preserve background image settings
    QString currentStyle = this->styleSheet();

    // Update only the border color part while preserving the background image
    if (selected)
    {
        currentStyle.replace(UNSELECTED_BORDER_COLOR, SELECTED_BORDER_COLOR);
    }
    else
    {
        currentStyle.replace(SELECTED_BORDER_COLOR, UNSELECTED_BORDER_COLOR);
    }

    this->setStyleSheet(currentStyle);

    emit selectionChanged(selected);
}

void ClickableLabel::updateStyle()
{
    QString color = m_selected ? SELECTED_BORDER_COLOR : UNSELECTED_BORDER_COLOR;
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