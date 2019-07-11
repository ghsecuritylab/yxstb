
#include "JseHWAdditional.h"
#include "JseFunctionCall.h"

//TODO
static int JseSwitchaAddOnSubtitle(const char *param, char *value, int len)
{
    return 0;
}

JseHWAdditionalSubtitle::JseHWAdditionalSubtitle()
    : JseGroupCall("additionalSubtitle")
{
    JseCall* call;

    call = new JseFunctionCall("switch", 0, JseSwitchaAddOnSubtitle);
    regist(call->name(), call);
}


JseHWAdditionalSubtitle::~JseHWAdditionalSubtitle()
{
}

//TODO
static int JseSwitchaAddOnLyric(const char *param, char *value, int len)
{
    return 0;
}

JseHWAdditionalLyric::JseHWAdditionalLyric()
    : JseGroupCall("additionallyric")
{
    JseCall* call;

    call = new JseFunctionCall("switch", 0, JseSwitchaAddOnLyric);
    regist(call->name(), call);
}

JseHWAdditionalLyric::~JseHWAdditionalLyric()
{
}
