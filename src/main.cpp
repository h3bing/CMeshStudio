#include <QApplication>
#include <QIcon>
#include "mainwindow.h"

int main(int argc, char *argv[]) {
  QApplication a(argc, argv);
  // 设置应用程序图标
  a.setWindowIcon(QIcon::fromTheme("application-x-executable"));
  PMainWindow w;
  w.show();
  return a.exec();
}
