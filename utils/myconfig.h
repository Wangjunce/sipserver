#ifndef CONFIG_H
#define CONFIG_H
#include <map>
#include <string>

//读配置文件或者读命令行配置类  单例模式
class MyConfig{
    public:
        MyConfig(const MyConfig&)=delete;
        MyConfig& operator=(const MyConfig&)=delete;
        static MyConfig* getInstance();
        std::map<std::string, std::string> getConfigMap(std::string);
        std::map<std::string, std::map<std::string, std::string>> getConfigInfo();
        std::string getArgString(char);
        //从配置文件中读
        bool parseFile();
        //从命令行参数中读
        bool parseArg(int argc, char*argv[]);
    private:
        MyConfig();
        ~MyConfig();
        static MyConfig* instance;
        std::map<std::string, std::map<std::string, std::string>> configInfo;
        std::map<char, std::string> argInfo;
};

#endif