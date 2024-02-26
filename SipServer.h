#ifndef BXC_SIPSERVER_SIPSERVER_H
#define BXC_SIPSERVER_SIPSERVER_H
extern "C" {
#include <osip2/osip_mt.h>
#include <eXosip2/eXosip.h>
}
#include <iostream>
#include <sstream>
#include <map>
#include <tuple>
#include <string>
#include <mutex>
#include <vector>

class ServerInfo {
public:
    ServerInfo(const char* ua, const char* nonce, const char* ip, int port, const char* mediaip, int rtpPort, int rtpStartPort, 
        int mediaHttpPort, const char* sipId, const char* sipRealm, const char* sipPass, int sipTimeout, int sipExpiry, 
        std::string mediaSecret) :
        mUa(ua), mNonce(nonce), mIp(ip), mPort(port), mediaIp(mediaip), mRtpPort(rtpPort), mRtpStartPort(rtpStartPort),
        mMediaHttpPort(mediaHttpPort), mSipId(sipId), mSipRealm(sipRealm), mSipPass(sipPass), mSipTimeout(sipTimeout), 
        mSipExpiry(sipExpiry), mMediaSecret(mediaSecret) {}
    ~ServerInfo() = default;
public:
    const char* getUa() const {
        return mUa;
    }
    const char* getNonce() const {
        return mNonce;
    }
    const char* getIp() const {
        return mIp;
    }
    int getPort() const {
        return mPort;
    }
    const char* getMediaIp() const {
        return mediaIp;
    }
    int getRtpPort() const {
        return mRtpPort;
    }
    int getRtpStartPort() const {
        return mRtpStartPort;
    }
    int getMediaHttpPort() const {
        return mMediaHttpPort;
    }
    const char* getSipId() const {
        return mSipId;
    }
    const char* getSipRealm() const {
        return mSipRealm;
    }
    const char* getSipPass() const {
        return mSipPass;
    }
    int getTimeout() const {
        return mSipTimeout;
    }
    int getExpiry() const {
        return mSipExpiry;
    }
    std::string getMediaSecret() const {
        return mMediaSecret;
    }

private:
    const char* mUa;
    const char* mNonce;//SIP服务随机数值
    const char* mIp;//SIP服务IP
    int         mPort;//SIP服务端口
    const char* mediaIp;//媒体服务器IP
    const char* mSipId; //SIP服务器ID
    const char* mSipRealm;//SIP服务器域
    const char* mSipPass;//SIP password
    int mSipTimeout; //SIP timeout
    int mSipExpiry;// SIP到期
    int mRtpPort; //SIP-RTP默认服务端口
    int mRtpStartPort; //SIP-RTP初始服务端口
    int mMediaHttpPort;//媒体服务器http端口
    std::string mMediaSecret;//媒体服务器http的密钥
};


class Client {
public:
    Client(const char* ip, int port, const char* device) :
        mIp(ip),            //客户端(摄像头)ip
        mPort(port),        //客户端端口
        mRtpPort(0),        //此客户端对应的服务器rtp端口，如果不为0说明已经在推流了
        mDevice(device),    //客户端sipid
        mIsReg(false) {
    
        
        ciddidInfo[1] = std::make_tuple(0, 0, "", 0);
        ciddidInfo[2] = std::make_tuple(0, 0, "", 0);
    }
    ~Client() = default;

    void setRtpPort(int rtpPort) {
        mRtpPort = rtpPort;
    }

    void setReg(bool isReg) {
        mIsReg = isReg;
    }
    const char* getDevice() const {
        return mDevice;
    }
    const char* getIp() const {
        return mIp;
    }
    int getPort() const {
        return mPort;
    }
    int getRtpPort() const {
        return mRtpPort;
    }
    bool setInviteInfo(int type, int cid, int did, std::string streamName = ""){

        std::get<0>(ciddidInfo[type]) = cid;
        std::get<1>(ciddidInfo[type]) = did;
        if(streamName != ""){
            std::get<2>(ciddidInfo[type]) = streamName;
        }
        
        return true;
    }

