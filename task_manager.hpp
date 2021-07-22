

/**
 * @file    task_manager.hpp
 * @brief   task manager class \
 *          all task described in configuration file will be managed, and this manager is globally single
 * @author  Byunghun Hwang<bh.hwang@iae.re.kr>
 */


#ifndef _OPENEDGE_TASK_MANAGER_HPP_
#define _OPENEDGE_TASK_MANAGER_HPP_

#include <openedge/arch/singleton.hpp>
#include <map>
#include <string>
#include <openedge/core/task.hpp>
#include <openedge/util/uuid.hpp>
#include <openedge/core/driver.hpp>

using namespace std;
using namespace oe;

namespace oe::edge {

    class task_manager : public oe::arch::singleton<task_manager> {
        public:

        typedef std::map<util::uuid_t, oe::core::task::driver*> taskContainer_t; //task container
        typedef std::map<string, util::uuid_t> taskContainer_map;   //task map

        task_manager();
        ~task_manager();

        bool install(const char* taskname = nullptr);
        void uninstall(const char* taskname = nullptr); //install without system configuration

        void msg_recv(const char* taskname = nullptr);
        void run(const char* taskname = nullptr);        
        void cleanup(const char* taskname = nullptr);

        int size() const { return _task_container.size(); }

        oe::core::task::driver* get_driver(const char* taskname = nullptr);
        oe::core::task::runnable* get_task(const char* taskname = nullptr);

        protected:
        task_manager(const task_manager&) = delete;
        task_manager& operator = (const task_manager&) = delete;

        private:
            taskContainer_t _task_container;
            taskContainer_map _container_map;
            util::uuid_generator _uuid_gen;

    };

} //namespace oe::edge

#define edge_task_manager oe::edge::task_manager::instance()

#endif