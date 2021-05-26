#ifndef WIDGET_H
#define WIDGET_H
#include <QProcess>
#include <QWidget>
#include <QString>
#include <QLabel>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QMainWindow>
#include <QScrollBar>
#include <QTextEdit>
#include <QPushButton>
#include <QFile>
#include <QStyleOption>
#include <QPainter>
#include <QGSettings>

class About : public QWidget
{
    Q_OBJECT

public:
    About(QWidget *parent = nullptr);
    ~About();

    void connectStyleChange();

private:
    void getIconCopyrightNameInfo();
    void getKernelVersionInfo();
    void getVersionActivation();
    bool getOsRelease();
    QString getCommissionVersion();
    QString getLsbRealse(QString key);
    QString getDescriptionVersion();

    void disPlay();
    void logoChange();
    void mate_about_run();
    QLabel *label_logo;
    QLabel *label_title;
    QLabel *label_info;
    QLabel *label_copyright;
    QLabel *label_website;
    QTextEdit  *text_copyright;

    QGSettings *gsettings;
    QGSettings *gsettings_style;
    int transparency;

    const char* program_name = "Kylin";
    const char* icon = "/usr/share/mate-about/logo.png";
    const char* desktop_icon = "desktop";
    const char* website = "http://www.kylinos.cn/";

    //主界面显示的版权信息
    QString copy_right;
    //主界面显示的图标信息
    QString icon_name;

protected:
    void paintEvent(QPaintEvent *);
};
#endif // WIDGET_H
