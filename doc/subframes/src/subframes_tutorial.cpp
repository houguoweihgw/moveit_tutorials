/*********************************************************************
* Software License Agreement (BSD License)
*
*  Copyright (c) 2019, Felix von Drigalski
*  All rights reserved.
*
*  Redistribution and use in source and binary forms, with or without
*  modification, are permitted provided that the following conditions
*  are met:
*
*   * Redistributions of source code must retain the above copyright
*     notice, this list of conditions and the following disclaimer.
*   * Redistributions in binary form must reproduce the above
*     copyright notice, this list of conditions and the following
*     disclaimer in the documentation and/or other materials provided
*     with the distribution.
*   * Neither the name of OMRON SINIC X nor the names of its
*     contributors may be used to endorse or promote products derived
*     from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
*  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
*  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
*  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
*  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
*  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
*  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
*  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
*  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
*  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
*  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
*  POSSIBILITY OF SUCH DAMAGE.
*********************************************************************/

/* Author: Felix von Drigalski*/

// ROS
#include <ros/ros.h>

// MoveIt
#include <moveit/planning_scene_interface/planning_scene_interface.h>
#include <moveit/move_group_interface/move_group_interface.h>

// TF2
#include <tf2_geometry_msgs/tf2_geometry_msgs.h>

// BEGIN_SUB_TUTORIAL plan1
//
// Creating the planning request
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// In this tutorial, we use a small helper function to create our planning requests and move the robot.
bool moveToCartPose(geometry_msgs::PoseStamped pose, moveit::planning_interface::MoveGroupInterface& group,
                    std::string end_effector_link)
{
  // To use subframes of objects that are attached to the robot in planning, you need to set the end effector of your
  // move_group to the subframe of the object. The format has to be ``object_name/subframe_name``, as shown
  // in the "*Example 1*" line.
  // Do not forget to reset your end_effector_link to a robot link when you detach your object, and the subframe
  // is not part of your robot anymore!
  group.clearPoseTargets();
  group.setEndEffectorLink(end_effector_link);
  /*
  group.setEndEffectorLink("cylinder/tip");    // Example 1
  group.setEndEffectorLink("panda_hand");      // Example 2
  */
  group.setStartStateToCurrentState();
  group.setPoseTarget(pose);

  // The rest of the planning is done as usual. Naturally, you can also use the ``go()`` command instead of
  // ``plan()`` and ``execute()``.
  ROS_INFO_STREAM("Planning motion to pose:");
  ROS_INFO_STREAM(pose.pose.position.x << ", " << pose.pose.position.y << ", " << pose.pose.position.z);
  moveit::planning_interface::MoveGroupInterface::Plan myplan;
  if (group.plan(myplan) && group.execute(myplan))
    return true;

  ROS_WARN("Failed to perform motion.");
  return false;
}
// END_SUB_TUTORIAL

// BEGIN_SUB_TUTORIAL object1
//
// Defining two CollisionObjects with subframes
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// This helper function creates two objects and publishes them to the PlanningScene: a box and a cylinder.
// The box spawns in front of the gripper, the cylinder at the tip of the gripper, as if it had been grasped.

