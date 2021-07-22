
#include "task_manager.hpp"
#include <openedge/log.hpp>
#include <openedge/core/global.hpp>
#include <openedge/util/validation.hpp>

namespace oe::edge {

    task_manager::task_manager(){

    }

    task_manager::~task_manager(){
        this->uninstall();
    }

    bool task_manager::install(const char* taskname){
        static int ntasks = 0;
        if(ntasks>__TASKS_LIMITS__){
            console::error("Task Container is Full!! (LIMIT={})", __TASKS_LIMITS__);
            return false;
        }

        if(!taskname){
            console::warn("Task was not specified to be installed");
            return false;
        }

        if(!util::exist(taskname)){
            console::error("{} file does not exist", taskname);
            return false;
        }
            
        //insert task into container with uuid
        //(todo) requries file existance in task respository path
        _container_map.insert(taskContainer_map::value_type(taskname, _uuid_gen.generate()));
        _task_container.insert(taskContainer_t::value_type(_container_map[taskname], new core::task::driver(taskname)));

        console::info("Installed {} Component(UUID:{})", taskname, _container_map[taskname].str());

        if(!_task_container[_container_map[taskname]]->configure()){
            uninstall(taskname);
        }
        return true;
    }

    void task_manager::uninstall(const char* taskname){
        if(!taskname){
            if(_task_container.size()>0){
                console::info("Uninstalling all tasks...");
                //all tasks will be uninstalled
                for(taskContainer_t::iterator itr = _task_container.begin(); itr!=_task_container.end();++itr){
                    itr->second->cleanup();
                    delete itr->second;
                }
                _task_container.clear();
                _container_map.clear();
            }
        }
        else {
            console::info("Uninstalling {}...", taskname);
            _task_container[_container_map[taskname]]->cleanup();
            delete _task_container[_container_map[taskname]];
            _task_container.erase(_container_map[taskname]);
            _container_map.erase(taskname);
        }
    }

    void task_manager::run(const char* taskname){
        if(!taskname){
            for(taskContainer_t::iterator itr = _task_container.begin(); itr!=_task_container.end();++itr){
                itr->second->execute();
            }
        }
        else {
            taskContainer_t::iterator itr = _task_container.find(_container_map[taskname]);
            if(itr!=_task_container.end())
                _task_container[_container_map[taskname]]->execute();
            else
                console::warn("{} is already installed", taskname);
        }
        
    }

    void task_manager::msg_recv(const char* taskname){
        if(!taskname){
            for(taskContainer_t::iterator itr = _task_container.begin(); itr!=_task_container.end();++itr){
                itr->second->on_msg();
            }
        }
        else {
            taskContainer_t::iterator itr = _task_container.find(_container_map[taskname]);
            if(itr!=_task_container.end())
                _task_container[_container_map[taskname]]->on_msg();
            else
                console::warn("{} is already installed", taskname);
        }
        
    }

    void task_manager::cleanup(const char* taskname){
        if(_task_container[_container_map[taskname]])
            _task_container[_container_map[taskname]]->cleanup();
    }

    oe::core::task::driver* task_manager::get_driver(const char* taskname){
        return nullptr;
    }

} //namespace oe::edge