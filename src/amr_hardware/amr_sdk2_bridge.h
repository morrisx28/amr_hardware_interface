#ifndef AMR_SDK2_BRIDGE_H
#define AMR_SDK2_BRIDGE_H

#include <iostream>
#include <chrono>
#include <cstring>

#include <unitree/robot/channel/channel_publisher.hpp>
#include <unitree/robot/channel/channel_subscriber.hpp>
#include <unitree/idl/go2/LowState_.hpp>
#include <unitree/idl/go2/LowCmd_.hpp>
#include "../canfd/include/damiao.h"

using namespace unitree::common;
using namespace unitree::robot;
using namespace std;

#define TOPIC_LOWSTATE "rt/lowstate"
#define TOPIC_LOWCMD "rt/lowcmd"
#define MOTOR_SENSOR_NUM 3
#define NUM_MOTOR_IDL_GO 20

class AMRSdk2Bridge
{
public:
    AMRSdk2Bridge(const char *dev_sn);
    ~AMRSdk2Bridge();

    void LowCmdGoHandler(const void *msg);
    void PublishLowStateGo();

    // Motor related
    void SetMotorToZero();
    void AddMotorOffset();

    ChannelSubscriberPtr<unitree_go::msg::dds_::LowCmd_> low_cmd_go_suber_;

    unitree_go::msg::dds_::LowState_ low_state_go_{};

    ChannelPublisherPtr<unitree_go::msg::dds_::LowState_> low_state_go_puber_;

    ThreadPtr lowStatePuberThreadPtr;


    int num_motor_ = 8;
    int dim_motor_sensor_ = 0;

    int set_zero_ = false;
    vector<uint16_t> can_id_list{0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08}; // FL_steering FR_steering RL_steering RR_steering FL_wheel FR_wheel RL_wheel RR_wheel
    vector<uint16_t> mst_id_list{0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18};
    vector<int> motor_type{damiao::DM8009, damiao::DM8009, damiao::DM8009, damiao::DM8009, damiao::DM6006, damiao::DM6006, damiao::DM6006, damiao::DM6006};
    
    vector<double> motor_offset{0, 0, 0, 0, 0, 0, 0, 0};
    vector<double> direction{1.0, 1.0, 1.0, 1.0, 1.0, -1.0, -1.0, 1.0};
    private:

    bool is_running = true;
    // Motor related
    std::shared_ptr<damiao::Motor_Control> motor_control;
    uint32_t nom_baud =1000000;
    uint32_t dat_baud =5000000;
    vector<damiao::DmActData> dm_data_list;


};

#endif
