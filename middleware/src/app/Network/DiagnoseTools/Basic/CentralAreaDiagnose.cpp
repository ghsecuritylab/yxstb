#include "NetworkAssertions.h"
#include "CentralAreaDiagnose.h"

#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <errno.h>
#include <sys/select.h>
#include <curl/curl.h>

#include "oping.h"

static void _Callback(void *arg, int status, int timeouts, struct hostent *host);

typedef struct _DnsContext {
    int index;
    int result;
    char* hostname;
    char* hostaddr;
}DnsContext_t;

typedef struct _CallBackContext {
    int index;
    void* diagnose;
}CallBackContext_t;

typedef struct _PingContext {
    int send;
    int recv;
    double max;
}PingContext_t;

CentralAreaDiagnose::CentralAreaDiagnose(int mode) 
    : mDiagMode(mode) , mServers(0)
    , mMaxDelay(-0.1), mSuccessRate(0.0)
{
}

CentralAreaDiagnose::~CentralAreaDiagnose()
{
    struct ares_addr_node* s = 0;
    while (mServers) {
        s = mServers;
        mServers = mServers->next;
        free(s);
    }
    mServers = 0;

    while (mDiagState != eFinish) 
        usleep(100000);

    mTestUrl.clear();
    mResolvIP.clear();
    mResolvST.clear();
}

void
CentralAreaDiagnose::addDns(const char* dns)
{
    if (dns) {
        struct ares_addr_node *srvr = 0, *last = mServers;
        srvr = (struct ares_addr_node *)malloc(sizeof(struct ares_addr_node));
        if (srvr) {
            if (ares_inet_pton(AF_INET, dns, &srvr->addr.addr4) > 0)
                srvr->family = AF_INET;
            else if (ares_inet_pton(AF_INET6, dns, &srvr->addr.addr6) > 0)
                srvr->family = AF_INET6;
            else 
                free (srvr);

            srvr->next = 0;
            if (last) {
                while (last->next)
                    last = last->next;
                last->next = srvr;
            } else
                mServers = srvr;
        }
    }
}

void 
CentralAreaDiagnose::setUrl(const char* url)
{
    if (!url || !strlen(url))
        return;
    if (mTestUrl.size() < kHostMaxCount) {
        mTestUrl.push_back(url);
        mResolvIP.push_back("");
        mResolvST.push_back(RS_DNS_NON);
    }
}

int 
CentralAreaDiagnose::_DnsResolv(int family, char** hosts, int tCount)
{
    NETWORK_LOG_INFO("family:%d, tCount:%d\n", family, tCount);
    const int kTimeout = kDnsTimeout;
    int i = 0, ret = -1, status = -1, nfds = 0;
    fd_set rfds, wfds;
    struct timeval tv, maxtv;
    ares_channel channel;
    struct ares_options options;
    struct ares_addr_node *servers = 0;
    CallBackContext_t* context[kHostMaxCount];

    ret = ares_library_init(ARES_LIB_INIT_ALL);
    if (ret != ARES_SUCCESS)
        return eDNS_ERR;

    options.timeout = 500; //ms
    options.tries = 2;
    ret = ares_init_options(&channel, &options, ARES_OPT_TIMEOUTMS | ARES_OPT_TRIES);
    if (ret != ARES_SUCCESS) {
        ares_library_cleanup();
        return eDNS_ERR;
    }

    if (mServers) { //if null, use the file /etc/resolv.conf
        ret = ares_set_servers(channel, mServers);
        if (ret != ARES_SUCCESS) {
            status = eDNS_ERR;
            goto END;
        }
    }

    ret = ares_get_servers(channel, &servers);
    if (ret != ARES_SUCCESS || !servers) {
        status = eDNS_NOADDR;
        goto END;
    }
    ares_free_data(servers);

    for (i = 0; i < tCount; ++i) {
        context[i] = (CallBackContext_t*)malloc(sizeof(CallBackContext_t));
        if (!context[i])
            continue;
        context[i]->index = i;
        context[i]->diagnose = (void*)this;
        NETWORK_LOG_INFO("hosts[%d] = %s\n", i, hosts[i]);
        ares_gethostbyname(channel, hosts[i], family, _Callback, (void*)context[i]);
    }

    status = eDNS_TIMEOUT;
    for (i=0;;) {
        if (mDiagState != eRun)
            break;
        if (i > kTimeout)
            break;
        maxtv.tv_sec = kTimeout;
        maxtv.tv_usec = 0;
        ares_timeout(channel, &maxtv, &tv);
        i += tv.tv_sec;

        FD_ZERO(&rfds);
        FD_ZERO(&wfds);
        nfds = ares_fds(channel, &rfds, &wfds);
        if (nfds == 0)
            break;
        select(nfds, &rfds, &wfds, 0, &tv);
        ares_process(channel, &rfds, &wfds);
    }
    for (i = 0; i < tCount; ++i) {
        if (isResolvOK(i))
            status = eDNS_OK;
        if (context[i])
            free(context[i]);
        context[i] = 0;
    }

END:
    ares_library_cleanup();
    ares_destroy(channel);
    return mDiagState == eRun ? status : ePING_STOP;
}

