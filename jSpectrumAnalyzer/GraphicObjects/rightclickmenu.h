#ifndef RIGHTCLICKMENU_H
#define RIGHTCLICKMENU_H

//C System-Headers
//
//C++ System headers
//
//Qt Headers
#include <QObject>
#include <QMenu>
#include <QAction>
//OpenCV Headers
//
//Boost Headers
//
//Project specific headers
//

class RightClickMenu : public QMenu {

    Q_OBJECT

  public:
    RightClickMenu(QWidget *parent);

  private:
    //

  signals:
    void Scaling();
};

#endif // RIGHTCLICKMENU_H
