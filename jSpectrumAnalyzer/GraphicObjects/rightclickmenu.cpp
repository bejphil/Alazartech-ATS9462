#include "rightclickmenu.h"


RightClickMenu::RightClickMenu( QWidget *parent ) : QMenu( parent ) {

    auto scale_action = new QAction("Scaling",this);
    connect(scale_action, &QAction::triggered, [=]() { emit Scaling(); });
    this->addAction( scale_action );

}
