#include <QWidget>
#include "ui_mainwindow.h"
#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), c(new(SocketConnection)), d(new(DisplayDataStream)), ui(new(Ui::MainWindow))
{

	setupActions();
	QPixmap pixmap( 200, 330 );
	pixmap.fill( Qt::white );

	QPainter painter( &pixmap );
	painter.setPen( Qt::black );
    

	QPoint point = QPoint( 10, 20 );
	painter.drawText( point, "You can draw text from a point..." );
	painter.drawLine( point+QPoint(-5, 0), point+QPoint(5, 0) );
	painter.drawLine( point+QPoint(0, -5), point+QPoint(0, 5) );
  }

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupActions()
{
    QAction* connectAction = new QAction(this);
    connectAction->setText("Connect");

//    connectAction->setIcon(QIcon::fromTheme(""));
//    actionCollection()->setDefaultShortcut(clearAction, Qt::CTRL + Qt::Key_W);
    actionCollection()->addAction("connect", connectAction);
    connect(c, &SocketConnection::dataStreamComplete, this, &MainWindow::processDataStream);

    connect(connectAction, triggered(bool), this, SLOT(makeConnection()));
    
//    KStandardAction::quit(qApp, SLOT(quit()), actionCollection());
    
//    setupGUI(Default, "k3270ui.rc");
}

void MainWindow::processDataStream()
{
	printf("Found something to process!\n");
	fflush(stdout);
}
	
void MainWindow::paintEvent(QPaintEvent *event)
{
    //Drawing Texts


	
//	painter.save();
	
  //  painter.setFont(QFont("Times", 16, QFont::Bold));
//	painter.setPen( Qt::black );
//    painter.drawText(QPoint(10,10), "Qt5 Text Drawing");
//	QFont font = painter.font();
//	font.setPixelSize(15);
//	painter.setFont(font);

//	const QRect rectangle = QRect(0, 0, 100, 50);
	//QRect boundingRect;
//	painter.drawText(rectangle, 0, tr("Hello"), &boundingRect);

 
    // Giving Style To Texts
 
 
  //  QTextDocument document;
  //  QRect rect(0,0,250,250 );
  //  painter.translate(100,50);
 
 //   document.setHtml("<b>Hello</b><font color='red' size='30'>Qt5 C++ </font>");
 //   document.drawContents(&painter, rect);
//	
//	painter.restore();

}

void MainWindow::makeConnection()
{		
	c->connectMainframe(QHostAddress("127.0.0.1"), 3270);
//	qDebug() << "Bytes available: " << c.checkData();
}

