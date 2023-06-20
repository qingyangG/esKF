#include "imu_gps_localizer_yq/gps_processor.h"

#include "imu_gps_localizer_yq/utils.h"
#include "math.h"
#include <glog/logging.h>
#include <iostream>
namespace ImuGpsLocalization_yq {

GpsProcessor::GpsProcessor(const Eigen::Vector3d& I_p_Gps) : I_p_Gps_(I_p_Gps) { }

bool GpsProcessor::UpdateStateByGpsPosition(const Eigen::Vector3d& init_lla, const GpsPositionDataPtr gps_data_ptr, State* state, int &consecutiveBadGpsS) {
    Eigen::Matrix<double, 3, 15> H;
    Eigen::Vector3d residual;
    ComputeJacobianAndResidual(init_lla, gps_data_ptr, *state, &H, &residual);
    const Eigen::Matrix3d& V = gps_data_ptr->cov;

    // EKF.
    const Eigen::MatrixXd& P = state->cov;
    // yqtest 
    const Eigen::MatrixXd invM = (H * P * H.transpose() + V).inverse();
    if (invM.hasNaN()) {
        consecutiveBadGpsS++;
        return false;
    }
    
    // 添加卡方检验
    const Eigen::Matrix3d& Var = V + H* P * H.transpose();
    //double chi2test = (residual.transpose() * Var.inverse() * residual)(0,0);
    double chi2test = (residual.block<2,1>(0, 0).transpose() * Var.block<2,2>(0,0).inverse() * residual.block<2,1>(0, 0) )(0,0);
    //if (chi2test > 1.5 * 16.27) {
    if (chi2test >  13.82) {
        LOG(INFO)<< "chi2test: " << chi2test << "residual" << residual(0) << residual(1) << "\n";
        // std::cout << "V:" << std::endl << V << std::endl;
        if (consecutiveBadGpsS < 5) {
            consecutiveBadGpsS++;
            return false;
        }
     }
    consecutiveBadGpsS = 0;

    // const Eigen::MatrixXd K = P * H.transpose() * (H * P * H.transpose() + V).inverse();
    const Eigen::MatrixXd K = P * H.transpose() * invM;
    const Eigen::VectorXd delta_x = K * residual;
     
    // Add delta_x to state.
    AddDeltaToState(delta_x, state);

    // Covarance.
    const Eigen::MatrixXd I_KH = Eigen::Matrix<double, 15, 15>::Identity() - K * H;
    state->cov = I_KH * P * I_KH.transpose() + K * V * K.transpose();
    //尝试另外一种稳定形式更新Covariance
    // less stable than above
    //state->cov = P - K * (H*P*H.transpose() + V) * K.transpose();
    return true;
}

void GpsProcessor::ComputeJacobianAndResidual(const Eigen::Vector3d& init_lla,  
                                              const GpsPositionDataPtr gps_data, 
                                              const State& state,
                                              Eigen::Matrix<double, 3, 15>* jacobian,
                                              Eigen::Vector3d* residual) {
    const Eigen::Vector3d& G_p_I   = state.G_p_I;
    const Eigen::Matrix3d& G_R_I   = state.G_R_I;

    // Convert wgs84 to ENU frame.
    Eigen::Vector3d G_p_Gps;
    ConvertLLAToENU(init_lla, gps_data->lla, &G_p_Gps);

    // Compute residual.
    *residual = G_p_Gps - (G_p_I + G_R_I * I_p_Gps_);

    // Compute jacobian.
    jacobian->setZero();
    jacobian->block<3, 3>(0, 0)  = Eigen::Matrix3d::Identity();
    jacobian->block<3, 3>(0, 6)  = - G_R_I * GetSkewMatrix(I_p_Gps_);
    // 这里应该是dH/d delta_x = dH / dx * dx / d delta_x, 作者只算了dH / dx.
    // Eigen::Matrix<double, 15,15> dx_deltax = Eigen::Matrix<double, 15,15>::Identity();
    // Eigen::AngleAxisd rv(G_R_I);
    // dx_deltax.block<3,3>(6,6) = ComputeJRinvByAA(rv.angle() * rv.axis());
    // *jacobian = *jacobian * dx_deltax;
    
    // 这里x取(p, v, deltaTheta, b_a, b_g), 则dx/d delta_x == I;
    
}

void AddDeltaToState(const Eigen::Matrix<double, 15, 1>& delta_x, State* state) {
    state->G_p_I     += delta_x.block<3, 1>(0, 0);
    state->G_v_I     += delta_x.block<3, 1>(3, 0);
    state->acc_bias  += delta_x.block<3, 1>(9, 0);
    state->gyro_bias += delta_x.block<3, 1>(12, 0);

    if (delta_x.block<3, 1>(6, 0).norm() > 1e-12) {
        state->G_R_I *= Eigen::AngleAxisd(delta_x.block<3, 1>(6, 0).norm(), delta_x.block<3, 1>(6, 0).normalized()).toRotationMatrix();
    }
}

}  // namespace ImuGpsLocalization