// Publish robot status into Rviz for visualization and debugging


#include <iostream>
#include <stdlib.h>
#include "Beliefs.h"
#include <sstream>

#include <ros/ros.h>
#include <ros/console.h>
#include <geometry_msgs/Twist.h>
#include <geometry_msgs/PoseStamped.h>
#include <geometry_msgs/PoseArray.h>
#include <geometry_msgs/PointStamped.h>
#include <nav_msgs/OccupancyGrid.h>
#include <nav_msgs/Path.h>
#include <sensor_msgs/LaserScan.h>
#include <tf/transform_datatypes.h>
#include <visualization_msgs/Marker.h>
#include <visualization_msgs/MarkerArray.h>
#include <std_msgs/String.h>
#include <string>
#include <semaforr/CrowdModel.h>

using namespace std;


class Visualizer
{
private:
  //! We will be publishing to the "target_point" topic to display target point on rviz
  ros::Publisher target_pub_;
  ros::Publisher waypoint_pub_;
  ros::Publisher all_targets_pub_;
  ros::Publisher remaining_targets_pub_;
  ros::Publisher region_pub_;
  ros::Publisher conveyor_pub_;
  ros::Publisher occupancy_pub_;
  ros::Publisher trails_pub_;
  ros::Publisher plan_pub_;
  ros::Publisher nodes1_pub_;
  ros::Publisher nodes2_pub_;
  ros::Publisher edges_pub_;
  ros::Publisher edges_cost_pub_;
  ros::Publisher stats_pub_;
  ros::Publisher doors_pub_;
  ros::Publisher walls_pub_;
  Controller *con;
  Beliefs *beliefs;
  ros::NodeHandle *nh_;
  int visualized;

public:
  //! ROS node initialization
  Visualizer(ros::NodeHandle *nh, Controller *c)
  {
    nh_ = nh;
    visualized = false;
    //set up the publisher for the cmd_vel topic
    target_pub_ = nh_->advertise<geometry_msgs::PointStamped>("target_point", 1);
    waypoint_pub_ = nh_->advertise<geometry_msgs::PointStamped>("waypoint", 1);
    all_targets_pub_ = nh_->advertise<geometry_msgs::PoseArray>("all_targets", 1);
    remaining_targets_pub_ = nh_->advertise<geometry_msgs::PoseArray>("remaining_targets", 1);
    plan_pub_ = nh_->advertise<nav_msgs::Path>("plan", 1);

    conveyor_pub_ = nh_->advertise<nav_msgs::OccupancyGrid>("conveyor", 1);
    occupancy_pub_ = nh_->advertise<nav_msgs::OccupancyGrid>("occupancy", 1);
    region_pub_ = nh_->advertise<visualization_msgs::MarkerArray>("region", 1);
    nodes1_pub_ = nh_->advertise<visualization_msgs::Marker>("nodes1", 1);
    nodes2_pub_ = nh_->advertise<visualization_msgs::Marker>("nodes2", 1);
    edges_pub_ = nh_->advertise<visualization_msgs::MarkerArray>("edges", 1);
    edges_cost_pub_ = nh_->advertise<visualization_msgs::MarkerArray>("edges_cost", 1);
    //trails_pub_ = nh_->advertise<nav_msgs::Path>("trail", 1);
    trails_pub_ = nh_->advertise<visualization_msgs::Marker>("trail", 1);
    stats_pub_ = nh_->advertise<std_msgs::String>("decision_log", 1);
    doors_pub_ = nh_->advertise<visualization_msgs::Marker>("door", 1);
    walls_pub_ = nh_->advertise<visualization_msgs::Marker>("walls", 1);
    //declare and create a controller with task, action and advisor configuration
    con = c;
    beliefs = con->getBeliefs();
  }

  void publish(){
	if(beliefs->getAgentState()->getCurrentTask() != NULL){
		publish_next_target();
		publish_next_waypoint();
		publish_plan();
		publish_all_targets();
		publish_remaining_targets();
	}
	//publish_nodes();
	//publish_reachable_nodes();
	//publish_edges();
        /*if(visualized < 5){
		publish_nodes();
		publish_reachable_nodes();
		//publish_edges();
		visualized++;
	}*/
	//publish_edges_cost();
	publish_conveyor();
	publish_region();
	publish_trails();
	publish_doors();
	publish_walls();
	//publish_occupancy();
  }


