#include "ui_mainwindow.h"
#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), c(new(SocketConnection)), d(new(DisplayDataStream)), ui(new(Ui::MainWindow))
{

	setupActions();
    ui->setupUi(this);

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
//    connect(connectAction, &QAction::triggered, this, &MainWindow::makeConnection);
//    actionCollection()->addAction("connect", connectAction);
    connect(c, &SocketConnection::dataStreamComplete, this, &MainWindow::processDataStream);

//    connect(connectAction, &QAction::triggered, this, SLOT(makeConnection()));
    
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

    QPainter painter(this);

	
    painter.save();
	
    painter.setFont(QFont("Times", 16, QFont::Bold));
    painter.setPen( Qt::black );
    painter.drawText(QPoint(10,10), "Qt5 Text Drawing");
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
    painter.restore();

}
/*
void MainWindow::makeConnection()
{		

//	qDebug() << "Bytes available: " << c.checkData();
}
*/
void MainWindow::on_actionConnect_triggered(bool checked)
{
    c->connectMainframe(QHostAddress("127.0.0.1"), 3270);
    repaint();
}
