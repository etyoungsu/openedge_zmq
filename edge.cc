/**
 * @file    edge.cc
 * @brief   OpenEdge Task Runtime Engine on Preemptive Realtime OS
 * @author  Byunghun Hwang <bh.hwang@iae.re.kr>
 */

/*
Copyright (c) 2020 Byunghun,Hwang
Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:
The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
*/

#include <3rdparty/cxxopts.hpp>
#include <unistd.h>
#include <csignal>
#include <sys/mman.h>
#include "instance.hpp"
#include <iostream>
#include <openedge/log.hpp>
#include <openedge/core.hpp>
#include <stdexcept>

using namespace std;

void terminate() {
  oe::app::cleanup();
  console::info("Successfully terminated");
  exit(EXIT_SUCCESS);
}

void cleanup(int sig) {
  switch(sig){
    case SIGSEGV: { console::warn("Segmentation violation"); } break;
    case SIGABRT: { console::warn("Abnormal termination"); } break;
    case SIGKILL: { console::warn("Process killed"); } break;
    case SIGBUS: { console::warn("Bus Error"); } break;
    case SIGTERM: { console::warn("Termination requested"); } break;
    case SIGINT: { console::warn("interrupted"); } break;
    default:
      console::info("Cleaning up the program");
  }
  ::terminate(); 
}


int main(int argc, char* argv[])
{
  console::stdout_color_st("console");

  const int signals[] = { SIGINT, SIGTERM, SIGBUS, SIGKILL, SIGABRT, SIGSEGV };
  for(const int& s:signals)
    signal(s, cleanup);

  //signal masking
  sigset_t sigmask;
  if(!sigfillset(&sigmask)){
    for(int signal:signals)
      sigdelset(&sigmask, signal); //delete signal from mask
  }
  else {
    console::error("Signal Handling Error");
    ::terminate(); //if failed, do termination
  }

  if(pthread_sigmask(SIG_SETMASK, &sigmask, nullptr)!=0){ // signal masking for this thread(main)
    console::error("Signal Masking Error");
    ::terminate();
  }

  mlockall(MCL_CURRENT|MCL_FUTURE); //avoid swaping

  int optc = 0;
  string _conf;  //configuration file

  while((optc=getopt(argc, argv, "s:c:i:u:lvrh"))!=-1){
    switch(optc){
      case 'i': //install task
      {
        console::info("install {}", optarg);
      }
      break;

      case 'u': //uninstall task
      {
        console::info("uninstall {}", optarg);
      }
      break;

      case 'l': //task list
      {
        console::info("list of tasks");
      }
      break;

      case 'v': //version
      {
        console::info("{} (built {}/{})", _OE_VER_, __DATE__, __TIME__);
      }
      break;

      case 'r': //realtime
      {
        console::info("Enable working as realtime");
      }
      break;

      case 'h': //help
      {
        console::info("usage : ");
      }
      break;

      case 'c': //config
      {
        _conf = optarg;
        console::info("Load configuration file(*.config) : {}", _conf);
      }
      break;

      case '?': //unkown
        
      break;
    }

  }

  try{
    if(!_conf.empty())
      if(oe::app::initialize(_conf.c_str())){
        oe::app::msg_recv();
        oe::app::run();
        pause(); //wait until getting SIGINT
      }
  }
  catch(const std::exception& e){
    console::error("Exception : {}", e.what());
  }

  // cxxopts::Options options(argv[0], "-  Options");
	// options.add_options()
  //       ("s,service", "openedge service") //it works as process manager (it assigns to RT timer signal index or status monitoring)
  //       ("c,config", "Load Configuration File(*.config)", cxxopts::value<std::string>(), "File Path") //require rerun avoiding
  //       ("i,install", "Install RT Task", cxxopts::value<std::string>(), "RT Task Component")
  //       ("u,unintall", "Uninstall RT Task", cxxopts::value<std::string>(), "RT Task Component")
  //       ("v,version", "Openedge Service Engine Version")
  //       ("r,rt", "Enable working as RT(RealTime) System")
  //       ("h,help", "Print Usage");
       
  // try
  // {
  //   auto args = options.parse(argc, argv);
    
  //   if(args.count("version")) { cout << _OE_VER_ << endl; ::terminate(); }
  //   else if(args.count("rt")) { cout << "Not Support yet" << endl; ::terminate(); }
  //   else if(args.count("install")) { cout << "Not Support yet" << endl; ::terminate(); }
  //   else if(args.count("uninstall")) { cout << "Not Support yet" << endl; ::terminate(); }
  //   else if(args.count("service")) { cout << "Not Support yet" << endl; ::terminate(); }
  //   else if(args.count("help")) { cout << options.help() << endl; ::terminate(); }
  //   //edge configuruation
  //   else if(args.count("config")){
  //     string _conf = args["config"].as<std::string>();

  //     console::info("Starting Openedge Service Engine {} (built {}/{})", _OE_VER_, __DATE__, __TIME__);
  //     console::info("Load Configuration File : {}", _conf);

  //     //run task engine
  //     if(oe::app::initialize(_conf.c_str()))
  //         oe::app::run();
      
  //     pause(); //wait until getting SIGINT
  //   }
  // }
  // catch(const cxxopts::OptionException& e){
  //   console::error("Argument parse exception : {}", e.what());
  // }

  ::terminate();
  return EXIT_SUCCESS;
}