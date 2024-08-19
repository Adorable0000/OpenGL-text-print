#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>

#include <QMainWindow>
#include <QTimer>
#include <QGridLayout>
#include <vector>
#include <math.h>
#include <iostream>
#include <QPushButton>

#include "rendertext.h"

class Plot : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core
{
  Q_OBJECT

public:
  explicit Plot(QWidget *parent = nullptr);
  ~Plot();
  std::vector<double> x;
  std::vector<double> y;

  void updatePixels();

protected:
  void initializeGL() override;
  void resizeGL(int width, int height) override;
  void paintGL() override;

private:
  RenderText rendertext;

  Proj proj;
  double pixelWidth;
  double pixelHeight;

  int wgtWidth;
  int wgtHeight;
};


class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow(QWidget *parent = nullptr);
  ~MainWindow();
  Plot *plot;
  QWidget *w;

public slots:
  void replot();
};
#endif // MAINWINDOW_H
