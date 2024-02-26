#pragma once
#include <string>
#include <tuple>
#include <iostream>
#include <libwebsockets.h>
#include "curl/curl.h"
#include "rapidjson/document.h"
#include "json/json.h"
using std::tuple;
using std::string;
using std::cout;
using std::endl;
using namespace rapidjson;

#ifdef WIN32
//UTF-8תUnicode 
std::wstring Utf82Unicode(const std::string& utf8string);
//unicode תΪ ascii 
std::string WideByte2Acsi(std::wstring& wstrcode);
//utf-8 ת ascii 
std::string UTF_82ASCII(std::string& strUtf8Code);
//ascii ת Unicode 
std::wstring Acsi2WideByte(std::string& strascii);
//Unicode ת Utf8 
std::string Unicode2Utf8(const std::wstring& widestring);
//ascii ת Utf8 
std::string ASCII2UTF_8(std::string& strAsciiCode);

std::string UTF8_to_GB2312(const char* utf8);

std::string GB2312_to_UTF8(const char* gb2312);


std::string WChar2Ansi(LPCWSTR pwszSrc);

std::string ws2s(std::wstring& inputws);

std::wstring Ansi2WChar(LPCSTR pszSrc, int nLen);

std::wstring s2ws(const std::string& s);
#endif

std::string time_t2string(const time_t time_t_time);

std::string md5(const std::string& content);

void md5(const std::string& content, std::string& digist);

size_t req_reply(void* ptr, size_t size, size_t nmemb, void* stream);

string getUrlInfo(string url);

tuple<int, int, string> openRtpPort(const string& arg_sercret, const string& http_ip, const int& http_port, 
	int startPort, string ssrc);

tuple<int, int, string> getStartSendRtpPort(const string& arg_sercret, const string& http_ip, const int& http_port,
	const string& ssrc, string& dst_url, string& dst_port, string& streamName, int src_port, int type);

tuple<string, string, string, string, string, string, string, string> transferSDP(const char* sdp);

bool check_valid(int y, int m, int d);

//解析json, 自己写的，能解base64图片 但是不能解数组
std::map<std::string, std::string> getJsonInfo(const char* buff_data, int data_len);

std::string json2str(const Json::Value& v, bool needFormat);
bool str2json(const std::string& str, Json::Value& v);

//websocket的函数：发送http请求、两个回调
void send_http(lws *wsi, unsigned char* response_body, int body_len);
int callback_websocket(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len);
int callback_http(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len);

//读取系统命令
int _System(const std::string cmd, std::string &output);

//获取本纪ip
string get_local_ip_using_ifconf();