  void publish_nodes(){
	visualization_msgs::Marker marker;
	marker.header.frame_id = "map";
    	marker.header.stamp = ros::Time::now();
	marker.ns = "basic_shapes";
	marker.type = visualization_msgs::Marker::POINTS;
    	marker.pose.position.x = 0;
    	marker.pose.position.y = 0;
    	marker.pose.position.z = 0;
    	marker.pose.orientation.x = 0.0;
    	marker.pose.orientation.y = 0.0;
    	marker.pose.orientation.z = 0.0;
    	marker.pose.orientation.w = 1.0;
	// Set the scale of the marker -- 1x1x1 here means 1m on a side
	marker.scale.x = marker.scale.y = 0.15;
	marker.scale.z = 0.15;
	// Set the color -- be sure to set alpha to something non-zero!
	marker.color.r = 0.0f;
	marker.color.g = 0.0f;
	marker.color.b = 1.0f;
	marker.color.a = 0.5;
	marker.lifetime = ros::Duration();

	vector<Edge*> edges = con->getPlanner()->getGraph()->getEdges();
	vector<Node*> nodes = con->getPlanner()->getGraph()->getNodes();

	for( int i = 0; i < nodes.size(); i++ ){
		if(nodes[i]->getInBuffer()){
			float x = nodes[i]->getX()/100.0;
			float y = nodes[i]->getY()/100.0;
			geometry_msgs::Point point;
			point.x = x;
			point.y = y;
			point.z = 0;
			marker.points.push_back(point);
		}
	}
	nodes1_pub_.publish(marker);
  }

  void publish_reachable_nodes(){
	cout << "publish nodes" << endl;
	visualization_msgs::Marker marker;
	marker.header.frame_id = "map";
    	marker.header.stamp = ros::Time::now();
	marker.ns = "basic_shapes";
	marker.type = visualization_msgs::Marker::POINTS;
    	marker.pose.position.x = 0;
    	marker.pose.position.y = 0;
    	marker.pose.position.z = 0;
    	marker.pose.orientation.x = 0.0;
    	marker.pose.orientation.y = 0.0;
    	marker.pose.orientation.z = 0.0;
    	marker.pose.orientation.w = 1.0;
	// Set the scale of the marker -- 1x1x1 here means 1m on a side
	marker.scale.x = marker.scale.y = 0.15;
	marker.scale.z = 0.15;
	// Set the color -- be sure to set alpha to something non-zero!
	marker.color.r = 0.0f;
	marker.color.g = 0.0f;
	marker.color.b = 1.0f;
	marker.color.a = 0.5;
	marker.lifetime = ros::Duration();

	vector<Edge*> edges = con->getPlanner()->getGraph()->getEdges();
	vector<Node*> nodes = con->getPlanner()->getGraph()->getNodes();

	for( int i = 0; i < nodes.size(); i++ ){
		if(!nodes[i]->getInBuffer()){
			float x = nodes[i]->getX()/100.0;
			float y = nodes[i]->getY()/100.0;
			geometry_msgs::Point point;
			point.x = x;
			point.y = y;
			point.z = 0;
			marker.points.push_back(point);
		}
	}
	nodes2_pub_.publish(marker);
  }

