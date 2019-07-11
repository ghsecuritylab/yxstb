
#include "JseHWMedia.h"
#include "JseFunctionCall.h"

#include "mid_stream.h"

#include <stdio.h>

static int JseMediaBufferRateRead(const char *param, char *value, int len)
{
    snprintf(value, len, "%d", mid_stream_hls_buffrate());
    return 0;
}

static int JseGetCurrentDownloadTimeRead(const char *param, char *value, int len)
{
    sprintf(value, "%d", mid_stream_hls_recordrate());
    return 0;
}

JseHWMedia::JseHWMedia()
    : JseGroupCall("Media")
{
    JseCall* call;

    call = new JseFunctionCall("BufferRate", JseMediaBufferRateRead, 0);
    regist(call->name(), call);

    //C10 regist
    call = new JseFunctionCall("getCurrentDownloadTime", JseGetCurrentDownloadTimeRead, 0);
    regist(call->name(), call);
}

JseHWMedia::~JseHWMedia()
{
}
