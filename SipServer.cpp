#pragma once

#ifdef WIN32
#include "SipServer.h"
#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include "./SipServer.h"
#include <arpa/inet.h>
#endif 

#include <cstring>
#include <algorithm>
#include <thread>
#include "utils/Log.h"
#include "utils/func.h"
#include "myconfig.h"
#include "mylog.h"

extern "C" {
#include "utils/HTTPDigest.h"
}

//UAS赋值的认证随机数
#define NONCE "9bd055"
//UAS默认加密算法
#define ALGORITHTHM "MD5"

extern string currentIP;

SipServer* SipServer::instance = nullptr;
SipServer* SipServer::getInstance()
{
    if(SipServer::instance == nullptr){
        SipServer::instance = new SipServer();
    }
    return SipServer::instance;
}

SipServer::SipServer() 
{
}

//构造初始化
void SipServer::initial(ServerInfo* info)
{
    mQuit = false;
    mSipCtx = nullptr;
    mInfo = info;
    LOGI("%s:%d", mInfo->getIp(), mInfo->getPort());
    #ifdef WIN32
        WSADATA wsaData;
        //异步套接字启动
        if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        {
            LOGE("WSAStartup Error");
            return;
        }
    #endif // WIN32
    MyConfig* myconfig = MyConfig::getInstance();
    auto configInfo = myconfig->getConfigInfo();
    ClientDeviceMap = configInfo["device-ssrc"];


    //本机IP
    if(std::stoi(configInfo["other"]["ipType"]) == 1){
        mCurrentIP = get_local_ip_using_ifconf();
    }else{
        std::string cmd = "curl -s ifconfig.cc";
        int ret = 0;
        ret  = _System(cmd, mCurrentIP);
        if(ret != 0){
            MyLog* mylog = MyLog::getInstance();
            mylog->root.info("current IP error");
            return;
        }
    }
}

SipServer::~SipServer() {
    LOGI("析构");
    //客户端的指针释放
    this->clearClientMap();
    #ifdef WIN32
    //终止套接字
    WSACleanup();
    #endif // WIN32
    delete SipServer::instance;
}

int SipServer::sip_event_handle(eXosip_event_t* evtp) {

    switch (evtp->type) {

    case EXOSIP_CALL_ACK://12:这个是语音对讲摄像头返回的消息，不需要返回
        LOGI("EXOSIP_CALL_ACK type=%d", evtp->type);
        this->recive_invite_ack(evtp);
        break;
    case EXOSIP_CALL_MESSAGE_NEW://14:announce new incoming request.
        LOGI("EXOSIP_CALL_MESSAGE_NEW type=%d", evtp->type);
        this->dump_request(evtp);
        this->dump_response(evtp);
        break;

    case EXOSIP_CALL_MESSAGE_ANSWERED://16:主动关闭对讲后会收到
        LOGI("EXOSIP_CALL_MESSAGE_ANSWERED type=%d", evtp->type);
        this->dump_request(evtp);
        //this->dump_response(evtp);
        break;

    case EXOSIP_CALL_CLOSED://21:a BYE was received for this call
        LOGI("EXOSIP_CALL_CLOSED type=%d", evtp->type);
        this->dump_request(evtp);
        this->dump_response(evtp);
        break;

    case EXOSIP_CALL_RELEASED://22:call context is cleared.这个在关闭推流后会触发
        LOGI("EXOSIP_CALL_RELEASED type=%d", evtp->type);
        this->dump_request(evtp);
        // this->dump_response(evtp);

        this->clearClientMap();
        break;
    case EXOSIP_MESSAGE_NEW://23:announce new incoming request.这和上面的一样啊，没法，文档上就这么写的,注册第一条信息走的这个
        LOGI("EXOSIP_MESSAGE_NEW type=%d", evtp->type);

        //Test if the message is a REGISTER REQUEST：注册消息
        if (MSG_IS_REGISTER(evtp->request)) {
            this->response_register(evtp);
        }
        //Test if the message is a MESSAGE REQUEST：其他消息 Catalog 和 Keepalive
        else if (MSG_IS_MESSAGE(evtp->request)) {
            this->response_message(evtp);
        }
        else if (strncmp(evtp->request->sip_method, "BYE", 3) != 0) {
            LOGE("unknown1");
        }
        else {
            LOGE("unknown2");
        }
        break;
    case EXOSIP_MESSAGE_ANSWERED://announce a 200ok
        this->dump_request(evtp);
        break;
    case EXOSIP_MESSAGE_REQUESTFAILURE://announce a failure.
        LOGI("EXOSIP_MESSAGE_REQUESTFAILURE type=%d: Receive feedback on sending failure after actively sending a message", evtp->type);
        this->dump_request(evtp);
        this->dump_response(evtp);
        break;
    case EXOSIP_CALL_INVITE://announce a new call 摄像头发来邀请
        LOGI("EXOSIP_CALL_INVITE type=%d: The server receives the Invite request actively sent by the client", evtp->type);
        this->response_invite_client(evtp);
        break;
    case EXOSIP_CALL_PROCEEDING://5:announce processing by a remote app
        LOGI("EXOSIP_CALL_PROCEEDING type=%d: When the server receives the Invite (SDP) confirmation reply from the client", evtp->type);
        this->dump_request(evtp);
        this->dump_response(evtp);
        break;
    case EXOSIP_CALL_ANSWERED:// 7:announce start of call
        LOGI("EXOSIP_CALL_ANSWERED type=%d: The server receives an invite (SDP) confirmation reply from the client", evtp->type);
        this->dump_request(evtp);
        this->dump_response(evtp);

        this->response_invite_ack(evtp);
        break;
    case EXOSIP_CALL_SERVERFAILURE://announce a server failure
        LOGI("EXOSIP_CALL_SERVERFAILURE type=%d", evtp->type);
        break;
    case EXOSIP_IN_SUBSCRIPTION_NEW://announce new incoming SUBSCRIBE/REFER
        LOGI("EXOSIP_IN_SUBSCRIPTION_NEW type=%d", evtp->type);
        break;
    default:
        LOGI("type=%d unknown", evtp->type);
        break;
    }

    return 0;
}

