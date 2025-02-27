/***************************************************
 * Title: UUV Simulator
 * Author: The UUV Simulator Authors
 * Date: 2016
 * Availability: https://uuvsimulator.github.io/
***************************************************/

/// \file UnderwaterObjectROSPlugin.hh Publishes the underwater object's
/// Gazebo topics and parameters into ROS standards

#ifndef __UNDERWATER_OBJECT_ROS_PLUGIN_HH__
#define __UNDERWATER_OBJECT_ROS_PLUGIN_HH__

#include <string>

#include <gazebo_plugins_sim/UnderwaterObjectPlugin.hh>

#include <boost/scoped_ptr.hpp>
#include <gazebo/gazebo.hh>
#include <gazebo/common/Plugin.hh>
#include <ros/ros.h>
#include <geometry_msgs/WrenchStamped.h>
#include <geometry_msgs/Vector3.h>
#include <std_msgs/Bool.h>
#include <visualization_msgs/Marker.h>
#include <gazebo_ros_plugins_msgs/SetUseGlobalCurrentVel.h>
#include <gazebo_ros_plugins_msgs/UnderwaterObjectModel.h>
#include <gazebo_ros_plugins_msgs/GetModelProperties.h>
#include <gazebo_ros_plugins_msgs/SetFloat.h>
#include <gazebo_ros_plugins_msgs/GetFloat.h>
#include <geometry_msgs/TransformStamped.h>
#include <tf2_ros/transform_broadcaster.h>
#include <tf2/LinearMath/Quaternion.h>

#include <map>

namespace uuv_simulator_ros
{
  class UnderwaterObjectROSPlugin : public gazebo::UnderwaterObjectPlugin
  {
    /// \brief Constructor
    public: UnderwaterObjectROSPlugin();

    /// \brief Destructor
    public: virtual ~UnderwaterObjectROSPlugin();

    /// \brief Load module and read parameters from SDF.
    public: void Load(gazebo::physics::ModelPtr _parent, sdf::ElementPtr _sdf);

    /// \brief Initialize Module.
    public: virtual void Init();

    /// \brief Reset Module.
    public: virtual void Reset();

    /// \brief Update the simulation state.
    /// \param[in] _info Information used in the update event.
    public: virtual void Update(const gazebo::common::UpdateInfo &_info);

    /// \brief Update the local current velocity, this data will be used only
    /// if the useGlobalCurrent flag is set to false.
    public: void UpdateLocalCurrentVelocity(
      const geometry_msgs::Vector3::ConstPtr &_msg);

    /// \brief Set flag to use the global current velocity topic input
    public: bool SetUseGlobalCurrentVel(
      gazebo_ros_plugins_msgs::SetUseGlobalCurrentVel::Request& _req,
      gazebo_ros_plugins_msgs::SetUseGlobalCurrentVel::Response& _res);

    /// \brief Return the model properties, along with parameters from the
    /// hydrodynamic and hydrostatic models
    public: bool GetModelProperties(
      gazebo_ros_plugins_msgs::GetModelProperties::Request& _req,
      gazebo_ros_plugins_msgs::GetModelProperties::Response& _res);

    /// \brief Set the scaling factor for the added-mass matrix
    public: bool SetScalingAddedMass(
      gazebo_ros_plugins_msgs::SetFloat::Request& _req,
      gazebo_ros_plugins_msgs::SetFloat::Response& _res);

    /// \brief Return current scaling factor for the added-mass matrix
    public: bool GetScalingAddedMass(
      gazebo_ros_plugins_msgs::GetFloat::Request& _req,
      gazebo_ros_plugins_msgs::GetFloat::Response& _res);

    /// \brief Set a scaling factor for the overall damping matrix
    public: bool SetScalingDamping(
      gazebo_ros_plugins_msgs::SetFloat::Request& _req,
      gazebo_ros_plugins_msgs::SetFloat::Response& _res);

    /// \brief Return the scaling factor for the overall damping matrix
    public: bool GetScalingDamping(
      gazebo_ros_plugins_msgs::GetFloat::Request& _req,
      gazebo_ros_plugins_msgs::GetFloat::Response& _res);

    /// \brief Set scaling factor for the model's volume used for buoyancy
    /// force computation
    public: bool SetScalingVolume(
      gazebo_ros_plugins_msgs::SetFloat::Request& _req,
      gazebo_ros_plugins_msgs::SetFloat::Response& _res);

    /// \brief Get scaling factor for the model's volume used for buoyancy
    /// force computation
    public: bool GetScalingVolume(
      gazebo_ros_plugins_msgs::GetFloat::Request& _req,
      gazebo_ros_plugins_msgs::GetFloat::Response& _res);

    /// \brief Set new fluid density (this will alter the value for the
    /// buoyancy force)
    public: bool SetFluidDensity(
      gazebo_ros_plugins_msgs::SetFloat::Request& _req,
      gazebo_ros_plugins_msgs::SetFloat::Response& _res);

