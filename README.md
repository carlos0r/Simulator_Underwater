# Simulator Underwater

# Installation 

**WARING !!! : It may be neccessary to install the library libprotobuf-c-dev, make sure you have it installed before installing the simulator** 


Copy the files in your Workspace 

cd ~/name_of_workspace

    git clone https://github.com/castilloherrera/Simulator_Underwater.git

Then, build your workspace using

    cd ~/catkin_ws

    catkin_make install

or

    cd ~/catkin_ws

    catkin build

(in case you are using catkin_tools.)


# Start

Start an empty underwater environment using either

    roslaunch worlds ocean_waves.launch
