#ifndef CLICKABLELABEL_H
#define CLICKABLELABEL_H

#include <QLabel>
#include <QWidget>
#include <Qt>

class ClickableLabel : public QLabel
{
    Q_OBJECT

public:
    static constexpr const char *SELECTION_BORDER_STYLE = "3px solid";
    static constexpr const char *SELECTED_BORDER_COLOR = "#0000ff";
    static constexpr const char *UNSELECTED_BORDER_COLOR = "transparent";
    static constexpr const char *WIN_BORDER_COLOR = "#00ff00";
    static constexpr const char *LOSE_BORDER_COLOR = "#ff0000";

    explicit ClickableLabel(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
    ~ClickableLabel();

    bool isSelected() const { return selected; }
    void setSelected(bool value);

signals:
    void clicked();
    void selectionChanged(bool selected);

protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    bool selected = false;
    void updateStyle();
};

#endif // CLICKABLELABEL_H