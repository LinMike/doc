#include <qapplication.h>
#include <qpushbutton.h>
#include <qslider.h>
#include <qlcdnumber.h>
#include <qfont.h>
#include <qwidget.h>

class MyWidget : public QVBox {
public:
    MyWidget(QWidget *parent, const char *name=0);
};

MyWidget::MyWidget(QWidget *parent, const char *name) : QVBox(parent, name) {

}

int main(int argc, char** argv)
{
	QApplication a(argc, argv);
	QPushButton hello("hello world", 0);
	hello.resize(100, 30);

//	a.setMainWidget(&hello); no need after qt3
	hello.show();
	return a.exec();
}
