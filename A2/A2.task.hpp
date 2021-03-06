/**
 * @file    dx3000.control.task.hpp
 * @brief   DX3000 AC Motor Controller Task
 * @author  Byunghun Hwang<bh.hwang@iae.re.kr>
 */

#ifndef _OPENEDGE_DX3000_CONTROL_TASK_HPP_
#define _OPENEDGE_DX3000_CONTROL_TASK_HPP_

#include <openedge/core.hpp>
#include <string>
#include <3rdparty/mosquitto/cpp/mosquittopp.h>
#include <map>

namespace zmq
{
#include <czmq.h>
}

using namespace oe;
using namespace std;

namespace oe::device
{
    class controller;
    class bus;
}

class a2Task : public oe::core::task::runnable, private mosqpp::mosquittopp
{

public:
    a2Task() : mosqpp::mosquittopp(){};
    virtual ~a2Task() = default;

    //component common interface
    bool configure() override;
    void execute() override;
    void cleanup() override;
    void pause() override;
    void resume() override;

private:
    //MQTT Callback functions
    virtual void on_connect(int rc) override;
    virtual void on_disconnect(int rc) override;
    virtual void on_publish(int mid) override;
    virtual void on_message(const struct mosquitto_message *message) override;
    virtual void on_subscribe(int mid, int qos_count, const int *granted_qos) override;
    virtual void on_unsubscribe(int mid) override;
    virtual void on_log(int level, const char *str) override;
    virtual void on_error() override;

private:
    void *ctx = nullptr;
    void *pub = nullptr;

private: //for mqtt
    string _mqtt_broker{"127.0.0.1"};
    int _mqtt_port{1883};
    string _mqtt_pub_topic = {"undefined"};
    int _mqtt_pub_qos = 2;
    int _mqtt_keep_alive = {60};
    vector<string> _mqtt_sub_topics;
};

EXPORT_TASK_API

#endif