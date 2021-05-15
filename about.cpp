#include "about.h"
#include <QApplication>
#include <QProcess>
#include <QDebug>
#include <QDesktopWidget>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/utsname.h>
#include <QDebug>
#include<QDesktopServices>
#include<QUrl>
#include <QIcon>

/* qt会将glib里的signals成员识别为宏，所以取消该宏
 * 后面如果用到signals时，使用Q_SIGNALS代替即可
 **/
#ifdef signals
#undef signals
#endif

extern "C" {
#include <glib.h>
#include <gio/gio.h>
}


#define LSB_RELEASE "/etc/lsb-release"
#define DISTRIB_ID "DISTRIB_ID"
#define DISTRIB_RELEASE "DISTRIB_RELEASE"
#define DISTRIB_CODENAME "DISTRIB_CODENAME"
#define DISTRIB_DESCRIPTION "DISTRIB_DESCRIPTION"
#define DISTRIB_KYLIN_RELEASE "DISTRIB_KYLIN_RELEASE"
#define DISTRIB_VERSION_TYPE "DISTRIB_VERSION_TYPE"
#define DISTRIB_VERSION_MODE "DISTRIB_VERSION_MODE"
#define DISTRIB_VERSION_CUSTOM "DISTRIB_VERSION_TYPE"


#define LINE_BUFF_SIZE_128 128
#define LINE_BUFF_SIZE_64 64
#define LINE_BUFF_SIZE_32 32

#define KYINFO_FILE "/etc/.kyinfo"
#define LICENSE_FILE "/etc/LICENSE"
#define BUFF_SIZE 256

#define LEAVE_BLANK_HIGHT   10
#define ABOUT_WIDGET_WIDTH  700
#define LOGO_WIDTH          378
#define LOGO_HIGTH          140

#ifndef mate_gettext
#define mate_gettext(package, locale, codeset) \
    bindtextdomain(package, locale); \
    bind_textdomain_codeset(package, codeset); \
    textdomain(package);
#endif

GError *error = NULL;
char homefile[80], info[1024];
int fd;
char *name= NULL;
//kernel_name 是内核版本
gchar *kernel_name = NULL;
//version 是系统版本代号
gchar *version = NULL;
char kyinfoTerm[BUFF_SIZE] = {0};
char licenseTerm[BUFF_SIZE] = {0};

gchar* get_lsb_release_value(char *p_key)
{
    FILE *fp;
    char line[50] = {0};
    fp = fopen(LSB_RELEASE, "r");
    if (!fp)
    {
        printf("open /etc/lsb-release file error!");
        //        return -1;
    }
    while(NULL != fgets(line, LINE_BUFF_SIZE_32, fp))
    {
        char tmp_key[LINE_BUFF_SIZE_64] = {0};
        char tmp_value[LINE_BUFF_SIZE_64] = {0};
        sscanf(line,"%[^=]=%s",tmp_key,tmp_value);
        //        qDebug()<<"get_lsb_release_value   tmp_value"<<tmp_value   <<"p_key : "<<p_key;
        if(0 == strcmp(tmp_key,p_key))
        {
            fclose(fp);
            printf("get_lsb_release_value:%s",tmp_value);
            return g_strdup_printf("%s",tmp_value);
        }
    }
    fclose(fp);
    return NULL;
}

//DISTRIB_VERSION_TYPE=community为社区版本标志
gboolean is_business_version()
{
    char *version_type = get_lsb_release_value(DISTRIB_VERSION_TYPE);
    if(version_type == NULL)
        return FALSE;
    if(0 == strcmp(version_type,"community"))
    {
        g_free(version_type);
        return FALSE;
    }
    g_free(version_type);
    return TRUE;
}

//DISTRIB_VERSION_CUSTOM=pks为社区版pks
gboolean is_pks_version()
{
    char *version_type = get_lsb_release_value(DISTRIB_VERSION_CUSTOM);
    if(version_type == NULL)
        return FALSE;
    if(0 == strcmp(version_type,"pks"))
    {
        g_free(version_type);
        return TRUE;
    }
    g_free(version_type);
    return FALSE;
}


