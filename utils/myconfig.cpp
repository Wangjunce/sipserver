#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "myconfig.h"

using std::cout;
using std::endl;

MyConfig* MyConfig::instance = nullptr;
MyConfig::MyConfig()
{

}

//log4cpp的东西会自动处理
MyConfig::~MyConfig()
{
    //chatgpt说不需要删除
    delete MyConfig::instance;
}
MyConfig* MyConfig::getInstance()
{
    if(MyConfig::instance == nullptr){
        MyConfig::instance = new MyConfig();
    }
    return MyConfig::instance;
}

bool MyConfig::parseFile()
{
    configInfo.clear();
    const int lize_size = 256;
    char c[lize_size];
    //以二进制模式打开 in.txt 文件
    std::ifstream inFile("../conf/config.ini", std::ios::in);
    //判断文件是否正常打开
    if (!inFile) {
        cout << "error" << endl;
        return false;
    }
    //连续以行为单位，读取 in.txt 文件中的数据
    char config_type[lize_size] = {0}, conf_first[lize_size] = {0}, conf_second[lize_size] = {0};
    std::map<std::string, std::string> config_value_info{};
    int hangLength = 0, i = 0;
    while (inFile.getline(c, lize_size)) {
        if(c[0] == '\0' || c[0] == ' ') continue;
        hangLength = strlen(c);
        if(c[0] == '['){
            //push进一组config
            if(strlen(config_type) > 0){
                configInfo.emplace(config_type, config_value_info);
                memset(config_type, '\0', sizeof(config_type));
                config_value_info.clear();
            }

            memcpy(config_type, c+1, hangLength-2);
            //cout << "config_type:" << config_type << endl;
        }else{
            if(strlen(config_type) == 0 || c[0] == ' ' || c[0] == '#'){
                continue;
            }

            while(c[i] != '\0'){
                if(c[i] == '=') break;
                ++i;
            }
            memset(conf_first, '\0', sizeof(conf_first));
            memcpy(conf_first, c, i);
            memset(conf_second, '\0', sizeof(conf_second));
            memcpy(conf_second, c+i+1, hangLength-i-1);
            i = 0;

            config_value_info.emplace(conf_first, conf_second);          
        }
    }

    //最后一组：push进一组config
    if(strlen(config_type) > 0){
        configInfo.emplace(config_type, config_value_info);
        memset(config_type, '\0', sizeof(config_type));
        config_value_info.clear();
    }

    inFile.close();
    return true;
}

bool MyConfig::parseArg(int argc, char*argv[])
{
    argInfo.clear();
    int opt;
    const char *str = "a:b:c:d:e:f:g:h:i:j:k:l:m:n:o:p:q:r:s:t:u:v:w:x:y:z:";
    while ((opt = getopt(argc, argv, str)) != -1)
    {
        argInfo.emplace(opt, optarg);   
        // switch (opt)
        // {
        //     case 'p':
        //     {
        //         PORT = atoi(optarg);
        //         break;
        //     }
        //     case 'l':
        //     {
        //         LOGWrite = atoi(optarg);
        //         break;
        //     }
        //     case 'm':
        //     {
        //         TRIGMode = atoi(optarg);
        //         break;
        //     }
        //     case 'o':
        //     {
        //         OPT_LINGER = atoi(optarg);
        //         break;
        //     }
        //     case 's':
        //     {
        //         sql_num = atoi(optarg);
        //         break;
        //     }
        //     case 't':
        //     {
        //         thread_num = atoi(optarg);
        //         break;
        //     }
        //     case 'c':
        //     {
        //         close_log = atoi(optarg);
        //         break;
        //     }
        //     case 'a':
        //     {
        //         actor_model = atoi(optarg);
        //         break;
        //     }
        //     default:
        //         break;
        // }
    }
    return true;
}

std::map<std::string, std::map<std::string, std::string>> MyConfig::getConfigInfo()
{
    return MyConfig::instance->configInfo;
}

std::map<std::string, std::string> MyConfig::getConfigMap(std::string key)
{
    return MyConfig::instance->configInfo[key];
}

std::string MyConfig::getArgString(char key)
{
    return MyConfig::instance->argInfo[key];
}