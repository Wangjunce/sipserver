#ifndef LOG_H
#define LOG_H

#include <log4cpp/Category.hh>
#include <log4cpp/Appender.hh>
#include <log4cpp/FileAppender.hh>
#include <log4cpp/OstreamAppender.hh>
#include <log4cpp/Priority.hh>
#include <log4cpp/PatternLayout.hh>
#include <log4cpp/RollingFileAppender.hh>
#include <iostream>

//日志单例 拥有一个输出到控制台一个循环输出到文件,可以控制什么时候暂停或者启动OsAppender
class MyLog{
    public:
        MyLog(const MyLog& mylog)=delete;               //禁止拷贝
        MyLog& operator=(const MyLog& mylog)=delete;    //禁止赋值重载
        bool removeOsAppender();
        bool addOsAppender();
        static MyLog* getInstance();
        log4cpp::Category& root = log4cpp::Category::getRoot().getInstance("RootName");
    private:
        MyLog();    //私有构造
        ~MyLog();   //私有析构
        static MyLog* instance;
        
        log4cpp::PatternLayout* pLayout1;
        log4cpp::PatternLayout* pLayout2;
        log4cpp::RollingFileAppender* rollfileAppender;
        log4cpp::OstreamAppender* osAppender;
        
};

#endif