int SipServer::init_sip_server() {

    //这几步是初始化sipctx的结构体和开始监听
    mSipCtx = eXosip_malloc();
    if (!mSipCtx) {
        LOGE("eXosip_malloc error");
        return -1;
    }
    if (eXosip_init(mSipCtx)) {
        LOGE("eXosip_init error");
        return -1;
    }
    if (eXosip_listen_addr(mSipCtx, IPPROTO_UDP, nullptr, mInfo->getPort(), AF_INET, 0)) {
        LOGE("eXosip_listen_addr error");
        return -1;
    }

    //这两步在之前c++代码中没找到
    //Set the SIP User-Agent: header string.
    eXosip_set_user_agent(mSipCtx, mInfo->getUa());
    //Add authentication credentials. These are used when an outgoing request comes back with an authorization required response.
    if (eXosip_add_authentication_info(mSipCtx, mInfo->getSipId(), mInfo->getSipId(), mInfo->getSipPass(), NULL, mInfo->getSipRealm())) {
        LOGE("eXosip_add_authentication_info error");
        return -1;
    }

    return 0;
}

void SipServer::loop() {

    if (this->init_sip_server() != 0) {
        return;
    }
    while (!mQuit) {
        //Wait for an eXosip event.
        eXosip_event_t* evtp = eXosip_event_wait(mSipCtx, 0, 20);

        //这里的意思是如果没等到消息 就调用一次自动处理方法，如果等到了就调用自己写的方法
        if (!evtp) {
            /*  Retry with credentials upon reception of 401 / 407. Retry with higher Session -
                Expires upon reception of 422. Refresh REGISTER and SUBSCRIBE / REFER
                before the expiration delay.Retry with Contact header upon reception of 3xx request.
                Send automatic UPDATE for session - timer feature.
            */
            //401/407自动处理
            eXosip_automatic_action(mSipCtx);
            osip_usleep(100000);
            continue;
        }
        eXosip_automatic_action(mSipCtx);
        this->sip_event_handle(evtp);
        eXosip_event_free(evtp);
    }
}

void SipServer::response_message_answer(eXosip_event_t* evtp, int code, std::string type) {

    int returnCode = 0;
    osip_message_t* pRegister = nullptr;
    returnCode = eXosip_message_build_answer(mSipCtx, evtp->tid, code, &pRegister);

    bool bRegister = false;
    if (pRegister) {
        bRegister = true;
    }
    if (returnCode == 0 && bRegister)
    {
        eXosip_lock(mSipCtx);
        eXosip_message_send_answer(mSipCtx, evtp->tid, code, pRegister);
        eXosip_unlock(mSipCtx);
    }
    else {
        LOGE("code=%d,returnCode=%d,bRegister=%d", code, returnCode, bRegister);
    }

}

//注册返回
void SipServer::response_register(eXosip_event_t* evtp) {

    int expires = 0;
    osip_authorization_t* auth = nullptr;
    //Structure for SIP Message (REQUEST and RESPONSE).
    osip_message_get_authorization(evtp->request, 0, &auth);

    //获取expires来判断是注册还是注销
    osip_header_t* header = NULL;
    osip_message_header_get_byname(evtp->request, "expires", 0, &header);
    if (NULL != header && NULL != header->hvalue) {
        expires = atoi(header->hvalue);
    }

    //第二次注册发200 这是第二次进来或者是注销进来
    if ( (auth && auth->username) || !expires) {

        char* method = NULL, // REGISTER
            * algorithm = NULL, // MD5
            * username = NULL,// 340200000013200000024
            * realm = NULL, // sip服务器传给客户端，客户端携带并提交上来的sip服务域
            * nonce = NULL, //sip服务器传给客户端，客户端携带并提交上来的nonce
            * nonce_count = NULL,
            * uri = NULL; // sip:34020000002000000001@3402000000

        osip_contact_t* contact = nullptr;
        //Get one Contact header.
        osip_message_get_contact(evtp->request, 0, &contact);

        method = evtp->request->sip_method;
        char calc_response[HASHHEXLEN];
        HASHHEX HA1, HA2 = "", Response;

        username = evtp->request->from->url->username;
        Client* currentClient = this->getClientByDevice(username);
        //非0是注册，0是注销
        if (expires) {
            LOGI("%s 注册", username);

            //提取内容
    #define SIP_STRDUP(field) if (auth->field) (field) = osip_strdup_without_quote(auth->field)
            SIP_STRDUP(algorithm);
            SIP_STRDUP(realm);
            SIP_STRDUP(nonce);
            SIP_STRDUP(nonce_count);
            SIP_STRDUP(uri);

            //已注册过 清除这个客户端
            int hasRegister = 0;
            if (currentClient) {
                hasRegister = 1;
            }

            //这里是平台鉴权:具体为什么这么干 我也不知道
            DigestCalcHA1(algorithm, username, realm, mInfo->getSipPass(), nonce, nonce_count, HA1);
            DigestCalcResponse(HA1, nonce, nonce_count, auth->cnonce, auth->message_qop, 0, method, uri, HA2, Response);

            HASHHEX temp_HA1;
            HASHHEX temp_response;
            DigestCalcHA1("REGISTER", username, mInfo->getSipRealm(), mInfo->getSipPass(), mInfo->getNonce(), NULL, temp_HA1);
            DigestCalcResponse(temp_HA1, mInfo->getNonce(), NULL, NULL, NULL, 0, method, uri, NULL, temp_response);
            memcpy(calc_response, temp_response, HASHHEXLEN);

            //重复注册不发invite
            if (hasRegister == 1) {
                if (!memcmp(calc_response, Response, HASHHEXLEN)) {
                    this->response_message_answer(evtp, 200, "register");
                    LOGI("Camera second registration succee,ip=%s,port=%d,device=%s", currentClient->getIp(), currentClient->getPort(), currentClient->getDevice());
                }
                else {
                    this->response_message_answer(evtp, 401);
                    LOGI("Camera second registration error, p=%s,port=%d,device=%s", currentClient->getIp(), currentClient->getPort(), currentClient->getDevice());
                }
            }
            else {
                //新注册
                Client* client = new Client(strdup(contact->url->host), atoi(contact->url->port), strdup(username));
                LOGI("register contact:ip=%s,port=%s", contact->url->host, contact->url->port);

                if (!memcmp(calc_response, Response, HASHHEXLEN)) {
                    this->response_message_answer(evtp, 200, "register");
                    LOGI("Camera registration success,ip=%s,port=%d,device=%s", client->getIp(), client->getPort(), client->getDevice());

                    //设置注册时的via
                    osip_via_t* temp_osip_via_t = nullptr;
                    osip_message_get_via(evtp->request, 0, &temp_osip_via_t);
                    osip_via_clone(temp_osip_via_t, &client->m_osip_via_t);
                    // osip_via_set_received(m_osip_via_t, const_cast<char*>("110.42.208.205"));
                    osip_via_to_str(client->m_osip_via_t, &m_via_char);
                    LOGI("register-m_via_char:%s",m_via_char);
                    mClientMap.insert(std::make_pair(client->getDevice(), client));

                    //发送邀请
                    this->request_invite(ClientDeviceMap[client->getDevice()], client->getIp(), client->getPort());

                }
                else {
                    this->response_message_answer(evtp, 401);
                    LOGI("Camera registration error, p=%s,port=%d,device=%s", client->getIp(), client->getPort(), client->getDevice());

                    delete client;
                }
            }

            __QUIT:

                osip_free(algorithm);
                //osip_free(username);
                osip_free(realm);
                osip_free(nonce);
                osip_free(nonce_count);
                osip_free(uri);
        }
        else {
            //注销需要释放这个客户端
            LOGI("%s 注销", username);
            this->response_message_answer(evtp, 200);
            //清除这个客户端  删除注册列表
            if (currentClient) {
                const char* currentDeviceId = currentClient->getDevice();
                delete currentClient;
                currentClient = nullptr;
                mClientMap.erase(currentDeviceId);
                LOGI("清除客户端完毕");
            }
        }
    }
    else {
        //这是第一次进来
        response_register_401unauthorized2(evtp);
        //response_register_401unauthorized(evtp);
    }

}

