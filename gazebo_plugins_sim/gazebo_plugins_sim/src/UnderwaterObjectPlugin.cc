/***************************************************
 * Title: UUV Simulator
 * Author: The UUV Simulator Authors
 * Date: 2016
 * Availability: https://uuvsimulator.github.io/
***************************************************/

#include <gazebo/gazebo.hh>
#include <gazebo/physics/Collision.hh>
#include <gazebo/physics/Link.hh>
#include <gazebo/physics/Model.hh>
#include <gazebo/physics/PhysicsEngine.hh>
#include <gazebo/physics/Shape.hh>
#include <gazebo/physics/World.hh>
#include <gazebo/transport/TransportTypes.hh>
#include <gazebo/transport/transport.hh>

#include <gazebo_plugins_sim/UnderwaterObjectPlugin.hh>
#include <gazebo_plugins_sim/Def.hh>

namespace gazebo {

GZ_REGISTER_MODEL_PLUGIN(UnderwaterObjectPlugin)

/////////////////////////////////////////////////
UnderwaterObjectPlugin::UnderwaterObjectPlugin() : useGlobalCurrent(true)
{
}

/////////////////////////////////////////////////
UnderwaterObjectPlugin::~UnderwaterObjectPlugin()
{
#if GAZEBO_MAJOR_VERSION >= 8
  this->updateConnection.reset();
#else
  event::Events::DisconnectWorldUpdateBegin(this->updateConnection);
#endif
}

/////////////////////////////////////////////////
void UnderwaterObjectPlugin::Load(physics::ModelPtr _model,
                                  sdf::ElementPtr _sdf)
{
  GZ_ASSERT(_model != NULL, "Invalid model pointer");
  GZ_ASSERT(_sdf != NULL, "Invalid SDF element pointer");

  this->model = _model;
  this->world = _model->GetWorld();

  // Initialize the transport node
  this->node = transport::NodePtr(new transport::Node());
  std::string worldName;
#if GAZEBO_MAJOR_VERSION >= 8
  worldName = this->world->Name();
#else
  worldName = this->world->GetName();
#endif
  this->node->Init(worldName);

  // If fluid topic is available, subscribe to it
  if (_sdf->HasElement("flow_velocity_topic"))
  {
    std::string flowTopic = _sdf->Get<std::string>("flow_velocity_topic");
    GZ_ASSERT(!flowTopic.empty(),
              "Fluid velocity topic tag cannot be empty");

    gzmsg << "Subscribing to current velocity topic: " << flowTopic
        << std::endl;
    this->flowSubscriber = this->node->Subscribe(flowTopic,
      &UnderwaterObjectPlugin::UpdateFlowVelocity, this);
  }

  double fluidDensity = 1028.0;
  // Get the fluid density, if present
  if (_sdf->HasElement("fluid_density"))
    fluidDensity = _sdf->Get<double>("fluid_density");

  if (_sdf->HasElement("use_global_current"))
    this->useGlobalCurrent = _sdf->Get<bool>("use_global_current");

  // Get the debug flag, if available
  bool debugFlag = false;
  if (_sdf->HasElement("debug"))
    debugFlag = static_cast<bool>(_sdf->Get<int>("debug"));

  // Center of buoyancy
  ignition::math::Vector3d cob;
  // g
  double gAcc;
#if GAZEBO_MAJOR_VERSION >= 8
  gAcc = std::abs(this->world->Gravity().Z());
#else
  gAcc = std::abs(this->world->GetPhysicsEngine()->GetGravity().z);
#endif
  this->baseLinkName = std::string();
  if (_sdf->HasElement("link"))
  {
    for (sdf::ElementPtr linkElem = _sdf->GetElement("link"); linkElem;
         linkElem = linkElem->GetNextElement("link"))
    {
      physics::LinkPtr link;
      std::string linkName = "";

      if (linkElem->HasAttribute("name"))
      {
        linkName = linkElem->Get<std::string>("name");
        std::size_t found = linkName.find("base_link");
        if (found != std::string::npos)
        {
          this->baseLinkName = linkName;
          gzmsg << "Name of the BASE_LINK: " << this->baseLinkName << std::endl;
        }

        link = this->model->GetLink(linkName);
        if (!link)
        {
          gzwarn << "Specified link [" << linkName << "] not found."
                 << std::endl;
          continue;
        }
      }
      else
      {
        gzwarn << "Attribute name missing from link [" << linkName
               << "]" << std::endl;
        continue;
      }

      // Creating a new hydrodynamic model for this link
      HydrodynamicModelPtr hydro;
      hydro.reset(
        HydrodynamicModelFactory::GetInstance().CreateHydrodynamicModel(
        linkElem, link));
      hydro->SetFluidDensity(fluidDensity);
      hydro->SetGravity(gAcc);

      if (debugFlag)
        this->InitDebug(link, hydro);

      this->models[link] = hydro;
      this->models[link]->Print("all");
    }  // for each link mentioned in plugin sdf
  }

  // Connect the update event callback
  this->Connect();
}

/////////////////////////////////////////////////
void UnderwaterObjectPlugin::InitDebug(physics::LinkPtr _link,
  HydrodynamicModelPtr _hydro)
{
  // Create topics for the individual hydrodynamic and hydrostatic forces
  std::string rootTopic = "/debug/forces/" + _link->GetName() + "/";
  std::vector<std::string> topics {"restoring", "damping", "added_mass",
    "added_coriolis"};
  for (auto topic : topics)
  {
    this->hydroPub[_link->GetName() + "/" + topic] =
      this->node->Advertise<msgs::WrenchStamped>(rootTopic + topic);
  }

  // Setup the vectors to be stored
  _hydro->SetDebugFlag(true);
  _hydro->SetStoreVector(RESTORING_FORCE);
  _hydro->SetStoreVector(DAMPING_FORCE);
  _hydro->SetStoreVector(DAMPING_TORQUE);
  _hydro->SetStoreVector(ADDED_CORIOLIS_FORCE);
  _hydro->SetStoreVector(ADDED_CORIOLIS_TORQUE);
  _hydro->SetStoreVector(ADDED_MASS_FORCE);
  _hydro->SetStoreVector(ADDED_MASS_TORQUE);
}

/////////////////////////////////////////////////
void UnderwaterObjectPlugin::Init()
{
  // Doing nothing for now
}

/////////////////////////////////////////////////
void UnderwaterObjectPlugin::Update(const common::UpdateInfo &_info)
{
  double time = _info.simTime.Double();

  for (std::map<gazebo::physics::LinkPtr,
       HydrodynamicModelPtr>::iterator it = models.begin();
       it != models.end(); ++it)
  {
    physics::LinkPtr link = it->first;
    HydrodynamicModelPtr hydro = it->second;
    // Apply hydrodynamic and hydrostatic forces and torques
    double linearAccel, angularAccel;
#if GAZEBO_MAJOR_VERSION >= 8
    linearAccel = link->RelativeLinearAccel().Length();
    angularAccel = link->RelativeAngularAccel().Length();
#else
    linearAccel = link->GetRelativeLinearAccel().GetLength();
    angularAccel = link->GetRelativeAngularAccel().GetLength();
#endif

    GZ_ASSERT(!std::isnan(linearAccel) && !std::isnan(angularAccel),
      "Linear or angular accelerations are invalid.");

    hydro->ApplyHydrodynamicForces(time, this->flowVelocity);
    this->PublishRestoringForce(link);
    this->PublishHydrodynamicWrenches(link);
    this->PublishCurrentVelocityMarker();
    this->PublishIsSubmerged();
  }
}

/////////////////////////////////////////////////
void UnderwaterObjectPlugin::Connect()
{
  // Connect the update event
  this->updateConnection = event::Events::ConnectWorldUpdateBegin(
        boost::bind(&UnderwaterObjectPlugin::Update,
                    this, _1));
}

/////////////////////////////////////////////////
void UnderwaterObjectPlugin::PublishCurrentVelocityMarker()
{
  // Does nothing for now
  return;
}

/////////////////////////////////////////////////
void UnderwaterObjectPlugin::PublishIsSubmerged()
{
  // Does nothing for now
  return;
}

/////////////////////////////////////////////////
void UnderwaterObjectPlugin::UpdateFlowVelocity(ConstVector3dPtr &_msg)
{
  if (this->useGlobalCurrent)
  {
    this->flowVelocity.X() = _msg->x();
    this->flowVelocity.Y() = _msg->y();
    this->flowVelocity.Z() = _msg->z();
  }
}

/////////////////////////////////////////////////
void UnderwaterObjectPlugin::PublishRestoringForce(
  physics::LinkPtr _link)
{
  if (this->models.count(_link))
  {
    if (!this->models[_link]->GetDebugFlag())
      return;

    ignition::math::Vector3d restoring = this->models[_link]->GetStoredVector(
      RESTORING_FORCE);

    msgs::WrenchStamped msg;
    this->GenWrenchMsg(restoring, ignition::math::Vector3d(0, 0, 0), msg);
    this->hydroPub[_link->GetName() + "/restoring"]->Publish(msg);
  }
}

/////////////////////////////////////////////////
void UnderwaterObjectPlugin::PublishHydrodynamicWrenches(
  physics::LinkPtr _link)
{
  if (this->models.count(_link))
  {
    if (!this->models[_link]->GetDebugFlag())
      return;
    msgs::WrenchStamped msg;
    ignition::math::Vector3d force, torque;

    // Publish wrench generated by the acceleraton of fluid aroung the object
    force = this->models[_link]->GetStoredVector(ADDED_MASS_FORCE);
    torque = this->models[_link]->GetStoredVector(ADDED_MASS_TORQUE);

    this->GenWrenchMsg(force, torque, msg);
    this->hydroPub[_link->GetName() + "/added_mass"]->Publish(msg);

    // Publish wrench generated by the acceleraton of fluid aroung the object
    force = this->models[_link]->GetStoredVector(DAMPING_FORCE);
    torque = this->models[_link]->GetStoredVector(DAMPING_TORQUE);

    this->GenWrenchMsg(force, torque, msg);
    this->hydroPub[_link->GetName() + "/damping"]->Publish(msg);

    // Publish wrench generated by the acceleraton of fluid aroung the object
    force = this->models[_link]->GetStoredVector(ADDED_CORIOLIS_FORCE);
    torque = this->models[_link]->GetStoredVector(ADDED_CORIOLIS_TORQUE);

    this->GenWrenchMsg(force, torque, msg);
    this->hydroPub[_link->GetName() + "/added_coriolis"]->Publish(msg);
  }
}

/////////////////////////////////////////////////
void UnderwaterObjectPlugin::GenWrenchMsg(ignition::math::Vector3d _force,
  ignition::math::Vector3d _torque, gazebo::msgs::WrenchStamped &_output)
{
  common::Time curTime;
#if GAZEBO_MAJOR_VERSION >= 8
  curTime = this->world->SimTime();
#else
  curTime = this->world->GetSimTime();
#endif

  msgs::Wrench * wrench = _output.mutable_wrench();
  msgs::Time * t = _output.mutable_time();
  msgs::Vector3d * msgForce = wrench->mutable_force();
  msgs::Vector3d * msgTorque = wrench->mutable_torque();

  msgs::Set(msgTorque,
    ignition::math::Vector3d(_torque.X(), _torque.Y(), _torque.Z()));
  msgs::Set(msgForce,
    ignition::math::Vector3d(_force.X(), _force.Y(), _force.Z()));

  t->set_sec(curTime.sec);
  t->set_nsec(curTime.nsec);
}
}
