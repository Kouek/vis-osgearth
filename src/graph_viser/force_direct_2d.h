#ifndef SCIVIS_GRAPH_VISER_FORCE_DIRECT_2D_H
#define SCIVIS_GRAPH_VISER_FORCE_DIRECT_2D_H

#include <random>

#include <string>
#include <unordered_set>
#include <vector>

namespace SciVis {
namespace GraphViser2D {

class Vec2D {
  public:
    double x;
    double y;
    // double z;
    // Vec2D(double x, double y, double z);
    Vec2D(double x, double y);
    static double distance(Vec2D v1, Vec2D v2);
    static Vec2D zero();
    static Vec2D random(double range);

    friend Vec2D operator+(const Vec2D &lhs, const Vec2D &rhs);
    friend Vec2D operator-(const Vec2D &lhs, const Vec2D &rhs);
    friend Vec2D operator*(const double lhs, const Vec2D &rhs);
    friend Vec2D operator*(const Vec2D &lhs, const double rhs);
    friend Vec2D operator/(const Vec2D &lhs, const Vec2D &rhs);
    friend std::ostream &operator<<(std::ostream &os, const Vec2D &v);
};

inline Vec2D operator+(const Vec2D &lhs, const Vec2D &rhs) {
    return Vec2D(lhs.x + rhs.x, lhs.y + rhs.y);
}
inline Vec2D operator-(const Vec2D &lhs, const Vec2D &rhs) {
    return Vec2D(lhs.x - rhs.x, lhs.y - rhs.y);
}
inline Vec2D operator*(const double lhs, const Vec2D &rhs) {
    return Vec2D(lhs * rhs.x, lhs * rhs.y);
}
inline Vec2D operator*(const Vec2D &lhs, const double rhs) {
    return Vec2D(rhs * lhs.x, rhs * lhs.y);
}
inline Vec2D operator/(const Vec2D &lhs, const Vec2D &rhs) {
    return Vec2D(lhs.x / rhs.y, lhs.y / rhs.y);
}
inline std::ostream &operator<<(std::ostream &os, const Vec2D &v) {
    os << "(" << v.x << "," << v.y << ")";
    return os;
}

class Node {
  public:
    int id;
    std::string data;

    // Physical properties
    Vec2D pos;
    Vec2D vel;
    Vec2D acc;
    double mass;
    int radius;
    double repulsion;
    double stiffness;
    double damping;
    int degree;

    Node(int id, std::string data);
};
// Adjacency list representation of a graph
class Graph {
  public:
    Graph();

    // A vector of neighbor nodes for each node id
    // e.g. adj_list[0] represents the neighbors of node 0
    std::vector<std::unordered_set<int>> adj_list;
    std::vector<Node> node_list;

    // Add a new node to the graph
    // Returns the id of the newly added node
    int add_node(std::string data);

    // Add an edge connecting node_1 and node_2
    void add_edge(int node_id_1, int node_id_2);

    // Update the degree of each node
    void update_degrees();
};

class Universe {
  public:
    double dt;
    double repulsion;
    double spring_k;
    double damping;
    double gravity;
    int n_iterations;
    Graph graph;
    Universe(Graph graph, double dt, double repulsion, double spring_k, double damping,
             double gravity);

    void update(double deltaT);
    Vec2D compute_spring_force(Node n1, Node n2);
    Vec2D compute_repulsion_force(Node n1, Node n2);
    Vec2D compute_spring_force_general(double k, Vec2D v1, Vec2D v2);
    void set_graph(Graph graph);
};

} // namespace GraphViser
} // namespace SciVis

#endif // !SCIVIS_GRAPH_VISER_FORCE_DIRECT_2D_H