void SipServer::response_register_401unauthorized2(eXosip_event_t* evtp) {

    int iReqId = evtp->tid;

    std::stringstream stream;
    string method = evtp->request->sip_method;


    string from_addrCod = evtp->request->from->url->username;
    //string from_addrI = evtp->request->from->url->host;
    //string from_addrPor = evtp->request->from->url->port;
    string from_addrI;
    string from_addrPor;

    string to_addrCod = evtp->request->to->url->username;
    //string to_addrI = evtp->request->to->url->host;
    //string to_addrPor = evtp->request->to->url->port;
    string to_addrI = currentIP;
    MyConfig* myconfig = MyConfig::getInstance();
    auto configInfo = myconfig->getConfigInfo();
    string to_addrPor = configInfo["sip"]["sipport"];

    //获取expires
    int expires = 0;
    osip_header_t* header = NULL;
    {
        osip_message_header_get_byname(evtp->request, "expires", 0, &header);
        if (NULL != header && NULL != header->hvalue)
        {
            expires = atoi(header->hvalue);
        }
    }
    //contact字段
    osip_contact_t* contact = NULL;
    osip_message_get_contact(evtp->request, 0, &contact);
    string expires_addrCod;
    string expires_addrI;
    string expires_addrPor;
    if (NULL != contact)
    {
        expires_addrCod = contact->url->username;
        expires_addrI = contact->url->host;
        expires_addrPor = contact->url->port;
        from_addrI = expires_addrI;
        from_addrPor = expires_addrPor;
    }
    //注册返回 由发送方维护的请求ID 接收方接收后原样返回即可
    //CALL_ID
    string callId;
    {
        stream.str("");
        stream << evtp->request->call_id->number;
        callId = stream.str();
    }
    //解析content消息
    osip_body_t* body = NULL;
    osip_message_get_body(evtp->request, 0, &body);
    string content;
    if (body != NULL)
    {
        stream.str("");
        stream << body->body;
        content = stream.str();
    }
    //鉴权信息
    osip_authorization_t* authentication = NULL;
    bool isAuthNull = false;
    string userName;
    string algorithm;
    string digestRealm;
    string nonce;
    string response;
    string uri;
    {
        osip_message_get_authorization(evtp->request, 0, &authentication);
        if (NULL == authentication)
        {
            isAuthNull = true;
        }
        else
        {
            isAuthNull = false;
            stream.str("");
            stream << authentication->username;
            userName = stream.str();
            stream.str("");
            stream << authentication->algorithm;
            algorithm = stream.str();
            stream.str("");
            stream << authentication->realm;
            digestRealm = stream.str();
            stream.str("");
            stream << authentication->nonce;
            nonce = stream.str();
            stream.str("");
            stream << authentication->response;
            response = stream.str();
            stream.str("");
            stream << authentication->uri;
            uri = stream.str();
        }
    }
    authentication = NULL;


    //打印报文
    cout << "接收到报文：" << endl;
    cout << "=============================================="
        "==================" << endl;

    cout << "method:" << method << endl;

    stream.str("");
    stream << "<sip: " << from_addrCod << "@" << from_addrI << ":" << from_addrPor << ">";
    cout << "from:    " << stream.str() << endl;

    stream.str("");
    stream << "<sip: " << to_addrCod << "@" << to_addrI << ":" << to_addrPor << ">";
    cout << "to:" << stream.str() << endl;

    stream.str("");
    if (expires) {
        stream << "<sip: " << expires_addrCod << "@" << expires_addrI << ":" << expires_addrPor << ">;" << "expires=" << expires;
    }
    else {
        stream << "<sip: " << expires_addrCod << "@" << expires_addrI << ":" << expires_addrPor << ">";
    }
    
    cout << "contact:" << stream.str() << endl;

    //注册返回 由发送方维护的请求ID 接收方接收后原样返回即可
    cout << "sipRequestId:" << iReqId << endl;
    //CALL_ID
    cout << "CallId:" << callId << endl;
    //解析content消息
    cout << "body:" << content << endl;
    //获取expires
    cout << "expires:" << expires << endl;
    //鉴权信息
    if (isAuthNull)
    {
        cout << "当前报文未提供鉴权信息!!!" << endl;
    }
    else
    {
        cout << "当前报文鉴权信息如下:" << endl;
        cout << "username:" << userName << endl;
        cout << "algorithm:" << algorithm << endl;
        cout << "Realm:" << digestRealm << endl;
        cout << "nonce:" << nonce << endl;
        cout << "response:" << response << endl;
        cout << "uri:" << uri << endl;
    }
    cout << "=================================================="
        "==============" << endl;


    //发送
    osip_message_t* answer = NULL;
    int iStatus;
    if (isAuthNull)
    {
        iStatus = 401;
    }
    else
    {
        iStatus = 200;
    }
    eXosip_lock(mSipCtx);
    {
        int result = ::eXosip_message_build_answer(mSipCtx, iReqId,iStatus, &answer);
        if (iStatus == 401)
        {
            //由SIP库生成认证方法和认证参数发送客户端
            std::stringstream stream;
            string nonce = NONCE;
            string algorithm = ALGORITHTHM;
            stream << "Digest realm=\"" << mInfo->getSipRealm()
                << "\",nonce=\"" << mInfo->getNonce()
                << "\",algorithm=" << algorithm;

            osip_message_set_header(answer, "WWW-Authenticate",
                stream.str().c_str());
            cout << "======================================================="
                "=========" << endl;
            cout << "发送401报文" << endl;
            cout << "========================================================"
                "========" << endl;
        }
        else if (iStatus == 200)
        {
            stream.str("");
            stream << "<sip: " << expires_addrCod << "@" << expires_addrI << ":" << expires_addrPor << ">;" << "expires=" << expires;

            osip_message_set_header(answer, "Contact", stream.str().c_str());
            cout << "========================================================="
                "=======" << endl;
            cout << "发送200报文" << endl;
            cout << "=========================================================="
                "======" << endl;
            //string_t b = "<sip: 100110000101000000@192.168.31.18:5061>;expires=600";
            //osip_message_set_header(answer, "Contact", b.c_str());
        }
        else
        {
            //Do nothing
        }

        if (OSIP_SUCCESS != result)
        {
            ::eXosip_message_send_answer(mSipCtx, iReqId, 400, NULL);
        }
        else
        {
            //发送消息体
            ::eXosip_message_send_answer(mSipCtx,iReqId, iStatus, answer);
        }
        if (0 == expires)
        {
            eXosip_register_remove(mSipCtx, iReqId);
        }
    }
    eXosip_unlock(mSipCtx);
}