int
CentralAreaDiagnose::_AdoptPing(int family)
{
    NETWORK_LOG_INFO("ping!\n");
    const int kTimeout = kPingTimeout; //unit: s
    int status = 0, i = 0;
    double latency = 0, rate = 0.0;
    pingobj_t *ping = 0;
    pingobj_iter_t *iter = 0;
    PingContext_t* context = 0;
    size_t bufflen = 0;
    char *hosts[kHostMaxCount] = {0};
    int tCount = mTestUrl.size();

    for (i = 0; i < tCount; ++i) {
        hosts[i] = (char*)calloc(1, kHostUrlLength);
        if (hosts[i])
            SplitUrl2Host(mTestUrl[i].c_str(), hosts[i], kHostUrlLength-1, 0);
    }

    status = _DnsResolv(family, hosts, tCount);
    if (eDNS_OK != status)
        goto END;

    ping = ping_construct();
    if (!ping) {
        status = ePING_ERR;
        goto END;
    }
    status = ePING_OK;

    for (i = 0; i < tCount; i++) {
        if (!isResolvOK(i))
            continue;
        ping_host_add(ping, mResolvIP[i].c_str()); 
    }

    for (iter = ping_iterator_get(ping); iter; iter = ping_iterator_next(iter)) {
        context = (PingContext_t*)malloc(sizeof(PingContext_t));
        if (context) {
            context->send = 0;
            context->recv = 0;
            context->max = -1.0;
            ping_iterator_set_context(iter, context);
        }
    }

    for (i=0;;++i) {
        if (mDiagState != eRun)
            break;
        if (i > kTimeout) 
            break;
        if (ping_send(ping) < 0)
            continue;
        for (iter = ping_iterator_get(ping); iter; iter = ping_iterator_next(iter)) {
            context = (PingContext_t*)ping_iterator_get_context(iter);
            if (!context)
                continue;
            latency = -1.0;
            bufflen = sizeof(latency);
            ping_iterator_get_info(iter, PING_INFO_LATENCY, &latency, &bufflen);
            context->send++;
            if (latency > 0.0) {
                context->recv++;
                if (context->max < 0.0 || context->max < latency)
                    context->max = latency;
            }
        }
        sleep(1);
    }

    for (iter = ping_iterator_get(ping); iter; iter = ping_iterator_next(iter)) {
        context = (PingContext_t*)ping_iterator_get_context(iter);
        if (!context)
            continue;

        if (context->recv > 0.0)
            rate = (100.0 * context->recv) / context->send;

        if (mSuccessRate < rate) {
            mMaxDelay = context->max;
            mSuccessRate = rate;
        }
        free(context);
    }
    ping_destroy (ping);

END:
    for (i = 0; i < tCount; ++i) {
        if (hosts[i])
            free(hosts[i]);
    }
    return mDiagState == eRun ? status : ePING_STOP;
}

