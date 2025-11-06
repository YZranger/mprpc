#include <iostream>
#include <string>
#include "user.pb.h"
#include "mprpcapplication.h"
#include "rpcprovider.h"
#include "Logger.h"

/**
 * UserService原来是一个本地服务，提供了两个进程内的本地方法，Login和GetFriendLists
 */

class UserService : public fixbug::UserServiceRpc // 使用在rpc服务发布端（rpc服务提供者）
{
public:
    bool Login(std::string name, std::string pwd)
    {
        std::cout << "doing local service : Login" << std::endl;
        std::cout << "name: " << name << "pwd: " << pwd << std::endl;
        return true;
    }

    bool Register(uint32_t id, std::string name, std::string pwd)
    {
        std::cout << "doing local service : Register" << std::endl;
        std::cout << "id: " << id << "name: " << name << "pwd: " << pwd << std::endl;
        return true; 
    }
    /**
     * 重写基类的UserServiceRpc的虚函数，下面这些方法都是框架直接调用的
     * 1. caller ===> Login(LoginRequest) ===> muduo ===> callee
     * 2. callee ===> Login(LoginRequest) ===> 交到下面重写的这个Login方法上  
     */ 
    void Login(::google::protobuf::RpcController* controller,
                       const ::fixbug::LoginRequest* request,
                       ::fixbug::LoginResponse* response,
                       ::google::protobuf::Closure* done)
    {
        // 框架给业务上报了请求参数LoginRequest, 应用取出响应数据做本地业务
        std::string name = request->name();
        std::string pwd = request->pwd();

        // 做本地业务
        bool login_result = Login(name, pwd);
        // 把响应 包括错误码、错误消息、返回值写入
        response->set_success(login_result);
        fixbug::ResultCode* rc = response->mutable_result();
        rc->set_errcode(0);
        rc->set_errmsg("");

        // 执行回调函数，执行响应对象数据的序列化和网络发送（都是由框架完成的）
        done->Run();
    }

    void Register(::google::protobuf::RpcController* controller,
                       const ::fixbug::RegisterRequest* request,
                       ::fixbug::RegisterResponse* response,
                       ::google::protobuf::Closure* done)
    {
        uint32_t id = request->id();
        std::string name = request->name();
        std::string pwd = request->pwd();

        bool ret = Register(id, name, pwd);

        response->mutable_result()->set_errcode(0);
        response->mutable_result()->set_errmsg("");
        response->set_success(ret);

        done->Run();
    }
    
private:
};

int main(int argc, char** argv)
{
    LOG_INFO("first log message!");
    LOG_ERROR("%s:%s:%d", __FILE__, __FUNCTION__, __LINE__);

    // 调用框架的初始化操作
    MprpcApplication::Init(argc, argv);
    
    // provider是一个rpc网络服务对象，把UserService对象发布到rpc节点上
    RpcProvider provider;

    // 把UserService对象发布到rpc节点上
    provider.NotifyService(new UserService);
    
    // 启动一个rpc服务发布节点 Run以后，进程进入阻塞状态，等待远程的rpc调用请求
    provider.Run();

    return 0;
}