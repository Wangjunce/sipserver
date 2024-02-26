#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libwebsockets.h>
#include <algorithm>
#include <string>
#include <regex>
#include <vector>
#include <map>
#include <iostream>
#include <exception>
#include <ctime>
#include <mutex>
#include <thread>
#include <mysql/mysql.h>

#ifdef WIN32
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <windows.h>
#include <tchar.h>
#include <strsafe.h>
#pragma comment(lib, "ws2_32.lib")
#else
#endif

#include "SipServer.h"
#include "Log.h"
#include "func.h"
#include "myconfig.h"
#include "mylog.h"

/**
 * 待解决：感觉还需要做一个关闭对讲后延迟一会才能开始第二次对讲，不然request_message函数中的eXosip_message_send_request会报错 而且是致命错误
*/

//本机IP
std::string currentIP;

//注册协议,一种协议，对应一套处理方案（类似驱动中的设备树）
struct lws_protocols protocols[] = {
    {
        "http",
        callback_http,
        0,
        0,
    },
    {
        "websocket",
        callback_websocket,
        0,
        0,
    },
    { NULL, NULL, 0, 0 } /* end of list */
};



void funcWebsocket()
{
    MyLog* mylog = MyLog::getInstance();
    MyConfig* myconfig = MyConfig::getInstance();

    //服务器信息结构体，服务器句柄需要
    struct lws_context_creation_info info;
    //服务器句柄
    struct lws_context *context;

    std::map<std::string, std::map<std::string, std::string>> configInfo = myconfig->getConfigInfo();
    int port = atoi(configInfo["http"]["websocketPORT"].c_str());
 
    memset(&info, 0, sizeof(info));
    info.port = port;           //端口
    info.protocols = protocols; //支持的协议类型和相应的回调
 
    //创建服务器句柄
    context = lws_create_context(&info);
 
    if (!context) {
        mylog->root.error("Error creating WebSocket context");
        return;
    }
 
    mylog->root.info("WebSocket server started on port %d", port);

    while (1) {
        //运行服务器，第二个参数是超时时间
        lws_service(context, 50);
    }
 
    //销毁上下文
    lws_context_destroy(context);
}


int main(int argc, char* argv[]) {

    //读一次文件就够了
    MyConfig* myconfig = MyConfig::getInstance();
    myconfig->parseFile();
    auto configInfo = myconfig->getConfigInfo();

    std::thread t(funcWebsocket);
    t.detach();
    
    //本机IP
    if(std::stoi(configInfo["other"]["ipType"]) == 1){
        currentIP = get_local_ip_using_ifconf();
    }else{
        std::string cmd = "curl -s ifconfig.cc";
        int ret = 0;
        ret  = _System(cmd, currentIP);
        if(ret != 0){
            MyLog* mylog = MyLog::getInstance();
            mylog->root.info("current IP error");
            return 1;
        }
    }
    
    //初始化sip服务器相关信息
    ServerInfo info(
        configInfo["sip"]["sipuseragent"].c_str(),
        configInfo["sip"]["siprand"].c_str(),                   //SIP服务随机数值
        currentIP.c_str(),                                      //SIP服务IP
        std::stoi(configInfo["sip"]["sipport"]),                //SIP服务端口
        currentIP.c_str(),                                      //媒体服务器ip
        std::stoi(configInfo["media"]["mediaRTPPortDefault"]),  //媒体服务器默认RTP的port
        std::stoi(configInfo["media"]["mediastartRTPPORT"]),    //媒体服务器申请的初始RTP端口，逐个+2
        std::stoi(configInfo["media"]["mediaHTTPPORT"]),        //媒体服务器的http端口
        configInfo["sip"]["sipID"].c_str(),                     //SIP服务器ID
        configInfo["sip"]["sipreal"].c_str(),                   //SIP服务器域
        configInfo["sip"]["sippassword"].c_str(),               //SIP服务器password
        std::stoi(configInfo["sip"]["siptimeout"]),             //SIP timeout
        std::stoi(configInfo["sip"]["siptimeout2"]),            // SIP到期
        configInfo["media"]["mediaSERCRET"].c_str()             // 媒体服务器http的密钥
        );        
    SipServer* sipServer = SipServer::getInstance();
    sipServer->initial(&info);
    sipServer->loop();


    return 0;
}

