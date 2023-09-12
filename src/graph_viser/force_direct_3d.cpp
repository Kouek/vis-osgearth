#include <assert.h>
#include <ctime>
#include <functional>
#include <iostream>
#include <math.h>
#include <random>
#include <windows.h>

#include "force_direct_3d.h"

using namespace std;
using namespace SciVis::GraphViser3D;

Node::Node(int id, string data)
    : id{id}, data{data}, pos{Vec3D::random(1.0)}, vel{Vec3D::random(1.0)}, acc{Vec3D::random(1.0)},
      mass{1.0}, radius{1}, repulsion{1.0}, stiffness{1.0}, damping{1.0}, degree{0} {}

Vec3D::Vec3D(double x, double y, double z) : x{x}, y{y}, z{z} {}

double Vec3D::distance(Vec3D v1, Vec3D v2) {
    double distance = sqrt(pow(v1.x - v2.x, 2.0) + pow(v1.y - v2.y, 2.0) + pow(v1.z - v2.z, 2.0));
    return distance;
}

Vec3D Vec3D::zero() { return Vec3D(0.0, 0.0, 0.0); }

Vec3D Vec3D::random(double range) {
    double lower_bound = -range;
    double upper_bound = range;
    double rand_x = (double)rand() / RAND_MAX * (upper_bound - lower_bound) + lower_bound;
    double rand_y = (double)rand() / RAND_MAX * (upper_bound - lower_bound) + lower_bound;
    double rand_z = (double)rand() / RAND_MAX * (upper_bound - lower_bound) + lower_bound;

    return Vec3D(rand_x, rand_y, rand_z);
}

Universe::Universe(Graph graph, double dt, double repulsion, double spring_k, double damping,
                   double gravity)
    : graph{graph}, dt{dt}, repulsion{repulsion}, spring_k{spring_k}, damping{damping},
      gravity{gravity}, n_iterations{0} {}

Graph::Graph() {
    this->adj_list.clear();
    this->node_list.clear();
}

int Graph::add_node(string data) {
    int new_node_id = adj_list.size();
    Node new_node(new_node_id, data);
    // cout<<"new_node_id"<<new_node_id;
    this->adj_list.push_back(unordered_set<int>());
    this->node_list.push_back(new_node);
    return new_node_id;
}

void Graph::add_edge(int node_id_1, int node_id_2) {
    if (node_id_1 == node_id_2) {
        return;
    }
    this->adj_list[node_id_1].insert(node_id_2);
    this->adj_list[node_id_2].insert(node_id_1);
}

void Graph::update_degrees() {
    for (int i = 0; i < this->adj_list.size(); i++) {
        this->node_list[i].degree = this->adj_list[i].size();
    }
}
// Hooke's Law: F_spring = kx
// Computes the force of n2 "pulling" on n1
Vec3D Universe::compute_spring_force(Node n1, Node n2) { return spring_k * (n2.pos - n1.pos); }
Vec3D Universe::compute_spring_force_general(double k, Vec3D v1, Vec3D v2) { return k * (v1 - v2); }

// Coloumb's Law: F_repulsion = k (q1 * q2) / r^2
Vec3D Universe::compute_repulsion_force(Node n1, Node n2) {
    double dist = Vec3D::distance(n1.pos, n2.pos);
    return repulsion * (1 / pow(dist, 2.0)) *
           Vec3D((n1.pos.x - n2.pos.x), (n1.pos.y - n2.pos.y), (n1.pos.z - n2.pos.z));
}
void Universe::set_graph(Graph graph) {
    this->graph = graph;
    this->graph.update_degrees();
    n_iterations = 0;
}

void Universe::update(double deltaT) {
    // Do Euler integration (O(n^2))
    for (int i = 0; i < this->graph.adj_list.size(); i++) {

        Node &n1 = graph.node_list[i];
        // cout<<"original: "<<graph.node_list[i].pos<<endl;

        // Vec3D f_spring = Vec3D::random(10.0);
        // Vec3D f_repulsion = Vec3D::random(10.0);
        Vec3D f_spring = Vec3D::zero();
        Vec3D f_repulsion = Vec3D::zero();

        for (int j = 0; j < this->graph.adj_list.size(); j++) {
            if (i == j) {
                continue;
            }

            Node &n2 = graph.node_list[j];

            // Apply a repulsion force
            f_repulsion = f_repulsion + compute_repulsion_force(n1, n2);

            // Check if n1 and n2 are adjacent
            // If so, apply a spring force on both of them
            if (this->graph.adj_list[i].count(j)) {
                // Apply spring force
                f_spring = f_spring + compute_spring_force(n1, n2);
            }
        }
        // Optional: apply a "gravitational force", aka pull towards the origin
        f_spring = f_spring - compute_spring_force_general(gravity, n1.pos, Vec3D::zero());

        // Integrate laws of motion
        Vec3D f_net = f_spring + f_repulsion;

        // Compute acceleration
        // a = F_net/m
        Vec3D new_acc = (f_net - n1.vel * damping) * (1.0 / n1.mass);
        Vec3D new_vel = n1.vel + new_acc * deltaT;
        Vec3D new_pos = n1.pos + new_vel * deltaT;

        n1.pos = new_pos;
        n1.vel = new_vel;
        n1.acc = new_acc;
        // cout<<"update: "<<graph.node_list[i].pos<<endl;
        //  update
        // graph.node_list[i] = n1;
    }
    n_iterations += 1;
}

//// main
//#define WIN_WIDTH 1024
//#define WIN_HEIGHT 1024
//using namespace std;
//
//int n_iterations = INT_FAST32_MAX;
//Graph graph;
//int n_random_nodes = 200;
//
//// Universe settings
//float timeDelta = 0.2;
//float repulsion_force = 1.0f;
//float spring_force = 1.0f;
//float damping_coefficient = 0.5f;
//float gravitational_force = 0.5f;
//
//Universe universe(graph, timeDelta, repulsion_force, spring_force, damping_coefficient,
//                  gravitational_force);
//
//void init_graph() {
//    graph.adj_list.clear();
//    graph.node_list.clear();
//
//    std::mt19937 e(time(0));
//
//    // cout<<n_random_nodes;
//    for (int i = 0; i < n_random_nodes; i++) {
//        int n3 = graph.add_node("C");
//        uniform_real_distribution<double> u(0, n3);
//        int rd = u(e);
//        // cout<<rd<<endl;
//        // cout << graph.node_list[rd].pos<<endl;;
//        //  cout<<n_random_nodes;
//        graph.add_edge(n3, rd);
//    }
//    // cout<<endl;
//    universe.set_graph(graph);
//    cout << "=======inital=======" << endl;
//    for (int i = 0; i < graph.node_list.size(); i++) {
//        cout << graph.node_list[i].pos;
//    }
//}
//
//int main() {
//    srand(time(0));
//    cout << "Initiating graph..." << endl;
//    init_graph();
//    cout << "Done initiating graph with n=" << universe.graph.node_list.size() << endl;
//    // Run update logic
//
//    clock_t start, end;
//    start = clock();
//    while (universe.n_iterations < n_iterations) {
//        end = clock();
//        if (end - start > 10 * 1000)
//            break;
//        universe.update(timeDelta);
//    }
//
//    cout << "=======updated========" << endl;
//    for (int i = 0; i < graph.node_list.size(); i++) {
//        cout << universe.graph.node_list[i].pos;
//    }
//}