void spawnCollisionObjects(moveit::planning_interface::PlanningSceneInterface& planning_scene_interface)
{
  double z_offset_box = .25;  // The z-axis points away from the gripper
  double z_offset_cylinder = .12;

  // First, we start defining the `CollisionObject <http://docs.ros.org/api/moveit_msgs/html/msg/CollisionObject.html>`_
  // as usual.
  moveit_msgs::CollisionObject box;
  box.id = "box";
  box.header.frame_id = "panda_hand";
  box.primitives.resize(1);
  box.primitive_poses.resize(1);
  box.primitives[0].type = box.primitives[0].BOX;
  box.primitives[0].dimensions.resize(3);
  box.primitives[0].dimensions[0] = 0.05;
  box.primitives[0].dimensions[1] = 0.1;
  box.primitives[0].dimensions[2] = 0.02;
  box.primitive_poses[0].position.z = z_offset_box;

  // Then, we define the subframes of the CollisionObject. The subframes are defined in the ``frame_id`` coordinate
  // system, just like the shapes that make up the object. Each subframe consists of a name and a pose.
  // In this tutorial, we set the orientation of the subframes so that the z-axis of the subframe
  // points away from the object.
  // This is not strictly necessary, but it is helpful to follow a convention, and it avoids confusion when
  // setting the orientation of the target pose later on.
  box.subframe_names.resize(5);
  box.subframe_poses.resize(5);

  box.subframe_names[0] = "bottom";
  box.subframe_poses[0].position.y = -.05;
  box.subframe_poses[0].position.z = 0.0 + z_offset_box;

  tf2::Quaternion orientation;
  orientation.setRPY((90.0 / 180.0 * M_PI), 0, 0);
  box.subframe_poses[0].orientation = tf2::toMsg(orientation);
  // END_SUB_TUTORIAL

  box.subframe_names[1] = "top";
  box.subframe_poses[1].position.y = .05;
  box.subframe_poses[1].position.z = 0.0 + z_offset_box;
  orientation.setRPY(-(90.0 / 180.0 * M_PI), 0, 0);
  box.subframe_poses[1].orientation = tf2::toMsg(orientation);

  box.subframe_names[2] = "corner_1";
  box.subframe_poses[2].position.x = -.025;
  box.subframe_poses[2].position.y = -.05;
  box.subframe_poses[2].position.z = -.01 + z_offset_box;
  orientation.setRPY((90.0 / 180.0 * M_PI), 0, 0);
  box.subframe_poses[2].orientation = tf2::toMsg(orientation);

  box.subframe_names[3] = "corner_2";
  box.subframe_poses[3].position.x = .025;
  box.subframe_poses[3].position.y = -.05;
  box.subframe_poses[3].position.z = -.01 + z_offset_box;
  orientation.setRPY((90.0 / 180.0 * M_PI), 0, 0);
  box.subframe_poses[3].orientation = tf2::toMsg(orientation);

  box.subframe_names[4] = "side";
  box.subframe_poses[4].position.x = .0;
  box.subframe_poses[4].position.y = .0;
  box.subframe_poses[4].position.z = -.01 + z_offset_box;
  orientation.setRPY(0, (180.0 / 180.0 * M_PI), 0);
  box.subframe_poses[4].orientation = tf2::toMsg(orientation);

  // Next, define the cylinder
  moveit_msgs::CollisionObject cylinder;
  cylinder.id = "cylinder";
  cylinder.header.frame_id = "panda_hand";
  cylinder.primitives.resize(1);
  cylinder.primitive_poses.resize(1);
  cylinder.primitives[0].type = box.primitives[0].CYLINDER;
  cylinder.primitives[0].dimensions.resize(2);
  cylinder.primitives[0].dimensions[0] = 0.06;   // height (along x)
  cylinder.primitives[0].dimensions[1] = 0.005;  // radius
  cylinder.primitive_poses[0].position.x = 0.0;
  cylinder.primitive_poses[0].position.y = 0.0;
  cylinder.primitive_poses[0].position.z = 0.0 + z_offset_cylinder;
  orientation.setRPY(0, (90.0 / 180.0 * M_PI), 0);
  cylinder.primitive_poses[0].orientation = tf2::toMsg(orientation);

  cylinder.subframe_poses.resize(1);
  cylinder.subframe_names.resize(1);
  cylinder.subframe_names[0] = "tip";
  cylinder.subframe_poses[0].position.x = 0.03;
  cylinder.subframe_poses[0].position.y = 0.0;
  cylinder.subframe_poses[0].position.z = 0.0 + z_offset_cylinder;
  orientation.setRPY(0, (90.0 / 180.0 * M_PI), 0);
  cylinder.subframe_poses[0].orientation = tf2::toMsg(orientation);

  // BEGIN_SUB_TUTORIAL object2
  // Lastly, the objects are published to the PlanningScene. In this tutorial, we publish a box and a cylinder.
  box.operation = moveit_msgs::CollisionObject::ADD;
  cylinder.operation = moveit_msgs::CollisionObject::ADD;
  std::vector<moveit_msgs::CollisionObject> collision_objects = { box, cylinder };
  planning_scene_interface.applyCollisionObjects(collision_objects);
}
// END_SUB_TUTORIAL

