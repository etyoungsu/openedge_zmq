#include "A.task.hpp"
#include <openedge/log.hpp>
#include <cstring>

// pub

//static component instance that has only single instance
static aTask *_instance = nullptr;
oe::core::task::runnable *create()
{
    if (!_instance)
        _instance = new aTask();
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

bool aTask::configure()
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
    pub = zmq::zmq_socket(ctx, ZMQ_PUB);
    int rc = zmq::zmq_bind(pub, "tcp://*:5500");
    console::info("zmq bind completed");
    // char end_point[1024];
    // size_t size = sizeof(end_point);
    // zmq::zmq_getsockopt(pub, ZMQ_LAST_ENDPOINT, &end_point, &size);
    // console::info("{}", end_point);
    // char *port = strtok(end_point, ":");
    // port = strtok(end_point, ":");
    // port = strtok(end_point, ":");
    // console::info("{}", port);
    return true;
}

void aTask::execute()
{
    int rc = zmq::zstr_send(pub, "1");
    console::info("1 msg sent");               
}

void aTask::cleanup()
{
    //MQTT connection close
    this->disconnect();
    this->loop_stop();
    mosqpp::lib_cleanup();
    //    zmq::zmq_ctx_destroy(ctx);
}

void aTask::pause()
{
}

void aTask::resume()
{
}

void aTask::on_connect(int rc)
{
    if (rc == MOSQ_ERR_SUCCESS)
        console::info("Successfully connected to MQTT Brocker({})", rc);
    else
        console::warn("MQTT Broker connection error : {}", rc);
}

void aTask::on_disconnect(int rc)
{
}

void aTask::on_publish(int mid)
{
}

void aTask::use_msg(const char* msg) {

}

void aTask::on_message(const struct mosquitto_message *message)
{
#define MAX_BUFFER_SIZE 4096

    char *buffer = new char[MAX_BUFFER_SIZE];
    memset(buffer, 0, sizeof(char) * MAX_BUFFER_SIZE);
    memcpy(buffer, message->payload, sizeof(char) * message->payloadlen);
    string strmsg = buffer;
    delete[] buffer;

    console::info("mqtt data({}) : {}", message->payloadlen, strmsg);
}

void aTask::on_subscribe(int mid, int qos_count, const int *granted_qos)
{
}

void aTask::on_unsubscribe(int mid)
{
}

void aTask::on_log(int level, const char *str)
{
}

void aTask::on_error()
{
}