//格式化转时间戳
long int string_2_time(char *str_time)
{
    struct tm stm;
    int iY=0, iM=0, iD=0, iH=0, iMin=0, iS=0;

    memset(&stm,0,sizeof(stm));

    iY = atoi(str_time);
    iM = atoi(str_time+5);
    iD = atoi(str_time+8);
    //    iH = atoi(str_time+11);
    //    iMin = atoi(str_time+14);
    //    iS = atoi(str_time+17);

    stm.tm_year=iY-1900;
    stm.tm_mon=iM-1;
    stm.tm_mday=iD;
    stm.tm_hour=iH;
    stm.tm_min=iMin;
    stm.tm_sec=iS;

    printf("%d-%0d-%0d %0d:%0d:%0d\n", iY, iM, iD, iH, iMin, iS);

    return mktime(&stm);
}

About::About(QWidget *parent)
    : QWidget(parent)
{
    setWindowIcon(QIcon::fromTheme("distributor-logo-kylin"));
    disPlay();

}

About::~About()
{
}

void About::mate_about_run(void)
{
    GError *error = NULL;
    int fd;
    char *name= NULL;
    memset(homefile, 0, 80);
    memset(info, 0, 1024);
}

/*
 * 获取当前系统版本激活情况
*/
void About::getVersionActivation()
{
    //    if(is_business_version()) //getSystemInfo.py里面已处理
    version = get_lsb_release_value("DISTRIB_RELEASE");
    if (0 == access(KYINFO_FILE, F_OK)&&0==access(LICENSE_FILE, F_OK))
    {
        FILE *kyinfoFd,*licenseFD;
        char line[BUFF_SIZE] = {0};
        int len;
        if((kyinfoFd = fopen(KYINFO_FILE,"r")) == NULL)
        {
            perror("KYINFO_FILE fail to read\n");
        }
        if((licenseFD = fopen(LICENSE_FILE,"r")) == NULL)
        {
            perror("LICENSE_FILE fail to read\n");
        }
        while(fgets(line,BUFF_SIZE,kyinfoFd) != NULL)
        {
            if(strstr(line,"term="))
            {
                len = strlen(line);
                line[len-1] = '\0';  /*去掉换行符*/
                sscanf(line,"term=%s",kyinfoTerm);
                memset(line, 0, BUFF_SIZE);
                break;
            }
            memset(line, 0, BUFF_SIZE);
        }

        while(fgets(line,BUFF_SIZE,licenseFD) != NULL)
        {
            if(strstr(line,"TERM:"))
            {
                len = strlen(line);
                line[len-1] = '\0';  /*去掉换行符*/
                sscanf(line,"TERM:%s",licenseTerm);
                memset(line, 0, BUFF_SIZE);
                break;
            }
            memset(line, 0, BUFF_SIZE);
        }
        fclose(kyinfoFd);
        fclose(licenseFD);
        printf("m_pKyinfoTerm=%s\n",kyinfoTerm);
        printf("m_pLicenseTerm=%s\n",licenseTerm);
    }

    if(kyinfoTerm[0]!='\0'&&licenseTerm[0]!='\0')
    {
        if(string_2_time(kyinfoTerm)==string_2_time(licenseTerm))//没有激活
        {
            time_t t;
            if (g_file_test("/usr/share/ukui/getSystemInfo.py", G_FILE_TEST_EXISTS)) {
                system("python3 /usr/share/ukui/getSystemInfo.py noShowTerm");
                sprintf(homefile, "%s/.info", getenv("HOME"));
                fd = open(homefile, O_RDONLY);
                read(fd, info, sizeof(info));
                //                close(fd);
            }
            if(time(&t) > string_2_time(kyinfoTerm))//试用已过期
            {
                strcat(info,"\n为保证服务质量和使用体验，请尽快购买正式版本。");
            }
        }
        else
        {
            if (g_file_test("/usr/share/ukui/getSystemInfo.py", G_FILE_TEST_EXISTS)) {
                system("python3 /usr/share/ukui/getSystemInfo.py ShowTerm");
                sprintf(homefile, "%s/.info", getenv("HOME"));
                fd = open(homefile, O_RDONLY);
                read(fd, info, sizeof(info));
                //                close(fd);
            }
        }
    }
    else
    {
        qDebug()<<"kyinfoTerm & licenseTerm  =0 ";
        if (g_file_test("/usr/share/ukui/getSystemInfo.py", G_FILE_TEST_EXISTS)) {
            system("python3 /usr/share/ukui/getSystemInfo.py ShowTerm");
            sprintf(homefile, "%s/.info", getenv("HOME"));
            fd = open(homefile, O_RDONLY);
            read(fd, info, sizeof(info));
            //            close(fd);
        }
    }
}
/*
 * 获取系统内核及版本号
*/
void About::getKernelVersionInfo()
{
    /*
     * utsname
     * 获取当前内核名称和其它信息。
    */
    struct utsname uts;
    /*
     * Linux uname（英文全拼：unix name）命令用于显示系统信息。
     * uname 可显示电脑以及操作系统的相关信息
    */
    if (uname(&uts) >= 0) {
        kernel_name = g_strdup(uts.release);
    }
}