int 
CentralAreaDiagnose::_AdoptHttp(int* httpcode)
{
    NETWORK_LOG_INFO("http!\n");
    const int kTimeout = kHttpTimeout;
    int i = 0, rnum = 0, maxfd = 0, ret = -1;
    int status = 0, pending = 0, errcode = -1;
    struct timeval tv;
    fd_set rfds, wfds, efds;
    CURLM *multi = 0;
    CURL *easy[kHostMaxCount] = {0};
    CURLMsg *msg = 0;
    char url[kHostUrlLength] = {0};
    int ports[kHostMaxCount] = {0};
    char tokens[kHostMaxCount][16] = {{0}};
    char *hosts[kHostMaxCount] = {0};
    char *paths[kHostMaxCount] = {0};
    int tCount = mTestUrl.size();

    for (i = 0; i < tCount; ++i) {
        if (std::string::npos != mTestUrl[i].find("https://"))
            strncpy(tokens[i], "https://", 8);
        hosts[i] = (char*)calloc(1, kHostUrlLength);
        if (hosts[i]) {
            paths[i] = SplitUrl2Host(mTestUrl[i].c_str(), hosts[i], kHostUrlLength-1, &ports[i]);
            NETWORK_LOG_INFO("hosts[%d] = %s\n", i, hosts[i]);
        }
    }

    status = _DnsResolv(AF_INET, hosts, tCount);
    if (status != eDNS_OK)
        goto END;

    multi = curl_multi_init();
    if (!multi) {
        status = eHTTP_UNKNOW;
        goto END;
    }

    for (i = 0; i < tCount; ++i) {
        if (!isResolvOK(i))
            continue;
        
        if (ports[i])
            snprintf(url, kHostUrlLength, "%s%s:%d%s", tokens[i], mResolvIP[i].c_str(), ports[i], paths[i] ? paths[i] : "");
        else 
            snprintf(url, kHostUrlLength, "%s%s%s", tokens[i], mResolvIP[i].c_str(), paths[i] ? paths[i] : "");

        NETWORK_LOG_INFO("url: %s\n", url);

        easy[i] = curl_easy_init();
        if (!easy[i]) 
            continue;
        curl_easy_setopt(easy[i], CURLOPT_URL, url);
        curl_easy_setopt(easy[i], CURLOPT_CONNECTTIMEOUT, kTimeout);
        //curl_easy_setopt(easy[i], CURLOPT_VERBOSE, 1L);
        curl_easy_setopt(easy[i], CURLOPT_NOBODY, 1L);
        curl_multi_add_handle(multi, easy[i]);
    }

    curl_multi_perform(multi, &rnum);
    while (rnum) {
        if (mDiagState != eRun)
            break;
        FD_ZERO(&rfds);
        FD_ZERO(&wfds);
        FD_ZERO(&efds);
        curl_multi_fdset(multi, &rfds, &wfds, &efds, &maxfd);
        tv.tv_sec = 1;
        tv.tv_usec = 0;
        ret = select(maxfd+1, &rfds, &wfds, &efds, &tv);
        if (-1 == ret) { //system call interrupt
            if (EINTR == errno)
                continue;
            break;
        }
        curl_multi_perform(multi, &rnum);
    }

    status = eHTTP_UNKNOW;
    msg = curl_multi_info_read(multi, &pending);
    while (msg) {
        if (CURLMSG_DONE == msg->msg) {
            switch (msg->data.result) {
            case CURLE_OK:
                curl_easy_getinfo(msg->easy_handle, CURLINFO_RESPONSE_CODE, &errcode);
                *httpcode = errcode;
                if (200 == errcode)
                    status = eHTTP_OK;
                else 
                    status = eHTTP_RESPONSE;
                break;
            case CURLE_COULDNT_CONNECT:
                curl_easy_getinfo(msg->easy_handle, CURLINFO_HTTP_CONNECTCODE, &errcode);
                *httpcode = errcode;
                //TODO
                status = eHTTP_REJECT;
                break;
            case CURLE_OPERATION_TIMEDOUT:
                status = eHTTP_TIMEOUT;
                break;
            default:
                curl_easy_getinfo(msg->easy_handle, CURLINFO_HTTP_CONNECTCODE, &errcode);
                if (!errcode)
                    curl_easy_getinfo(msg->easy_handle, CURLINFO_RESPONSE_CODE, &errcode);
                *httpcode = errcode;
                status = eHTTP_UNKNOW;
            }
        }
        if (eHTTP_OK == status)
            break;
        msg = curl_multi_info_read(multi, &pending);
    }

    for (i = 0; i < tCount; ++i) {
        if (!isResolvOK(i))
            continue;
        if (easy[i]) {
            curl_multi_remove_handle(multi, easy[i]);
            curl_easy_cleanup(easy[i]);
        }
    }
    curl_multi_cleanup(multi);

END:
    for (i = 0; i < tCount; ++i) {
        if (hosts[i])
            free(hosts[i]);
    }
    return mDiagState == eRun ? status : eHTTP_STOP;
}

