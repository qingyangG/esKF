#include "imu_gps_localizer_yq/imu_processor.h"

#include <glog/logging.h>
#include <Eigen/Dense>
#include "imu_gps_localizer_yq/utils.h"

namespace ImuGpsLocalization_yq {

ImuProcessor::ImuProcessor(const double acc_noise, const double gyro_noise,
                           const double acc_bias_noise, const double gyro_bias_noise,
                           const Eigen::Vector3d& gravity)
    : acc_noise_(acc_noise), gyro_noise_(gyro_noise), 
      acc_bias_noise_(acc_bias_noise), gyro_bias_noise_(gyro_bias_noise),
      gravity_(gravity) { 
        lastDtheta.setZero();
      }

void ImuProcessor::Predict(const ImuDataPtr last_imu, const ImuDataPtr cur_imu, State* state) {
    // Time.
    const double delta_t = cur_imu->timestamp - last_imu->timestamp;
    const double delta_t2 = delta_t * delta_t;

    // Set last state.
    State last_state = *state;

    // Acc and gyro.
    const Eigen::Vector3d acc_unbias = 0.5 * (last_imu->acc + cur_imu->acc) - last_state.acc_bias;
    const Eigen::Vector3d gyro_unbias = 0.5 * (last_imu->gyro + cur_imu->gyro) - last_state.gyro_bias;

    // Normal state. 
    // Using P58. of "Quaternion kinematics for the error-state Kalman Filter".
    state->G_p_I = last_state.G_p_I + last_state.G_v_I * delta_t + 
                   0.5 * (last_state.G_R_I * acc_unbias + gravity_) * delta_t2;
    state->G_v_I = last_state.G_v_I + (last_state.G_R_I * acc_unbias + gravity_) * delta_t;
    Eigen::Vector3d delta_angle_axis = gyro_unbias * delta_t; 
    Eigen::Vector3d rotVec;
    // 考虑双子样假设
    Eigen::Vector3d temp = delta_angle_axis;
    delta_angle_axis =  delta_angle_axis + lastDtheta.cross(delta_angle_axis) / 12;
    lastDtheta = temp;

    if (delta_angle_axis.norm() > 1e-12) {
        state->G_R_I = last_state.G_R_I * Eigen::AngleAxisd(delta_angle_axis.norm(), delta_angle_axis.normalized()).toRotationMatrix();
    }

    // Error-state. Not needed.

    // Covariance of the error-state.   
    Eigen::Matrix<double, 15, 15> Fx = Eigen::Matrix<double, 15, 15>::Identity();
    Fx.block<3, 3>(0, 3)   = Eigen::Matrix3d::Identity() * delta_t;
    Fx.block<3, 3>(3, 6)   = - last_state.G_R_I * GetSkewMatrix(acc_unbias) * delta_t;
    Fx.block<3, 3>(3, 9)   = - last_state.G_R_I * delta_t;
    if (delta_angle_axis.norm() > 1e-12) {
        Fx.block<3, 3>(6, 6) = Eigen::AngleAxisd(delta_angle_axis.norm(), delta_angle_axis.normalized()).toRotationMatrix().transpose();
    } else {
        Fx.block<3, 3>(6, 6).setIdentity();
    }
    Fx.block<3, 3>(6, 12)  = - Eigen::Matrix3d::Identity() * delta_t;
    // yqtest: order 2
    Fx.block<3,3>(0,6) = Fx.block<3, 3>(3, 6) * delta_t * 0.5;
    Fx.block<3,3>(0,9) = Fx.block<3, 3>(3, 9) * delta_t * 0.5;
    Fx.block<3,3>(3,12) = - Fx.block<3, 3>(0, 6);


    Eigen::Matrix<double, 15, 12> Fi = Eigen::Matrix<double, 15, 12>::Zero();
    Fi.block<12, 12>(3, 0) = Eigen::Matrix<double, 12, 12>::Identity();

    Eigen::Matrix<double, 12, 12> Qi = Eigen::Matrix<double, 12, 12>::Zero();
    Qi.block<3, 3>(0, 0) = delta_t2 * acc_noise_ * Eigen::Matrix3d::Identity();
    Qi.block<3, 3>(3, 3) = delta_t2 * gyro_noise_ * Eigen::Matrix3d::Identity();
    Qi.block<3, 3>(6, 6) = delta_t * acc_bias_noise_ * Eigen::Matrix3d::Identity();
    Qi.block<3, 3>(9, 9) = delta_t * gyro_bias_noise_ * Eigen::Matrix3d::Identity();

    state->cov = Fx * last_state.cov * Fx.transpose() + Fi * Qi * Fi.transpose();

    // Time and imu.
    state->timestamp = cur_imu->timestamp;
    state->imu_data_ptr = cur_imu;
}

}  // namespace ImuGpsLocalization