    void setInviteInfoPort(int type, int id){
        std::get<3>(ciddidInfo[type]) = id;
    }

    std::tuple<int, int, std::string, int> getInviteInfo(int type){
        //找到了
        if(ciddidInfo.find(type) != ciddidInfo.end()){
            return ciddidInfo[type];
        }else{
            std::make_tuple(0, 0, "", 0);
        }
    }

    osip_via_t* m_osip_via_t = nullptr; //每个设备在外网注册后需要保留的via信息，里面有rport和recieved

private:
    // step1
    const char* mIp; // client ip
    int mPort; // client port
    const char* mDevice;// 340200000013200000024
    // step2
    bool mIsReg;
    // step3
    int mRtpPort;
    //这个map 1代表推流 2代表语音对讲 tuple里的三个参数依次是cid did 和流媒体的url(待用) 以及占用的流媒体端口(对讲)
    //注意这里的cid不是wireshark抓报的cid 而是exosip的cid
    //推流时response_invite_ack函数里添加 在注销和停止推流时删除
    std::map<int, std::tuple<int, int, std::string, int>> ciddidInfo;
};


//单例模式
class SipServer {
public:
    static SipServer* getInstance();
    SipServer(const ServerInfo& s)=delete;
    SipServer operator =(const ServerInfo& s) = delete;
    ~SipServer();

    void initial(ServerInfo* info);
    int request_message(Client* currentClient);  //给摄像头发message,语音广播前置消息
    int request_invite(std::string ssrc, const char* userIp, int userPort);  //邀请相机
    int request_bye(int index, std::string ssrc, int type = 2);// 通知相机停止推流 index1代表推流 index2代表语音对讲 type 1 表示不需要主动关闭 2表示需要主动关闭

    std::map<std::string, Client*> getClientInfo();
    std::string getDeviceIDBySSRC(std::string ssrc);
    Client* getClientByDevice(const char* device);

    void loop();
private:
    static SipServer* instance;
    explicit SipServer();

    int init_sip_server();
    int sip_event_handle(eXosip_event_t* evtp);

    void response_message_answer(eXosip_event_t* evtp, int code, std::string type = "");
    void response_register(eXosip_event_t* evtp);

    void response_register_401unauthorized2(eXosip_event_t* evtp);
    void response_register_401unauthorized(eXosip_event_t* evt);
    void response_message(eXosip_event_t* evtp);
    void response_invite_ack(eXosip_event_t* evtp);
    void recive_invite_ack(eXosip_event_t* evtp);   //摄像头语音对讲ack返回后处理
    //对讲tcp函数 需要分线程，所以单独做一个函数
    void tcpBroadCast(std::string mediaIp, std::string y_params, std::string client_ip, std::string audio_port, 
                        std::string streamName, int src_port, std::string deviceId, int type);
    void response_invite_client(eXosip_event_t* evtp);

    int parse_xml(const char* data, const char* s_mark, bool with_s_make, const char* e_mark, bool with_e_make, char* dest);
    void dump_request(eXosip_event_t* evtp);
    void dump_response(eXosip_event_t* evtp);
    
    //获取能用的麦克风推流端口
    int getMKFport(); 

private:
    bool mQuit;
    struct eXosip_t* mSipCtx;
    ServerInfo* mInfo;
    std::mutex m_mutex;
    std::map<std::string, Client*> mClientMap;          //这个是客户端列表
    int clearClientMap();
    std::map<std::string, std::string> ClientDeviceMap; //#设备-ssrc比如：34020000001320000001-wjc001 这是配置文件里的
    std::string mCurrentIP;
    char *m_via_char = nullptr;                         //这个用于注册时生成via
};


#endif //BXC_SIPSERVER_SIPSERVER_H
