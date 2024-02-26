#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <ctime>
#include <exception>
#include <memory>
#include <regex>
#include <openssl/md5.h>
#include <openssl/evp.h>
#include <mutex>
#include <string>
#include <algorithm>
#include <vector>
#include <thread>
#include <map>
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
//使用 ifconf结构体和ioctl函数时需要用到该头文件
#include <net/if.h>
#include <sys/ioctl.h>
//使用ifaddrs结构体时需要用到该头文件
#include <ifaddrs.h>
#include <string.h>
#include "func.h"
#include "mylog.h"
#include "myconfig.h"
#include "../SipServer.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#ifdef WIN32
#include <windows.h>




//UTF-8转Unicode 
std::wstring Utf82Unicode(const std::string& utf8string) {
    int widesize = ::MultiByteToWideChar(CP_UTF8, 0, utf8string.c_str(), -1, NULL, 0);
    if (widesize == ERROR_NO_UNICODE_TRANSLATION)
    {
        throw std::exception("Invalid UTF-8 sequence.");
    }
    if (widesize == 0)
    {
        throw std::exception("Error in conversion.");
    }
    std::vector<wchar_t> resultstring(widesize);
    int convresult = ::MultiByteToWideChar(CP_UTF8, 0, utf8string.c_str(), -1, &resultstring[0], widesize);
    if (convresult != widesize)
    {
        throw std::exception("La falla!");
    }
    return std::wstring(&resultstring[0]);
}


//unicode 转为 ascii 
std::string WideByte2Acsi(std::wstring& wstrcode) {
    int asciisize = ::WideCharToMultiByte(CP_OEMCP, 0, wstrcode.c_str(), -1, NULL, 0, NULL, NULL);
    if (asciisize == ERROR_NO_UNICODE_TRANSLATION)
    {
        throw std::exception("Invalid UTF-8 sequence.");
    }
    if (asciisize == 0)
    {
        throw std::exception("Error in conversion.");
    }
    std::vector<char> resultstring(asciisize);
    int convresult = ::WideCharToMultiByte(CP_OEMCP, 0, wstrcode.c_str(), -1, &resultstring[0], asciisize, NULL, NULL);
    if (convresult != asciisize)
    {
        throw std::exception("La falla!");
    }
    return std::string(&resultstring[0]);
}



//utf-8 转 ascii 
std::string UTF_82ASCII(std::string& strUtf8Code) {
    using namespace std;
    string strRet = "";
    //先把 utf8 转为 unicode 
    wstring wstr = Utf82Unicode(strUtf8Code);
    //最后把 unicode 转为 ascii 
    strRet = WideByte2Acsi(wstr);
    return strRet;
}



//ascii 转 Unicode 
std::wstring Acsi2WideByte(std::string& strascii) {
    using namespace std;
    int widesize = MultiByteToWideChar(CP_ACP, 0, (char*)strascii.c_str(), -1, NULL, 0);
    if (widesize == ERROR_NO_UNICODE_TRANSLATION)
    {
        throw std::exception("Invalid UTF-8 sequence.");
    }
    if (widesize == 0)
    {
        throw std::exception("Error in conversion.");
    }
    std::vector<wchar_t> resultstring(widesize);
    int convresult = MultiByteToWideChar(CP_ACP, 0, (char*)strascii.c_str(), -1, &resultstring[0], widesize);
    if (convresult != widesize)
    {
        throw std::exception("La falla!");
    }
    return std::wstring(&resultstring[0]);
}


//Unicode 转 Utf8 
std::string Unicode2Utf8(const std::wstring& widestring) {
    using namespace std;
    int utf8size = ::WideCharToMultiByte(CP_UTF8, 0, widestring.c_str(), -1, NULL, 0, NULL, NULL);
    if (utf8size == 0)
    {
        throw std::exception("Error in conversion.");
    }
    std::vector<char> resultstring(utf8size);
    int convresult = ::WideCharToMultiByte(CP_UTF8, 0, widestring.c_str(), -1, &resultstring[0], utf8size, NULL, NULL);
    if (convresult != utf8size)
    {
        throw std::exception("La falla!");
    }
    return std::string(&resultstring[0]);
}


//ascii 转 Utf8 
std::string ASCII2UTF_8(std::string& strAsciiCode) {
    using namespace std;
    string strRet("");
    //先把 ascii 转为 unicode 
    wstring wstr = Acsi2WideByte(strAsciiCode);
    //最后把 unicode 转为 utf8 
    strRet = Unicode2Utf8(wstr);
    return strRet;
}



std::string UTF8_to_GB2312(const char* utf8)
{
    int len = MultiByteToWideChar(CP_UTF8, 0, utf8, -1, NULL, 0);
    wchar_t* wstr = new wchar_t[len + 1];
    memset(wstr, 0, len + 1);
    MultiByteToWideChar(CP_UTF8, 0, utf8, -1, wstr, len);
    len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
    char* str = new char[len + 1];
    memset(str, 0, len + 1);
    WideCharToMultiByte(CP_ACP, 0, wstr, -1, str, len, NULL, NULL);
    delete[] wstr;
    std::string gb2312_str = str;
    delete[] str;
    return gb2312_str;
}

std::string GB2312_to_UTF8(const char* gb2312)
{
    int len = MultiByteToWideChar(CP_ACP, 0, gb2312, -1, NULL, 0);
    wchar_t* wstr = new wchar_t[len + 1];
    memset(wstr, 0, len + 1);
    MultiByteToWideChar(CP_ACP, 0, gb2312, -1, wstr, len);
    len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, NULL, 0, NULL, NULL);
    char* str = new char[len + 1];
    memset(str, 0, len + 1);
    WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, NULL, NULL);
    delete[] wstr;
    std::string utf8_str = str;
    delete[] str;
    return utf8_str;
}


