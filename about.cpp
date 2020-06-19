#include "about.h"
#include <QProcess>
#include <QDebug>
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
char licenseTerm[BUFF_SIZE] = {0};

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
char *copy_right =NULL;
char *icon_name = NULL;
char kyinfoTerm[BUFF_SIZE] = {0};


int match_systemname(char *name)
{
    FILE *fp;
    char line[LINE_BUFF_SIZE_128] = {0};
    char substr[LINE_BUFF_SIZE_32] = {0};
    fp = fopen(LSB_RELEASE, "r");
    if (!fp)
    {
    printf("open /etc/lsb-release file error!");
        return -1;
    }
    fgets(line, LINE_BUFF_SIZE_128, fp);
    fclose(fp);
    strncpy(substr, line + 11, strlen(line) - 11);
    if (strcmp(substr, name) == 0)
    {
    return 0;
    }
    return -1;
}

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
    char homefile[80], info[1024];
    int fd;
    char *name= NULL;
    QString copy_right;
    char *icon_name = NULL;
    char kyinfoTerm[BUFF_SIZE] = {0};
    char licenseTerm[BUFF_SIZE] = {0};
    memset(homefile, 0, 80);
    memset(info, 0, 1024);
    gchar *kernel_name = NULL;
    gchar *version = NULL;
    struct utsname uts;
    if (uname(&uts) >= 0) {
        kernel_name = g_strdup(uts.release);
        printf("VERSION:[%s], RELEASE:[%s]\n", uts.version, kernel_name);
    }

//    if(is_business_version()) //getSystemInfo.py里面已处理
    version = get_lsb_release_value("4.0.2");
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

    if(TRUE == is_pks_version())
    {

        if (kernel_name)
        {
            if (strstr(kernel_name, ".server"))
            {
                name = "Kylin Server PKS";
            }
            else
            {
                name = "Kylin Desktop PKS";
            }
            g_free(kernel_name);
            kernel_name = NULL;    setWindowIcon(QIcon::fromTheme("distributor-logo-kylin"));
        }
        else
        {
            name = "Kylin Desktop PKS";
        }

        icon_name = "/usr/share/ukui/kylin.png";
        copy_right = tr("All rights reserved by 2009-2020 KylinOS. all rights reserved.\n Kylin %1 and its user interface is protected by intellectual property laws trademark law in China and other countries and other regions to be enacted or enacted.").arg(version);
    }
    else if (match_systemname("Kylin\n") == 0)
    {
        name = "Kylin";
        icon_name = "/usr/share/ukui/kylin.png";
        copy_right = tr("All rights reserved by 2009-2020 KylinOS. all rights reserved.\n Kylin %1 and its user interface is protected by intellectual property laws trademark law in China and other countries and other regions to be enacted or enacted.").arg(version);
    }
    else if (match_systemname("YHKylin\n") == 0)
    {
        name = "YHKylin";
        icon_name = "/usr/share/mate-about/yhkylin.png";
        copy_right = tr("All rights reserved by 2009-2020 YHKylinOS. all rights reserved. \n YHKylin version 4 and its user interface is protected by intellectual property laws trademark law in China and other countries and other regions to be enacted or enacted.");
    }
    else if (match_systemname("NeoKylin\n") == 0)
    {
        name = "NeoKylin";
        icon_name = "/usr/share/mate-about/neokylin.png";
        copy_right = tr("All rights reserved by 2009-2020 NeoKylinOS. all rights reserved.\n NeoKylin version 10 and its user interface is protected by intellectual property laws trademark law in China and other countries and other regions to be enacted or enacted.");
    }

    QString minfo="1\n 2\n 3\n 4\n 5\n 6\n 7\n 8\n 9\n 10\n 1\n 2\n 3\n 4\n 5\n 6\n 7\n 8\n 9\n 10\n";
    QPixmap kylinicon(icon_name);
    kylinicon=kylinicon.scaled(LOGO_WIDTH,LOGO_HIGTH);
    label_logo=new QLabel(this);
    label_logo->setPixmap(kylinicon);
    label_logo->setAlignment(Qt::AlignCenter);
    label_logo->adjustSize();
    label_logo->setGeometry(0,LEAVE_BLANK_HIGHT,ABOUT_WIDGET_WIDTH,label_logo->height());

    label_info=new QLabel(this);
    label_info->setText(minfo);
    label_info->setWordWrap(true);
    label_info->setAlignment(Qt::AlignCenter);
    label_info->adjustSize();
    QFont font;
    /* 使用QFont的setPointSize，此时字体的大小会跟随DPI改变而改变。
     * 使用QFont的setPixelSize，此时字体的大小不会随DPI的改变而发生变化。
     * 但两者都存在的问题是遇到无info文字被遮挡的问题
　　　*/
//    font.setPointSize(20);
    label_info->setFont(font);
    label_info->setGeometry(0,LEAVE_BLANK_HIGHT+label_logo->height(),ABOUT_WIDGET_WIDTH,label_info->height());

    label_copyright=new QLabel(this);
    label_copyright->setText(copy_right);
    label_copyright->setAlignment(Qt::AlignCenter);
    label_copyright->adjustSize();
    label_copyright->setGeometry(0,LEAVE_BLANK_HIGHT+label_logo->height()+label_info->height(),ABOUT_WIDGET_WIDTH,label_copyright->height());

    label_website=new QLabel(this);
    label_website->setOpenExternalLinks(true);
    label_website->setText(QString::fromLocal8Bit("<a style='color: blue;' href = http://www.kylinos.cn> http://www.kylinos.cn</a>"));
    label_website->setAlignment(Qt::AlignCenter);
    label_website->setGeometry(0,LEAVE_BLANK_HIGHT+label_logo->height()+label_info->height()+label_copyright->height(),ABOUT_WIDGET_WIDTH,label_website->height());
    qDebug()<<"";
    this->setFixedSize(ABOUT_WIDGET_WIDTH,LEAVE_BLANK_HIGHT*2+label_logo->height()+label_info->height()+label_copyright->height()+label_website->height());
}


void About::disPlay()
{
    mate_about_run();
    setWindowTitle(tr("about Kylin"));
}

