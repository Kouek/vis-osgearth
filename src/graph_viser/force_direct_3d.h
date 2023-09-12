#ifndef SCIVIS_GRAPH_VISER_FORCE_DIRECT_3D_H
#define SCIVIS_GRAPH_VISER_FORCE_DIRECT_3D_H

#include <random>
#include <string>
#include <unordered_set>
#include <vector>

namespace SciVis {
namespace GraphViser3D {

class Vec3D {
  public:
    double x;
    double y;
    double z;
    Vec3D(double x, double y, double z);
    static double distance(Vec3D v1, Vec3D v2);
    static Vec3D zero();
    static Vec3D random(double range);

    friend Vec3D operator+(const Vec3D &lhs, const Vec3D &rhs);
    friend Vec3D operator-(const Vec3D &lhs, const Vec3D &rhs);
    friend Vec3D operator*(const double lhs, const Vec3D &rhs);
    friend Vec3D operator*(const Vec3D &lhs, const double rhs);
    friend Vec3D operator/(const Vec3D &lhs, const Vec3D &rhs);
    friend std::ostream &operator<<(std::ostream &os, const Vec3D &v);
};

inline Vec3D operator+(const Vec3D &lhs, const Vec3D &rhs) {
    return Vec3D(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z);
}
inline Vec3D operator-(const Vec3D &lhs, const Vec3D &rhs) {
    return Vec3D(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z);
}
inline Vec3D operator*(const double lhs, const Vec3D &rhs) {
    return Vec3D(lhs * rhs.x, lhs * rhs.y, lhs * rhs.z);
}
inline Vec3D operator*(const Vec3D &lhs, const double rhs) {
    return Vec3D(rhs * lhs.x, rhs * lhs.y, rhs * lhs.z);
}

inline Vec3D operator/(const Vec3D &lhs, const Vec3D &rhs) {
    return Vec3D(lhs.x / rhs.y, lhs.y / rhs.y, lhs.z / rhs.z);
}

inline std::ostream &operator<<(std::ostream &os, const Vec3D &v) {
    os << "(" << v.x << "," << v.y << "," << v.z << ")";
    return os;
}

class Node {
  public:
    int id;
    std::string data;

    // Physical properties
    Vec3D pos;
    Vec3D vel;
    Vec3D acc;
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
    Vec3D compute_spring_force(Node n1, Node n2);
    Vec3D compute_repulsion_force(Node n1, Node n2);
    Vec3D compute_spring_force_general(double k, Vec3D v1, Vec3D v2);
    void set_graph(Graph graph);
};

} // namespace GraphViser3D
} // namespace SciVis

#endif // !SCIVIS_GRAPH_VISER_FORCE_DIRECT_3D_H