  void publish_edges_cost(){
	visualization_msgs::MarkerArray markerArrayCost;

	Graph *graph = con->getPlanner()->getGraph();
	vector<Edge*> edges = graph->getEdges();
	vector<Node*> nodes = graph->getNodes();

	for( int i = 0; i < edges.size(); i++ ){
		visualization_msgs::Marker costMarker;

		costMarker.header.frame_id = "map";
	    	costMarker.header.stamp = ros::Time::now();
		costMarker.ns = "basic_shapes";
		costMarker.id = i;
		costMarker.type = visualization_msgs::Marker::TEXT_VIEW_FACING;
		costMarker.action = visualization_msgs::Marker::ADD;
	    	costMarker.pose.orientation.x = 0.0;
	    	costMarker.pose.orientation.y = 0.0;
	    	costMarker.pose.orientation.z = 0.0;
	    	costMarker.pose.orientation.w = 1.0;
		// Set the scale of the marker -- 1x1x1 here means 1m on a side
		costMarker.scale.x = 1;

		// Set the color -- be sure to set alpha to something non-zero!
		costMarker.color.r = 0.0f;
		costMarker.color.g = 1.0f;
		costMarker.color.b = 0.0f;
		costMarker.color.a = 0.5;
		costMarker.lifetime = ros::Duration();


		Node *from = graph->getNodePtr(edges[i]->getFrom());
		Node *to = graph->getNodePtr(edges[i]->getTo());
		geometry_msgs::Point f;
		f.x = from->getX()/100.0;
		f.y = from->getY()/100.0;
		f.z = 0;
		geometry_msgs::Point t;
		t.x = to->getX()/100.0;
		t.y = to->getY()/100.0;
		t.z = 0;

		costMarker.pose.position.x = (f.x + t.x)/2.0;
		costMarker.pose.position.y = (f.y + t.y)/2.0;
		costMarker.pose.position.z = 0;
		//to_string(edges[i]->getCost(true)) + " " + to_string(edges[i]->getCost(false));
		costMarker.text = "a";

		markerArrayCost.markers.push_back(costMarker);
	}
	edges_cost_pub_.publish(markerArrayCost);
  }

  void publish_edges(){
	visualization_msgs::MarkerArray markerArray;

	Graph *graph = con->getPlanner()->getGraph();
	vector<Edge*> edges = graph->getEdges();
	vector<Node*> nodes = graph->getNodes();

	for( int i = 0; i < edges.size(); i++ ){
		visualization_msgs::Marker marker;
		marker.header.frame_id = "map";
	    	marker.header.stamp = ros::Time::now();
		marker.ns = "basic_shapes";
		marker.id = i;
		marker.type = visualization_msgs::Marker::LINE_LIST;
	    	marker.pose.position.x = 0;
	    	marker.pose.position.y = 0;
	    	marker.pose.position.z = 0;
	    	marker.pose.orientation.x = 0.0;
	    	marker.pose.orientation.y = 0.0;
	    	marker.pose.orientation.z = 0.0;
	    	marker.pose.orientation.w = 1.0;
		// Set the scale of the marker -- 1x1x1 here means 1m on a side
		marker.scale.x = 0.1;

		// Set the color -- be sure to set alpha to something non-zero!
		marker.color.r = 0.0f;
		marker.color.g = 1.0f;
		marker.color.b = 0.0f;
		marker.color.a = 0.5;
		marker.lifetime = ros::Duration();

		Node *from = graph->getNodePtr(edges[i]->getFrom());
		Node *to = graph->getNodePtr(edges[i]->getTo());
		geometry_msgs::Point f;
		f.x = from->getX()/100.0;
		f.y = from->getY()/100.0;
		f.z = 0;
		geometry_msgs::Point t;
		t.x = to->getX()/100.0;
		t.y = to->getY()/100.0;
		t.z = 0;
		marker.points.push_back(f);
		marker.points.push_back(t);

		markerArray.markers.push_back(marker);
	}
	edges_pub_.publish(markerArray);
  }

  void publishLog(FORRAction decision, double overallTimeSec, double computationTimeSec){
	publish_log(decision, overallTimeSec, computationTimeSec);
  }

  void publish_next_target(){
	ROS_DEBUG("Inside visualization tool!!");
	geometry_msgs::PointStamped target;
	target.header.frame_id = "map";
	target.header.stamp = ros::Time::now();
	target.point.x = beliefs->getAgentState()->getCurrentTask()->getTaskX();
	target.point.y = beliefs->getAgentState()->getCurrentTask()->getTaskY();
	target.point.z = 0;
	target_pub_.publish(target);
  }

