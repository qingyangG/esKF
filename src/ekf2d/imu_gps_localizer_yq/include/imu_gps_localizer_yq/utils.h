#pragma once

#include <Eigen/Core>
#include <LocalCartesian.hpp>

namespace ImuGpsLocalization_yq{

constexpr double kDegreeToRadian = M_PI / 180.;
constexpr double kRadianToDegree = 180. / M_PI;

inline void ConvertLLAToENU(const Eigen::Vector3d& init_lla, 
                            const Eigen::Vector3d& point_lla, 
                            Eigen::Vector3d* point_enu) {
    static GeographicLib_yq::LocalCartesian local_cartesian;
    local_cartesian.Reset(init_lla(0), init_lla(1), init_lla(2));
    local_cartesian.Forward(point_lla(0), point_lla(1), point_lla(2), 
                            point_enu->data()[0], point_enu->data()[1], point_enu->data()[2]);
}

inline void ConvertENUToLLA(const Eigen::Vector3d& init_lla, 
                            const Eigen::Vector3d& point_enu,
                            Eigen::Vector3d* point_lla) {
    static GeographicLib_yq::LocalCartesian local_cartesian;
    local_cartesian.Reset(init_lla(0), init_lla(1), init_lla(2));
    local_cartesian.Reverse(point_enu(0), point_enu(1), point_enu(2), 
                            point_lla->data()[0], point_lla->data()[1], point_lla->data()[2]);                            
}

inline Eigen::Matrix3d GetSkewMatrix(const Eigen::Vector3d& v) {
    Eigen::Matrix3d w;
    w <<  0.,   -v(2),  v(1),
          v(2),  0.,   -v(0),
         -v(1),  v(0),  0.;

    return w;
}

inline Eigen::Matrix3d ComputeJRinvByAA(const Eigen::AngleAxisd& rv){
    double theta = rv.angle();
    double half_theta = 0.5 * theta;
    const Eigen::Vector3d normal = -rv.axis();
    Eigen::Matrix3d jrInv =  half_theta *cos(half_theta)/sin(half_theta) * Eigen::Matrix3d::Identity()
        + (1 - half_theta *cos(half_theta)/sin(half_theta) ) * normal * normal.transpose()
        - half_theta * GetSkewMatrix(normal);
    return jrInv;
}
}  // namespace ImuGpsLocalization