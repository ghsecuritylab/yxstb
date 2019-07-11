
#include "JseHWSQATools.h"
#include "JseFunctionCall.h"

//TODO
static int JseQueryLevelRead(const char *param, char *value, int len)
{
    return 0;
}

//TODO
static int JseQueryLevelWrite(const char *param, char *value, int len)
{
    return 0;
}

JseHWSQATools::JseHWSQATools()
    : JseGroupCall("Tools")
{
    JseCall* call;

    call = new JseFunctionCall("QueryLevel", JseQueryLevelRead, JseQueryLevelWrite);
    regist(call->name(), call);
}

JseHWSQATools::~JseHWSQATools()
{
}