//第一次注册发401 已弃用 有的摄像头不适用 比如水星的
void SipServer::response_register_401unauthorized(eXosip_event_t* evtp) {

    char* dest = nullptr;
    osip_message_t* reg = nullptr;
    osip_www_authenticate_t* header = nullptr;

    osip_www_authenticate_init(&header);
    osip_www_authenticate_set_auth_type(header, osip_strdup("Digest"));
    osip_www_authenticate_set_realm(header, osip_enquote(mInfo->getSipRealm()));
    osip_www_authenticate_set_nonce(header, osip_enquote(mInfo->getNonce()));

    //std::stringstream stream;
    //stream << "Digest realm=\"" << mInfo->getSipRealm()
    //    << "\",nonce=\"" << mInfo->getNonce()
    //    << "\",algorithm=" << ALGORITHTHM;
    //osip_message_set_header(reg, "WWW-Authenticate",
    //    stream.str().c_str());

    osip_www_authenticate_to_str(header, &dest);
    int ret = eXosip_message_build_answer(mSipCtx, evtp->tid, 401, &reg);
    if (ret == 0 && reg != nullptr) {
        osip_message_set_www_authenticate(reg, dest);
        osip_message_set_content_type(reg, "Application/MANSCDP+xml");
        eXosip_lock(mSipCtx);
        eXosip_message_send_answer(mSipCtx, evtp->tid, 401, reg);
        eXosip_unlock(mSipCtx);
        LOGI("response_register_401unauthorized success");
    }
    else {
        LOGI("response_register_401unauthorized error");
    }

    osip_www_authenticate_free(header);
    osip_free(dest);

}

void SipServer::response_message(eXosip_event_t* evtp) {

    osip_body_t* body = nullptr;
    char CmdType[64] = { 0 };
    char DeviceID[64] = { 0 };
    //Get one body header.
    osip_message_get_body(evtp->request, 0, &body);
    if (body) {
        parse_xml(body->body, "<CmdType>", false, "</CmdType>", false, CmdType);
        parse_xml(body->body, "<DeviceID>", false, "</DeviceID>", false, DeviceID);
    }

    //    Client *client = getClientByDevice(DeviceID);
    //    if(client){
    //        LOGI("response_message：%s 已注册",DeviceID);
    //    }else{
    //        LOGE("response_message：%s 未注册",DeviceID);
    //    }
    LOGI("CmdType=%s,DeviceID=%s", CmdType, DeviceID);

    if (!strcmp(CmdType, "Catalog")) {
        this->response_message_answer(evtp, 200);
        // 需要根据对方的Catelog请求，做一些相应的应答请求
    }
    else if (!strcmp(CmdType, "Keepalive")) {
        this->response_message_answer(evtp, 200);
    }
    else {
        this->response_message_answer(evtp, 200);
    }

}

void SipServer::tcpBroadCast(std::string mediaIp, std::string y_params, std::string client_ip, std::string audio_port, 
                            std::string streamName, int src_port, std::string deviceId, int type){

    Client* currentClient = getClientByDevice(deviceId.c_str());
    tuple<int, int, string> loacl_port_info = getStartSendRtpPort(mInfo->getMediaSecret(), mediaIp.c_str(), 
    mInfo->getMediaHttpPort(), y_params, client_ip, audio_port, streamName, src_port, type);
    int local_port = std::get<1>(loacl_port_info);
    if (local_port == 0) {
        cout << "申请推流端口失败,code:" << std::get<0>(loacl_port_info) << " msg:"
        #ifdef WIN32
            << UTF8_to_GB2312(std::get<2>(loacl_port_info).c_str()) << endl;
        #else
            << std::get<2>(loacl_port_info).c_str() << endl;
        #endif

        return;
    }
    else {
        currentClient->setInviteInfoPort(2, local_port);
        cout << "申请推流端口成功,port:" << local_port << endl;
    }
}