  void publish_next_waypoint(){
	ROS_DEBUG("Inside visualization tool!!");
	geometry_msgs::PointStamped waypoint;
	waypoint.header.frame_id = "map";
	waypoint.header.stamp = ros::Time::now();
	waypoint.point.x = beliefs->getAgentState()->getCurrentTask()->getX();
	waypoint.point.y = beliefs->getAgentState()->getCurrentTask()->getY();
	waypoint.point.z = 0;
	waypoint_pub_.publish(waypoint);
  }


  void publish_plan(){
	ROS_DEBUG("Inside publish plan!!");
	nav_msgs::Path path;
	path.header.frame_id = "map";
	path.header.stamp = ros::Time::now();

	vector <CartesianPoint> waypoints = beliefs->getAgentState()->getCurrentTask()->getWaypoints();

	for(int i = 0; i < waypoints.size(); i++){
		geometry_msgs::PoseStamped poseStamped;
		poseStamped.header.frame_id = "map";
		poseStamped.header.stamp = path.header.stamp;
		poseStamped.pose.position.x = waypoints[i].get_x();
		poseStamped.pose.position.y = waypoints[i].get_y();
		path.poses.push_back(poseStamped);
	}
	plan_pub_.publish(path);
 }

  void publish_conveyor(){
	ROS_DEBUG("Inside publish conveyor");
	nav_msgs::OccupancyGrid grid;

	grid.header.frame_id = "map";
	grid.header.stamp = ros::Time::now();
	grid.info.map_load_time = ros::Time::now();

	grid.info.origin.orientation.w = 0;
	grid.info.resolution = beliefs->getSpatialModel()->getConveyors()->getGranularity();
	grid.info.width = beliefs->getSpatialModel()->getConveyors()->getBoxWidth();
	grid.info.height = beliefs->getSpatialModel()->getConveyors()->getBoxHeight();

	vector< vector<int> > conveyors = beliefs->getSpatialModel()->getConveyors()->getConveyors();
	for(int j = 0; j < grid.info.height; j++){
	    for(int i = 0; i < grid.info.width; i++){
      		grid.data.push_back(conveyors[i][j]);
    	    }
  	}
	conveyor_pub_.publish(grid);
  }



  void publish_occupancy(){
	ROS_DEBUG("Inside publish occupancy");
	nav_msgs::OccupancyGrid grid;

	grid.header.frame_id = "map";
	grid.header.stamp = ros::Time::now();
	grid.info.map_load_time = ros::Time::now();

	grid.info.origin.orientation.w = 0;
	vector< vector <bool> > occupancyGrid = con->getPlanner()->getMap()->getOccupancyGrid();
	int gridSize = con->getPlanner()->getMap()->getOccupancySize();
	grid.info.resolution = gridSize / 100.0;
	grid.info.width = (int)(beliefs->getSpatialModel()->getConveyors()->getMapWidth() / grid.info.resolution);
	grid.info.height = (int)(beliefs->getSpatialModel()->getConveyors()->getMapHeight() / grid.info.resolution);

	for(int j = 0; j < grid.info.height; j++){
	    for(int i = 0; i < grid.info.width; i++){
		if(occupancyGrid[i][j]){
      			grid.data.push_back(50);
		}
		else{
			grid.data.push_back(0);
		}
    	    }
  	}
	occupancy_pub_.publish(grid);
  }

