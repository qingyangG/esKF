#include "localization_wrapper.h"

#include <iomanip>

#include <glog/logging.h>

#include "imu_gps_localizer_yq/base_type.h"
#include "imu_gps_localizer_yq/utils.h"
using namespace ImuGpsLocalization_yq;

LocalizationWrapper::LocalizationWrapper(ros::NodeHandle& nh) {
    // Load configs.
    double acc_noise, gyro_noise, acc_bias_noise, gyro_bias_noise;
    nh.param("acc_noise",       acc_noise, 1e-2);
    nh.param("gyro_noise",      gyro_noise, 1e-4);
    nh.param("acc_bias_noise",  acc_bias_noise, 1e-6);
    nh.param("gyro_bias_noise", gyro_bias_noise, 1e-8);

    double x, y, z;
    nh.param("I_p_Gps_x", x, 0.);
    nh.param("I_p_Gps_y", y, 0.);
    nh.param("I_p_Gps_z", z, 0.);
    const Eigen::Vector3d I_p_Gps(x, y, z);

    std::string log_folder = "/home";
    ros::param::get("log_folder", log_folder);

    // Log.
    file_state_.open(log_folder + "/state.csv");
    file_gps_.open(log_folder +"/gps.csv");

    // record bad gps number
    consecutiveBadGpsS = 0;
    consecutiveBadGpsSInit = 0;

    // Initialization imu gps localizer.
    imu_gps_localizer_ptr_ = 
        std::make_unique<ImuGpsLocalization_yq::ImuGpsLocalizer>(acc_noise, gyro_noise,
                                                              acc_bias_noise, gyro_bias_noise,
                                                              I_p_Gps);

    // Subscribe topics.
    imu_sub_ = nh.subscribe("/imu/data", 100,  &LocalizationWrapper::ImuCallback, this);
    gps_position_sub_ = nh.subscribe("/fix", 10,  &LocalizationWrapper::GpsPositionCallback, this);

    state_pub_ = nh.advertise<nav_msgs::Path>("fused_path", 100);
    state_pub_onlyGPS_ = nh.advertise<nav_msgs::Path>("fused_path_onlyGPS", 10);
}

LocalizationWrapper::~LocalizationWrapper() {
    file_state_.close();
    file_gps_.close();
}

void LocalizationWrapper::ImuCallback(const sensor_msgs::ImuConstPtr& imu_msg_ptr) {
    ImuGpsLocalization_yq::ImuDataPtr imu_data_ptr = std::make_shared<ImuGpsLocalization_yq::ImuData>();
    imu_data_ptr->timestamp = imu_msg_ptr->header.stamp.toSec();
    imu_data_ptr->acc << imu_msg_ptr->linear_acceleration.x, 
                         imu_msg_ptr->linear_acceleration.y,
                         imu_msg_ptr->linear_acceleration.z;
    imu_data_ptr->gyro << imu_msg_ptr->angular_velocity.x,
                          imu_msg_ptr->angular_velocity.y,
                          imu_msg_ptr->angular_velocity.z;
    
    ImuGpsLocalization_yq::State fused_state;
    const bool ok = imu_gps_localizer_ptr_->ProcessImuData(imu_data_ptr, &fused_state);
    if (!ok) {
        return;
    }

    // Publish fused state.
    ConvertStateToRosTopic(fused_state);
    state_pub_.publish(ros_path_);

    // Log fused state.
    LogState(fused_state);
}

