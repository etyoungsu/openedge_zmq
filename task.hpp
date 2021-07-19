
/**
 * @file    task.hpp
 * @brief   Task abstraction class
 * @author  Byunghun Hwang <bh.hwang@iae.re.kr>
 */

#ifndef _OPENEDGE_CORE_TASK_HPP_
#define _OPENEDGE_CORE_TASK_HPP_

#include <string>
#include <map>
#include <vector>
#include <memory>
#include <openedge/core/profile.hpp>
#include <3rdparty/jsonrpccxx/iclientconnector.hpp>
#include <3rdparty/jsonrpccxx/server.hpp>


#include <cxxabi.h>
inline const char* __demangle__(const char* name){
    int status;
    char* demangled = abi::__cxa_demangle(name, 0, 0, &status);
    std::string str(demangled);
    return str.c_str();
}
#define _THIS_COMPONENT_ __demangle__((const char*)typeid(*this).name()).get()

using namespace std;

namespace oe {
    namespace core { class iService; }
    namespace core::task {
            
            /**
             * @brief   runnable class interface for task
             */
            class runnable {

                friend class oe::core::task::driver;
                
                public:
                    enum class Status : int { STOPPED=0, RUNNING, PAUSED };

                    virtual Status getStatus() { return status; }
                    void setStatus(Status s) { status = s;  }
                    
                    //task common interfaces (very important!!!!)
                    virtual void execute() = 0;
                    virtual bool configure() = 0;
                    virtual void cleanup() = 0;
                    virtual void pause() = 0;
                    virtual void resume() = 0;
                    virtual void on_msg(char msg) = 0;

                protected:
                    const core::profile* getProfile() { 
                        assert(_profile!=nullptr);
                        return _profile.get();
                    }

                protected:
                    typedef struct serviceHandle_t {
                        public:
                            void* pfHandle { nullptr }; //component file handler
                            core::iService* pService { nullptr }; //service implementation ptr
                            bool isValid() { return (!pfHandle && !pService); }
                            const string getName() const { return name; }
                            serviceHandle_t(){}
                            serviceHandle_t(const char* svcname):name(svcname){}
                        private:
                            string name {""};

                    } serviceHandle;
                    map<string /*service name*/, serviceHandle /*service handle*/> serviceContainer;

                    enum class fault_type_t : int {
                        CRITICAL = 0,   //drop the all tasks, and show alert
                        IGNORE = 100    //ignore the fault, no actions!
                    };

                    string taskname { "unknown" };
                    Status status { Status::STOPPED };
                    fault_type_t fault_level { fault_type_t::IGNORE };

                private:
                    unique_ptr<core::profile> _profile;
                    bool _check_jitter {false};
                    bool _check_overrun {false};

            }; //class runnable

            /**
             * @brief   Local Service Connector for RPC Client
             */
            class localServiceConnector : public jsonrpccxx::IClientConnector {
            public:
                explicit localServiceConnector(jsonrpccxx::JsonRpcServer &server) : server(server) {}
                std::string Send(const std::string &request) override { return server.HandleRequest(request); }
            private:
                jsonrpccxx::JsonRpcServer &server;
        };
    } //namespace core

    typedef oe::core::task::runnable*(*create_task)(void);
    typedef void(*release_task)(void);

    #define EXPORT_TASK_API extern "C" {oe::core::task::runnable* create(void); void release(void);}

} //namespace oe

#endif