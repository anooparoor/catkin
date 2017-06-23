#ifndef GRAPH_H
#define GRAPH_H
#include "Node.h"
#include "Edge.h"
#include <vector>
#include <queue>
#include <math.h>
#include "Map.h"
#include <assert.h>

class Graph {
private:
  vector<Node*> nodes; 
  vector<Edge*> edges; 

  double proximity;           // proximity between 2 nodes ( in cm, 1m = 100cm ) 

  Map * map ; 

  void generateNavGraph();    

  // this keeps a list of nodes which the edges around them are temporarily disabled (not usable)
  vector<int> obstacles;

  // this keeps a list of nodes which the cost of edges around them are temporarily increased
  vector<int> tempObstacles;
  
  void toggleObstacle(bool b, int x1, int y1, int x2, int y2);

  void toggleTempObstacle(bool access, int x1, int y1, int x2, int y2);

  bool isEdge(Edge e); 
 
public: 
  Graph(Map*, int);

  ~Graph();

  Map* getMap() { return map; }

  int getProximity() const { return proximity; }

  vector<Node*> getNodes() const { return nodes; }

  vector<Edge*> getEdges() const { return edges; }

  // this function is used to retreive node id that is at (x,y). Used during populating neigbors of nodes, 
  // during construction of navGraph
  int getNodeID(int x, int y);

  Node getNode(int n);
  
  Node* getNodePtr(int i); 

  Edge* getEdge(int n1, int n2);

  void updateEdgeCost(int i, double cost){
	edges[i]->setCost(cost);
  }

  //! returns the neigbors of the node with index n. Calls directly Node::getNeighbors 
  vector<int> getNeighbors(Node n); 

  //! populates the graph with nodes and assigns their immediate neighbors
  void populateNodeNeighbors(); 

  bool isNode(Node n); 

  int numNodes() const { return nodes.size(); }

  int numEdges() const { return edges.size(); }

  void printGraph() ;

  vector<Node*> getNodesInRegion( int x, int y, double r);

  vector<Node*> getNodesInRegion( int x1, int y1, int x2, int y2);

  //! sets all the nodes in the graph as accessible
  void clearGraph();

  void addObstacle(int x1, int y1, int x2, int y2);

  void removeObstacle(int x1, int y1, int x2, int y2);

  void addObstacle(int x, int y, double dist);

  void removeObstacle(int x, int y, double dist);

  void removeAllObstacles();

  vector<int> getObstacles() { return obstacles; }

  void addTempObstacle(int x1, int y1, int x2, int y2);

  void removeTempObstacle(int x1, int y1, int x2, int y2);

  void addTempObstacle(int x, int y, double dist);

  void removeTempObstacle(int x, int y, double dist);

  void removeAllTempObstacles();

  vector<int> getTempObstacles() { return tempObstacles; }

  double calcEdgeCost(Node, Node);

  double calcCost(Node, Node);
};

#endif