/*
 * 获取主界面图标，copyright，名称等信息
*/
void About::getIconCopyrightNameInfo()
{
    icon_name = "/usr/share/ukui/kylin-dark.png";
    copy_right = tr("All rights reserved by 2009-2020 KylinOS. all rights reserved. Kylin %1 and its user interface is protected by intellectual property laws trademark law in China and other countries and other regions to be enacted or enacted.").arg(getDescriptionVersion());
}

//通过 os-release 获取系统版本
bool About::getOsRelease()
{
    QFile file("/etc/os-release");
    if (!file.open(QIODevice::ReadOnly)) qDebug() << "Read file Failed.";
    while (!file.atEnd()) {
        QByteArray line = file.readLine();
        QString str(line);
        if (str.contains("VERSION=")){
            if(str.contains("Professional") || str.contains("SP1")){
            return true;
            }
        }
    }
    return false;
}

QString About::getLsbRealse(QString key)
{
    if(key == DISTRIB_VERSION_MODE){
        QFile file("/etc/lsb-release");
        if (!file.open(QIODevice::ReadOnly)) qDebug() << "Read file Failed.";
        while (!file.atEnd()) {
            QByteArray line = file.readLine();
            QString str(line);
            if (str.contains(DISTRIB_VERSION_MODE)){
                return str.remove("DISTRIB_VERSION_MODE=");
            }
        }
    }
}

QString About::getCommissionVersion()
{
    QFile file("/etc/lsb-release");
    if (!file.open(QIODevice::ReadOnly)) qDebug() << "Read file Failed.";
    while (!file.atEnd()) {
        QByteArray line = file.readLine();
        QString str(line);
        if (str.contains("DISTRIB_VERSION_MODE=")){
            if(str.contains("gf") || str.contains("GF"))
                return "gf";
        }
    }
    return "normal";
}

QString About::getDescriptionVersion()
{
    QFile file("/etc/lsb-release");
    if (!file.open(QIODevice::ReadOnly)) qDebug() << "Read file Failed.";
    while (!file.atEnd()) {
        QByteArray line = file.readLine();
        QString str(line);
        if (str.contains("DISTRIB_DESCRIPTION=")){
            str.remove("\"");
            str.remove("DISTRIB_DESCRIPTION=Kylin V10 ");
            str = str.simplified();
            if(str=="GF") return tr("GF");
            else return str;
        }
    }
    return "SP1";
}