void SipServer::response_invite_client(eXosip_event_t* evtp) {

    char* s;
    size_t len;
    osip_body_t* osipBody = nullptr;

    cout << "=========================all-invite-start==========================" << endl;
    osip_message_to_str(evtp->request, &s, &len);
    printf("%s\n", s);

    osip_content_length_t* content_length_t = osip_message_get_content_length(evtp->request);
    int content_length = std::stoi(content_length_t->value);;
    cout << "content_length:" <<content_length;
    //内网测试有221
    if(content_length < 100){
        LOGE("body not find, content_length:%d\n", content_length);
        return;
    }

    osip_body_init(&osipBody);
    osip_message_get_body(evtp->request, 0, &osipBody);
    cout << "========================all-invite-end=========================" << endl;

    LOGI("osipBody->body: %s\n", osipBody->body);
    if(osipBody->body == NULL || osipBody->body == nullptr){
        LOGE("body not find");
        return;
    }

    //Subject:媒体流发送者设备编码:发送端媒体流序列号, 媒体流接收者设备编码 : 接收端媒体流序列号

    auto sdpInfo = transferSDP(osipBody->body);
    string audio_port = std::get<0>(sdpInfo); 
    string audio_params = std::get<1>(sdpInfo); 
    string rtp_params = std::get<2>(sdpInfo);
    string setup_params = std::get<3>(sdpInfo);
    string y_params = std::get<4>(sdpInfo);
    string f_params = std::get<5>(sdpInfo);
    string deviceId = std::get<6>(sdpInfo);
    string client_ip = std::get<7>(sdpInfo);

    Client* currentClient = getClientByDevice(deviceId.c_str());
    auto inviteInfo = currentClient->getInviteInfo(2);
    string streamName = std::get<2>(inviteInfo);

    //osip_body_free(osipBody);
    memset(s, '\0', strlen(s));
    osip_free(s);

    MyConfig* myconfig = MyConfig::getInstance();
    auto configInfo = myconfig->getConfigInfo();
    string mediaIp = "127.0.0.1";
    if (std::stoi(configInfo["other"]["mediaType"]) == 2) {
        mediaIp = mInfo->getMediaIp();
    }

    //获取recieved和rport
    osip_via_t* via_request = nullptr;
    osip_message_get_via(evtp->request, 0, &via_request);
    char *via_char_request = nullptr;
    osip_via_to_str(via_request, &via_char_request);

    cout << "response_invite_client-via_char_request:"<< via_char_request << endl;

    //拼rport和received
    osip_generic_param_t* br_rport = nullptr;
    osip_generic_param_t* br_received = nullptr;
    char via_rport[128] = {};
    osip_via_param_get_byname(via_request, const_cast<char *>("rport"), &br_rport);
    if(br_rport && br_rport->gvalue)
    {
        sprintf(via_rport, ";rport=%s", br_rport->gvalue);
    }
    char via_received[128] = {};
    osip_via_param_get_byname (via_request, const_cast<char *>("received"), &br_received);
    if (br_received != NULL && br_received->gvalue != NULL){
        sprintf(via_received, ";received=%s", br_received->gvalue);
        client_ip = br_received->gvalue;
    }

    int src_port = getMKFport();

    //udp对讲
    // tuple<int, int, string> loacl_port_info = getStartSendRtpPort(mInfo->getMediaSecret(), mediaIp.c_str(), 
    // mInfo->getMediaHttpPort(), y_params, client_ip, audio_port, streamName, src_port, 2);
    // int local_port = std::get<1>(loacl_port_info);
    // if (local_port == 0) {
    //     cout << "申请推流端口失败,code:" << std::get<0>(loacl_port_info) << " msg:"
    //     #ifdef WIN32
    //         << UTF8_to_GB2312(std::get<2>(loacl_port_info).c_str()) << endl;
    //     #else
    //         << std::get<2>(loacl_port_info).c_str() << endl;
    //     #endif
    //     return;
    // }
    // else {
    //     currentClient->setInviteInfoPort(2, local_port);
    //     cout << "申请推流端口成功,port:" << local_port << endl;
    // }

    //tcp对讲线程 主动tcp
    std::thread tcpThread(&SipServer::tcpBroadCast, this, mediaIp, y_params, client_ip, audio_port, streamName, src_port, deviceId, 1);
    tcpThread.detach();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    char session_exp[1024] = { 0 };
    osip_message_t* msg = nullptr;

    char from[1024] = { 0 };
    char to[1024] = { 0 };
    char contact[1024] = { 0 };
    char sdp[2048] = { 0 };
    char head[1024] = { 0 };
    sprintf(from, "sip:%s@%s:%d", deviceId.c_str(), mInfo->getIp(), mInfo->getPort());
    sprintf(contact, "sip:%s@%s:%d", mInfo->getSipId(), mInfo->getIp(), mInfo->getPort());
    sprintf(to, "sip:%s@%s:%d", mInfo->getSipId(), client_ip.c_str(), currentClient->getPort());
    if(setup_params == ""){
        snprintf(sdp, 2048,
            "v=0\r\n"                                   // protocol version
            "o=%s 0 0 IN IP4 %s\r\n"                    // owner/creatorand sessionidentifier
            "s=Play\r\n"                                        //session name
            "c=IN IP4 %s\r\n"                               //connection information-not required if included in all media
            "t=0 0\r\n"                                     //time the session is active
            "m=audio %d %s\r\n"                             //media name and transport address
            "%s\r\n"                                        //zero or more media attribute lines
            "a=sendonly\r\n"                                
            "y=%s\r\n"                                      //SSRC
            "%s\r\n",                                   //f= v/编码格式/分辨率/帧率/码率类型/码率大小a/编码格式/码率大小/采样率
            deviceId.c_str(), mInfo->getMediaIp(), mInfo->getMediaIp(), src_port, audio_params.c_str(), 
            rtp_params.c_str(), y_params.c_str(), f_params.c_str());
    }else{
        snprintf(sdp, 2048,
            "v=0\r\n"                                   // protocol version
            "o=%s 0 0 IN IP4 %s\r\n"                    // owner/creatorand sessionidentifier
            "s=Play\r\n"                                        //session name
            "c=IN IP4 %s\r\n"                               //connection information-not required if included in all media
            "t=0 0\r\n"                                     //time the session is active
            "m=audio %d %s\r\n"                             //media name and transport address
            "%s\r\n"                                        //zero or more media attribute lines
            "a=setup:passive\r\n"                           //主动tcp
            "a=sendonly\r\n"                                
            "y=%s\r\n"                                      //SSRC
            "%s\r\n",                                   //f= v/编码格式/分辨率/帧率/码率类型/码率大小a/编码格式/码率大小/采样率
            deviceId.c_str(), mInfo->getMediaIp(), mInfo->getMediaIp(), src_port, audio_params.c_str(), 
            rtp_params.c_str(), y_params.c_str(), f_params.c_str());
    }

    //获取from的tag
    osip_uri_param_t* from_tag = nullptr;
    osip_uri_param_get_byname((&(evtp->request->from)->gen_params), const_cast<char*>("tag"), &from_tag);

    //to_tag随机数生成
    char to_tag[200];
    snprintf(to_tag, 200, "wjcG4bK%u", osip_build_random_number());

    osip_message_init(&msg);
    osip_message_set_version(msg, osip_strdup("SIP/2.0"));
    osip_message_set_status_code(msg, 200);
    osip_message_set_reason_phrase(msg, osip_strdup("OK"));
    osip_message_set_to(msg, to);
    osip_message_set_from(msg, from);
    osip_uri_param_add((&(msg->from)->gen_params), osip_strdup("tag"), osip_strdup(from_tag->gvalue));
    osip_uri_param_add((&(msg->to)->gen_params), osip_strdup("tag"), osip_strdup(to_tag));
    osip_message_set_contact(msg, osip_strdup(contact));
    osip_message_set_body(msg, sdp, strlen(sdp));
    osip_message_set_content_type(msg, "application/sdp");
    snprintf(session_exp, sizeof(session_exp) - 1, "%i;refresher=uac", mInfo->getTimeout());
    osip_message_set_header(msg, "Session-Expires", session_exp);
    //osip_message_set_call_id(msg, evtp->request->call_id->host);

    osip_message_set_via(msg, via_char_request);

    char* new_call_id;
    osip_call_id_to_str(evtp->request->call_id, &new_call_id);
    osip_message_set_call_id(msg, new_call_id);

    char* new_cseq;
    osip_cseq_to_str(evtp->request->cseq, &new_cseq);
    osip_message_set_cseq(msg, new_cseq);

    osip_message_set_header(msg, (const char*)"User-Agent", "BXC_SipServer");

    //这玩意儿竟能能返回0
    int call_id = eXosip_call_send_answer(mSipCtx, evtp->tid, 200, msg);
    //int call_id = eXosip_call_send_initial_invite(mSipCtx, msg);
    if (call_id >= 0) {
        LOGI("invite success:callid=%d", call_id);
    }
    else {
        LOGE("invite error: call_id=%d", call_id);
    }
}