    /// \brief Get current value for the fluid density
    public: bool GetFluidDensity(
      gazebo_ros_plugins_msgs::GetFloat::Request& _req,
      gazebo_ros_plugins_msgs::GetFloat::Response& _res);

    /// \brief Set offset factor for the model's volume (this will alter the
    /// value for the buoyancy force)
    public: bool SetOffsetVolume(
      gazebo_ros_plugins_msgs::SetFloat::Request& _req,
      gazebo_ros_plugins_msgs::SetFloat::Response& _res);

    /// \brief Return the offset factor for the model's volume
    public: bool GetOffsetVolume(
      gazebo_ros_plugins_msgs::GetFloat::Request& _req,
      gazebo_ros_plugins_msgs::GetFloat::Response& _res);

    /// \brief Set the offset factor for the added-mass matrix
    public: bool SetOffsetAddedMass(
      gazebo_ros_plugins_msgs::SetFloat::Request& _req,
      gazebo_ros_plugins_msgs::SetFloat::Response& _res);

    /// \brief Return the offset factor for the added-mass matrix
    public: bool GetOffsetAddedMass(
      gazebo_ros_plugins_msgs::GetFloat::Request& _req,
      gazebo_ros_plugins_msgs::GetFloat::Response& _res);

    /// \brief Set the offset factor for the linear damping matrix
    public: bool SetOffsetLinearDamping(
      gazebo_ros_plugins_msgs::SetFloat::Request& _req,
      gazebo_ros_plugins_msgs::SetFloat::Response& _res);

    /// \brief Return the offset factor for the linear damping matrix
    public: bool GetOffsetLinearDamping(
      gazebo_ros_plugins_msgs::GetFloat::Request& _req,
      gazebo_ros_plugins_msgs::GetFloat::Response& _res);

    /// \brief Set the offset factor for the linear forward speed damping
    /// matrix
    public: bool SetOffsetLinearForwardSpeedDamping(
      gazebo_ros_plugins_msgs::SetFloat::Request& _req,
      gazebo_ros_plugins_msgs::SetFloat::Response& _res);

    /// \brief Return the offset factor for the linear forward speed damping
    /// matrix
    public: bool GetOffsetLinearForwardSpeedDamping(
      gazebo_ros_plugins_msgs::GetFloat::Request& _req,
      gazebo_ros_plugins_msgs::GetFloat::Response& _res);

    /// \brief Set the offset factor for the nonlinear damping
    /// matrix
    public: bool SetOffsetNonLinearDamping(
      gazebo_ros_plugins_msgs::SetFloat::Request& _req,
      gazebo_ros_plugins_msgs::SetFloat::Response& _res);

    /// \brief Return the offset factor for the nonlinear damping
    /// matrix
    public: bool GetOffsetNonLinearDamping(
      gazebo_ros_plugins_msgs::GetFloat::Request& _req,
      gazebo_ros_plugins_msgs::GetFloat::Response& _res);

    /// \brief Publish restoring force
    /// \param[in] _link Pointer to the link where the force information will
    /// be extracted from
    protected: virtual void PublishRestoringForce(
      gazebo::physics::LinkPtr _link);

    /// \brief Publish hydrodynamic wrenches
    /// \param[in] _link Pointer to the link where the force information will
    /// be extracted from
    protected: virtual void PublishHydrodynamicWrenches(
      gazebo::physics::LinkPtr _link);

    /// \brief Returns the wrench message for debugging topics
    /// \param[in] _force Force vector
    /// \param[in] _torque Torque vector
    /// \param[in] _output Stamped wrench message to be updated
    protected: virtual void GenWrenchMsg(
      ignition::math::Vector3d _force, ignition::math::Vector3d _torque,
      geometry_msgs::WrenchStamped &_output);

    /// \brief Sets the topics used for publishing the intermediate data during
    /// the simulation
    /// \param[in] _link Pointer to the link
    /// \param[in] _hydro Pointer to the hydrodynamic model
    protected: virtual void InitDebug(gazebo::physics::LinkPtr _link,
      gazebo::HydrodynamicModelPtr _hydro);

    /// \brief Publishes the current velocity marker
    protected: virtual void PublishCurrentVelocityMarker();

    /// \brief Publishes the state of the vehicle (is submerged)
    protected: virtual void PublishIsSubmerged();

    /// \brief Pointer to this ROS node's handle.
    private: boost::scoped_ptr<ros::NodeHandle> rosNode;

    /// \brief Subscriber to locally calculated current velocity
    private: ros::Subscriber subLocalCurVel;

    /// \brief Publisher for current actual thrust.
    private: std::map<std::string, ros::Publisher> rosHydroPub;

    /// \brief Map of services
    private: std::map<std::string, ros::ServiceServer> services;

    private: geometry_msgs::TransformStamped nedTransform;

    private: tf2_ros::TransformBroadcaster tfBroadcaster;
  };
}

#endif  // __UNDERWATER_OBJECT_ROS_PLUGIN_HH__