  void publish_region(){
	ROS_DEBUG("Inside publish regions");

	visualization_msgs::MarkerArray markerArray;
	vector<FORRRegion> regions = beliefs->getSpatialModel()->getRegionList()->getRegions();
	cout << "There are currently " << regions.size() << " regions" << endl;
	for(int i = 0 ; i < regions.size(); i++){
		//regions[i].print();
		visualization_msgs::Marker marker;
		marker.header.frame_id = "map";
    		marker.header.stamp = ros::Time::now();
		marker.ns = "basic_shapes";
    		marker.id = i;
		marker.type = visualization_msgs::Marker::CYLINDER;

		marker.pose.position.x = regions[i].getCenter().get_x();
    		marker.pose.position.y = regions[i].getCenter().get_y();
    		marker.pose.position.z = 0;
    		marker.pose.orientation.x = 0.0;
    		marker.pose.orientation.y = 0.0;
    		marker.pose.orientation.z = 0.0;
    		marker.pose.orientation.w = 1.0;

    		// Set the scale of the marker -- 1x1x1 here means 1m on a side
    		marker.scale.x = marker.scale.y = regions[i].getRadius()*2;
    		marker.scale.z = 1.0;

    		// Set the color -- be sure to set alpha to something non-zero!
    		marker.color.r = 0.0f;
    		marker.color.g = 1.0f;
    		marker.color.b = 0.0f;
    		marker.color.a = 0.5;

    		marker.lifetime = ros::Duration();
		markerArray.markers.push_back(marker);

	}
	region_pub_.publish(markerArray);
  }
  void publish_trails(){
	ROS_DEBUG("Inside publish trail");
	//Goal here is to publish all trails

	FORRTrails *trails = beliefs->getSpatialModel()->getTrails();
	/*nav_msgs::Path path;
	path.header.frame_id = "map";
	path.header.stamp = ros::Time::now();

	for(int i = 0; i < trails->getSize(); i++){
		vector<TrailMarker> trail = trails->getTrail(i);
		//fill up the path using the trail
		for(int j = 0; j < trail.size(); j++){
			double x = trail[j].coordinates.get_x();
			double y = trail[j].coordinates.get_y();
			geometry_msgs::PoseStamped poseStamped;
			poseStamped.header.frame_id = "map";
			poseStamped.header.stamp = path.header.stamp;
			poseStamped.pose.position.x = x;
			poseStamped.pose.position.y = y;
			path.poses.push_back(poseStamped);
		}
	}*/

	visualization_msgs::Marker line_list;
	line_list.header.frame_id = "map";
	line_list.header.stamp = ros::Time::now();
	line_list.ns = "basic_shapes";
	line_list.action = visualization_msgs::Marker::ADD;
	line_list.id = 1;
	line_list.type = visualization_msgs::Marker::LINE_LIST;
	line_list.pose.orientation.w = 1.0;
	line_list.scale.x = 0.1;
	line_list.color.r = 1.0;
	line_list.color.a = 1.0;

	for(int i = 0 ; i < trails->getSize(); i++){
		vector<TrailMarker> trail = trails->getTrail(i);
		for(int j = 0; j < trail.size()-1; j++){
			geometry_msgs::Point p1, p2;
			p1.x = trail[j].coordinates.get_x();
			p1.y = trail[j].coordinates.get_y();
			p1.z = 0;

			p2.x = trail[j+1].coordinates.get_x();
			p2.y = trail[j+1].coordinates.get_y();
			p2.z = 0;

			line_list.points.push_back(p1);
			line_list.points.push_back(p2);
		}
	}
	trails_pub_.publish(line_list);

  }

  void publish_doors(){
	ROS_DEBUG("Inside publish doors");

	std::vector< std::vector<Door> > doors = beliefs->getSpatialModel()->getDoors()->getDoors();
	cout << "There are currently " << doors.size() << " regions" << endl;
	visualization_msgs::Marker line_list;
	line_list.header.frame_id = "map";
	line_list.header.stamp = ros::Time::now();
	line_list.ns = "basic_shapes";
	line_list.action = visualization_msgs::Marker::ADD;
	line_list.id = 1;
	line_list.type = visualization_msgs::Marker::LINE_LIST;
	line_list.pose.orientation.w = 1.0;
	line_list.scale.x = 0.3;
	line_list.color.r = 1.0;
	line_list.color.a = 1.0;

	for(int i = 0 ; i < doors.size(); i++){
		for(int j = 0; j < doors[i].size(); j++){
			geometry_msgs::Point p1, p2;
			p1.x = doors[i][j].startPoint.getExitPoint().get_x();
			p1.y = doors[i][j].startPoint.getExitPoint().get_y();
			p1.z = 0;

			p2.x = doors[i][j].endPoint.getExitPoint().get_x();
			p2.y = doors[i][j].endPoint.getExitPoint().get_y();
			p2.z = 0;

			line_list.points.push_back(p1);
			line_list.points.push_back(p2);
		}
	}
	doors_pub_.publish(line_list);
  }

