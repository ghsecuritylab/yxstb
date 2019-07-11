
#include  "rapidxml.hpp"
using namespace rapidxml;

#include <iostream>
#include <string>
#include <fstream>
using namespace std;

#include <stdio.h>
#include <stdlib.h>

int   main()
{
        xml_document<>  doc;
        int             len;
        char    *       buffer;
        FILE    *       fp;
        fp = fopen("gouji.xml", "rb");
        if(fp == NULL)
                return -1;
        fseek(fp, 0, SEEK_END);
        len = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        buffer = (char *)malloc(len + 1);
        fread(buffer, len, 1, fp);
        fclose(fp);

        doc.parse<0>((char *)buffer);
        cout << "Name of first node is: " << doc.first_node()->name() << endl;
        xml_node<> *node = doc.first_node()->first_node("game");
        for(xml_node<> * rule = node->first_node();
                rule; rule = rule->next_sibling())
        {
                for(xml_attribute<> * attr = rule->first_attribute();
                        attr; attr = attr->next_attribute()
                   )
                {
                        cout << "rule " << attr->name() << " = " << attr->value() << endl;
                }
        }
        return 0;
}

