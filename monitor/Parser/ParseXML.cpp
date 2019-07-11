#include "ParseXML.h"

ParseXML::ParseXML(std::string strFileName)
{
	m_strMT = "MTname";
	m_strSTB = "STBname";
	m_strFileName = strFileName;
	myDocument = new TiXmlDocument(); 
	InitParseXML();
}

ParseXML::~ParseXML()
{
	delete myDocument;
	myDocument = NULL;
}


std::string ParseXML::GetSTBPara(std::string strMT)
{
	//std::cout << m_MapPara[strMT];
	return m_MapPara[strMT];
}


void ParseXML::InitParseXML()
{
	int ret = 0;
	//加载XML文件
	ret = myDocument->LoadFile(m_strFileName.c_str());
	if (ret == false){
		printf("LoadXMLFile error!\n");
		return;
	}
	
	//找到根结点
	TiXmlElement* rootElement = myDocument->RootElement();
	if (rootElement == NULL)
		return;
	
	//找到第一个子结点
	TiXmlElement* Element = rootElement->FirstChildElement();
	if (Element == NULL)
		return;
	
	while (Element!= NULL)
	{
		//找到ParamMTSTBMap
		std::string temp_str("ParamMTSTBMap");
		
		//找到参数表
		if (Element->Value() != temp_str)
		{
			Element = Element->NextSiblingElement();
			continue;
		}
		
		//找ParamMTSTBMap下面的子结点
		Element = Element->FirstChildElement();
		
		while (Element != NULL)
		{
			TiXmlAttribute *IDAttribute = Element->FirstAttribute();
			
			//循环输出各结点属性
			while (IDAttribute != NULL)
			{
				if (m_strMT == IDAttribute->Name())
					m_paraMT = IDAttribute->Value();
				if (m_strSTB == IDAttribute->Name())
					m_paraSTB = IDAttribute->Value();
					
				IDAttribute = IDAttribute->Next();
			}
			//存储到MAP中
			m_MapPara.insert(std::pair<std::string, std::string>(m_paraMT, m_paraSTB));
			
			Element = Element->NextSiblingElement();
		}
		
	}
	
}