  void publish_walls(){
	ROS_DEBUG("Inside publish walls");
	vector<Wall> walls = con->getPlanner()->getMap()->getWalls();
	cout << "There are currently " << walls.size() << " walls" << endl;
	visualization_msgs::Marker line_list;
	line_list.header.frame_id = "map";
    	line_list.header.stamp = ros::Time::now();
	line_list.ns = "basic_shapes";
	line_list.action = visualization_msgs::Marker::ADD;
    	line_list.id = 1;
	line_list.type = visualization_msgs::Marker::LINE_LIST;
	line_list.pose.orientation.w = 1.0;
	line_list.scale.x = 0.3;
	line_list.color.r = 0.5;
	line_list.color.a = 0.5;

	for(int i = 0 ; i < walls.size(); i++){
		geometry_msgs::Point p1, p2;
		p1.x = walls[i].x1/100.0;
		p1.y = walls[i].y1/100.0;
		p1.z = 0;
		p2.x = walls[i].x2/100.0;
		p2.y = walls[i].y2/100.0;
		p2.z = 0;

		line_list.points.push_back(p1);
		line_list.points.push_back(p2);
	}
	walls_pub_.publish(line_list);
  }

  void publish_all_targets(){
	ROS_DEBUG("Publish All targets as pose array!!");
	geometry_msgs::PoseArray targets;
	targets.header.frame_id = "map";
	targets.header.stamp = ros::Time::now();

	list<Task*> agenda = beliefs->getAgentState()->getAllAgenda();
	for(list<Task*>::iterator it = agenda.begin(); it != agenda.end(); it++){
		double x = (*it)->getX();
		double y = (*it)->getY();
		geometry_msgs::Pose pose;
		pose.position.x = x;
		pose.position.y = y;
		targets.poses.push_back(pose);
	}
	all_targets_pub_.publish(targets);
  }

  void publish_remaining_targets(){
	ROS_DEBUG("Publish remaining targets as pose array!!");
	geometry_msgs::PoseArray targets;
	targets.header.frame_id = "map";
	targets.header.stamp = ros::Time::now();

	list<Task*> agenda = beliefs->getAgentState()->getAgenda();
	for(list<Task*>::iterator it = agenda.begin(); it != agenda.end(); it++){
		double x = (*it)->getX();
		double y = (*it)->getY();
		geometry_msgs::Pose pose;
		pose.position.x = x;
		pose.position.y = y;
		targets.poses.push_back(pose);
	}
	remaining_targets_pub_.publish(targets);
  }