int main(int argc, char** argv)
{
  ros::init(argc, argv, "panda_arm_subframes");
  ros::NodeHandle nh;
  ros::AsyncSpinner spinner(1);
  spinner.start();

  ros::WallDuration(1.0).sleep();
  moveit::planning_interface::PlanningSceneInterface planning_scene_interface;
  moveit::planning_interface::MoveGroupInterface group("panda_arm");
  group.setPlanningTime(10.0);

  // BEGIN_SUB_TUTORIAL sceneprep
  // Preparing the scene
  // ^^^^^^^^^^^^^^^^^^^
  // In the main function, we first spawn the objects in the planning scene, then attach the cylinder to the robot.
  // Attaching the cylinder turns it purple in Rviz.
  spawnCollisionObjects(planning_scene_interface);
  moveit_msgs::AttachedCollisionObject att_coll_object;
  att_coll_object.object.id = "cylinder";
  att_coll_object.link_name = "panda_hand";
  att_coll_object.object.operation = att_coll_object.object.ADD;
  ROS_INFO_STREAM("Attaching cylinder to robot.");
  planning_scene_interface.applyAttachedCollisionObject(att_coll_object);
  // END_SUB_TUTORIAL

  // Define a pose in the robot base.
  tf2::Quaternion orientation, orientation_1, target_orientation;
  geometry_msgs::PoseStamped fixed_pose, temp_pose_stamped;
  fixed_pose.header.frame_id = "panda_link0";
  fixed_pose.pose.position.y = -.4;
  fixed_pose.pose.position.z = .3;
  orientation.setRPY(0, (-20.0 / 180.0 * M_PI), 0);
  fixed_pose.pose.orientation = tf2::toMsg(orientation);

  // BEGIN_SUB_TUTORIAL quaternions1
  // Setting the orientation
  // ^^^^^^^^^^^^^^^^^^^^^^^
  // We multiply tf2 quaternions to chain together a few rotations. This is usually simpler and easier
  // to debug than figuring out a series of euler angles in your head.
  tf2::Quaternion flip_around_y;  // This is used to rotate the orientation of the target pose.
  flip_around_y.setRPY(0, (180.0 / 180.0 * M_PI), 0);
  // END_SUB_TUTORIAL

  // Set up a small command line interface to make the tutorial interactive.
  int character_input;
  while (ros::ok())
  {
    ROS_INFO("==========================\n"
             "Press a key and hit Enter to execute an action. \n0 to exit"
             "\n1 to move cylinder tip to box bottom \n2 to move cylinder tip to box top"
             "\n3 to move cylinder tip to box corner 1 \n4 to move cylinder tip to box corner 2"
             "\n5 to move cylinder tip to side of box"
             "\n6 to return the robot to the start pose"
             "\n7 to move the robot's wrist to a cartesian pose"
             "\n8 to move cylinder/tip to the same cartesian pose"
             "\n----------"
             "\n10 to remove box and cylinder from the scene"
             "\n11 to spawn box and cylinder"
             "\n12 to attach the cylinder to the gripper\n");
    std::cin >> character_input;
    if (character_input == 0)
    {
      return 0;
    }
    else if (character_input == 1)
    {
      ROS_INFO_STREAM("Moving to bottom of box with cylinder tip");
      temp_pose_stamped.header.frame_id = "box/bottom";

      // BEGIN_SUB_TUTORIAL quaternions2
      // When constructing the target pose for the robot, we multiply the quaternions to get the
      // target orientation and convert the result to a ``geometry_msgs/orientation`` message.
      orientation_1.setRPY(-(90.0 / 180.0 * M_PI), 0, 0);
      target_orientation = flip_around_y * orientation_1;
      temp_pose_stamped.pose.orientation = tf2::toMsg(target_orientation);
      temp_pose_stamped.pose.position.z = 0.01;
      moveToCartPose(temp_pose_stamped, group, "cylinder/tip");
      // END_SUB_TUTORIAL
    }
    // BEGIN_SUB_TUTORIAL move_example
    // The command "2" moves the cylinder tip to the top of the box (the right side in the top animation).
    else if (character_input == 2)
    {
      ROS_INFO_STREAM("Moving to top of box with cylinder tip");
      temp_pose_stamped.header.frame_id = "box/top";
      orientation_1.setRPY((90.0 / 180.0 * M_PI), 0, 0);
      target_orientation = flip_around_y * orientation_1;
      temp_pose_stamped.pose.orientation = tf2::toMsg(target_orientation);
      temp_pose_stamped.pose.position.z = 0.01;
      moveToCartPose(temp_pose_stamped, group, "cylinder/tip");
    }
    // END_SUB_TUTORIAL
    else if (character_input == 3)
    {
      ROS_INFO_STREAM("Moving to top of box with cylinder tip");
      temp_pose_stamped.header.frame_id = "box/corner_1";
      orientation_1.setRPY(-(90.0 / 180.0 * M_PI), 0, 0);
      target_orientation = flip_around_y * orientation_1;
      temp_pose_stamped.pose.orientation = tf2::toMsg(target_orientation);
      temp_pose_stamped.pose.position.z = 0.01;
      moveToCartPose(temp_pose_stamped, group, "cylinder/tip");
    }
    else if (character_input == 4)
    {
      temp_pose_stamped.header.frame_id = "box/corner_2";
      orientation_1.setRPY(-(90.0 / 180.0 * M_PI), 0, 0);
      target_orientation = flip_around_y * orientation_1;
      temp_pose_stamped.pose.orientation = tf2::toMsg(target_orientation);
      temp_pose_stamped.pose.position.z = 0.01;
      moveToCartPose(temp_pose_stamped, group, "cylinder/tip");
    }
    else if (character_input == 5)
    {
      temp_pose_stamped.header.frame_id = "box/side";
      orientation_1.setRPY(-(90.0 / 180.0 * M_PI), 0, 0);
      target_orientation = flip_around_y * orientation_1;
      temp_pose_stamped.pose.orientation = tf2::toMsg(target_orientation);
      temp_pose_stamped.pose.position.z = 0.01;
      moveToCartPose(temp_pose_stamped, group, "cylinder/tip");
    }
    else if (character_input == 6)
    {
      // Go to neutral home pose
      group.clearPoseTargets();
      group.setNamedTarget("ready");
      group.move();
    }
    else if (character_input == 7)
    {
      ROS_INFO_STREAM("Moving to a pose with robot wrist");
      moveToCartPose(fixed_pose, group, "panda_hand");
    }
    else if (character_input == 8)
    {
      ROS_INFO_STREAM("Moving to a pose with cylinder tip");
      moveToCartPose(fixed_pose, group, "cylinder/tip");
    }
    else if (character_input == 10)
    {
      try
      {
        ROS_INFO_STREAM("Removing box and cylinder.");
        moveit_msgs::AttachedCollisionObject att_coll_object;
        att_coll_object.object.id = "box";
        att_coll_object.object.operation = att_coll_object.object.REMOVE;
        planning_scene_interface.applyAttachedCollisionObject(att_coll_object);

        att_coll_object.object.id = "cylinder";
        att_coll_object.object.operation = att_coll_object.object.REMOVE;
        planning_scene_interface.applyAttachedCollisionObject(att_coll_object);

        moveit_msgs::CollisionObject co1, co2;
        co1.id = "box";
        co1.operation = moveit_msgs::CollisionObject::REMOVE;
        co2.id = "cylinder";
        co2.operation = moveit_msgs::CollisionObject::REMOVE;
        std::vector<moveit_msgs::CollisionObject> collision_objects = { co1, co2 };
        planning_scene_interface.applyCollisionObjects(collision_objects);
      }
      catch (const std::exception& exc)
      {
        ROS_WARN_STREAM(exc.what());
      }
    }
    else if (character_input == 11)
    {
      ROS_INFO_STREAM("Respawning test box and cylinder.");
      spawnCollisionObjects(planning_scene_interface);
    }
    else if (character_input == 12)
    {
      moveit_msgs::AttachedCollisionObject att_coll_object;
      att_coll_object.object.id = "cylinder";
      att_coll_object.link_name = "panda_hand";
      att_coll_object.object.operation = att_coll_object.object.ADD;
      ROS_INFO_STREAM("Attaching cylinder to robot.");
      planning_scene_interface.applyAttachedCollisionObject(att_coll_object);
    }
    else
    {
      ROS_INFO("Could not read input. Quitting.");
      break;
    }
  }

  ros::waitForShutdown();
  return 0;
}

// BEGIN_TUTORIAL
// CALL_SUB_TUTORIAL object1
// CALL_SUB_TUTORIAL object2
// CALL_SUB_TUTORIAL plan1
//
// CALL_SUB_TUTORIAL sceneprep
//
// Interactively testing the robot
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
// We set up a small command line interface so you can interact with the simulation and see how it responds to certain
// commands. You can use it to experiment with the behavior of the robot when you remove the box and cylinder, respawn
// and reattach them, or create new planning requests. Try moving the robot into a new position and respawn the box
// and cylinder there (they are spawned relative to the robot wrist).
// Or try commands 7 and 8 to move different frames to the same position in space.
//
// CALL_SUB_TUTORIAL move_example
//
// CALL_SUB_TUTORIAL quaternions1
// CALL_SUB_TUTORIAL quaternions2
//
// END_TUTORIAL
