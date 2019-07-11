
#include "Assertions.h"
#include "build_info.h"
#include "config/pathConfig.h"
#include "Customer/Customer.h"

#include "MainThread.h"
#include "KeyDispatcher.h"
#include "NativeHandler.h"
#include "BrowserAgent.h"
#include "ITCPlayer.h"
#include "JseRoot.h"
#include "MessageTypes.h"
#include "MessageValueSystem.h"
#include "TerminalControl.h"

#include "AppSetting.h"
#include "SysSetting.h"
#if defined(CACHE_CONFIG)
#include "CacheSetting.h"
#endif
#ifdef SETTING_TRANSFORM
#include "app/Setting/Transform/SettingTransform.h"
#endif
#include "sys_msg.h"

#include "Tr069.h"

#if defined(BLUETOOTH)
#include "bt_parse.h"
#endif

#ifdef TVMS_OPEN
#include "tvms_setting.h"
#endif

#ifdef INCLUDE_cPVR
#include "CpvrJsCall.h"
#endif

#if defined(PAY_SHELL)
#include "PayShell.h"
#endif

#if defined(SQM_INCLUDED)
#include "sqm_port.h"
#endif

#ifdef TRANSFORM_WIFI_CONFIG
#include "TransformWifiConfig.h"
#endif

#include "mid_stream.h"
#include "mid/mid_sem.h"
#include "mid_sys.h"
#include "ind_mem.h"
#include "io_xkey.h"
#include "osex_net.h"
#include "codec.h"

#include "TAKIN_common.h"
#include "TAKIN_setting_type.h"

#include "libzebra.h"

#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <fcntl.h>

static char *input_url = NULL;


extern void Jvm_Main_Running(void);
extern void getStartupInfo();
extern void xmppServiceCreate();


static void bash(const char* filename);
static void* threadTakin(void* p);

static int CustomerStart(void);
static void app_stream_init(void);


int iptv_start(int argc, char* argv[])
{
#if defined(HUAWEI_C20) && (!defined(DEBUG_BUILD)) // clear log out put
    int fd = open("/dev/null", O_WRONLY | O_CREAT);
    fflush(stdout);
    dup2(fd, 1);
    dup2(fd, 2);
    close(fd);
#endif

#if !defined(ANDROID)
    system("rm /root/core*");
    // system("rm /home/hybroad/bin/core*"); //A read-only directory, so the operation of non significance
#endif

    yos_systemcall_startCmdEnv();
    Jvm_Main_Running(); // fork the Jvm process
#if defined(hi3560e)
    yhw_board_init();
#endif

    logThreadInit();
    CustomerCreate();
    mainThreadInit();
    keyDispatcherCreate();
    defNativeHandlerCreate();
    epgBrowserAgentCreate();
    mainITCPlayerCreate();
    browserEventQueueCreate();

    setSerialPortPrintState(0); // 关闭日志输出,0:close,1:open.
    JseRootInit(); // 新的JS接口注册

    bash(argv[0]);

    TR069_ROOT_INIT(); // 新的Port 接口

    yos_systemcall_runSystemCMD("killall -9 sshd", NULL); // 关闭sshd，防止SSHD启动后，IPTV异常挂起重启，SSHD不需要重新打开

#if defined(VIETTEL_HD) && defined(brcm7405)
    check_configfile_exist(CONFIG_FILE_DIR"/yx_config_system.ini");
#endif
    settingManagerInit();  // initialize config information
    settingManagerLoad(0); // read config information
    appSettingSetInt("mute", 0);
#ifdef ANDROID // 先加载。。
#ifndef NEW_ANDROID_SETTING
    IPTVMiddleware_SettingParamSync();
#endif
#endif
    mid_sem_t sem = mid_sem_create();
    pthread_t pHandle;
    pthread_create(&pHandle, NULL, threadTakin, sem);
    mid_sem_take(sem, 0x0fffffff, 1);

#if !defined(ANDROID)
    upgradeManagerCreate(1);

    if(argc > 1) {
        if(argv[1][0] == '-') {
            if(argv[1][1] == 's')
                epgBrowserAgentSetTakinSettings(TAKIN_SUPPORT_YUXING_SAVEFILE, "1", 2);
        } else {
            input_url = IND_MALLOC(strlen(argv[1]) + 1);
            IND_STRCPY(input_url, argv[1]);
        }
    }
#else
    input_url = IND_MALLOC(strlen(argv[1]) + 1);
    IND_STRCPY(input_url, argv[1]);
#endif

#ifdef INCLUDE_HYBROADMONITOR
    LogSysOperError("mgmt init....\n");
    mgmt_init();
#endif

#if defined(hi3716m) && defined(ENABLE_VISUALINFO)
    startCollectVisualizationDataThread(); //可视化 信息收集线程
#endif

#if defined(INCLUDE_LITTLESYSTEM) || defined(ANDROID)
    iptv_appInit();
#else
    sendMessageToNativeHandler(0, 0, 0, 0);
#endif

#ifndef ANDROID
    mainThreadRun();
#endif
    return 0;
}

