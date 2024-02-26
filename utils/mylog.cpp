#include "mylog.h"

MyLog* MyLog::instance = nullptr;

MyLog* MyLog::getInstance()
{
    if(MyLog::instance == nullptr){
        MyLog::instance = new MyLog();
        MyLog::instance->pLayout1 = new log4cpp::PatternLayout();
        MyLog::instance->pLayout2 = new log4cpp::PatternLayout();
        MyLog::instance->rollfileAppender = new log4cpp::RollingFileAppender(
                        "rollfileAppender","log_rollwxb.log",5*1024,1); // 超过5k自动回滚，最大文件数为1
                //控制台输出  因为打印非常占IO 所以一般大型程序用tringQueueAppender 原理就是先保存内存中 完了在特定地方一次全打印出来，这里只用OstreamAppender就行了
        MyLog::instance->osAppender = new log4cpp::OstreamAppender("osAppender", &std::cout);  
        MyLog::instance->pLayout1->setConversionPattern("%d: %p %c%x: %m%n");
        MyLog::instance->pLayout2->setConversionPattern("%d: %p %c%x: %m%n");
        MyLog::instance->rollfileAppender->setLayout(MyLog::instance->pLayout1);
        MyLog::instance->osAppender->setLayout(MyLog::instance->pLayout2);
        MyLog::instance->root.addAppender(MyLog::instance->rollfileAppender); //这个好像是常驻内存 最好不要放多次调用的函数里
        MyLog::instance->root.addAppender(MyLog::instance->osAppender);
        MyLog::instance->root.setPriority(log4cpp::Priority::DEBUG);

    }
    return MyLog::instance;
}

MyLog::MyLog()
{

}

//log4cpp的东西会自动处理
MyLog::~MyLog()
{
    delete MyLog::instance;
}

//这个方法有问题
bool MyLog::addOsAppender()
{
    log4cpp::AppenderSet a = root.getAllAppenders();
    for(auto i:a){
        if(i->getName() == "osAppender"){
            return false;
        }
    }
    //这里不管是不是重新生成 都会报错 真不知道怎么写了
    MyLog::instance->osAppender = new log4cpp::OstreamAppender("osAppender", &std::cout);  
    MyLog::instance->osAppender->setLayout(MyLog::instance->pLayout2);
    MyLog::instance->root.addAppender(MyLog::instance->osAppender);
    a = root.getAllAppenders();
    for(auto i:a){
        std::cout<<i->getName()<<std::endl;
    }
    return true;
}

//这里删除了添加后报错
bool MyLog::removeOsAppender()
{
    log4cpp::AppenderSet a = root.getAllAppenders();
    for(auto i:a){
        if(i->getName() == "osAppender"){
            root.removeAppender(MyLog::instance->osAppender);
            return true;
        }
    }
    return false;
}