#pragma once

#include "imu_gps_localizer_yq/base_type.h"

namespace ImuGpsLocalization_yq {

class ImuProcessor{
public:
    ImuProcessor(const double acc_noise, const double gyro_noise,
                 const double acc_bias_noise, const double gyro_bias_noise,
                 const Eigen::Vector3d& gravity);

    void Predict(const ImuDataPtr last_imu, const ImuDataPtr cur_imu, State* state);

private:
    const double acc_noise_;
    const double gyro_noise_;
    const double acc_bias_noise_;
    const double gyro_bias_noise_;

    const Eigen::Vector3d gravity_;

    Eigen::Vector3d lastDtheta;
};

}  // namespace ImuGpsLocalization