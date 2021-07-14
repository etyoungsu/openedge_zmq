{
    "md5":"603239CAF2F774498037F944F5592F52",
    "info":{
        "taskname":"uvlc.control.task",
        "version":"0.0.1",
        "cpu_affinity":1,
        "cycle_ns":1000000000,
        "policy":{
            "check_jitter":true,
            "check_overrun":true,
            "fault_level":0
        }
    },
    "configurations":{
        "mqtt":{
            "broker":"192.168.11.24",
            "port":1883,
            "pub_topic":"aop/uvlc/control",
            "pub_qos":2,
            "sub_topic":["pcan.mqtt.task/sysctrl", "aop/uvlc/sensor"],
            "keep_alive":60
        },
        "uvlc":{
            "limit_sensor":"0x600",
            "intensity_sensor":["0x700", "0x701", "0x702", "0x703"],
            "intensity_threshold":10.0
        },
        "zmq":{
            "prefix":["1","2", "3"]
        }
    }
}