// Converting a WChar string to a Ansi string
std::string WChar2Ansi(LPCWSTR pwszSrc)
{
    int nLen = WideCharToMultiByte(CP_ACP, 0, pwszSrc, -1, NULL, 0, NULL, NULL);
    if (nLen <= 0)
        return std::string("");
    char* pszDst = new char[nLen];
    if (NULL == pszDst)
        return std::string("");
    WideCharToMultiByte(CP_ACP, 0, pwszSrc, -1, pszDst, nLen, NULL, NULL);
    pszDst[nLen - 1] = 0;
    std::string strTemp(pszDst);
    delete[] pszDst;
    return strTemp;
}

std::string ws2s(std::wstring& inputws)
{
    return WChar2Ansi(inputws.c_str());
}

// Converting a Ansi string to WChar string
std::wstring Ansi2WChar(LPCSTR pszSrc, int nLen)
{
    int nSize = MultiByteToWideChar(CP_ACP, 0, (LPCSTR)pszSrc, nLen, 0, 0);
    if (nSize <= 0)
        return L"";
    WCHAR* pwszDst = new WCHAR[nSize + 1];
    if (NULL == pwszDst)
        return NULL;
    MultiByteToWideChar(CP_ACP, 0, (LPCSTR)pszSrc, nLen, pwszDst, nSize);
    pwszDst[nSize] = 0;
    if (pwszDst[0] == 0xFEFF) // skip Oxfeff
        for (int i = 0; i < nSize; i++)
            pwszDst[i] = pwszDst[i + 1];
    std::wstring wcharString(pwszDst);
    delete[] pwszDst;
    return wcharString;
}

std::wstring s2ws(const std::string& s)
{
    return Ansi2WChar(s.c_str(), s.size());
}
#endif

#define PJ_MAX_HOSTNAME  (128)
#define RUN_SUCCESS 0
#define RUN_FAIL -1

std::string time_t2string(const time_t time_t_time)
{
    char szTime[100] = { '\0' };

    tm* pTm = new tm;

    #ifdef WIN32
    localtime_s(pTm, &time_t_time);
    #else
    localtime(&time_t_time);
    #endif
    
    //pTm = localtime(&time_t_time);
    pTm->tm_year += 1900;
    pTm->tm_mon += 1;

    #ifdef WIN32
    sprintf_s(szTime, "%04d/%02d/%02d %02d:%02d:%02d",
        pTm->tm_year,
        pTm->tm_mon,
        pTm->tm_mday,
        pTm->tm_hour,
        pTm->tm_min,
        pTm->tm_sec);
    #else
    sprintf(szTime, "%04d/%02d/%02d %02d:%02d:%02d",
        pTm->tm_year,
        pTm->tm_mon,
        pTm->tm_mday,
        pTm->tm_hour,
        pTm->tm_min,
        pTm->tm_sec);
    #endif


    std::string strTime = szTime;

    delete pTm;
    pTm = NULL;

    return strTime;
}

std::string md5(const std::string& content)
{
   EVP_MD_CTX* context = EVP_MD_CTX_new();

   const EVP_MD* md = EVP_md5();
   unsigned char md_value[EVP_MAX_MD_SIZE];
   unsigned int  md_len;
   std::string output;

   EVP_DigestInit_ex2(context, md, NULL);
   EVP_DigestUpdate(context, content.c_str(), content.length());
   EVP_DigestFinal_ex(context, md_value, &md_len);
   EVP_MD_CTX_free(context);

   //库函数计算出的md_value是16进制，需要转化为10进制
   output.resize(md_len * 2);
   for (unsigned int i = 0; i < md_len; ++i) {
       // 这行就是把每个16进制位转化为两个10进制ascii码，但是第二个参数不能设定为output.length()，
       // 因为内存复制只转移了2位（外加'\0'）, 否则就会多复制内存（不够的默认为0xfe）,
       // 最终导致主函数退出时这个output析构报堆错误；
       // 参考：https://blog.csdn.net/u012104827/article/details/82840755
       //sprintf_s(&output[i * 2], output.length(), "%02x", md_value[i]);
       

    #ifdef WIN32
    sprintf_s(&output[i * 2], 3, "%02x", md_value[i]);
    #else
    sprintf(&output[i * 2], "%02x", md_value[i]);
    #endif

   }
       
   return output;
}

void md5(const std::string& content, std::string& digist)
{
   EVP_MD_CTX* context = EVP_MD_CTX_new();
   const EVP_MD* md = EVP_md5();
   unsigned char md_value[EVP_MAX_MD_SIZE];
   unsigned int  md_len;


   EVP_DigestInit_ex2(context, md, NULL);
   EVP_DigestUpdate(context, content.c_str(), content.length());
   EVP_DigestFinal_ex(context, md_value, &md_len);
   EVP_MD_CTX_free(context);

   digist.resize(md_len * 2);
   for (unsigned int i = 0; i < md_len; ++i){
        #ifdef WIN32
        sprintf_s(&digist[i * 2], 3, "%02x", md_value[i]);
        #else
        sprintf(&digist[i * 2], "%02x", md_value[i]);
        #endif
   }
       
}

size_t req_reply(void* ptr, size_t size, size_t nmemb, void* stream)
{
    //在注释的里面可以打印请求流，cookie的信息
    //cout << "----->reply" << endl;
    string* str = (string*)stream;
    //cout << *str << endl;
    (*str).append((char*)ptr, size * nmemb);

    return size * nmemb;
}

