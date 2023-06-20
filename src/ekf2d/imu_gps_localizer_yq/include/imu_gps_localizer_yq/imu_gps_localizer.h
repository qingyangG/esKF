#pragma once

#include <Eigen/Core>

#include "imu_gps_localizer_yq/base_type.h"
#include "imu_gps_localizer_yq/gps_processor.h"
#include "imu_gps_localizer_yq/imu_processor.h"
#include "imu_gps_localizer_yq/initializer.h"

namespace ImuGpsLocalization_yq {

class ImuGpsLocalizer {
public:
    ImuGpsLocalizer(const double acc_noise, const double gyro_noise,
                    const double acc_bias_noise, const double gyro_bias_noise,
                    const Eigen::Vector3d& I_p_Gps);

    bool ProcessImuData(const ImuDataPtr imu_data_ptr, State* fused_state);

    bool ProcessGpsPositionData(const GpsPositionDataPtr gps_data_ptr,  int &consecutiveBadGpsS);

    Eigen::Vector3d getInitLLA(){
        return init_lla_;
    };
    bool getInitialized(){
        return initialized_;
    };
private:
    std::unique_ptr<Initializer>  initializer_;
    std::unique_ptr<ImuProcessor> imu_processor_;
    std::unique_ptr<GpsProcessor> gps_processor_;

    bool initialized_;
    Eigen::Vector3d init_lla_; // The initial reference gps point.
    State state_;
};

}  // namespace ImuGpsLocalization