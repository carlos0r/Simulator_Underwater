/***************************************************
 * Title: UUV Simulator
 * Author: The UUV Simulator Authors
 * Date: 2016
 * Availability: https://uuvsimulator.github.io/
***************************************************/
/// \file Def.hh
/// \brief General definitions

#ifndef __GAZEBO_PLUGINS_SIM_DEF_HH__
#define __GAZEBO_PLUGINS_SIM_DEF_HH__

#include <vector>
#include <string>
#include <map>

#include <gazebo/gazebo.hh>

#include <eigen3/Eigen/Core>
#include <eigen3/Eigen/Geometry>

/// \brief Tags to debugging topics that publish the forces separetely
#define RESTORING_FORCE         "restoring_force"
#define DAMPING_FORCE           "damping_force"
#define DAMPING_TORQUE          "damping_torque"
#define ADDED_CORIOLIS_FORCE    "coriolis_force"
#define ADDED_CORIOLIS_TORQUE   "coriolis_torque"
#define ADDED_MASS_FORCE        "added_mass_force"
#define ADDED_MASS_TORQUE       "added_mass_torque"

namespace Eigen
{
/// \brief Definition of a 6x6 Eigen matrix
typedef Eigen::Matrix<double, 6, 6> Matrix6d;

/// \brief Definition of a 6 element Eigen vector
typedef Eigen::Matrix<double, 6, 1> Vector6d;
}

namespace gazebo
{
// Pi
#define PI 3.14159265359

/// \brief Conversion of a string to a double vector
inline std::vector<double> Str2Vector(std::string _input)
{
  std::vector<double> output;
  std::string buf;
  std::stringstream ss(_input);
  while (ss >> buf)
    output.push_back(std::stod(buf));
  return output;
}

/// \brief Returns the cross product operator matrix
/// for Eigen vectors
inline Eigen::Matrix3d CrossProductOperator(Eigen::Vector3d _x)
{
  Eigen::Matrix3d output;
  output << 0.0, -_x[2], _x[1], _x[2], 0.0, -_x[0], -_x[1], _x[0], 0.0;
  return output;
}

/// \brief Returns the cross product operator matrix
/// for Gazebo vectors
inline Eigen::Matrix3d CrossProductOperator(ignition::math::Vector3d _x)
{
  Eigen::Matrix3d output;
  output << 0.0, -_x[2], _x[1], _x[2], 0.0, -_x[0], -_x[1], _x[0], 0.0;
  return output;
}

inline Eigen::Vector3d ToEigen(const ignition::math::Vector3d &_x)
{
  return Eigen::Vector3d(_x[0], _x[1], _x[2]);
}

inline Eigen::Matrix3d ToEigen(const ignition::math::Matrix3d &_x)
{
  Eigen::Matrix3d m;
  m << _x(0, 0), _x(0, 1), _x(0, 2),
       _x(1, 0), _x(1, 1), _x(1, 2),
       _x(2, 0), _x(2, 1), _x(2, 2);
  return m;
}

inline Eigen::Vector6d EigenStack(const ignition::math::Vector3d &_x,
                                  const ignition::math::Vector3d &_y)
{
    Eigen::Vector3d xe = ToEigen(_x);
    Eigen::Vector3d ye = ToEigen(_y);
    Eigen::Vector6d out;
    out << xe, ye;
    return out;
}

inline ignition::math::Vector3d Vec3dToGazebo(const Eigen::Vector3d &_x)
{
  return ignition::math::Vector3d(_x[0], _x[1], _x[2]);
}

inline ignition::math::Matrix3d Mat3dToGazebo(const Eigen::Matrix3d &_x)
{
  return ignition::math::Matrix3d(_x(0, 0), _x(0, 1), _x(0, 2),
     _x(1, 0), _x(1, 1), _x(1, 2),
     _x(2, 0), _x(2, 1), _x(2, 2));
}

}

#endif