//这里 如果上面的response_invite_client发送给客户端的有乱码，那么即使客户端发来ack这里也解析不到
void SipServer::recive_invite_ack(eXosip_event_t* evtp){

    //为每个客户端设置语音对讲cid, did
    Client* c = mClientMap[evtp->request->from->url->username];

    c->setInviteInfo(2, evtp->cid, evtp->did);  //对讲 第一个参数是2
    auto inviteInfo = c->getInviteInfo(2);
    cout<<"recive_invite_ack-inviteInfo2:"<<std::get<0>(inviteInfo)<<" "<<std::get<1>(inviteInfo)<<endl;

    LOGE("recive_invite_ack success: cid=%d request_cid=%d, did=%d, username=%s", 
        evtp->cid, evtp->request->call_id, evtp->did, evtp->request->req_uri->username);
}

//invite 结尾 ack
void SipServer::response_invite_ack(eXosip_event_t* evtp) {

    osip_message_t* msg = nullptr;
    int ret = eXosip_call_build_ack(mSipCtx, evtp->did, &msg);
    if (!ret && msg) {
        eXosip_call_send_ack(mSipCtx, evtp->did, msg);

        //为每个客户端设置推流cid, did
        Client* c = mClientMap[evtp->request->req_uri->username];

        c->setInviteInfo(1, evtp->cid, evtp->did);  //推流 第一个参数是1

        LOGE("eXosip_call_send_ack success: cid=%d request_cid=%d, did=%d, username=%s", 
            evtp->cid, evtp->request->call_id, evtp->did, evtp->request->req_uri->username);
    }
    else {
        LOGE("eXosip_call_send_ack error=%d", ret);
    }

}

int SipServer::request_bye(int index, std::string ssrc, int type) {

    std::string deviceId = this->getDeviceIDBySSRC(ssrc);
    int ret = 0;
    if (deviceId != "") {

        Client* c = mClientMap[deviceId];
        if(type == 2 && index == 1){
            auto t_temp = c->getInviteInfo(index);
            int cid = std::get<0>(t_temp);
            int did = std::get<1>(t_temp);
            //这个函数只能关闭推流不能关闭对讲 关对讲会报析构错误
            eXosip_lock(mSipCtx);
            ret = eXosip_call_terminate(mSipCtx, cid, did);
            eXosip_unlock(mSipCtx);
            LOGE("accrod close %s return code=%d", ssrc, ret);
        }else{
            LOGE("passive close %s", ssrc);
        }
        if(c){
            c->setInviteInfo(index, 0, 0, "");
            c->setInviteInfoPort(index, 0);
        }

        return ret;
    }

    return ret; 
}

