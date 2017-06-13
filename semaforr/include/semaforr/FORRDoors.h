/************************************************
FORRDoors.h 
This file contains the class which contains information about the doors that FORR learns and uses

Written by Raj Korpan, adapted from Gil Dekel, 2017
**********************************************/

#ifndef FORRDOORS_H
#define FORRDOORS_H

#include <vector>
#include <string>
#include <limits>
#include <iostream>
#include <cmath>        //for atan2 and M_PI
#include <utility>      //for exit
#include <algorithm>
#include "FORRGeometry.h"
#include "FORRCircle.h"
#include "FORRExit.h"

/*
 * Struct for Door start and endpoints and strength.
 */
struct Door {
    FORRExit startPoint;
    FORRExit endPoint;
    int str;
    Door(): startPoint(), endPoint(), str(0) { }
    Door(FORRExit s, FORRExit e, int currStr): startPoint(s), endPoint(e), str(currStr) { }
};

/* FORRDoors class
 *
 * FORRDoors represent the doors that are learned by SemaFORR.
 * A Door will be (at least initially) a set of entry and exit
 * points that are within a certain proximity to one another.
 *
 */
class FORRDoors{
public:
    FORRDoors(){
        doors = std::vector< std::vector<Door> >();
    };
    std::vector< std::vector<Door> > getDoors(){return doors;}
    ~FORRDoors(){};

    void clearAllDoors(){
        doors.clear();
    }