string getUrlInfo(string url) {
    string response;

    CURL* curl = 0;
    CURLcode res_code;
    curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "GET");
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    /* example.com is redirected, so we tell libcurl to follow redirection */
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    /* Perform the request, res will get the return code */
    curl_easy_setopt(curl, CURLOPT_DEFAULT_PROTOCOL, "http");
    struct curl_slist* headers = NULL;
    //headers = curl_slist_append(headers, media_secret.c_str());
    //headers = curl_slist_append(headers, media_port.c_str());
    //headers = curl_slist_append(headers, media_enable_tcp.c_str());
    //headers = curl_slist_append(headers, media_stream_id.c_str());
    //headers = curl_slist_append(headers, "User-Agent: Apifox/1.0.0 (https://apifox.com)");
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &req_reply);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&response);

    res_code = curl_easy_perform(curl);
    /* Check for errors */
    if (res_code != CURLE_OK) {
        fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res_code));
        curl_easy_cleanup(curl);
        return "";
    }
    curl_easy_cleanup(curl);
    return response;
}

//向流媒体新开一个rtp服务器
tuple<int, int, string> openRtpPort(const string& arg_sercret, const string& http_ip, const int& http_port, 
    int startPort, string ssrc)
{
    static int rtp_port = startPort;
    //string media_secret = "secret:" + arg_sercret;
    //string media_port = "port:0";
    //string media_enable_tcp = "enable_tcp:1";
    //string media_stream_id = "stream_id:2";
    int code = -1, Rtpport = 0;
    string message = "";
    string url = "http://" + http_ip + ":" + std::to_string(http_port) + "/index/api/openRtpServer";
    url += "?secret=" + arg_sercret + "&port="+ std::to_string(rtp_port) +"&enable_tcp=1&stream_id=" + ssrc;

    cout << "openRtpPort-url:" << url << endl;

    string response = getUrlInfo(url);
    if (response != "") {
        //处理json数据解析
        Document document;
        document.Parse(response.c_str());

        auto iter = document.FindMember("code");
        if (iter != document.MemberEnd())
        {
            code = iter->value.GetInt();
        }

        iter = document.FindMember("port");
        if (iter != document.MemberEnd())
        {
            Rtpport = iter->value.GetInt();
        }

        iter = document.FindMember("msg");
        if (iter != document.MemberEnd())
        {
            message = iter->value.GetString();
        }

        ++rtp_port;
        ++rtp_port;
    }

    return std::make_tuple(code, Rtpport, message);
}

/*
*   功能：作为GB28181客户端，启动ps-rtp推流，支持rtp/udp方式；该接口支持rtsp/rtmp等协议转ps-rtp推流。
    第一次推流失败会直接返回错误，成功一次后，后续失败也将无限重试。
    http://{{defaultVhost}}/index/api/startSendRtp?secret={{secret}}&vhost={{defaultVhost}}&app=live
    &stream=test&ssrc=0000000002&dst_url=192.168.5.106&dst_port=49601&is_udp=1&pt=8&use_ps=0&only_audio=1
    水星摄像头use_ps=1会有嗒嗒嗒的杂音
*/
tuple<int, int, string> getStartSendRtpPort(const string& arg_sercret, const string& http_ip, const int& http_port, 
    const string& ssrc, string& dst_url, string& dst_port, string& streamName, int src_port, int type) {
    int code = -1, local_port = 0;
    string message = "";

    //udp方式只支持内网对讲
    string url = "";
    if(type == 2){
        url = "http://" + http_ip + ":" + std::to_string(http_port) + "/index/api/startSendRtp";
        url += "?secret=" + arg_sercret + "&vhost="+ http_ip +"&app=audio&stream="+ streamName +"&ssrc=" + ssrc + "&dst_url=" +
            dst_url + "&dst_port=" + dst_port + "&is_udp=1&src_port="+ std::to_string(src_port) +"&pt=8&use_ps=0&only_audio=1";
    }else{
        //tcp对讲
        url = "http://" + http_ip + ":" + std::to_string(http_port) + "/index/api/startSendRtpPassive";
        url += "?secret=" + arg_sercret + "&vhost="+ http_ip +"&app=audio&stream="+ streamName +"&ssrc=" + ssrc + 
        "&src_port="+ std::to_string(src_port) +"&pt=8&use_ps=0&only_audio=1";
    }

    cout << "getStartSendRtpPort-url:" << url << endl;

    string response = getUrlInfo(url);
    if (response != "") {
        //处理json数据解析
        Document document;
        document.Parse(response.c_str());

        auto iter = document.FindMember("code");
        if (iter != document.MemberEnd())
        {
            code = iter->value.GetInt();
        }

        iter = document.FindMember("local_port");
        if (iter != document.MemberEnd())
        {
            local_port = iter->value.GetInt();
        }

        iter = document.FindMember("msg");
        if (iter != document.MemberEnd())
        {
            message = iter->value.GetString();
        }
    }

    return std::make_tuple(code, local_port, message);
}

