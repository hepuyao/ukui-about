#include "about.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QTranslator>
#include <QDebug>
#include <QStandardPaths>
#include <fcntl.h>
#include <syslog.h>
#include <X11/Xlib.h>

void centerToScreen(QWidget* widget) {
    if (!widget)
      return;
    QDesktopWidget* m = QApplication::desktop();
    QRect desk_rect = m->screenGeometry(m->screenNumber(QCursor::pos()));
    int desk_x = desk_rect.width();
    int desk_y = desk_rect.height();
    int x = widget->width();
    int y = widget->height();
    widget->move(desk_x / 2 - x / 2 + desk_rect.left(), desk_y / 2 - y / 2 + desk_rect.top());
}

int main(int argc, char *argv[])
{
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    QApplication a(argc, argv);

    //Singleton
    QStringList homePath = QStandardPaths::standardLocations(QStandardPaths::HomeLocation);
    QString lockPath = homePath.at(0) + "/.config/ukui-about";
    int fd = open(lockPath.toUtf8().data(), O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    if (fd < 0) { exit(1); }
    if (lockf(fd, F_TLOCK, 0)) {
        syslog(LOG_ERR, "Can't lock single file, ukui-about is already running!");
        qDebug()<<"Can't lock single file, ukui-about is already running!";
        exit(0);
    }

    //tanslate
    QString locale = QLocale::system().name();
    QTranslator translator;
    if (locale == "zh_CN"){
        if (translator.load("ukui-about_zh_CN.qm", "/usr/share/ukui/"))
            a.installTranslator(&translator);
        else
            qDebug() << "Load translations file" << locale << "failed!";
    }
    About *about =new About;
    centerToScreen(about);
    about->show();
    return a.exec();
}
