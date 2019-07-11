
#include "JseHWStream.h"
#include "JseFunctionCall.h"

#include "mid_stream.h"

#include <stdio.h>

static int JseStreamingGetRateRead(const char *param, char *value, int len)
{
    snprintf(value, len, "%d", mid_stream_hls_playrate());
    return 0;
}

JseHWStream::JseHWStream()
    : JseGroupCall("Streaming")
{
    JseCall* call;

    call = new JseFunctionCall("GetRate", JseStreamingGetRateRead, 0);
    regist(call->name(), call);
}

JseHWStream::~JseHWStream()
{
}

