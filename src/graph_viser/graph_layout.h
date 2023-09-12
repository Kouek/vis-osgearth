#ifndef SCIVIS_GRAPH_VISER_GL_H
#define SCIVIS_GRAPH_VISER_GL_H

#include <chrono>
#include <random>

#include <osg/Group>
#include <osg/LineWidth>
#include <osg/Material>
#include <osg/ShapeDrawable>

#include "force_direct_2d.h"
#include "force_direct_3d.h"

namespace SciVis {
namespace GraphViser {

class GraphLayout2D {
  private:
    osg::ref_ptr<osg::Group> grp;

  public:
    GraphLayout2D() {
        int waitScnds = 20;
        int n_random_nodes = 200;
        int n_iterations = std::numeric_limits<int>::max();
        float timeDelta = 0.2;
        float repulsionForce = 1.0f;
        float springForce = 1.0f;
        float dampingCoefficient = 0.5f;
        float gravitationalForce = 0.5f;

        GraphViser2D::Graph graph;
        std::mt19937 e;
        for (int i = 0; i < n_random_nodes; i++) {
            int id = graph.add_node("C");
            std::uniform_real_distribution<double> u(0, id);
            int rd = u(e);
            graph.add_edge(id, rd);
        }
        GraphViser2D::Universe universe(graph, timeDelta, repulsionForce, springForce,
                                        dampingCoefficient, gravitationalForce);

        auto start = std::chrono::system_clock::now();
        while (universe.n_iterations < n_iterations) {
            auto curr = std::chrono::system_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::seconds>(curr - start);
            if (duration.count() > waitScnds)
                break;
            universe.update(timeDelta);
        }

        osg::Vec2 minPos(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
        osg::Vec2 maxPos(std::numeric_limits<float>::min(), std::numeric_limits<float>::min());
        for (int i = 0; i < universe.graph.node_list.size(); ++i) {
            osg::Vec2 p(universe.graph.node_list[i].pos.x, universe.graph.node_list[i].pos.y);
            minPos.x() = std::min(minPos.x(), p.x());
            minPos.y() = std::min(minPos.y(), p.y());
            maxPos.x() = std::max(maxPos.x(), p.x());
            maxPos.y() = std::max(maxPos.y(), p.y());
        }

        auto fromGraphVec2DToOSGVec3 = [](const GraphViser2D::Vec2D &v2) {
            return osg::Vec3(v2.x, 0.f, v2.y);
        };

        auto radius = (maxPos - minPos).length() / universe.graph.node_list.size();
        grp = new osg::Group;
        for (int i = 0; i < universe.graph.node_list.size(); ++i) {
            auto &pos = universe.graph.node_list[i].pos;
            grp->addChild(
                new osg::ShapeDrawable(new osg::Sphere(fromGraphVec2DToOSGVec3(pos), radius)));
        }

        osg::ref_ptr linkVerts = new osg::Vec3Array;

        struct Link {
            int id0, id1;
            Link(int id0, int id1) : id0(id0), id1(id1) {
                if (id0 > id1)
                    std::swap(id0, id1);
            }
        };
        struct LinkHs {
            size_t operator()(const Link &lnk) const {
                return std::hash<decltype(Link::id0)>()(lnk.id0) &
                       std::hash<decltype(Link::id0)>()(lnk.id1);
            }
        };
        struct LinkeEq {
            bool operator()(const Link &a, const Link &b) const {
                return a.id0 == b.id0 && a.id1 == b.id1;
            }
        };
        std::unordered_set<Link, LinkHs, LinkeEq> visiteds;

        for (int i = 0; i < universe.graph.adj_list.size(); ++i) {
            auto &pos0 = universe.graph.node_list[i].pos;
            for (auto neighbor : universe.graph.adj_list[i]) {
                if (auto itr = visiteds.find(Link(i, neighbor)); itr != visiteds.end())
                    continue;
                visiteds.emplace(i, neighbor);

                auto pos1 = universe.graph.node_list[neighbor].pos;
                linkVerts->push_back(fromGraphVec2DToOSGVec3(pos0));
                linkVerts->push_back(fromGraphVec2DToOSGVec3(pos1));
            }
        }
        osg::ref_ptr linkGeom = new osg::Geometry;
        linkGeom->setVertexArray(linkVerts.get());
        linkGeom->addPrimitiveSet(
            new osg::DrawArrays(osg::PrimitiveSet::LINES, 0, linkVerts->size()));

        osg::ref_ptr linkGeode = new osg::Geode;
        linkGeode->addDrawable(linkGeom);
        auto states = linkGeode->getOrCreateStateSet();
        states->setAttributeAndModes(new osg::LineWidth(5.f), osg::StateAttribute::ON);

        grp->addChild(linkGeode);
    }

    osg::Group *GetGroup() { return grp; }
};

class GraphLayout3D {
  private:
    osg::ref_ptr<osg::Group> grp;

