#include "widget.h"
#include "ui_widget.h"
#include <QProcess>
#include <QDebug>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/utsname.h>
#include <QDebug>
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
#define DISTRIB_ID "Kylin"
#define DISTRIB_RELEASE "4.0.2"
#define DISTRIB_CODENAME "juniper"
#define DISTRIB_DESCRIPTION "Kylin 4.0.2"
#define DISTRIB_KYLIN_RELEASE "4.0-2SP3"
#define DISTRIB_VERSION_TYPE "enterprise"       //true为社区版本
#define DISTRIB_VERSION_MODE "normal"
#define DISTRIB_VERSION_CUSTOM "Kylin"    //pks为社区版pks

#define LINE_BUFF_SIZE_128 128
#define LINE_BUFF_SIZE_64 64
#define LINE_BUFF_SIZE_32 32

#define KYINFO_FILE "/etc/.kyinfo"
#define LICENSE_FILE "/etc/LICENSE"
#define BUFF_SIZE 256

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

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    disPlay();

}

Widget::~Widget()
{
    delete ui;
}

void Widget::mate_about_run(void)
{
    GError *error = NULL;
    char homefile[80], info[1024];
    int fd;
    char *name= NULL;
    char *copy_right =NULL;
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
            if (g_file_test("/usr/bin/getSystemInfo.py", G_FILE_TEST_EXISTS)) {
                system("python3 /usr/bin/getSystemInfo.py noShowTerm");
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
            if (g_file_test("/usr/bin/getSystemInfo.py", G_FILE_TEST_EXISTS)) {
                system("python3 /usr/bin/getSystemInfo.py ShowTerm");
                sprintf(homefile, "%s/.info", getenv("HOME"));
                fd = open(homefile, O_RDONLY);
                read(fd, info, sizeof(info));
                qDebug()<<"info:    "<<info;
                ui->label_2->setText(info);
//                close(fd);
            }
        }
    }
    else
    {
        qDebug()<<"kyinfoTerm & licenseTerm  =0 ";
        if (g_file_test("/usr/bin/getSystemInfo.py", G_FILE_TEST_EXISTS)) {
            system("python3 /usr/bin/getSystemInfo.py ShowTerm");
            sprintf(homefile, "%s/.info", getenv("HOME"));
            fd = open(homefile, O_RDONLY);
            read(fd, info, sizeof(info));
                ui->label_2->setText(info);
                qDebug()<<"info    :   "<<info;
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
            kernel_name = NULL;
        }
        else
        {
            name = "Kylin Desktop PKS";
        }

        icon_name = "/usr/share/mate-about/kylin.png";
        copy_right = g_strdup_printf ("All rights reserved by 2009-2020 KylinOS. all rights reserved.\nKylin %s and its user interface is protected by intellectual property laws trademark law in China and other countries and other regions to be enacted or enacted.", version);

    }
    else if (match_systemname("Kylin\n") == 0)
    {
        name = "Kylin";
        icon_name = "/usr/share/mate-about/kylin.png";
        copy_right = g_strdup_printf ("All rights reserved by 2009-2020 KylinOS. all rights reserved.\nKylin %s and its user interface is protected by intellectual property laws trademark law in China and other countries and other regions to be enacted or enacted.", version);
    }
    else if (match_systemname("YHKylin\n") == 0)
    {
        name = "YHKylin";
        icon_name = "/usr/share/mate-about/yhkylin.png";
        copy_right = "All rights reserved by 2009-2020 YHKylinOS. all rights reserved.\nYHKylin version 4 and its user interface is protected by intellectual property laws trademark law in China and other countries and other regions to be enacted or enacted.";
    }
    else if (match_systemname("NeoKylin\n") == 0)
    {
        name = "NeoKylin";
        icon_name = "/usr/share/mate-about/neokylin.png";
        copy_right = "All rights reserved by 2009-2020 NeoKylinOS. all rights reserved.\nNeoKylin version 10 and its user interface is protected by intellectual property laws trademark law in China and other countries and other regions to be enacted or enacted.";
    }


//    qDebug()<<"name    *****:    "<<name;
//    qDebug()<<"version    *****:    "<<version;
//    qDebug()<<"copy_right    *****:    "<<copy_right;
//    qDebug()<<"info    ****:"<<info;


//    QIcon kylinicon=QIcon("/home/kylin/work/v101/about/ukui-about2/resource/kylin.png");
    QIcon kylinicon=QIcon("/usr/share/mate-about/kylin.png");
    ui->pushButton->setIcon(kylinicon);
    ui->pushButton->setIconSize(QSize(400,300));
    ui->pushButton->setText("");
    ui->label->setText(copy_right);

    ui->label_3->setText(website);

        //gtk 设置界面的方式
#if 0
    // name
    gtk_about_dialog_set_program_name(mate_about_dialog, gettext(name)); //版本名称

    GdkPixbuf *logo = gdk_pixbuf_new_from_file(icon_name, &error);
    if(!error)
        gtk_about_dialog_set_logo(mate_about_dialog, logo);              //logo
    else
    {
        g_print("Error:%s\n", error->message);
        g_error_free(error);
    }//modified by ph

    // version
    gtk_about_dialog_set_version(mate_about_dialog, version);             //版本

    // credits and website
    if(copy_right!=NULL)
        gtk_about_dialog_set_copyright(mate_about_dialog, copy_right);  //copyright
    gtk_about_dialog_set_website(mate_about_dialog, website);            //websete

    /**
     * This generate a random message.
     * The comments index must not be more than comments_count - 1
     */
    int count = 2;
    char *lang, *p = info, *welcome_str = "Welcome to use Kylin!";
    lang = g_getenv ("LANG");
    if(strncmp(lang,"zh_CN",strlen("zh_CN"))!=0){
        for(;count;)
            if(*(p++) == '\n')
                count--;
        sprintf(p, "%s", welcome_str);
        p += strlen(welcome_str);
        memset(p, 0, info + 1024 -p);
    }


    gtk_about_dialog_set_comments(mate_about_dialog, info);                  //info

//    mate_about_dialog_set_authors(mate_about_dialog, authors);
//    mate_about_dialog_set_artists(mate_about_dialog, artists);
//    mate_about_dialog_set_documenters(mate_about_dialog, documenters);
    /* Translators should localize the following string which will be
     * displayed in the about box to give credit to the translator(s). */
//    mate_about_dialog_set_translator_credits(mate_about_dialog, _("translator-credits"));

    #if GTK_CHECK_VERSION(3, 0, 0)
        gtk_window_set_application(GTK_WINDOW(mate_about_dialog), mate_about_application);
    #endif

    // start and destroy
    gtk_dialog_run((GtkDialog*) mate_about_dialog);
    gtk_widget_destroy((GtkWidget*) mate_about_dialog);

#endif

#if 1

#endif
}


void Widget::disPlay()
{
    mate_about_run();
    setWindowTitle("关于银河麒麟");
//    setWindowIcon(QIcon::fromTheme("firefox"));
    ui->label->setAlignment(Qt::AlignCenter);
    ui->label_2->setText("");

}
