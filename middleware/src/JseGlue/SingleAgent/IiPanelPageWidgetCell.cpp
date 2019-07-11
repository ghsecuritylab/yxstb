#include "JseGlueAssertions.h"
#include "IiPanelPageWidgetCell.h"
#include "Hippo_Context.h"

#include <stdio.h>

using Hippo::HippoContext;

IiPanelPageWidgetCell::IiPanelPageWidgetCell()
{
    LogJseGlueDebug("new\n");

    m_mainObj = HippoContext::getContextInstance( )->getBrowserActiveObj( );
    m_impl = m_mainObj->createWidget( );
}

IiPanelPageWidgetCell::~IiPanelPageWidgetCell()
{
    LogJseGlueDebug("delete\n");
}



void
IiPanelPageWidgetCell::close()
{
    if(m_impl == m_mainObj->getCurrentActiveObj( ))
        m_mainObj->releaseCurrentActiveObj();
}

void
IiPanelPageWidgetCell::focus()
{
    m_mainObj->setCurrentActiveObj(m_impl);
}

IiPanelPageWidget* createIiPanelPageWidget(const char* name)
{
    LogJseGlueDebug("name<%s>\n", name);
    return new IiPanelPageWidgetCell;
}


