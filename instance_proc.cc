
#include "instance.hpp"
#include <fstream>
#include <3rdparty/json.hpp>
#include <3rdparty/spdlog/spdlog.h>
#include "exception.hpp"
#include <vector>
#include <string>
#include <map>
#include "task_manager.hpp"
#include "global.hpp"
#include <sys/sysinfo.h>
#include <openedge/core/task.hpp>
#include <openedge/core/registry.hpp>

using namespace std;
using json = nlohmann::json;

namespace oe::manager {

    bool initialize_proc(){

        spdlog::info("* Process ID = {}", getpid());
        spdlog::info("* System CPUs = {}", get_nprocs());
        spdlog::info("* System Clock Ticks = {}", sysconf(_SC_CLK_TCK));

        

        //getting hostname (default hostname will be set by configuration file)
        char hostname[256] = {0,};
        if(!gethostname(hostname, sizeof(hostname))){
            registry->insert("HOST_NAME", std::make_any<std::string>(hostname));
            spdlog::info("+ Registry : Hostname = {}", hostname);
        }
        else {
            std::string host = CONFIG_HOSTNAME.get<std::string>();
            spdlog::warn("Cannot be recognized the hostname. Default hostname({}) will be set.", host);
        }
            
        //install default tasks
        vector<string> default_tasks = CONFIG_TASKS.get<std::vector<string>>();
        for(string& task:default_tasks){
            edge_task_manager->install(task.c_str());
        }
        spdlog::info("Totally installed : {}", edge_task_manager->size());
    
        return true;
    }

    void msg_recv() {
        edge_task_manager->msg_recv();
    }

    //start edge
    void run(){
        edge_task_manager->run();
    }

    void cleanup(){
        edge_task_manager->uninstall();
    }

} //namespace edge