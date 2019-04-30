/***************************************************
 * Title: This file was modified from the original version within Gazebo
 * Author: Nils Bore
 * Date: 2014
 * Availability: nbore@kth.se
***************************************************/

#ifndef GAZEBO_ROS_DEPTH_CAMERA_HH
#define GAZEBO_ROS_DEPTH_CAMERA_HH

// ros stuff
#include <ros/ros.h>
#include <ros/callback_queue.h>
#include <ros/advertise_options.h>

// ros messages stuff
#include <sensor_msgs/PointCloud2.h>
#include <sensor_msgs/Image.h>
#include <sensor_msgs/CameraInfo.h>
#include <sensor_msgs/fill_image.h>
#include <std_msgs/Float64.h>
#include <image_transport/image_transport.h>

// gazebo stuff
#include <sdf/Param.hh>
#include <gazebo/physics/physics.hh>
#include <gazebo/transport/TransportTypes.hh>
#include <gazebo/msgs/MessageTypes.hh>
#include <gazebo/common/Time.hh>
#include <gazebo/sensors/SensorTypes.hh>
#include <gazebo/plugins/DepthCameraPlugin.hh>

// dynamic reconfigure stuff
#include <gazebo_plugins/GazeboRosCameraConfig.h>
#include <dynamic_reconfigure/server.h>

// boost stuff
#include <boost/thread/mutex.hpp>

// camera stuff
#include <gazebo_plugins/gazebo_ros_camera_utils.h>

#include <opencv2/core.hpp>

namespace gazebo
{
  class GazeboRosImageSonar : public SensorPlugin, GazeboRosCameraUtils
  {
    /// \brief Constructor
    /// \param parent The parent entity, must be a Model or a Sensor
    public: GazeboRosImageSonar();

    /// \brief Destructor
    public: ~GazeboRosImageSonar();

    /// \brief Load the plugin
    /// \param take in SDF root element
    public: virtual void Load(sensors::SensorPtr _parent, sdf::ElementPtr _sdf);

    /// \brief Advertise point cloud and depth image
    public: virtual void Advertise();

    /// \brief Update the controller
    protected: virtual void OnNewDepthFrame(const float *_image,
                   unsigned int _width, unsigned int _height,
                   unsigned int _depth, const std::string &_format);

    /// \brief Update the controller
    protected: virtual void OnNewRGBPointCloud(const float *_pcd,
                    unsigned int _width, unsigned int _height,
                    unsigned int _depth, const std::string &_format);

    /// \brief Update the controller
    protected: virtual void OnNewImageFrame(const unsigned char *_image,
                   unsigned int _width, unsigned int _height,
                   unsigned int _depth, const std::string &_format);

    /// \brief Put camera data to the ROS topic
    private: void FillPointdCloud(const float *_src);

    /// \brief push depth image data into ros topic
    private: void FillDepthImage(const float *_src);

    /// \brief push depth image data into ros topic
    private: void ComputeSonarImage(const float *_src);
	private: cv::Mat ComputeNormalImage(cv::Mat& depth);
	private: cv::Mat ConstructSonarImage(cv::Mat& depth, cv::Mat& normals);
    private: cv::Mat ConstructScanImage(cv::Mat& depth, cv::Mat& SNR);
    private: void ApplySpeckleNoise(cv::Mat& scan, float fov);
    private: void ApplySmoothing(cv::Mat& scan, float fov);
    private: void ApplyMedianFilter(cv::Mat& scan);
    private: cv::Mat ConstructVisualScanImage(cv::Mat& raw_scan);

    /// \brief Keep track of number of connctions for point clouds
    private: int point_cloud_connect_count_;
    private: void PointCloudConnect();
    private: void PointCloudDisconnect();

    /// \brief Keep track of number of connctions for point clouds
    private: int depth_image_connect_count_;
    private: void DepthImageConnect();
    private: void DepthImageDisconnect();
    private: void NormalImageConnect();
    private: void NormalImageDisconnect();
    private: void MultibeamImageConnect();
    private: void MultibeamImageDisconnect();
    private: void SonarImageConnect();
    private: void SonarImageDisconnect();
    private: void RawSonarImageConnect();
    private: void RawSonarImageDisconnect();
    private: common::Time last_depth_image_camera_info_update_time_;

    private: bool FillPointCloudHelper(sensor_msgs::PointCloud2 &point_cloud_msg,
                                  uint32_t rows_arg, uint32_t cols_arg,
                                  uint32_t step_arg, void* data_arg);

    private: bool FillDepthImageHelper( sensor_msgs::Image& image_msg,
                                  uint32_t rows_arg, uint32_t cols_arg,
                                  uint32_t step_arg, void* data_arg);

    /// \brief A pointer to the ROS node.  A node will be instantiated if it does not exist.
    private: ros::Publisher point_cloud_pub_;
    private: ros::Publisher depth_image_pub_;
    private: ros::Publisher normal_image_pub_;
    private: ros::Publisher multibeam_image_pub_;
    private: ros::Publisher sonar_image_pub_;
    private: ros::Publisher raw_sonar_image_pub_;

    /// \brief PointCloud2 point cloud message
    private: sensor_msgs::PointCloud2 point_cloud_msg_;
    private: sensor_msgs::Image depth_image_msg_;
    private: sensor_msgs::Image normal_image_msg_;
    private: sensor_msgs::Image multibeam_image_msg_;
    private: sensor_msgs::Image sonar_image_msg_;
    private: sensor_msgs::Image raw_sonar_image_msg_;

    private: double point_cloud_cutoff_;

    /// \brief ROS image topic name
    private: std::string point_cloud_topic_name_;
    std::default_random_engine generator;

    private: void InfoConnect();
    private: void InfoDisconnect();

    using GazeboRosCameraUtils::PublishCameraInfo;
    protected: virtual void PublishCameraInfo();

    /// \brief image where each pixel contains the depth information
    private: std::string depth_image_topic_name_;
    private: std::string depth_image_camera_info_topic_name_;
    private: int depth_info_connect_count_;
    private: void DepthInfoConnect();
    private: void DepthInfoDisconnect();

    // overload with our own
    private: common::Time depth_sensor_update_time_;
    protected: ros::Publisher depth_image_camera_info_pub_;

    private: event::ConnectionPtr load_connection_;
    
	// from DepthCameraPlugin
	protected: unsigned int width, height, depth;
    protected: std::string format;
		
    // precomputed things for the forward-looking sonar
    protected: cv::Mat dist_matrix_;
    std::vector<std::vector<int> > angle_range_indices_;
    std::vector<int> angle_nbr_indices_;

    protected: sensors::DepthCameraSensorPtr parentSensor;
    protected: rendering::DepthCameraPtr depthCamera;

    private: event::ConnectionPtr newDepthFrameConnection;
    private: event::ConnectionPtr newRGBPointCloudConnection;
    private: event::ConnectionPtr newImageFrameConnection;
  };

}
#endif