static void bash(const char* filename)
{
    char * p = getenv("STB_INFO");
    if (p && atoi(p) == 1) {
        printf("\n[YuXing Ltd.]  %s\n"
            "\tTime:     %s\n"
            "\tAuthor:   %s\n"
            "\tHost:     %s\n"
            "\tCustomer: %s\n"
            "\tHWBase:   C%d\n"
            "\tsvn:      r%d\n"
            "\tpath:     %s\n"
            "\tmwRev:    %s\n", // 这个留给ANDROID编译脚本来读版本号用，不要删了。
            filename,
            g_make_build_date,
            g_make_build_name,
            g_make_host_name,
            g_make_customer_name,
            _HW_BASE_VER_,
            g_make_svn_revision,
            g_make_svn_path,
            g_make_mw_revision
            );
    }
}

static void *threadTakin(void *p)
{
    mid_sem_t psem = (mid_sem_t)p;

    signal(SIGPIPE, SIG_IGN);

#if !defined(INCLUDE_LITTLESYSTEM) && !defined(ANDROID)
    NativeHandlerSetState(12); //Recovery
#endif

    codec_init(1); // init decoder, use PAL.
    GraphicsConfig();
    mid_task_init( );
	mid_timer_init( );
	mid_http_init( );
	mid_dns_init( );
    mid_net_init(); // middleware network init
    jse_init(); // jse register api
    BootImagesShowLogoInit(); // boot logo init and show
    io_xkey_reg((void*)sys_msg_port_irkey);
    mid_sem_give(psem);

    io_xkey_loop();
    while(1) {
        usleep(0x7fffffff);
    }
    return 0;
}

int iptv_appInit()
{

#if defined(BROWSER_INDEPENDENCE)
    MidwareAgentRun();  // fork the Browser process
#endif

    int topMagin = 0, leftMagin = 0;
    appSettingGetInt("topmagin", &topMagin, 0);
    appSettingGetInt("leftmagin", &leftMagin, 0);
    LayerMixerDeviceSetLeftVertices(leftMagin, topMagin);

#if defined(VIETTEL_HD)
    int size = 0;
    yhw_env_getSystemMemorySize(&size);
    if(size != 128 + 12)
	    yhw_env_setSystemMemorySize(128 + 12);
#endif

#ifdef HUAWEI_C20
    HttpRequestInit();
    PPVListInfoCreate();
#endif // HUAWEI_C20

#ifdef INCLUDE_DMR
    DMRManagerCreate();
#endif

    BrowserPlayerCodeListCreate();

    CustomerStart();

#if (defined(SQM_VERSION_C21) || defined( SQM_VERSION_C22 ) || defined(SQM_VERSION_C23) || defined(SQM_VERSION_C26) || defined(SQM_VERSION_C28) ||defined(SQM_VERSION_ANDROID))
    sqm_port_prepare();
#if (defined( SQM_VERSION_C22 ) || defined(SQM_VERSION_C23) || defined(SQM_VERSION_C26) || defined(SQM_VERSION_C28)||defined(SQM_VERSION_ANDROID))
    sqm_port_pushdata_open();
#endif
#if (defined(SQM_VERSION_C26 ) || defined(SQM_VERSION_C28)||defined(SQM_VERSION_ANDROID))
	sqm_port_pushmsg_open();
#endif
#endif

#ifdef SETTING_TRANSFORM
    settingTransform(); // add unified setting transform func
#endif

#ifdef TRANSFORM_WIFI_CONFIG
    app_transWifiConfig_toNew();
#endif

#if defined(Jiangsu) && defined(EC1308H)
    int upgrade_state = 0;
    sysSettingGetInt("upgrade_state", &upgrade_state, 0);
    if (1 == upgrade_state) {
        sysSettingSetString("upgradeUrl", "http://58.223.80.17:8082/EDS/jsp/upgrade.jsp");
        sysSettingSetInt("upgrade_state", 0);
    }
#endif

#if defined(ANDROID)
    TAKIN_browser_setSetting(TAKIN_SUPPORT_YUXING_SAVEFILE, "0", 2);
    app_Init();
    mid_sys_boot_set(0);
#endif

#ifdef INCLUDE_cPVR
    cpvr_module_init();
#endif

#ifdef INCLUDE_DVBS // Test Use, Can delete.
    dvb_module_init();
#endif

#if defined(XMPP)
    xmppServiceCreate();
#endif

    controlSSHDebug();
#if defined(Huawei_v5) && defined(DEBUG_BUILD)
    setSSHState(1);
    setMonitorState(1);
#endif //HUAWEI_C20
#if defined(Huawei_v5)
    yos_systemcall_runSystemCMD("chmod 777 /root/MicroHei.ttf*", NULL);//zm add ,temporarily methord
    yos_systemcall_runSystemCMD("chmod 700 /root/template", NULL);//zm add ,temporarily methord
    yos_systemcall_runSystemCMD("chmod -R 777 /root/takincookies", NULL);//ld cha ,temporarily methord
    yos_systemcall_runSystemCMD("chmod 700 /root/vmdrm", NULL);//zm add ,temporarily methord
    yos_systemcall_runSystemCMD("chmod 600 /root/vmdrm/*", NULL);//zm add ,temporarily methord
    yos_systemcall_runSystemCMD("chmod 700 /root/sqmpro", NULL);//zm add ,temporarily methord
    yos_systemcall_runSystemCMD("chmod 700 /root/CA_file", NULL);//zm add ,temporarily methord
#endif

    mainOpenUrl(input_url);
    return 0;
}

