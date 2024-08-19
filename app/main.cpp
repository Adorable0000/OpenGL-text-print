#include "sources/mainwindow.h"
#include <stdio.h>

#include <QApplication>
#include <QLocale>
#include <qstylefactory.h>
#include <QFontDatabase>


int main(int argc, char *argv[])
{
  QApplication a(argc, argv);
  a.setDesktopSettingsAware(false);
  a.setStyle(QStyleFactory::create("Fusion"));

  MainWindow w;
//  w.setMinimumSize(1800,300);
  w.show();
  return a.exec();
}