    void learnDoors(std::vector<FORRCircle> reg) {
        std::vector<FORRCircle> regions = reg;
	std::cout << "Number of regions = " << regions.size() << std::endl;
        for (int i = 0; i < regions.size(); i++) {
            std::vector<Door> regionDoors;
            vector< std::pair<double, FORRExit> > exitAngles = calculateExitAngles(regions[i]);
            std::cout << "Number of exits for region " << i << " = " << exitAngles.size() << std::endl;
            //Door will be considered for regions with two exit points and above
            if (regions[i].getExits().size() >= 2) {
                //A formula to calculate the maximum allowed distance between two points in radians
                float epsilon = -1.3*(1/(1+((2*M_PI)/regions[i].getExits().size())))+1.35;
                std::cout << epsilon << std::endl;
                
                vector< std::pair<double, FORRExit> >::iterator idx = exitAngles.begin();
                Door *doorToPush = new Door(FORRExit(CartesianPoint(-1,-1),-1), FORRExit(CartesianPoint(-1,-1),-1), 0);
                doorToPush->startPoint = idx->second;    //Put the first exit point in a door object.
                ++doorToPush->str;                       //Door now has strength 1
                double doorToPushStartAngle = idx->first;
                double firstStartAngle = idx->first;
                std::cout << "doorToPush->startPoint = " << doorToPush->startPoint.getExitPoint().get_x() << ", " << doorToPush->startPoint.getExitPoint().get_y() << std::endl;
                std::cout << "doorToPush->str = " << doorToPush->str << std::endl;
                std::cout << "doorToPushStartAngle = " << doorToPushStartAngle << ", firstStartAngle = " << firstStartAngle << std::endl;

                for (idx = exitAngles.begin(); idx != exitAngles.end()-1; idx++){
                    std::cout << "idx->first = " << idx->first << ", (idx+1)->first = " << (idx+1)->first << ", (idx+1)->first - idx->first = " << ((idx+1)->first - idx->first) << ", epsilon = " << epsilon << std::endl;
                    //if the two point are close enough, extend the end of the current door to the new point
                    if ((idx+1)->first - idx->first < epsilon) {
                        doorToPush->endPoint = (idx+1)->second;
                        ++doorToPush->str;  //increase the strength of the door each time a point is added to the door.
                        std::cout << "doorToPush->endPoint = " << doorToPush->endPoint.getExitPoint().get_x() << ", " << doorToPush->endPoint.getExitPoint().get_y() << std::endl;
                        std::cout << "doorToPush->str = " << doorToPush->str << std::endl;
                    } else {
                        //push the new door only if it is not a single point.
                        if (!(doorToPush->endPoint.getExitPoint() == CartesianPoint(-1,-1))) {
                            regionDoors.push_back(*doorToPush);
                            std::cout << "regionDoors.size() = " << regionDoors.size() << std::endl;
                        }

                        doorToPush = new Door(FORRExit(CartesianPoint(-1,-1),-1), FORRExit(CartesianPoint(-1,-1),-1), 0); //prepare a new door
                        doorToPush->startPoint = (idx+1)->second;    //set the start position to the new further out point.
                        ++doorToPush->str;
                        doorToPushStartAngle = idx->first;
                        std::cout << "doorToPush->startPoint = " << doorToPush->startPoint.getExitPoint().get_x() << ", " << doorToPush->startPoint.getExitPoint().get_y() << std::endl;
                        std::cout << "doorToPush->str = " << doorToPush->str << std::endl;
                        std::cout << "doorToPushStartAngle = " << doorToPushStartAngle << ", firstStartAngle = " << firstStartAngle << std::endl;
                    }
                }

                std::cout << "doorToPush->startPoint = " << doorToPush->startPoint.getExitPoint().get_x() << ", " << doorToPush->startPoint.getExitPoint().get_y() << std::endl;
                std::cout << "doorToPush->endPoint = " << doorToPush->endPoint.getExitPoint().get_x() << ", " << doorToPush->endPoint.getExitPoint().get_y() << std::endl;
                //Take care of the last door
                if (!(doorToPush->startPoint.getExitPoint() == CartesianPoint(-1,-1)) && !(doorToPush->endPoint.getExitPoint() == CartesianPoint(-1,-1))) {
                    regionDoors.push_back(*doorToPush);
                    std::cout << "regionDoors.size() = " << regionDoors.size() << std::endl;
    
                //Take care of the last point on the circle before going back to the terminal.
                } else if (!(doorToPush->startPoint.getExitPoint() == CartesianPoint(-1,-1)) && (doorToPush->endPoint.getExitPoint() == CartesianPoint(-1,-1))) {
                    if (2*M_PI - (doorToPushStartAngle - exitAngles.begin()->first) < epsilon) {
                        doorToPush->endPoint = exitAngles.begin()->second;
                        ++doorToPush->str;
                        std::cout << "doorToPush->endPoint = " << doorToPush->endPoint.getExitPoint().get_x() << ", " << doorToPush->endPoint.getExitPoint().get_y() << std::endl;
                        std::cout << "doorToPush->str = " << doorToPush->str << std::endl;
                        regionDoors.push_back(*doorToPush);
                        std::cout << "regionDoors.size() = " << regionDoors.size() << std::endl;
                    }
                }

                //Check if the first and last doors can be merged into one over the terminal
                //Only if there is more than one door.
                if (regionDoors.size() > 1) {
                std::cout << "regionDoors.back().endPoint.getExitPoint() = " << regionDoors.back().endPoint.getExitPoint().get_x() << ", " << regionDoors.back().endPoint.getExitPoint().get_y() << std::endl;
                std::cout << "regionDoors[0].startPoint.getExitPoint() = " << regionDoors[0].startPoint.getExitPoint().get_x() << ", " << regionDoors[0].startPoint.getExitPoint().get_y() << std::endl;
                std::cout << "calculateFixedAngle(regions[i].getCenter().get_x(), regions[i].getCenter().get_y(), regionDoors.back().endPoint.getExitPoint().get_x(), regionDoors.back().endPoint.getExitPoint().get_y()) = " << calculateFixedAngle(regions[i].getCenter().get_x(), regions[i].getCenter().get_y(), regionDoors.back().endPoint.getExitPoint().get_x(), regionDoors.back().endPoint.getExitPoint().get_y()) << std::endl;
                std::cout << "calculateFixedAngle(regions[i].getCenter().get_x(), regions[i].getCenter().get_y(), regionDoors[0].startPoint.getExitPoint().get_x(), regionDoors[0].startPoint.getExitPoint().get_y()) = " << calculateFixedAngle(regions[i].getCenter().get_x(), regions[i].getCenter().get_y(), regionDoors[0].startPoint.getExitPoint().get_x(), regionDoors[0].startPoint.getExitPoint().get_y()) << std::endl;
                std::cout << "Difference = " << (calculateFixedAngle(regions[i].getCenter().get_x(), regions[i].getCenter().get_y(), regionDoors.back().endPoint.getExitPoint().get_x(), regionDoors.back().endPoint.getExitPoint().get_y()) - calculateFixedAngle(regions[i].getCenter().get_x(), regions[i].getCenter().get_y(), regionDoors[0].startPoint.getExitPoint().get_x(), regionDoors[0].startPoint.getExitPoint().get_y()) ) << std::endl;
                    if (regionDoors.back().endPoint.getExitPoint() == regionDoors[0].startPoint.getExitPoint() ||
                    (2*M_PI - (calculateFixedAngle(regions[i].getCenter().get_x(), regions[i].getCenter().get_y(), regionDoors.back().endPoint.getExitPoint().get_x(), regionDoors.back().endPoint.getExitPoint().get_y()) - calculateFixedAngle(regions[i].getCenter().get_x(), regions[i].getCenter().get_y(), regionDoors[0].startPoint.getExitPoint().get_x(), regionDoors[0].startPoint.getExitPoint().get_y()) )) < epsilon) {
                        std::cout << "never thought I'll make it here!\n";
                        //Set the starting point of the first door to the starting point of the last door
                        regionDoors[0].startPoint = regionDoors.back().startPoint;
                        regionDoors[0].str += regionDoors.back().str;                 //update the strength of the merged door
                        std::cout << "regionDoors[0].startPoint.getExitPoint() = " << regionDoors[0].startPoint.getExitPoint().get_x() << ", " << regionDoors[0].startPoint.getExitPoint().get_y() << std::endl;
                        std::cout << "regionDoors[0].str = " << regionDoors[0].str << std::endl;
                        //Removed the merged last door.
                        regionDoors.pop_back();
                        std::cout << "regionDoors.size() = " << regionDoors.size() << std::endl;
                    }
                }
            }
            std::cout << "regionDoors.size() = " << regionDoors.size() << std::endl;
            doors.push_back(regionDoors);
            std::cout << "doors.size() = " << doors.size() << std::endl;
        }
    }

    vector< std::pair<double, FORRExit> > calculateExitAngles(FORRCircle region){
        vector<FORRExit> exits = region.getExits();
        double regionX = region.getCenter().get_x();
        double regionY = region.getCenter().get_y();
        vector< std::pair<double, FORRExit> > exitAngles;
        for(int i = 0; i < exits.size(); i++){
            double exitX = exits[i].getExitPoint().get_x();
	    double exitY = exits[i].getExitPoint().get_y();
            double fixedAngle = calculateFixedAngle(regionX, regionY, exitX, exitY);
            exitAngles.push_back(std::pair<double, FORRExit>(fixedAngle, exits[i]));
        }
        std::sort(exitAngles.begin(), exitAngles.end());
        return exitAngles;
    }

    double calculateFixedAngle(double regionX, double regionY, double exitX, double exitY){
        //Calculate the angle of the exit from the center of the region
        double angle = atan2((exitY - regionY), (exitX - regionX));
        double fixedAngle = angle;
        //If the angle is negative (clockwise), translate it to standard counter clockwise.
        if (angle < 0) {
            fixedAngle = angle + 2*M_PI;
        }
        return fixedAngle;
    }

private:
    std::vector< std::vector<Door> > doors;
};

#endif