  public:
    GraphLayout3D() {
        int waitScnds = 20;
        int n_random_nodes = 200;
        int n_iterations = std::numeric_limits<int>::max();
        float timeDelta = 0.2;
        float repulsionForce = 1.0f;
        float springForce = 1.0f;
        float dampingCoefficient = 0.5f;
        float gravitationalForce = 0.5f;

        GraphViser3D::Graph graph;
        std::mt19937 e;
        for (int i = 0; i < n_random_nodes; i++) {
            int id = graph.add_node("C");
            std::uniform_real_distribution<double> u(0, id);
            int rd = u(e);
            graph.add_edge(id, rd);
        }
        GraphViser3D::Universe universe(graph, timeDelta, repulsionForce, springForce,
                                        dampingCoefficient, gravitationalForce);

        auto start = std::chrono::system_clock::now();
        while (universe.n_iterations < n_iterations) {
            auto curr = std::chrono::system_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::seconds>(curr - start);
            if (duration.count() > waitScnds)
                break;
            universe.update(timeDelta);
        }

        osg::Vec2 minPos(std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
        osg::Vec2 maxPos(std::numeric_limits<float>::min(), std::numeric_limits<float>::min());
        for (int i = 0; i < universe.graph.node_list.size(); ++i) {
            osg::Vec2 p(universe.graph.node_list[i].pos.x, universe.graph.node_list[i].pos.y);
            minPos.x() = std::min(minPos.x(), p.x());
            minPos.y() = std::min(minPos.y(), p.y());
            maxPos.x() = std::max(maxPos.x(), p.x());
            maxPos.y() = std::max(maxPos.y(), p.y());
        }

        auto fromGraphVec3DToOSGVec3 = [](const GraphViser3D::Vec3D &v2) {
            return osg::Vec3(v2.x, v2.y, v2.z);
        };

        auto radius = (maxPos - minPos).length() / universe.graph.node_list.size();
        grp = new osg::Group;
        for (int i = 0; i < universe.graph.node_list.size(); ++i) {
            auto &pos = universe.graph.node_list[i].pos;
            grp->addChild(
                new osg::ShapeDrawable(new osg::Sphere(fromGraphVec3DToOSGVec3(pos), radius)));
        }

        osg::ref_ptr linkVerts = new osg::Vec3Array;

        struct Link {
            int id0, id1;
            Link(int id0, int id1) : id0(id0), id1(id1) {
                if (id0 > id1)
                    std::swap(id0, id1);
            }
        };
        struct LinkHs {
            size_t operator()(const Link &lnk) const {
                return std::hash<decltype(Link::id0)>()(lnk.id0) &
                       std::hash<decltype(Link::id0)>()(lnk.id1);
            }
        };
        struct LinkeEq {
            bool operator()(const Link &a, const Link &b) const {
                return a.id0 == b.id0 && a.id1 == b.id1;
            }
        };
        std::unordered_set<Link, LinkHs, LinkeEq> visiteds;

        for (int i = 0; i < universe.graph.adj_list.size(); ++i) {
            auto &pos0 = universe.graph.node_list[i].pos;
            for (auto neighbor : universe.graph.adj_list[i]) {
                if (auto itr = visiteds.find(Link(i, neighbor)); itr != visiteds.end())
                    continue;
                visiteds.emplace(i, neighbor);

                auto pos1 = universe.graph.node_list[neighbor].pos;
                linkVerts->push_back(fromGraphVec3DToOSGVec3(pos0));
                linkVerts->push_back(fromGraphVec3DToOSGVec3(pos1));
            }
        }
        osg::ref_ptr linkGeom = new osg::Geometry;
        linkGeom->setVertexArray(linkVerts.get());
        linkGeom->addPrimitiveSet(
            new osg::DrawArrays(osg::PrimitiveSet::LINES, 0, linkVerts->size()));

        osg::ref_ptr linkGeode = new osg::Geode;
        linkGeode->addDrawable(linkGeom);
        auto states = linkGeode->getOrCreateStateSet();
        states->setAttributeAndModes(new osg::LineWidth(5.f), osg::StateAttribute::ON);

        grp->addChild(linkGeode);
    }

    osg::Group *GetGroup() { return grp; }
};

} // namespace GraphViser
} // namespace SciVis

#endif // !SCIVIS_GRAPH_VISER_GL_H