/*
* 转换sdp数据 用于获取相应的东西:音频端口号，音频参数，rtp参数，y参数，f参数
* 例：const char* sdp = 
		"v=0\r\n"
		"o=34020000001320000002 0 0 IN IP4 192.168.5.106\r\n"
		"s=Play\r\n"
		"c=IN IP4 192.168.5.106\r\n"
		"t=0 0\r\n"
		"m=audio 43639 TCP/RTP/AVP 8\r\n"
		"a=recvonly\r\n"
		"a=rtpmap:8 PCMA/8000\r\n"
		"a=setup:active\r\n"
		"a=connection:new\r\n"
		"y=0000000034\r\n"
		"f=v/////a/1/8/1\r\n";

    输出：  audio_port:43639
            audio_params:TCP/RTP/AVP 8
            rtp_params:a=rtpmap:8 PCMA/8000
            setup_params:a=setup:active
            y_params:0000000034
            f_params:f=v/a/1/8/1
            sid:34020000001320000002
            ip:192.168.5.106
*/
tuple<string, string, string, string, string, string, string, string> transferSDP(const char* sdp) {
    string audio_port, audio_params, rtp_params, setup_params, y_params, f_params, sid, ip;
    char temp_line[128] = { 0 };
    char temp[32] = { 0 };
    int i = 0, start_i = 0;
    int j = 0, start_j = 0, j_index = 0;
    while (sdp[i] != '\0') {
        if (sdp[i] != '\n' && sdp[i] != '\r') {
            ++i;
        }
        else {
            if (i == start_i) {
                ++i;
                ++start_i;
                continue;
            }

            memset(temp_line, '\0', sizeof(temp_line));
            memcpy(temp_line, sdp + start_i, i - start_i);
            //cout << temp_line;

            if (temp_line[0] == 'm') {
                while (temp_line[j] != '\0') {
                    if (temp_line[j] != ' ') {
                        ++j;
                    }
                    else {

                        memset(temp, '\0', sizeof(temp));
                        memcpy(temp, temp_line + start_j, j - start_j);

                        if (j_index == 1) {
                            audio_port = temp;
                        }
                        else if (j_index > 1) {
                            audio_params += temp;
                            audio_params += " ";
                        }

                        //cout << temp << endl;
                        ++j;
                        start_j = j;
                        ++j_index;
                    }
                }

                //最后一截
                memset(temp, '\0', sizeof(temp));
                memcpy(temp, temp_line + start_j, j - start_j);
                audio_params += temp;
                //cout << temp << endl;
                j = 0;	//重置
                j_index = 0;
                start_j = 0;

            }
            else if (temp_line[0] == 'o') {
                while (temp_line[j] != '\0') {
                    if (temp_line[j] != ' ') {
                        ++j;
                    }
                    else {

                        memset(temp, '\0', sizeof(temp));
                        memcpy(temp, temp_line + start_j, j - start_j);

                        if (j_index == 0) {
                            //去除o=
                            sid = &temp[2];
                        }

                        //cout << temp << endl;
                        ++j;
                        start_j = j;
                        ++j_index;
                    }
                }

                //最后一截
                memset(temp, '\0', sizeof(temp));
                memcpy(temp, temp_line + start_j, j - start_j);
                ip = temp;
                //cout << temp << endl;
                j = 0;	//重置
                j_index = 0;
                start_j = 0;
            }
            else if (temp_line[0] == 'a') {
                if (temp_line[2] == 'r' && temp_line[3] == 't' && temp_line[4] == 'p') {
                    while (temp_line[j] != '\0') {
                        rtp_params += temp_line[j];
                        ++j;
                    }
                    j = 0;
                }
                else if (temp_line[2] == 's' && temp_line[3] == 'e' && temp_line[4] == 't') {
                    while (temp_line[j] != '\0') {
                        setup_params += temp_line[j];
                        ++j;
                    }
                    j = 0;
                }
            }
            else if (temp_line[0] == 'y') {
                while (temp_line[j] != '\0') {
                    if (temp_line[j] == 'y' || temp_line[j] == '=') {
                        ++j;
                        continue;
                    }
                    y_params += temp_line[j];
                    ++j;
                }
                j = 0;
            }
            else if (temp_line[0] == 'f') {
                while (temp_line[j] != '\0') {
                    f_params += temp_line[j];
                    ++j;
                }
                j = 0;
            }
            ++i;
            start_i = i;	//start_i从i下一个pos开始算
        }
    }
    return std::make_tuple(audio_port, audio_params, rtp_params, setup_params, y_params, f_params, sid, ip);
}