int 
CentralAreaDiagnose::start(NetworkDiagnose* netdiag)
{
    NETWORK_LOG_INFO("diagnose:%s\n", netdiag->getTestType());

    mDiagState = eRun;

    if (mTestUrl.size() <= 0) {
        netdiag->setTestState(ND_STATE_FAIL);
        netdiag->setTestResult(ND_RESULT_FAIL);
        mDiagState = eFinish;
        return 0;
    }

    int status, httpcode = 0;

    if (ND_MODE_ICMP == mDiagMode)
        status = _AdoptPing(AF_INET);
    else
        status = _AdoptHttp(&httpcode);

    NETWORK_LOG_INFO("ret = %d\n", status);
    switch (status) {
    case eDNS_ERR:
    case eDNS_NOADDR:
        netdiag->setTestState(ND_STATE_FAIL);
        netdiag->setTestResult(ND_RESULT_FAIL);
        netdiag->setErrorCode(108004);
        break;
    case eDNS_TIMEOUT:
        netdiag->setTestState(ND_STATE_FINISH);
        netdiag->setTestResult(ND_RESULT_FAIL);
        netdiag->setErrorCode(108002);
        break;
    case ePING_ERR:
        netdiag->setTestState(ND_STATE_FAIL);
        netdiag->setTestResult(ND_RESULT_FAIL);
        netdiag->setErrorCode(102040);
        break;
    case eHTTP_TIMEOUT:
        netdiag->setTestState(ND_STATE_FINISH);
        netdiag->setTestResult(ND_RESULT_FAIL);
        netdiag->setErrorCode(102047);
        break;
    case eHTTP_REJECT:
        netdiag->setTestState(ND_STATE_FINISH);
        netdiag->setTestResult(ND_RESULT_FAIL);
        netdiag->setErrorCode(102048);
        break;
    case eHTTP_RESPONSE:
        netdiag->setTestState(ND_STATE_FINISH);
        netdiag->setTestResult(ND_RESULT_FAIL);
        netdiag->setErrorCode(102049);
        break;
    case eHTTP_UNKNOW:
        netdiag->setTestState(ND_STATE_FAIL);
        netdiag->setTestResult(ND_RESULT_FAIL);
        netdiag->setErrorCode(102050);
        break;
    case ePING_OK:
        netdiag->setTestState(ND_STATE_FINISH);
        if (100.0 == mSuccessRate) {
            if (mMaxDelay < 800.0)
                netdiag->setTestResult(ND_RESULT_SUCCESS);
            else {
                netdiag->setTestResult(ND_RESULT_FAIL);
                netdiag->setErrorCode(102038);
            }
        } else {
            netdiag->setTestResult(ND_RESULT_FAIL);
            if (0.0 == mSuccessRate)
                netdiag->setErrorCode(102040);
            else
                netdiag->setErrorCode(102039);
        }
        break;
    case eHTTP_OK:
        netdiag->setTestState(ND_STATE_FINISH);
        netdiag->setTestResult(ND_RESULT_SUCCESS);
        break;
    case eDNS_STOP:
    case ePING_STOP:
    case eHTTP_STOP:
    default:
        netdiag->setTestState(ND_STATE_FAIL);
        netdiag->setTestResult(ND_RESULT_FAIL);
        break;
    }

    mDiagState = eFinish;
    return 0;
}

int 
CentralAreaDiagnose::stop()
{
    NETWORK_LOG_INFO("diagnose:%d\n", type());
    if (mDiagState == eRun)
        mDiagState = eStop;
    return 0;
}

int 
CentralAreaDiagnose::doResult(int result, int arg1, void* arg2)
{
    NETWORK_LOG_INFO("Resolve[%d]: %s\n", arg1, (char*)arg2);
    if (result == ARES_SUCCESS) {
        mResolvIP[arg1] = (char*)arg2;
        mResolvST[arg1] = RS_DNS_OK;
    } else
        mResolvST[arg1] = RS_DNS_ERR;
    return 0;
}

static void _Callback(void *arg, int status, int timeouts, struct hostent *host)
{
    if (!arg)
        return;

    char **list;
    char addr[46] = { 0 };
    CallBackContext_t* context = (CallBackContext_t*)arg;
    CentralAreaDiagnose* diagnose = (CentralAreaDiagnose*)context->diagnose;

    if (status == ARES_SUCCESS) {
        for (list = host->h_addr_list; *list; list++) {
            if (ares_inet_ntop(host->h_addrtype, *list, addr, sizeof(addr)))
                break;
        }
    }
    diagnose->doResult(status, context->index, addr); 
}
