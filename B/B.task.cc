
#include "B.task.hpp"
#include <openedge/log.hpp>
#include <cstring>
#include <vector>

bool _thread_except = false;
int num =0;

void threadFn(bTask* pclass, void *sub)
{
    console::info("thread");

    while(1){
        if(pclass){
            pclass->zmq_proc(sub);
        }
        std::this_thread::sleep_for(100ms);
        if (_thread_except == true) {
            _thread_except = false;
            break;
        }
    }
    console::info("Thread Termination");
}

//static component instance that has only single instance
static bTask *_instance = nullptr;
oe::core::task::runnable *create()
{
    if (!_instance)
        _instance = new bTask();
    return _instance;
}
void release()
{
    if (_instance)
    {
        delete _instance;
        _instance = nullptr;
    }
}

void bTask::zmq_proc(void* sub){
    if(sub){
        char *string = zmq::zstr_recv(sub);
        if (string != nullptr) {
            console::info("received {}", string);
            num += *string-'0';
        }
        else {
            _thread_except = true;
            console::info("received nothing");
        }
        zmq::zstr_free(&string);
    }
    
}

bool bTask::configure()
{
    ctx = zmq::zmq_ctx_new();
    //initialize mosquitto
    if (const int ret = mosqpp::lib_init() != MOSQ_ERR_SUCCESS)
    {
        console::error("({}){}", ret, mosqpp::strerror(ret));
        return false;
    }

    //read configuration from profile
    json config = json::parse(getProfile()->get("configurations"));

    //read MQTT parameters & connect to the broker
    if (config.find("mqtt") != config.end())
    {
        json mqtt_param = config["mqtt"];
        if (mqtt_param.find("broker") != mqtt_param.end())
            _mqtt_broker = mqtt_param["broker"].get<string>();
        if (mqtt_param.find("port") != mqtt_param.end())
            _mqtt_port = mqtt_param["port"].get<int>();
        if (mqtt_param.find("pub_topic") != mqtt_param.end())
            _mqtt_pub_topic = mqtt_param["pub_topic"].get<string>();
        if (mqtt_param.find("pub_qos") != mqtt_param.end())
            _mqtt_pub_qos = mqtt_param["pub_qos"].get<int>();
        if (mqtt_param.find("keep_alive") != mqtt_param.end())
            _mqtt_keep_alive = mqtt_param["keep_alive"].get<int>();
        if (mqtt_param.find("sub_topic") != mqtt_param.end())
        {
            for (json::iterator itr = mqtt_param["sub_topic"].begin(); itr != mqtt_param["sub_topic"].end(); ++itr)
            {
                _mqtt_sub_topics.emplace_back(*itr);
            }
        }

        console::info("> set MQTT Broker : {}", _mqtt_broker);
        console::info("> set MQTT Port : {}", _mqtt_port);
        console::info("> set MQTT Pub. Topic : {}", _mqtt_pub_topic);
        console::info("> set MQTT Pub. QoS : {}", _mqtt_pub_qos);
        console::info("> set MQTT Keep-alive : {}", _mqtt_keep_alive);

        //connect to MQTT broker
        if (const int conret = this->connect_async(_mqtt_broker.c_str(), _mqtt_port, _mqtt_keep_alive) == MOSQ_ERR_SUCCESS)
        {
            for (string topic : _mqtt_sub_topics)
            {
                this->subscribe(nullptr, topic.c_str(), 2);
                console::info("> set MQTT Sub. Topic : {}", topic);
            }

            if (const int ret = this->loop_start() != MOSQ_ERR_SUCCESS)
                console::warn("({}){}", ret, mosqpp::strerror(ret));
        }
        else
            console::warn("({}){}", conret, mosqpp::strerror(conret));
    }

    if (config.find("zmq") != config.end()) {
        json zmq_param = config["zmq"];
        if (zmq_param.find("prefix") != zmq_param.end())
        {
            for (json::iterator itr = zmq_param["prefix"].begin(); itr != zmq_param["prefix"].end(); ++itr)
            {
                _zmq_sub_topics.emplace_back(*itr);
            }
        }
    }
    int sub_size = _zmq_sub_topics.size();
    for(int i=0; i<sub_size; i++){
        sub.push_back(zmq::zmq_socket(ctx, ZMQ_SUB));
        zmq::zmq_connect(sub[i], "tcp://192.168.11.13:5000");
        // const char *topic_const = _zmq_sub_topics[i].c_str();
        // char *topic = (char*)topic_const;
        string topic = _zmq_sub_topics[i];
        zmq::zmq_setsockopt(sub[i], ZMQ_SUBSCRIBE, topic.data(), topic.size());
        int timeout = 10000;
        zmq::zmq_setsockopt(sub[i], ZMQ_RCVTIMEO, &timeout, sizeof(int));
        console::info("> set ZMQ Sub. Topic : {}", _zmq_sub_topics[i]);
        _read.push_back(new std::thread(&threadFn, this, sub[i]));
    }
//    _read->joinable();
//  console::info("{}", std::thread::hardware_concurrency());
    return true;
}

void bTask::execute()
{
    console::info("sum {}", num);
}

void bTask::cleanup()
{
    for (int i=0; i<sub.size(); i++) {
        _read[i]->join();
        delete _read[i];
        _read[i] = nullptr;
    }

    //MQTT connection close
    this->disconnect();
    this->loop_stop();
    mosqpp::lib_cleanup();
    //    zmq::zmq_ctx_destroy(ctx);
}

void bTask::pause()
{
}

void bTask::resume()
{
}

void bTask::on_connect(int rc)
{
    if (rc == MOSQ_ERR_SUCCESS)
        console::info("Successfully connected to MQTT Brocker({})", rc);
    else
        console::warn("MQTT Broker connection error : {}", rc);
}

void bTask::on_disconnect(int rc)
{
}

void bTask::on_publish(int mid)
{
}

void bTask::on_message(const struct mosquitto_message *message)
{
#define MAX_BUFFER_SIZE 4096

    char *buffer = new char[MAX_BUFFER_SIZE];
    memset(buffer, 0, sizeof(char) * MAX_BUFFER_SIZE);
    memcpy(buffer, message->payload, sizeof(char) * message->payloadlen);
    string strmsg = buffer;
    delete[] buffer;

    console::info("mqtt data({}) : {}", message->payloadlen, strmsg);
}

void bTask::on_subscribe(int mid, int qos_count, const int *granted_qos)
{
}

void bTask::on_unsubscribe(int mid)
{
}

void bTask::on_log(int level, const char *str)
{
}

void bTask::on_error()
{
}