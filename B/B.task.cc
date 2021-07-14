
#include "B.task.hpp"
#include <openedge/log.hpp>
#include <cstring>

bool _thread_except = false;
int num =0;

void threadFn(bTask* pclass, int a)
{
    console::info("thread");

    while(1){
        if(pclass){
            if (a == 1) {
                pclass->zmq_proc(pclass->sub1);
            }
            else if (a == 2) {
                pclass->zmq_proc(pclass->sub2);
            }
            else if (a == 3) {
                pclass->zmq_proc(pclass->sub3);
            }                        
        }
        std::this_thread::sleep_for(100ms);
        if (_thread_except == true) {
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
    ctx = zmq::zmq_ctx_new();
    sub1 = zmq::zmq_socket(ctx, ZMQ_SUB);
    sub2 = zmq::zmq_socket(ctx, ZMQ_SUB);
    sub3 = zmq::zmq_socket(ctx, ZMQ_SUB);
    int rc = zmq::zmq_connect(sub1, "tcp://192.168.11.25:5600");
    rc = zmq::zmq_connect(sub2, "tcp://192.168.11.25:5600");
    rc = zmq::zmq_connect(sub3, "tcp://192.168.11.25:5600");
    rc = zmq::zmq_setsockopt(sub1, ZMQ_SUBSCRIBE, "1", 1);
    rc = zmq::zmq_setsockopt(sub2, ZMQ_SUBSCRIBE, "2", 1);
    rc = zmq::zmq_setsockopt(sub3, ZMQ_SUBSCRIBE, "3", 1);    
    int timeout = 10000;
    rc = zmq::zmq_setsockopt(sub1, ZMQ_RCVTIMEO, &timeout, sizeof(int));
    rc = zmq::zmq_setsockopt(sub2, ZMQ_RCVTIMEO, &timeout, sizeof(int));
    rc = zmq::zmq_setsockopt(sub3, ZMQ_RCVTIMEO, &timeout, sizeof(int));
    console::info("ready?");
    _read1 = new std::thread(&threadFn, this, 1);
    _read2 = new std::thread(&threadFn, this, 2);
    _read3 = new std::thread(&threadFn, this, 3);
//    _read->joinable();
//  console::info("{}", std::thread::hardware_concurrency());
    return true;
}

void bTask::execute()
{
    console::info("{}", num);
}

void bTask::cleanup()
{
    if(_read1){
        _read1->join();
        delete _read1;
        _read1 = nullptr;
    }
    if(_read2){
        _read2->join();
        delete _read2;
        _read2 = nullptr;
    }
    if(_read3){
        _read3->join();
        delete _read3;
        _read3 = nullptr;
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