int SipServer::request_invite(string ssrc, const char* userIp, int userPort) {
    LOGI("INVITE");

    std::string deviceId = this->getDeviceIDBySSRC(ssrc);
    //这里要先判断是否该客户端已经在推流，若没有就开始invite
    //?.....后续添加判断

    //申请RTP的端口
    tuple<int, int, string> portInfo;
    {
        std::lock_guard<std::mutex> mylockguard(m_mutex);
        MyConfig* myconfig = MyConfig::getInstance();
        auto configInfo = myconfig->getConfigInfo();
        string mediaIp = "127.0.0.1";
        if(std::stoi(configInfo["other"]["mediaType"]) == 2){
            mediaIp = mInfo->getMediaIp();
        }
        portInfo = openRtpPort(mInfo->getMediaSecret(), mediaIp.c_str(),
            mInfo->getMediaHttpPort(), mInfo->getRtpStartPort(), ssrc);
    }
    int mediaPort = std::get<1>(portInfo);
    if (mediaPort == 0) {
        cout << "申请流媒体RTP端口失败,code:" << std::get<0>(portInfo) << " msg:"
            
        #ifdef WIN32
                    << UTF8_to_GB2312(std::get<2>(portInfo).c_str()) << endl;
        #else
                    << std::get<2>(portInfo).c_str() << endl;
        #endif

    }
    else {
        cout << "申请流媒体RTP端口成功,port:" << mediaPort << endl;
    }

    char session_exp[1024] = { 0 };
    osip_message_t* msg = nullptr;
    char from[1024] = { 0 };
    char to[1024] = { 0 };
    char contact[1024] = { 0 };
    char sdp[2048] = { 0 };

    //拼rport和received 数据从注册而来
    osip_generic_param_t* br_rport = nullptr;
    osip_generic_param_t* br_received = nullptr;
    char via_rport[128] = {};
    osip_via_param_get_byname(mClientMap[deviceId]->m_osip_via_t, const_cast<char *>("rport"), &br_rport);
    if(br_rport && br_rport->gvalue)
    {
        sprintf(via_rport, ";rport=%s", br_rport->gvalue);
    }
    char via_received[128] = {};
    osip_via_param_get_byname (mClientMap[deviceId]->m_osip_via_t, const_cast<char *>("received"), &br_received);
    if (br_received != NULL && br_received->gvalue != NULL){
        sprintf(via_received, ";received=%s", br_received->gvalue);
    }

    sprintf(from, "sip:%s@%s:%d", mInfo->getSipId(), mInfo->getIp(), mInfo->getPort());
    sprintf(contact, "sip:%s@%s:%d", mInfo->getSipId(), mInfo->getIp(), mInfo->getPort());
    if(via_received[0] != '\0'){
        //外网
        sprintf(to, "sip:%s@%s:%s", deviceId.c_str(), br_received->gvalue, br_rport->gvalue);
    }else{
        //内网
        sprintf(to, "sip:%s@%s:%d", deviceId.c_str(), userIp, userPort);
    }

    snprintf(sdp, 2048,
        "v=0\r\n"
        "o=%s 0 0 IN IP4 %s\r\n"
        "s=Play\r\n"
        "c=IN IP4 %s\r\n"
        "t=0 0\r\n"
        "m=video %d TCP/RTP/AVP 96 98 97\r\n"
        "a=recvonly\r\n"
        "a=rtpmap:96 PS/90000\r\n"
        "a=rtpmap:98 H264/90000\r\n"
        "a=rtpmap:97 MPEG4/90000\r\n"
        "a=setup:passive\r\n"
        "a=connection:new\r\n"
        "y=0100000001\r\n"
        "f=\r\n", mInfo->getSipId(), mInfo->getMediaIp(), mInfo->getMediaIp(), mediaPort == 0 ? mInfo->getRtpPort() : mediaPort);

    int ret = eXosip_call_build_initial_invite(mSipCtx, &msg, to, from, nullptr, nullptr);
    if (ret) {
        LOGE("eXosip_call_build_initial_invite error: %s %s ret:%d", from, to, ret);
        return -1;
    }

    osip_message_set_body(msg, sdp, strlen(sdp));
    osip_message_set_content_type(msg, "application/sdp");
    snprintf(session_exp, sizeof(session_exp) - 1, "%i;refresher=uac", mInfo->getTimeout());
    osip_message_set_header(msg, "Session-Expires", session_exp);
    osip_message_set_supported(msg, "timer");

    //拼rport和received 数据从注册而来
    osip_via_t* via_msg = nullptr;
    osip_message_get_via(msg, 0, &via_msg);

    char *via_char = nullptr;
    osip_via_to_str(via_msg, &via_char);
    string str_via_char = via_char;
    string fnd = ";rport";

    //去除rport
    if(via_rport[0] != '\0'){
        string rep = via_rport;
        str_via_char = str_via_char.replace(str_via_char.find(fnd), fnd.length(), rep); 
    }
    if(via_received[0] != '\0'){
        str_via_char += via_received;
    }

    osip_via_free(via_msg);
    osip_via_init(&via_msg);
    osip_via_parse(via_msg, str_via_char.c_str());

    int call_id = eXosip_call_send_initial_invite(mSipCtx, msg);

    char* s;
    size_t len;
    osip_message_to_str(msg, &s, &len);
    printf("osip_message:\n%s\n", s);

    if (call_id > 0) {
        LOGI("eXosip_call_send_initial_invite success: call_id=%d", call_id);
        //设置本客户端的端口号
        Client* currentClient = this->getClientByDevice(deviceId.c_str());
        if (currentClient) {
            currentClient->setRtpPort(mediaPort);
        }
        else {
            LOGI("currentClient cannot find:%s", deviceId);
        }
    }
    else {
        LOGE("eXosip_call_send_initial_invite error: call_id=%d", call_id);
    }
    return 1;
}