/* 线程方面的初始化，没有对优先级以及线程级别的等待做限制 */
static int CustomerStart(void)
{
#ifdef ENABLE_IGMPV3 //igmp v2 / v3
    mid_net_igmpver_set2proc("all", 0);
#else
    osex_igmp_version_set(NULL, "2");
#endif // ENABLE_IGMPV3

#if (defined(SQM_VERSION_C26) || defined(SQM_VERSION_C28)||defined(SQM_VERSION_ANDROID))
    sqm_file_check();
#endif

    a_HippoContext_Init(); // 启动浏览器之前,初始Hippo和msgproc. add by teddy 2011-1-8 13:13:34
#ifndef ANDROID
    mid_ntp_init();
#endif

#ifdef INCLUDE_DLNA
#ifndef HUBEI_HD
    mid_dlna_init();
#endif
#endif // INCLUDE_DLNA

#ifdef INCLUDE_LocalPlayer
    a_LocalPlayer_JseMapInit();
#endif // INCLUDE_LocalPlayer

#ifdef INCLUDE_IMS
    YX_IMS_module_init("nothing");
#endif // INCLUDE_IMS

    int changeVideoMode = 0;
    sysSettingGetInt("changevideomode", &changeVideoMode, 0);
    codec_changemode(changeVideoMode);

#if !defined(ANDROID)
    int hdVideoFormat = 0;
    int sdVideoFormat = 0;
    sysSettingGetInt("hd_video_format", &hdVideoFormat, 0);
    sysSettingGetInt("videoformat", &sdVideoFormat, 0);

    if(sdVideoFormat >= VideoFormat_720P50HZ
       && (hdVideoFormat == VideoFormat_PAL || hdVideoFormat == VideoFormat_NTSC)) { // 旧基线切换到新基线hd_video_format 默认为1，需要转换成可用的制式
        hdVideoFormat = VideoFormat_1080I50HZ;
        sysSettingSetInt("hd_video_format", hdVideoFormat);
        settingManagerSave();
    }
#if (SUPPORTE_HD == 1)
    mid_sys_videoFormat_set(hdVideoFormat, 0); // 由于设置图形层大小之后，再次设置制式可能导致从新建立图形层。修改为先设置制式
#else
    mid_sys_videoFormat_set(sdVideoFormat, 0); // 由于设置图形层大小之后，再次设置制式可能导致从新建立图形层。修改为先设置制式
#endif // SUPPORTE_HD == 1
#endif

    DeviceConfig();

    app_stream_init();


    TR069_API_INIT( );

#ifdef TVMS_OPEN
    tvms_config_init();
    tvms_config_load(0);
#endif // TVMS_OPEN

#if defined(PAY_SHELL)
    pay_shell_init();
#endif

    NativeHandlerSetState(0); // init boot state handler
#ifdef U_CONFIG
    UDiskConfigInit();
#endif // U_CONFIG

    set_local_time_zone();
    set_saving_time_sec();



#if defined(BLUETOOTH)
    BluetoothParamCheck();
#endif // BLUETOOTH
    return 0;
}


static void app_stream_init(void)
{
#ifdef INCLUDE_cPVR
    mid_stream_init(3); //此值为3表示可以同时录两路码流。
#else
#ifdef INCLUDE_PIP
    mid_stream_init(2);
#else
    mid_stream_init(1);
#endif
#endif

#if defined(Sichuan)
    unsigned int hardwareVer = 0;
    yhw_board_getHWVersion(&hardwareVer);
    if(hardwareVer >= 0x300){
        mid_stream_set_apple_buffersize(40*1024*1316);
    } else {
        mid_stream_set_apple_buffersize(12*1024*1316);
    }
#endif
    mid_stream_recv_safe(1);
    mid_stream_rrs_timeout(2000);
    mid_stream_voddelay(0);
    mid_stream_heartbit_standard(1); // 所有局点都使用GET_PARAMETER方式心跳
    mid_stream_timeshift_second(1); // 默认支持时移二次调度
    // mid_stream_endclose(1); // 播放到尾立即关闭播放
    int transportProtocol = 0;
	sysSettingGetInt("TransportProtocol", &transportProtocol, 0);
    mid_stream_transport(transportProtocol); // 默认封装改成 UDP
    // mid_stream_timeshift_type(TIMESHIFT_TYPE_MANUAL); // 本地时移都有手动启动
    mid_stream_cawait(2000);

#if (defined(INCLUDE_PVR) || defined(INCLUDE_DOWNLOAD))
    mid_record_mount( );
#endif

#ifdef INCLUDE_SQA
    mid_stream_set_arq(0);
    mid_stream_set_burst(0);
#endif
    return ;
}