bool check_valid(int y, int m, int d)	// 判断日期是否合法
{
    int days[] = { -1, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    if (!m || m > 12) return false;
    if (!d) return false;
    if (m != 2)
    {
        if (d > days[m]) return false;
    }
    else
    {
        int leap = (y % 400 == 0) || (y % 4 == 0 && y % 100 != 0);
        if (d > 28 + leap) return false;
    }
    return true;
}

int callback_websocket(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{
    MyLog* mylog = MyLog::getInstance();
    mylog->root.info("websocket reason:%d", reason);

    switch (reason) {
        case LWS_CALLBACK_ESTABLISHED:
            mylog->root.info("WebSocket connection established");
            break;
        case LWS_CALLBACK_RECEIVE:
            mylog->root.info("Received data: %s", (char *)in);
            break;
        case LWS_CALLBACK_CLOSED:
            mylog->root.info("WebSocket connection closed");
            break;
        //我测试的网页websocket和http第一次都会从这里进
        case LWS_CALLBACK_EVENT_WAIT_CANCELLED:
            //mylog->root.info("这里进了");
            break;
        default:
            break;
    }
    return 0;
}

std::map<std::string, std::string> getJsonInfo(const char* buff_data, int data_len)
{
    std::map<std::string, std::string> data;
    std::string key = "";
    std::string value = "";
    std::string value2 = "";
    std::string json_type = "key";
    for(int i = 0; i < data_len; i ++){
        if(buff_data[i] == '{' || buff_data[i] == ' '){
            //判断是否为value为字符串时的中间位置 在这种情况下就是图片字符串中的字符要给value+= 下同
            if(value != "" && value[0] == '"' && value[value.length()-1] != '"'){
                value += buff_data[i];
            }else{
                continue;
            }

        }else if(buff_data[i] == ':'){
            //判断:是否为value为字符串时的中间位置 在这种情况下要给value+=
            if(value != "" && value[0] == '"' && value[value.length()-1] != '"'){
                value += buff_data[i];
            }else{
                json_type = "value";
            }

        }else if(buff_data[i] == ',' || buff_data[i] == '}'){
            //判断是否为value为字符串时的中间位置 在这种情况下就是图片字符串中的字符要给value+=
            if(value != "" && value[0] == '"' && value[value.length()-1] != '"'){
                value += buff_data[i];
                continue;
            }

            //这里需要处理一对数据了
            //key需要把string去掉
            if(key.length() == 0){
                key = "";
                value = "";
                json_type = "key";
                continue;
            }
            key.erase(std::remove(key.begin(), key.end(), '"'), key.end());
            //value也需要去掉string,但去掉之前需要判断这是字符串还是整型或浮点
            //value是字符串
            if(key.length() == 0){
                value2 = "";
            }else if(value[0] == '"'){
                value.erase(std::remove(value.begin(), value.end(), '"'), value.end());
                value2 = value;
            }else{
                value2 = value;
            }
            data.emplace(key, value2);

            key = "";
            value = "";
            json_type = "key";
        }else{
            //当下的字符是key
            if(json_type == "key"){
                key += buff_data[i];
            }else{
                //当下的字符是value
                value += buff_data[i];
            }
        }
    }

    return data;
}

std::string json2str(const Json::Value& v, bool needFormat)
{
    if (needFormat)
    {
        Json::StreamWriterBuilder writer;
        std::string jsonString = Json::writeString(writer, v);
        return jsonString;
    }
    else
    {
        Json::FastWriter fastWriter;
        return fastWriter.write(v);
    }
}

bool str2json(const std::string& str, Json::Value& v)
{
    Json::CharReaderBuilder readerBuilder;
    std::istringstream iss(str);

    std::string errs;
    bool parsingSuccessful = Json::parseFromStream(readerBuilder, iss, &v, &errs);
    if (!parsingSuccessful)
    {
        std::cout << "Failed to parse JSON: " << errs << std::endl;
        return false;
    }

    return true;
}

void send_http(lws *wsi, unsigned char* response_body, int body_len)
{
    // 设置 HTTP 响应头 chatgpt好用！
    const char *response_header = "HTTP/1.1 200 OK\r\n"
                            "Content-Type: application/json\r\n"
                            "Connection: close\r\n\r\n";
    lws_write(wsi, (unsigned char *)response_header, strlen(response_header), LWS_WRITE_HTTP_HEADERS);
    // 设置 HTTP 响应内容
    lws_write(wsi, response_body, body_len, LWS_WRITE_HTTP);
}

//这个函数用于接收websocket或者http来的消息，一次请求会调用多次
int callback_http(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{
    static MyLog* mylog = MyLog::getInstance();
    static MyConfig* myconfig = MyConfig::getInstance();
    static std::vector<struct lws *> ClientVector;
    //printf("http reason:%d\n", reason);
    mylog->root.info("http reason:%d", reason);

    //这是一个偏移量 也不知道偏移量里的内容有什么用 反正我没有用到 感觉可以去掉
    int pianyi = LWS_PRE;

    //这里buf有两种情况，一种是http的url和websockets的简短消息，这类消息在memcpy后要加上\0,所有是len+1
    //另一种情况是http_body 可能会分包，这时候就得用static变量存储
    char buf[LWS_PRE + len + 1];
    static char * buf_body = nullptr;
    static long long buf_body_len = 0;
    static char* bufurl = nullptr;    //保存这次的url,比如发的是http://192.168.239.128:12345/facefacef,那么值就是/facefacef 监权用
    static std::mutex m_mutex;
    //

    //printf("http wsi:%s\n", *wsi);

    switch (reason) {
        //新的websocket
        case LWS_CALLBACK_ESTABLISHED:
            //lwsl_user("Client connected\n");
            //printf("0 Client connected\n");
            ClientVector.emplace_back(wsi);
            mylog->root.info("new websocketed conneted, 现在一共有客户端数量为%d", ClientVector.size());
            break;
        //websocket消息过来 比如心跳
        case LWS_CALLBACK_RECEIVE: {
            memset(&buf[LWS_PRE], '\0', len);
            memcpy(&buf[LWS_PRE], in, len);
            buf[LWS_PRE + len] = '\0';
            lws_write(wsi, reinterpret_cast<unsigned char *>(&buf[LWS_PRE]), len, LWS_WRITE_TEXT);
            //lwsl_user("Received message: %s\n", &buf[LWS_PRE]);
            mylog->root.info("收到websocket消息：%s", &buf[LWS_PRE]);
            //printf("6 Received message: %s\n", &buf[LWS_PRE]);
            break;
        }

        //18 这里能收到url get 参数
        case LWS_CALLBACK_FILTER_HTTP_CONNECTION:
            // memset(&buf[LWS_PRE], '\0', sizeof(char)*(LWS_PRE+len));
            memcpy(&buf[LWS_PRE], in, len);
            //printf("18 Received message: %s\n", &buf[LWS_PRE]);
            mylog->root.info("18 Received message: %s", &buf[LWS_PRE]);
            break;

        //49
        case LWS_CALLBACK_HTTP_BIND_PROTOCOL:
            memset(&buf[LWS_PRE], '\0', len);
            memcpy(&buf[LWS_PRE], in, len);
            buf[LWS_PRE + len] = '\0';
            //printf("49 Received message: %s\n", &buf[LWS_PRE]);
            mylog->root.info("49 Received message: %s", &buf[LWS_PRE]);
            break;

        //12 这里跟18一样 也可以判断url
        case LWS_CALLBACK_HTTP:
            memcpy(&buf[LWS_PRE], in, len);
            buf[LWS_PRE + len] = '\0';
            //printf("12 Received message: %s\n", &buf[LWS_PRE]);

            bufurl = (char *)malloc(len);
            memcpy(bufurl, in, len);                  //保存http url
            bufurl[len] = '\0';
            //printf("bufurl: %s\n", bufurl);
            mylog->root.info("bufurl: %s", bufurl);
            //lws_callback_on_writable(wsi);

            break;

        //13 收httpbody也就是json 有可能多条
        case LWS_CALLBACK_HTTP_BODY:
            {
                /*
                * 以下代码会在第三次进来也就是第二次realloc的时候报realloc(): invalid next size错误， 查了一下说是空间上realloc不能和malloc相差太大
                if(buf_body == nullptr){
                    buf_body = (char *)malloc(len*sizeof(char));
                }else{
                    buf_body = (char *)realloc(buf_body, len*sizeof(char));
                }
                */

                char *currentBuf = (char *)malloc((buf_body_len+len)*sizeof(char));
                //第一次进来
                if(buf_body == nullptr){
                    memcpy(currentBuf, in, len);
                }else{
                    //非第一次
                    memcpy(currentBuf, buf_body, buf_body_len);
                    memcpy(currentBuf + buf_body_len, in, len);
                    free(buf_body);
                }
                buf_body = currentBuf;
                buf_body_len += len;
                
                memcpy(buf_body + buf_body_len - len, in, len);
                memset(&buf[LWS_PRE], '\0', len);
                memcpy(&buf[LWS_PRE], in, len);
                //printf("buf_body size: %lld\n", buf_body_len);
                //printf("13 Received message: %s\n", &buf[LWS_PRE]);
                //printf("buf_body: %s\n", buf_body);
            }
            break;

        //14 表示httpbody收完了 需要做三件事，一是返回http客户端，二是存数据库，三是群发websocket客户端
        case LWS_CALLBACK_HTTP_BODY_COMPLETION:
            {
                //printf("test-delay\n");
                //std::this_thread::sleep_for(std::chrono::seconds(300));
                //printf("buf_body: %s\n", buf_body);
                std::map<std::string, std::string> data;
                Json::Value jsonData;
                std::map<std::string, std::map<std::string, std::string>> configInfo = myconfig->getConfigInfo();
                std::string loginurl = configInfo["url"]["loginurl"];
                std::string refreshurl = configInfo["url"]["refreshurl"];
                std::string talkurl = configInfo["url"]["talkurl"];
                std::string stopTalkurl = configInfo["url"]["stopTalkurl"];
                //url不匹配
                {
                    if(strcmp(bufurl, loginurl.c_str()) && strcmp(bufurl, refreshurl.c_str()) 
                        && strcmp(bufurl, talkurl.c_str()) && strcmp(bufurl, stopTalkurl.c_str())){
                        
                        //printf("url不匹配, 当前url:%s, 正确的url:%s", bufurl, loginurl);
                        string msg = "url不匹配";
                        mylog->root.error("%s, 当前url:%s, 正确的url:%s", msg.c_str(), bufurl, loginurl.c_str());
                        char response_body[128] = {0};
                        sprintf(response_body, "{\"code\":-1, \"msg\":\"%s\"}", msg.c_str());
                        send_http(wsi, (unsigned char *)response_body, strlen(response_body));

                        goto __EXIT;
                    }
                }

                if(buf_body_len == 0){

                    string msg = "http body没收到数据";
                    mylog->root.error("%s", msg.c_str());
                    char response_body[128] = {0};
                    sprintf(response_body, "{\"code\":-2, \"msg\":\"%s\"}", msg.c_str());
                    send_http(wsi, (unsigned char *)response_body, strlen(response_body));

                    goto __EXIT;
                }
                
                //自己解析json
                //data = getJsonInfo(buf_body, buf_body_len);
                str2json(buf_body, jsonData);

                //接下来存数据库+//群发websocket
                {
                    std::lock_guard<std::mutex> mylockguard(m_mutex);
                    //登陆
                    if(!strcmp(bufurl, loginurl.c_str())){
                        if(jsonData["account"].asString() == "" || jsonData["password"].asString() == ""){

                            string msg = "没有帐号密码信息";
                            mylog->root.error("%s", msg.c_str());
                            char response_body[128] = {0};
                            sprintf(response_body, "{\"code\":-3, \"msg\":\"%s\"}", msg.c_str());
                            send_http(wsi, (unsigned char *)response_body, strlen(response_body));

                            goto __EXIT;
                        }

                        std::string account = jsonData["account"].asString();
                        std::string password = jsonData["password"].asString();
                        auto act_pwd = configInfo["act-pwd"];
                        if(act_pwd.find(account) != act_pwd.end() && act_pwd[account] == password){
                            std::string msg = "帐号密码验证成功";
                            mylog->root.info(msg);
                            char response_body[128] = {0};
                            sprintf(response_body, "{\"code\":200, \"msg\":\"%s\"}", msg.c_str());
                            send_http(wsi, (unsigned char *)response_body, strlen(response_body));

                            goto __EXIT;
                        }else{

                            string msg = "帐号或密码不正确";
                            mylog->root.error("%s", msg.c_str());
                            char response_body[128] = {0};
                            sprintf(response_body, "{\"code\":-4, \"msg\":\"%s\"}", msg.c_str());
                            send_http(wsi, (unsigned char *)response_body, strlen(response_body));    

                            goto __EXIT;   
                        }
                    }else if(!strcmp(bufurl, refreshurl.c_str())){
                        SipServer* sipServer = SipServer::getInstance();
                        std::map<std::string, Client*> clientInfo = sipServer->getClientInfo();
                        MyConfig* myconfig = MyConfig::getInstance();
                        auto configInfo = myconfig->getConfigInfo();
                        {
                            rapidjson::StringBuffer json_buffer;
                            rapidjson::Writer<rapidjson::StringBuffer> json_writer(json_buffer);
                            json_writer.StartObject();
                            json_writer.Key("code");
                            json_writer.Int(200);

                            json_writer.Key("activeList");
                            json_writer.StartArray();
                            if(!clientInfo.empty())
                            {
                                for(auto i:clientInfo)
                                {
                                    json_writer.String(configInfo["device-ssrc"][i.first].c_str());
                                }  
                            }
                            json_writer.EndArray();

                            json_writer.Key("totalList");
                            json_writer.StartArray();
                            for(auto i:configInfo["device-ssrc"])
                            {
                                json_writer.String(i.second.c_str());
                            }
                            json_writer.EndArray();
                            json_writer.EndObject();
                        
                            // 生成 JSON 字符串
                            const char* json_string = json_buffer.GetString();
                            mylog->root.info(json_string);
                            send_http(wsi, (unsigned char *)json_string, strlen(json_string));

                            goto __EXIT;
                        }
                    }else if(!strcmp(bufurl, talkurl.c_str())){
                        //这里开启对讲
                        SipServer* sipServer = SipServer::getInstance();
                        if(jsonData["ssrc"].asString() == ""){

                            string msg = "没有ssrc信息";
                            mylog->root.error("%s", msg.c_str());
                            char response_body[128] = {0};
                            sprintf(response_body, "{\"code\":-3, \"msg\":\"%s\"}", msg.c_str());
                            send_http(wsi, (unsigned char *)response_body, strlen(response_body));

                            goto __EXIT;
                        }

                        std::string ssrc = jsonData["ssrc"].asString();
                        std::string streamName = jsonData["streamName"].asString();
                        std::string deviceId = sipServer->getDeviceIDBySSRC(ssrc);
                        if(deviceId == "" || streamName == "" || sipServer->getClientByDevice(deviceId.c_str()) == nullptr){
                            string msg = "ssrc或stream信息有误,找不到设备";
                            mylog->root.error("%s", msg.c_str());
                            char response_body[128] = {0};
                            sprintf(response_body, "{\"code\":-4, \"msg\":\"%s\"}", msg.c_str());
                            send_http(wsi, (unsigned char *)response_body, strlen(response_body));

                            goto __EXIT;                   
                        }
                        Client* currentClient = sipServer->getClientByDevice(deviceId.c_str());
                        //判断是不是已经有人在对讲了
                        auto inviteInfo = currentClient->getInviteInfo(2);
                        int cid = std::get<0>(inviteInfo);
                        int did = std::get<1>(inviteInfo);
                        if(cid != 0 || did != 0){
                            // 这个功能还需要配合实现 客户端主动关闭进程后给服务器传递消息
                            string msg = "此设备已经有别人在对讲了";
                            mylog->root.error("%s", msg.c_str());
                            char response_body[128] = {0};
                            sprintf(response_body, "{\"code\":-5, \"msg\":\"%s\"}", msg.c_str());
                            send_http(wsi, (unsigned char *)response_body, strlen(response_body));
                            goto __EXIT;   
                        }
                        
                        //发送邀请
                        currentClient->setInviteInfo(2, 0, 0, streamName);
                        sipServer->request_message(currentClient);
                        std::string msg = "邀请对讲成功";
                        mylog->root.info(msg);

                        //等待另一个sip线程处理完消息
                        //std::this_thread::sleep_for(std::chrono::seconds(3));
                        int maxTime = 10;
                        int startTime = 0;
                        inviteInfo = currentClient->getInviteInfo(2);
                        while(!std::get<0>(inviteInfo) && !std::get<1>(inviteInfo) && startTime < maxTime){
                            std::this_thread::sleep_for(std::chrono::seconds(1));
                            ++startTime;
                            inviteInfo = currentClient->getInviteInfo(2);
                            cout<<"func.cpp-inviteInfo2:"<<std::get<0>(inviteInfo)<<" "<<std::get<1>(inviteInfo)<<endl;
                        }

                        //cid did都是0 对讲没成功
                        if(!std::get<0>(inviteInfo) && !std::get<1>(inviteInfo)){
                            char response_body[128] = {0};
                            string msg = "对讲没成功";
                            sprintf(response_body, "{\"code\":-6, \"msg\":\"%s\"}", msg.c_str());
                            send_http(wsi, (unsigned char *)response_body, strlen(response_body));
                        }else{
                            char response_body[128] = {0};
                            string msg = "成功开启对讲";
                            sprintf(response_body, "{\"code\":200, \"msg\":\"%s\"}", msg.c_str());
                            send_http(wsi, (unsigned char *)response_body, strlen(response_body));
                        }

                        goto __EXIT;
                    }else if(!strcmp(bufurl, stopTalkurl.c_str())){
                        const Json::Value ssrcList = jsonData["ssrcList"];
                        int type = jsonData["type"].asInt();
                        if(ssrcList.size() == 0){

                            string msg = "没有ssrcList信息";
                            mylog->root.error("%s", msg.c_str());
                            char response_body[128] = {0};
                            sprintf(response_body, "{\"code\":-3, \"msg\":\"%s\"}", msg.c_str());
                            send_http(wsi, (unsigned char *)response_body, strlen(response_body));

                            goto __EXIT;
                        }
                        //依次关闭对讲
                        SipServer* sipServer = SipServer::getInstance();
                        for (unsigned int i = 0; i < ssrcList.size(); i++)
                        {
                            std::string ssrc = ssrcList[i].toStyledString();
                            ssrc = ssrc.substr(1, ssrc.length() - 3);   //去掉首尾“和尾部换行
                            sipServer->request_bye(2, ssrc, type);
                            mylog->root.info("ssrcList[%d]:%s", i, ssrc.c_str());
                            //cout << "ssrcList[" << i << "] = " << ssrcList[i] << endl;
                        }

                        char response_body[128] = {0};
                        string msg = "依次关闭了对讲";
                        sprintf(response_body, "{\"code\":200, \"msg\":\"%s\"}", msg.c_str());
                        send_http(wsi, (unsigned char *)response_body, strlen(response_body));
                    }

                    /*
                    std::map<std::string, std::map<std::string, std::string>> configInfo = myconfig->getConfigInfo();
                    std::string servername = configInfo["mysql"]["servername"];
                    std::string username = configInfo["mysql"]["username"];
                    std::string password = configInfo["mysql"]["password"];
                    std::string dbname = configInfo["mysql"]["dbname"];
                    std::string port = configInfo["mysql"]["port"];
                    MYSQL *con = NULL;
                    con = mysql_init(con);
                    if(NULL == mysql_real_connect(con, servername.c_str(), username.c_str(), password.c_str(), dbname.c_str(), atoi(port.c_str()), NULL, 0)){
                            //printf("数据库插入失败\n");
                            mylog->root.error("数据库连接失败");
                            // 设置 HTTP 响应头 chatgpt好用！
                            const char *response_header = "HTTP/1.1 200 OK\r\n"
                                                    "Content-Type: text/plain\r\n"
                                                    "Connection: close\r\n\r\n";
                            lws_write(wsi, (unsigned char *)response_header, strlen(response_header), LWS_WRITE_HTTP_HEADERS);
                            // 设置 HTTP 响应内容
                            const char *response_body = "{\"code\":-5, \"msg\":\"数据库连接失败\"}";
                            lws_write(wsi, (unsigned char *)response_body, strlen(response_body), LWS_WRITE_HTTP);

                            goto __EXIT;
                    }
                    mysql_set_character_set(con, "utf8");

                    char sql[1024] = {0};
                    sprintf(sql, "");

                    //printf("sql: %s\n", sql);
                    try{
                        if(mysql_query(con, sql)){
                            //printf("数据库插入失败\n");
                            mylog->root.error("数据库插入失败");
                            // 设置 HTTP 响应头 chatgpt好用！
                            const char *response_header = "HTTP/1.1 200 OK\r\n"
                                                    "Content-Type: text/plain\r\n"
                                                    "Connection: close\r\n\r\n";
                            lws_write(wsi, (unsigned char *)response_header, strlen(response_header), LWS_WRITE_HTTP_HEADERS);
                            // 设置 HTTP 响应内容
                            const char *response_body = "{\"code\":-4, \"msg\":\"数据库插入失败\"}";
                            lws_write(wsi, (unsigned char *)response_body, strlen(response_body), LWS_WRITE_HTTP);

                            goto __EXIT;

                        }
                    }catch(std::exception &e){
                        std::cout << e.what() << std::endl;
                    }

                    //printf("数据库插入成功\n");
                    mylog->root.info("数据库插入成功");

                    free(sql);
                    mysql_close(con);
                    */

                    //群发websocket
                    /*
                    char s[1024] = {};
                    sprintf(s, "{\"msg\": \"htmlJump\", \"user_subsidy_id\": }");
                    for(auto i: ClientVector){
                        lws_write(i, reinterpret_cast<unsigned char *>(s), strlen(s), LWS_WRITE_TEXT);
                        //printf("%x:websocket 发送\n", i);
                        mylog->root.info("%x:websocket 发送", i);
                    }
                    */

                }

                //往http客户端返回消息并关闭链接
                {
                    //这种方式也可以发送但是会自动加上html标签和head等标签
                    // const char *response = "yes, it's ok";
                    // lws_return_http_status(wsi, HTTP_STATUS_OK, response);
                    // printf("http返回并主动关闭：%s\n", response);

                }

                __EXIT:
                    if(bufurl != nullptr)       free(bufurl);
                    if(buf_body != nullptr)     free(buf_body);
                    bufurl = nullptr;
                    buf_body = nullptr;
                    buf_body_len = 0;
                    //printf("释放所有buf\n");
                    mylog->root.info("释放所有buf");
                    //感谢gpt 这里是关闭链接的意思  如果不主动关闭，客户端长连的话 那下一次客户端发过来就没有12和18的消息了，也就没有url了
                    lws_close_free_wsi(wsi, LWS_CLOSE_STATUS_NOSTATUS, nullptr);
            }
            break;

        //http退出 其实上面14的应该写在这里，如果post没有东西的话 是收不到13和14的
        case LWS_CALLBACK_CLOSED_HTTP:

            break;

        //30 websocket和http断开都走这里     
        case LWS_CALLBACK_WSI_DESTROY:
            {
                auto iter = find(begin(ClientVector), end(ClientVector), wsi);
                if(iter != end(ClientVector)){
                    //printf("有一个websocket退出了\n");
                    mylog->root.info("%x:websocket退出了 现在一共有客户端数量为%d", iter, ClientVector.size());
                    ClientVector.erase(iter);
                }
                //不知道为什么调用这个函数会宕机，查了文档例程的
                //lws_close_reason(wsi, LWS_CLOSE_STATUS_NOSTATUS, nullptr, 0);  
            }
            break;
        
        default:
            break;
    }
    return 0;
}

int _System(const std::string cmd, std::string &output)
{
    FILE * fp;
    int res = -1;
    if ((fp = popen(cmd.c_str(), "r") ) == NULL)
    {
        printf("Popen Error!\n");
        return -2;
    }
    else
    {
        char pRetMsg[10240]={0};
        //get lastest result
        while(fgets(pRetMsg,10240, fp) != NULL)
        {
            output+=pRetMsg;
        }

        if ( (res = pclose(fp)) == -1)
        {
            printf("close popenerror!\n");
            return -3;
        }
        return 0;
    }
}

string get_local_ip_using_ifconf()  
{
    char str_ip[128] = {0};
    int sock_fd, intrface;
    struct ifreq buf[INET_ADDRSTRLEN];
    struct ifconf ifc;
    char *local_ip = NULL;
    int status = RUN_FAIL;

    if ((sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) >= 0)
    {
        ifc.ifc_len = sizeof(buf);
        ifc.ifc_buf = (caddr_t)buf;
        if (!ioctl(sock_fd, SIOCGIFCONF, (char *)&ifc))
        {
            intrface = ifc.ifc_len/sizeof(struct ifreq);
            while (intrface-- > 0)
            {
                if (!(ioctl(sock_fd, SIOCGIFADDR, (char *)&buf[intrface])))
                {
                    local_ip = NULL;
                    local_ip = inet_ntoa(((struct sockaddr_in*)(&buf[intrface].ifr_addr))->sin_addr);
                    if(local_ip)
                    {
                        strcpy(str_ip, local_ip);
                        status = RUN_SUCCESS;
                        if(strcmp("127.0.0.1", str_ip))
                        {
                            break;
                        }
                    }

                }
            }
        }
        close(sock_fd);
    }

    return string(str_ip);
}