void LocalizationWrapper::GpsPositionCallback(const sensor_msgs::NavSatFixConstPtr& gps_msg_ptr) {
    // Check the gps_status.
    if (gps_msg_ptr->status.status < 1) {
        //yqtest
        //ROS_INFO("status: %d", gps_msg_ptr->status.status);
        if (!imu_gps_localizer_ptr_->getInitialized()) {
            if (consecutiveBadGpsSInit < 25) {  // 5s
                ROS_INFO("status: %d", gps_msg_ptr->status.status);
                LOG(WARNING) << "[GpsCallBack]: Bad gps message for initialization!";
                consecutiveBadGpsSInit++;
                return;
            }
            else if (gps_msg_ptr->status.status >= 0) {
                ROS_INFO("status: %d", gps_msg_ptr->status.status);
                LOG(WARNING) << "[GpsCallBack]: Map will be constructed. But the current environment is bad and the map maybe not good.!";
            }
            else {
                LOG(WARNING) << "[GpsCallBack]: Intilization needs extra time because of no usable gps data!";
                return;
            }
        }
        // else if (consecutiveBadGpsS < 0)  { //  5hz, gps data stream.
        //     // gps不是很好情况下, 短时间允许不矫正.
        //     consecutiveBadGpsS++;
        //     return;
        // }
        else if ( gps_msg_ptr->status.status >= 0){
            //consecutiveBadGpsS = 0;
        }
        else {
            ROS_INFO("current gps is totally invalid!");
            consecutiveBadGpsS++;
            return;
        }
    }

    ImuGpsLocalization_yq::GpsPositionDataPtr gps_data_ptr = std::make_shared<ImuGpsLocalization_yq::GpsPositionData>();
    gps_data_ptr->timestamp = gps_msg_ptr->header.stamp.toSec();
    gps_data_ptr->lla << gps_msg_ptr->latitude,
                         gps_msg_ptr->longitude,
                         gps_msg_ptr->altitude;
    gps_data_ptr->cov = Eigen::Map<const Eigen::Matrix3d>(gps_msg_ptr->position_covariance.data());
    
    //yqtest: add origin gps path
    if (imu_gps_localizer_ptr_->getInitialized()) {
        Eigen::Vector3d G_p_Gps;
        ConvertLLAToENU(imu_gps_localizer_ptr_->getInitLLA(), gps_data_ptr->lla, &G_p_Gps);
        ConvertGPSToRosTopic(G_p_Gps);
        state_pub_onlyGPS_.publish(ros_path_onlyGPS_);
    }
    LogGps(gps_data_ptr);

    imu_gps_localizer_ptr_->ProcessGpsPositionData(gps_data_ptr, consecutiveBadGpsS);
}

void LocalizationWrapper::LogState(const ImuGpsLocalization_yq::State& state) {
    const Eigen::Quaterniond G_q_I(state.G_R_I);
    file_state_ << std::fixed << std::setprecision(15)
                << state.timestamp << ","
                << state.lla[0] << "," << state.lla[1] << "," << state.lla[2] << ","
                << state.G_p_I[0] << "," << state.G_p_I[1] << "," << state.G_p_I[2] << ","
                << state.G_v_I[0] << "," << state.G_v_I[1] << "," << state.G_v_I[2] << ","
                << G_q_I.x() << "," << G_q_I.y() << "," << G_q_I.z() << "," << G_q_I.w() << ","
                << state.acc_bias[0] << "," << state.acc_bias[1] << "," << state.acc_bias[2] << ","
                << state.gyro_bias[0] << "," << state.gyro_bias[1] << "," << state.gyro_bias[2] << "\n";
}

void LocalizationWrapper::LogGps(const ImuGpsLocalization_yq::GpsPositionDataPtr gps_data) {
    file_gps_ << std::fixed << std::setprecision(15)
              << gps_data->timestamp << ","
              << gps_data->lla[0] << "," << gps_data->lla[1] << "," << gps_data->lla[2] << "\n";
}

void LocalizationWrapper::ConvertStateToRosTopic(const ImuGpsLocalization_yq::State& state) {
    ros_path_.header.frame_id = "world";
    ros_path_.header.stamp = ros::Time::now();  

    geometry_msgs::PoseStamped pose;
    pose.header = ros_path_.header;

    pose.pose.position.x = state.G_p_I[0];
    pose.pose.position.y = state.G_p_I[1];
    pose.pose.position.z = state.G_p_I[2];

    const Eigen::Quaterniond G_q_I(state.G_R_I);
    pose.pose.orientation.x = G_q_I.x();
    pose.pose.orientation.y = G_q_I.y();
    pose.pose.orientation.z = G_q_I.z();
    pose.pose.orientation.w = G_q_I.w();

    // ROS_INFO("postion: %0.2f, %0.2f, %0.2f, orientation: %0.2f, %0.2f, %0.2f,%0.2f", 
    //     pose.pose.position.x , pose.pose.position.y, pose.pose.position.z,
    //     pose.pose.orientation.x, pose.pose.orientation.y, pose.pose.orientation.z, pose.pose.orientation.w);
    ros_path_.poses.push_back(pose);
}

void LocalizationWrapper::ConvertGPSToRosTopic(const Eigen::Vector3d& gps) {
    ros_path_onlyGPS_.header.frame_id = "world";
    ros_path_onlyGPS_.header.stamp = ros::Time::now();  

    geometry_msgs::PoseStamped pose;
    pose.header = ros_path_onlyGPS_.header;

    pose.pose.position.x = gps(0) ;
    pose.pose.position.y = gps(1);
    pose.pose.position.z = gps(2);

    const Eigen::Quaterniond G_q_I(Eigen::Matrix3d::Identity());
    pose.pose.orientation.x = G_q_I.x();
    pose.pose.orientation.y = G_q_I.y();
    pose.pose.orientation.z = G_q_I.z();
    pose.pose.orientation.w = G_q_I.w();

    ros_path_onlyGPS_.poses.push_back(pose);
}