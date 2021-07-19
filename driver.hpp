
/**
 * @file    driver.hpp
 * @brief   task driver class
 * @author  Byunghun Hwang <bh.hwang@iae.re.kr>
 */

#ifndef _OPENEDGE_CORE_TASK_DRIVER_HPP_
#define _OPENEDGE_CORE_TASK_DRIVER_HPP_

#include <openedge/core/task.hpp>
#include <openedge/core/profile.hpp>
#include <string>
#include <thread>
#include <signal.h>
#include <mutex>
#include <queue>

using namespace std;

namespace oe::core {
    namespace task {

        //RT timer jitter data
        typedef struct _time_jitter_t {
            unsigned long long max {0};
            unsigned long long min {1000000000};
            void set(unsigned long long val){
                if(val>max) max=val;
                if(val<min) min=val;
            }   
        } time_jitter;

        class driver {
            public:
                explicit driver(const char* taskname);
                virtual ~driver();

                //configure the task before execute
                bool configure();

                //start task to run
                void execute();

                //destory task
                void cleanup();

                //pause task
                void pause();

                //resume task
                void resume();

                //on_msg (Async) task
                void on_msg(const char* msg);

                //getting task name
                const char* getTaskname() const { return _taskImpl->taskname.c_str(); }

            private:
                //Load task by task name
                bool load(const char* taskname);

                //Unload all task
                void unload();

                //start task process concurrently
                void do_process();

                //msg save
                void save_msg();

                //msg use
                void use_msg();
                
                //set task time spec. 
                void set_rt_timer(unsigned long long nsec);

            private:
                task::runnable* _taskImpl = nullptr;    //concrete implementation
                void* _task_handle = nullptr;   //for dl
                std::thread* _ptrThread = nullptr;
                std::thread* _ptrThread1 = nullptr;
                std::thread* _ptrThread2 = nullptr;
                std::mutex _mutex;
                int _signalIndex {0};
                timer_t _timer_id {0};
                struct sigevent _sig_evt;
                struct itimerspec _time_spec;
                _time_jitter_t _jitter;
                bool _overrun { false };
    
            public:
                void *ctx = nullptr;
                void *sub = nullptr;
                queue<char> q;
                std::mutex m;
                std::condition_variable cv;
        };
    } //namespace task
} //namespace oe

#endif