int SipServer::request_message(Client* currentClient) {
    static int SNnumber = 1;
    osip_message_t* message = nullptr;
    char from[1024] = { 0 };
    char to[1024] = { 0 };
    char tmp[1024] = { 0 };
    std::string deviceId = currentClient->getDevice();
    sprintf(from, "sip:%s@%s:%d", mInfo->getSipId(), mInfo->getIp(), mInfo->getPort());

    osip_via_to_str(mClientMap[deviceId]->m_osip_via_t, &m_via_char);
    LOGI("message-m_via_char:%s",m_via_char);

    //拼rport和received 数据从注册而来
    osip_generic_param_t* br_rport = nullptr;
    osip_generic_param_t* br_received = nullptr;
    char via_rport[128] = {};
    osip_via_param_get_byname(mClientMap[deviceId]->m_osip_via_t, const_cast<char *>("rport"), &br_rport);
    if(br_rport && br_rport->gvalue)
    {
        sprintf(via_rport, ";rport=%s", br_rport->gvalue); 
    }

    char via_received[128] = {};
    osip_via_param_get_byname (mClientMap[deviceId]->m_osip_via_t, const_cast<char *>("received"), &br_received);
    if (br_received != NULL && br_received->gvalue != NULL){
        sprintf(via_received, ";received=%s", br_received->gvalue);
    }

    if(via_received[0] != '\0'){
        //外网
        sprintf(to, "sip:%s@%s:%s", deviceId.c_str(), br_received->gvalue, br_rport->gvalue);
    }else{
        //内网
        sprintf(to, "sip:%s@%s:%d", deviceId.c_str(), currentClient->getIp(), currentClient->getPort());
    }

    eXosip_message_build_request(mSipCtx, &message, "MESSAGE", to, from, nullptr);

    MyConfig* myconfig = MyConfig::getInstance();
    auto configInfo = myconfig->getConfigInfo();
    std::string serviceSipID = configInfo["sip"]["sipID"];
    char osip_body[1024] = {0};
    sprintf(osip_body,
    "<? xml version = \"1.0\" encoding=\"GB2312\"?>\r\n"
        "<Notify>\r\n"
        "<CmdType>Broadcast</CmdType>\r\n"                  //命令类型
        "<SN>%06d</SN>\r\n"                                 //用于与请求命令的匹配处理,随便指定
        "<SourceID>%s</SourceID>\r\n"                       //语音输入设备的设备编码
        "<TargetID>%s</TargetID>\r\n"                       //语音输出设备的设备编码
        "</Notify>\r\n",
        SNnumber, serviceSipID.c_str(), deviceId.c_str()
    );

    osip_message_set_body(message, osip_body, strlen(osip_body));

    osip_message_set_content_type(message, "Application/MANSCDP+xml");

    //拼rport和received 数据从注册而来
    osip_via_t* via_msg = nullptr;
    osip_message_get_via(message, 0, &via_msg);
    char *via_char = nullptr;
    osip_via_to_str(via_msg, &via_char);
    string str_via_char = via_char;
    string fnd = ";rport";
    if(via_rport[0] != '\0'){
        string rep = via_rport;
        str_via_char = str_via_char.replace(str_via_char.find(fnd), fnd.length(), rep); 
    }
    if(via_received[0] != '\0'){
        str_via_char += via_received;
    }
    osip_via_free(via_msg);
    osip_via_init(&via_msg);
    osip_via_parse(via_msg, str_via_char.c_str());

    eXosip_lock(mSipCtx);
    int ret = eXosip_message_send_request(mSipCtx, message);
    eXosip_unlock(mSipCtx);
    ++SNnumber;
    cout << "发送：" << ret << endl;

    return 0;
}

//清理所有客户端
//个人觉得22消息有点不太正常或者不知道发来干嘛的好像没什么用，不能直接全部释放
int SipServer::clearClientMap() {

    return 0;
}
Client* SipServer::getClientByDevice(const char* device) {

    auto it = mClientMap.find(device);
    if (it == mClientMap.end()) {
        return nullptr;
    }
    return it->second;

}

std::string SipServer::getDeviceIDBySSRC(std::string ssrc) {

    std::string deviceId = "";
    for (auto i : ClientDeviceMap) {
        if (i.second == ssrc) {
            return i.first;
        }
    }
    return deviceId;
}

int SipServer::parse_xml(const char* data, const char* s_mark, bool with_s_make, const char* e_mark, bool with_e_make, char* dest) {
    const char* satrt = strstr(data, s_mark);

    if (satrt != NULL) {
        const char* end = strstr(satrt, e_mark);

        if (end != NULL) {
            int s_pos = with_s_make ? 0 : strlen(s_mark);
            int e_pos = with_e_make ? strlen(e_mark) : 0;

            strncpy(dest, satrt + s_pos, (end + e_pos) - (satrt + s_pos));
        }
        return 0;
    }
    return -1;

}
void SipServer::dump_request(eXosip_event_t* evtp) {

    char* s;
    size_t len;
    osip_message_to_str(evtp->request, &s, &len);
    LOGI("\nprint request start\ntype=%d\n%s\nprint request end\n", evtp->type, s);
    osip_free(s);
}
void SipServer::dump_response(eXosip_event_t* evtp) {

    char* s;
    size_t len;
    osip_message_to_str(evtp->response, &s, &len);
    LOGI("\nprint response start\ntype=%d\n%s\nprint response end\n", evtp->type, s);
    osip_free(s);
}

std::map<std::string, Client*> SipServer::getClientInfo()
{
    return mClientMap;
}

int SipServer::getMKFport()
{
    int maxNumber = 100;
    MyConfig* myconfig = MyConfig::getInstance();
    auto configInfo = myconfig->getConfigInfo();
    int startPort = std::stoi(configInfo["media"]["mediaMKFPort"]);

    Client* currentClient = nullptr;
    std::vector<int> vPort;
    vPort.reserve(maxNumber);
    for(auto it = mClientMap.begin(); it != mClientMap.end(); ++it){
        currentClient = it->second;
        auto inviteInfo = currentClient->getInviteInfo(2);
        vPort.emplace_back(std::get<3>(inviteInfo));
    }
    std::sort(vPort.begin(), vPort.end());

    if(vPort.empty()){
        return startPort;
    }

    int vPortLength = vPort.size();
    for(int i = 0; i < maxNumber;){
        //数组中第一个就比开始端口大，选开始端口
        if(vPort[i] > startPort){
            break;
        }else if(vPort[i] < startPort){
            while(vPort[i] < startPort){
                ++i;
                //数组遍历完都找不到比开始端口大的，那肯定选择开始端口了
                if(i >= vPortLength){
                    break;
                }
            }

            //数组遍历完都找不到比开始端口大的，那肯定选择开始端口了
            if(i >= vPortLength){
                break;
            }

            //找到了比开始端口大的，这时候要把开始端口自增再和此端口相比
            ++startPort;

        }else{
            //数组最后一个和开始端口相等，那么开始端口++,再退出
            if(i == vPortLength-1){
                ++startPort;
                break;
            }else{
                //数组中间位置和开始端口相等，那么各自++后再相比较
                ++startPort;
                ++i;
            }
        }
    }

    return startPort;
}