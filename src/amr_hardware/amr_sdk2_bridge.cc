#include "amr_sdk2_bridge.h"

AMRSdk2Bridge::AMRSdk2Bridge(const char *dev_sn)
{
    for (int i=0; i < num_motor_; i++)
    {
        dm_data_list.push_back(damiao::DmActData{.motorType = static_cast<damiao::DM_Motor_Type>(motor_type[i]),
        .mode = damiao::POS_VEL_MODE,
        .can_id=static_cast<uint16_t>(can_id_list[i]),
        .mst_id=static_cast<uint16_t>(mst_id_list[i])});
    }
    motor_control = std::make_shared<damiao::Motor_Control>(nom_baud,dat_baud,
      dev_sn,&dm_data_list);

    if (set_zero_)
    {
        SetMotorToZero();
    }

    low_cmd_go_suber_.reset(new ChannelSubscriber<unitree_go::msg::dds_::LowCmd_>(TOPIC_LOWCMD));
    low_cmd_go_suber_->InitChannel(bind(&AMRSdk2Bridge::LowCmdGoHandler, this, placeholders::_1), 1);

    low_state_go_puber_.reset(new ChannelPublisher<unitree_go::msg::dds_::LowState_>(TOPIC_LOWSTATE));
    low_state_go_puber_->InitChannel();
    

    lowStatePuberThreadPtr = CreateRecurrentThreadEx("lowstate", UT_CPU_ID_NONE, 2000, &AMRSdk2Bridge::PublishLowStateGo, this);

}

AMRSdk2Bridge::~AMRSdk2Bridge()
{
    is_running = false;
    
}

void AMRSdk2Bridge::SetMotorToZero()
{
    for (int i = 0; i < num_motor_; i++) 
    {
        motor_control->set_zero_position(*motor_control->getMotor(can_id_list[i]));
    }
}

void AMRSdk2Bridge::LowCmdGoHandler(const void *msg)
{
    const unitree_go::msg::dds_::LowCmd_ *cmd = (const unitree_go::msg::dds_::LowCmd_ *)msg;
    for (int i = 0; i < num_motor_; i++)
    {
        double cmd_q = cmd->motor_cmd()[i].q() * direction[i] + motor_offset[i];
        double cmd_dq = cmd->motor_cmd()[i].dq() * direction[i];
        double cmd_tau = cmd->motor_cmd()[i].tau() * direction[i];

        motor_control->control_mit(*motor_control->getMotor(can_id_list[i]), cmd->motor_cmd()[i].kp(), cmd->motor_cmd()[i].kd(), 
                                                            cmd_q, cmd_dq, cmd_tau);
    }
}

void AMRSdk2Bridge::PublishLowStateGo()
{

    for (uint16_t i = 0;i < num_motor_; i++)
    {
        double pos = motor_control->getMotor(can_id_list[i])->Get_Position();
        double vel = motor_control->getMotor(can_id_list[i])->Get_Velocity();
        double tau = motor_control->getMotor(can_id_list[i])->Get_tau();
        
        low_state_go_.motor_state()[i].q() = (pos - motor_offset[i]) * direction[i];
        low_state_go_.motor_state()[i].dq() = vel * direction[i];
        low_state_go_.motor_state()[i].tau_est() = tau * direction[i];
    }
    
    low_state_go_puber_->Write(low_state_go_);

}