/*
 * ukui-about 界面主窗口显示
*/
void About::disPlay()
{
    mate_about_run();

    getVersionActivation();
    getIconCopyrightNameInfo();
    getKernelVersionInfo();

    setWindowTitle(tr("about Kylin"));
    /*
     * 将label_logo label_info label_copyright label_website
     * 通过setGeometry在主窗口显示
    */

    /*
     * label_logo
     * 为主界面显示logo
    */

    this->resize(500,400);

    QVBoxLayout *bodyLayout=new QVBoxLayout(this);
    bodyLayout->setContentsMargins(10,5,10,27);
    QHBoxLayout *titleLayout;
    titleLayout = new QHBoxLayout();

    QPushButton *btn=new QPushButton(this);
    btn->setIcon(QIcon::fromTheme("distributor-logo-kylin"));
    btn->setIconSize(QSize(16,16));
    btn->setFixedSize(16,16);
    QLabel *title_label=new QLabel(this);
    title_label->setText("About Kylin");
    QPushButton *btn_close=new QPushButton(this);
    btn_close->setIcon(QIcon::fromTheme("window-close-symbolic"));
    btn_close->setFixedSize(24,24);
    btn_close->setIconSize(QSize(12,12));

    btn_close->setProperty("isWindowButton", 0x2);
    btn_close->setProperty("useIconHighlightEffect", 0x8);
    btn_close->setFlat(true);
    connect(btn_close,&QPushButton::clicked,[this]{
        this->close();
    });

    titleLayout->addWidget(btn);
    titleLayout->addWidget(title_label);
    titleLayout->addWidget(btn_close);


    QPixmap kylinicon(icon_name);
    kylinicon=kylinicon.scaled(192,75);
    label_logo=new QLabel(this);
    label_logo->setPixmap(kylinicon);
    label_logo->setAlignment(Qt::AlignCenter);
    label_logo->adjustSize();
//    label_logo->setGeometry(154,LEAVE_BLANK_HIGHT+20,192,75);



    /*
     * label_title
     * 系统版本信息
    */
    label_title=new QLabel(this);
    label_title->setText(QString(tr("Kylin Desktop Operating System V10 %1")).arg(getDescriptionVersion()));
    label_title->setWordWrap(true);
    label_title->setAlignment(Qt::AlignLeft);

    /*
     * label_info
     * 系统版本信息
    */
    QString info_str(info);
    info_str.replace("Kylin V10",QString("Kylin V10(%1)").arg(getDescriptionVersion()));

    label_info=new QLabel(this);
    label_info->setText(info_str);
    label_info->setWordWrap(true);
    label_info->setAlignment(Qt::AlignLeft);

    /*
     * label_copyright
     * 版权信息
    */
    label_copyright=new QLabel(this);
    label_copyright->setText(copy_right);
    label_copyright->setWordWrap(true);
    label_copyright->adjustSize();
    label_copyright->setMaximumWidth(380);
    label_copyright->setAlignment(Qt::AlignLeft);


    /*
     * label_website 关于银河麒麟的网页跳转
    */
    label_website=new QLabel(this);
    label_website->setOpenExternalLinks(true);
    label_website->setText(QString::fromLocal8Bit("<a style='color: blue;' href = http://www.kylinos.cn> http://www.kylinos.cn</a>"));
    label_website->setAlignment(Qt::AlignLeft);

    QWidget *verticalLayoutWidget;
    verticalLayoutWidget=new QWidget();

    QWidget *scrollAreaWidgetContents;
    scrollAreaWidgetContents = new QWidget();
    scrollAreaWidgetContents->setObjectName(QString::fromUtf8("scrollAreaWidgetContents"));
    scrollAreaWidgetContents->setGeometry(QRect(0, 0, 358, 166));
    scrollAreaWidgetContents->setContentsMargins(25,37,25,37);
    QVBoxLayout *verticalLayout;
    verticalLayout = new QVBoxLayout(scrollAreaWidgetContents);


    verticalLayout->addWidget(label_title);
    verticalLayout->addWidget(label_info);
    verticalLayout->addWidget(label_copyright);
    verticalLayout->addWidget(label_website);

    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFixedSize(436,240);
    scrollArea->setContentsMargins(25,37,25,37);


    scrollArea->setGeometry(0,0,385,166);
    scrollArea->property("drawScrollBarGroove");
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    //利用主题框架设置滚动条属性
    scrollArea->verticalScrollBar()->setProperty("drawScrollBarGroove",false);
    scrollArea->horizontalScrollBar()->setProperty("drawScrollBarGroove",false);
    scrollArea->setWidget(scrollAreaWidgetContents);

    //将标题栏，logo和scrollArea放进layout中
    bodyLayout->addLayout(titleLayout);
    bodyLayout->addWidget(label_logo);
    bodyLayout->addWidget(scrollArea,1,Qt::AlignHCenter);
}

void About::paintEvent(QPaintEvent *e)
{
    QStyleOption opt;
    opt.init(this);
    QPainter p(this);
    p.setPen(Qt::NoPen);
    //double tran=transparency_gsettings->get(TRANSPARENCY_KEY).toDouble()*255;
    QColor color = palette().color(QPalette::Base);
    color.setAlpha(70);
    QBrush brush =QBrush(color);
    p.setBrush(brush);

    p.setRenderHint(QPainter::Antialiasing);
    p.drawRoundedRect(opt.rect,12,12);
    style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}
