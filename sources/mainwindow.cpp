#include "mainwindow.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <QPainter>
#include <QLabel>


Plot::Plot(QWidget *parent)
{
  (void)parent;
}


Plot::~Plot()
{

}


/*!
 * \brief Plot::initializeGL OpenGl initialization
 *
 * Function is called once after creating OpenGL widget.
 * To use OpenGL newer than 2.0, inherit class from
 * \class QOpenGLFunctions and call
 * \fn initializeOpenGLFunctions()
 */
void Plot::initializeGL()
{
  initializeOpenGLFunctions();

  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

  rendertext.initTextRender();
}


/*!
 * \brief Plot::resizeGL
 * \param width passed by Qt
 * \param height passed by Qt
 *
 * Overrided QOpenGLWighet virtual function,
 * called after all widget resize events.
 * Every time orthographic matrix is
 * calculated and passed to shaders
 */
void Plot::resizeGL(int width, int height)
{
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glViewport(0, 0, width, height);

  /*TEST VALUES-------------------------------------------*/
  proj.left = 0;
  proj.right = 10000;
  proj.bottom = 0;
  proj.top = 10;

  x = {proj.left, (proj.left + proj.right) * 0.2, (proj.left + proj.right) * 0.4,
       (proj.left + proj.right) * 0.6, (proj.left + proj.right) * 0.8, proj.right};

  y = {proj.bottom, (proj.bottom + proj.top) * 0.2, (proj.bottom + proj.top) * 0.4,
       (proj.bottom + proj.top) * 0.6, (proj.bottom + proj.top) * 0.8, proj.top};

  rendertext.setText(y, x);
  /*--------------------------------------------TEST VALUES*/

  wgtWidth = width;
  wgtHeight = height;

  updatePixels();
}


/*!
 * \brief Plot::updatePixels
 *
 * Set projection matrix of \class RenderText
 * and make it to be in integer pixel values
 * This is needed due to subpixel glyph interpolation
 * so the text won't look fuzzy. This is needed only
 * if you use projection matrix that is not in screen
 * coordinates.
 */
void Plot::updatePixels()
{
  rendertext.setProjMatrix(proj);

  pixelWidth = (proj.right - proj.left)/static_cast<double>(wgtWidth);
  pixelHeight = (proj.top - proj.bottom)/static_cast<double>(wgtHeight);

  rendertext.setPixelHeight(pixelHeight);
  rendertext.setPixelWidth(pixelWidth);

// reserve enough space in widget for text rendering
  rendertext.reserveSpace(wgtWidth, wgtHeight);

// update current value of pixel sizes
  pixelWidth = rendertext.getPixelWidth();
  pixelHeight = rendertext.getPixelHeight();

// update current projection matrix after reserving space
  proj = rendertext.getProjMatrix();

// load changed projection matrix to shaders
  rendertext.updateShaderMatrix();
  rendertext.updateTextPositions();
}


/*!
 * \brief Plot::paintGL paint event
 *
 * Overrided QOpenGLWighet virtual function,
 * called after all widget paint events. To
 * repaint the widget scene you need to call
 * \fn update(). Also, you need to repaint
 * all widget scene every time.
 */
void Plot::paintGL()
{
  makeCurrent();                            // Change render context
  glClear(GL_COLOR_BUFFER_BIT);             // Clear current color buffer

  glMatrixMode(GL_PROJECTION);              // Change to projection mode to enable multiplication between current and perspective matrix
  glLoadIdentity();                         // Clear current render matrix

  // Create perspective matrix with pixel based coordinates
  // to enable nice text interpolation so it won't be fuzzy
  glOrtho(proj.left,
          proj.right,
          proj.bottom,
          proj.top,
          -1, 1);
  glMatrixMode(GL_MODELVIEW);

//  rendertext.renderTextEasy(3654, ((proj.left + proj.right) / 4), 0, Arrange::horizontal);
//  rendertext.renderTextEasy(20189, ((proj.left + proj.right) / 2), 0, Arrange::horizontal);
//  rendertext.renderTextEasy(93459, ((proj.left + proj.right) * 0.75), 0, Arrange::horizontal);

//  rendertext.renderTextEasy(420.97, proj.left + (pixelWidth * 4), ((proj.bottom + proj.top) * 0.75), Arrange::vertical);
//  rendertext.renderTextEasy(563.41, 0, ((proj.bottom + proj.top) / 2), Arrange::vertical);
//  rendertext.renderTextEasy(0, 0, ((proj.bottom + proj.top) / 4), Arrange::vertical);
  rendertext.renderText();

}


MainWindow::MainWindow(QWidget *parent): QMainWindow(parent)
{
  QWidget *wgt = new QWidget();
  this->setCentralWidget(wgt);
  QGridLayout *grid = new QGridLayout();
  wgt->setLayout(grid);


  plot = new Plot();
  grid->addWidget(plot,0,0,1,1);
//  grid->addWidget(glplot1,1,0,1,1);

  QTimer *timer = new QTimer(this);
  timer->connect(timer, &QTimer::timeout, this, &MainWindow::replot);
  timer->start(120);
//  timer->singleShot(15,this, &MainWindow::replot);

  // debug testing QPainter text draw
//  int textsize = 80;
//  QFont qfont;
//  qfont.setPixelSize(textsize);

//  QFontMetrics qftmetrics(qfont);
//  int width = /*qftmetrics.boundingRect("0").width()*/ qftmetrics.horizontalAdvance("5") /*qftmetrics.horizontalAdvance('0')*/;
//  int height = qftmetrics.height();

//  QImage qimg(width*2, height, QImage::Format_Grayscale8);

//  QPainter qpaint(&qimg);
//  qpaint.setRenderHint(QPainter::TextAntialiasing, true);
//  qpaint.setRenderHint(QPainter::SmoothPixmapTransform, true);
//  qpaint.setRenderHint(QPainter::VerticalSubpixelPositioning, true);
//  qpaint.setFont(qfont);
//  qpaint.fillRect(0, 0, width*2, height, Qt::white);
//  qpaint.setBrush(Qt::black);
//  qpaint.setPen(Qt::black);
//  qpaint.drawText(0 - qftmetrics.leftBearing('5'), qftmetrics.tightBoundingRect("0").height() /*- qftmetrics.descent()*/, "5");
//  qpaint.drawText(0 - qftmetrics.leftBearing('8') + qftmetrics.horizontalAdvance('5'), qftmetrics.tightBoundingRect("0").height(), "8");
//  qpaint.drawText(0,height - qftmetrics.descent(),"5");
//  qpaint.drawText(width ,height - qftmetrics.descent(),"1");

//  w = new QWidget;
//  grid->addWidget(w,0,1,1,1);
//  QLabel *l = new QLabel();
//  l->setPixmap(QPixmap::fromImage(qimg));
//  l->setParent(w);
//  w->show();
}


MainWindow::~MainWindow()
{

}


void MainWindow::replot()
{
//  double time1 = clock() / static_cast<double>(CLOCKS_PER_SEC);

  plot->update();


//  double time2 = clock() / static_cast<double>(CLOCKS_PER_SEC);
//  double cpu_time = time2 - time1;

//  printf("CPU TIME: %.6f sec\n", cpu_time);
//  printf("\n");
}
