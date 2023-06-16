#pragma once

#include <fstream>
#include <memory>

#include <geometry_msgs/TwistStamped.h>
#include <nav_msgs/Path.h>
#include <ros/ros.h>
#include <sensor_msgs/NavSatFix.h>
#include <sensor_msgs/Imu.h>

#include "imu_gps_localizer_yq/imu_gps_localizer.h"

class LocalizationWrapper {
public:
    LocalizationWrapper(ros::NodeHandle& nh);
    ~LocalizationWrapper();

    void ImuCallback(const sensor_msgs::ImuConstPtr& imu_msg_ptr);

    void GpsPositionCallback(const sensor_msgs::NavSatFixConstPtr& gps_msg_ptr);

private:
    void LogState(const ImuGpsLocalization_yq::State& state);
    void LogGps(const ImuGpsLocalization_yq::GpsPositionDataPtr gps_data);

    void ConvertStateToRosTopic(const ImuGpsLocalization_yq::State& state);
    void ConvertGPSToRosTopic(const Eigen::Vector3d & gps);
    
    ros::Subscriber imu_sub_;
    ros::Subscriber gps_position_sub_;
    ros::Publisher state_pub_;
    ros::Publisher state_pub_onlyGPS_;

    std::ofstream file_state_;
    std::ofstream file_gps_;

    nav_msgs::Path ros_path_;
    nav_msgs::Path ros_path_onlyGPS_;

    std::unique_ptr<ImuGpsLocalization_yq::ImuGpsLocalizer> imu_gps_localizer_ptr_;
    int consecutiveBadGpsS;
    int consecutiveBadGpsSInit;
};