  void publish_log(FORRAction decision, double overallTimeSec, double computationTimeSec){
	ROS_DEBUG("Inside publish decision log!!");
	std_msgs::String log;
	double robotX = beliefs->getAgentState()->getCurrentPosition().getX();
	double robotY = beliefs->getAgentState()->getCurrentPosition().getY();
	double targetX;
	double targetY;
	double robotTheta = beliefs->getAgentState()->getCurrentPosition().getTheta();
	if(beliefs->getAgentState()->getCurrentTask() != NULL) {
		targetX = beliefs->getAgentState()->getCurrentTask()->getTaskX();
		targetY = beliefs->getAgentState()->getCurrentTask()->getTaskY();
	} else {
		targetX = 0;
		targetY = 0;
	}
	vector<CartesianPoint> laserEndpoints = beliefs->getAgentState()->getCurrentLaserEndpoints();
	sensor_msgs::LaserScan laserScan = beliefs->getAgentState()->getCurrentLaserScan();
	FORRAction max_forward = beliefs->getAgentState()->maxForwardAction();
	ROS_DEBUG("After max_forward");
	//vector< vector<CartesianPoint> > allTrace = beliefs->getAgentState()->getAllTrace();
	list<Task*>& agenda = beliefs->getAgentState()->getAgenda();
	list<Task*>& all_agenda = beliefs->getAgentState()->getAllAgenda();
	ROS_DEBUG("After all_agenda");
	vector<FORRRegion> regions = beliefs->getSpatialModel()->getRegionList()->getRegions();
	vector< vector< CartesianPoint> > trails =  beliefs->getSpatialModel()->getTrails()->getTrailsPoints();
	ROS_DEBUG("After trails");
	FORRActionType chosenActionType = decision.type;
	int chosenActionParameter = decision.parameter;
	int decisionTier = con->getCurrentDecisionStats()->decisionTier;
	string vetoedActions = con->getCurrentDecisionStats()->vetoedActions;
	string advisors = con->getCurrentDecisionStats()->advisors;
	string advisorComments = con->getCurrentDecisionStats()->advisorComments;
	string advisorInfluence = con->getCurrentDecisionStats()->advisorInfluence;
	cout << "vetoedActions = " << vetoedActions << " decisionTier = " << decisionTier << " advisors = " << advisors << " advisorComments = " << advisorComments << endl;
	vector< vector<int> > conveyors = beliefs->getSpatialModel()->getConveyors()->getConveyors();
	std::vector< std::vector<Door> > doors = beliefs->getSpatialModel()->getDoors()->getDoors();

	ROS_DEBUG("After decision statistics");
	int decisionCount = -1;
	int currentTask = -1;
	if(!agenda.empty()){
		currentTask = all_agenda.size() - agenda.size();
  		if(currentTask != 0)
			decisionCount = beliefs->getAgentState()->getCurrentTask()->getDecisionCount();
	}
	ROS_DEBUG("After decisionCount");


	std::stringstream lep;
	for(int i = 0; i < laserEndpoints.size(); i++){
		double x = laserEndpoints[i].get_x();
 		double y = laserEndpoints[i].get_y();
		lep << x << "," << y << ";";
	}
	ROS_DEBUG("After laserEndpoints");


	std::stringstream ls;
	double min_laser_scan = 25; //meters
	for(int i = 0; i < laserScan.ranges.size(); i++){
		double length = laserScan.ranges[i];
		if(length < min_laser_scan){
			min_laser_scan = length;
		}
		ls << length << ",";
	}
	ROS_DEBUG("After laserScan");
	/*int totalSize = 0;
	for(int i = 0; i < allTrace.size(); i++){
		totalSize += allTrace[i].size();
	}*/


	std::stringstream regionsstream;
	for(int i = 0; i < regions.size(); i++){
		regionsstream << regions[i].getCenter().get_x() << " " << regions[i].getCenter().get_y() << " " << regions[i].getRadius();
		vector<FORRExit> exits = regions[i].getExits();
		for(int j = 0; j < exits.size() ; j++){
			regionsstream << " " << exits[j].getExitPoint().get_x() << " "  << exits[j].getExitPoint().get_y() << " "  << exits[j].getExitRegion();
		}
		regionsstream << ";";
	}
	ROS_DEBUG("After regions");


	std::stringstream trailstream;
	for(int i = 0; i < trails.size(); i++){
		for(int j = 0; j < trails[i].size(); j++){
			trailstream << trails[i][j].get_x() << " " << trails[i][j].get_y() << " ";
		}
		trailstream << ";";
	}
	ROS_DEBUG("After trails");

	std::stringstream conveyorStream;
	for(int j = 0; j < conveyors.size()-1; j++){
		for(int i = 0; i < conveyors[j].size(); i++){
			conveyorStream << conveyors[j][i] << " ";
		}
		conveyorStream << ";";
	}
	ROS_DEBUG("After conveyors");
	

	std::stringstream doorStream;
	for(int i = 0; i < doors.size(); i++){
		for(int j = 0; j < doors[i].size(); j++){
			doorStream << doors[i][j].startPoint.getExitPoint().get_x() << " " << doors[i][j].startPoint.getExitPoint().get_y() << " " << doors[i][j].endPoint.getExitPoint().get_x() << " " << doors[i][j].endPoint.getExitPoint().get_y() << " " << doors[i][j].str << ", ";
		}
		doorStream << ";";
	}

	ROS_DEBUG("After doors");

	std::stringstream planStream;
	if(beliefs->getAgentState()->getCurrentTask() != NULL){
		vector <CartesianPoint> waypoints = beliefs->getAgentState()->getCurrentTask()->getWaypoints();

		for(int i = 0; i < waypoints.size(); i++){
			planStream << waypoints[i].get_x() << " " << waypoints[i].get_y();
			planStream << ";";
		}
	}
	ROS_DEBUG("After planStream");

	std::stringstream crowdStream;
	geometry_msgs::PoseArray crowdpose = beliefs->getAgentState()->getCrowdPose();

	for(int i = 0; i < crowdpose.poses.size(); i++){
		crowdStream << crowdpose.poses[i].position.x << " " << crowdpose.poses[i].position.y << " " << crowdpose.poses[i].orientation.x 
		<< " " << crowdpose.poses[i].orientation.y << " " << crowdpose.poses[i].orientation.z << " " << crowdpose.poses[i].orientation.w;
		crowdStream << ";";
	}
	ROS_DEBUG("After crowdStream");

	std::stringstream allCrowdStream;
	geometry_msgs::PoseArray crowdposeall = beliefs->getAgentState()->getCrowdPoseAll();

	for(int i = 0; i < crowdposeall.poses.size(); i++){
		allCrowdStream << crowdposeall.poses[i].position.x << " " << crowdposeall.poses[i].position.y << " " << crowdposeall.poses[i].orientation.x
		<< " " << crowdposeall.poses[i].orientation.y << " " << crowdposeall.poses[i].orientation.z << " " << crowdposeall.poses[i].orientation.w;
		allCrowdStream << ";";
	}
	ROS_DEBUG("After all crowdStream");

	std::stringstream crowdModel;
	semaforr::CrowdModel model = con->getPlanner()->getCrowdModel();
	int resolution = model.resolution;
	int height = model.height;
	int width = model.width;
	std::vector<double> densities = model.densities;
	crowdModel << height << " " << width << " " << resolution << " ";
	for(int i = 0; i < densities.size() ; i++){
		crowdModel << densities[i] << " ";
	}

	ROS_DEBUG("After all crowd model");

	std::stringstream output;

	//output << currentTask << "\t" << decisionCount << "\t" << overallTimeSec << "\t" << computationTimeSec << "\t" << targetX << "\t" << targetY << "\t" << robotX << "\t" << robotY << "\t" << robotTheta << "\t" << max_forward.parameter << "\t" << decisionTier << "\t" << vetoedActions << "\t" << chosenActionType << "\t" << chosenActionParameter << "\t" << advisors << "\t" << advisorComments << "\t" << advisorInfluence << "\t" << regionsstream.str() << "\t" << trailstream.str() << "\t" << doorStream.str() << "\t" << conveyorStream.str() << "\t" << planStream.str() << "\t" << lep.str() << "\t" << ls.str();

	//output << currentTask << "\t" << decisionCount << "\t"<< targetX << "\t" << targetY << "\t" << robotX << "\t" << robotY << "\t" << robotTheta << "\t" << planStream.str();// << "\t" << lep.str() << "\t" << ls.str();

	//std::stringstream output;

	output << currentTask << "\t" << decisionCount << "\t" << overallTimeSec << "\t" << computationTimeSec << "\t" << targetX << "\t" << targetY << "\t" << robotX << "\t" << robotY << "\t" << robotTheta << "\t" << max_forward.parameter << "\t" << decisionTier << "\t" << vetoedActions << "\t" << chosenActionType << "\t" << chosenActionParameter << "\t" << advisors << "\t" << advisorComments << "\t" << lep.str() << "\t" << ls.str() << "\t" << crowdStream.str() << "\t" << allCrowdStream.str() << "\t" << crowdModel.str() << "\t" << planStream.str();

	log.data = output.str();
	stats_pub_.publish(log);
	con->clearCurrentDecisionStats();
  }
};
