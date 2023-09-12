#ifndef SCIVIS_SCALAR_VISER_HMR_H
#define SCIVIS_SCALAR_VISER_HMR_H

#include <numbers>
#include <string>

#include <map>

#include <osg/CoordinateSystemNode>
#include <osg/CullFace>
#include <osg/ShapeDrawable>
#include <osg/Texture1D>
#include <osg/Texture3D>

#include <scivis/callback.h>

#include "def_val.h"

#include "shaders/generated/hmr_2d_frag.h"
#include "shaders/generated/hmr_2d_vert.h"
#include "shaders/generated/hmr_3d_frag.h"
#include "shaders/generated/hmr_3d_vert.h"

namespace SciVis {
namespace ScalarViser {

class HeatMap3DRenderer {
  private:
    struct PerRendererParam {
        osg::ref_ptr<osg::Group> grp;
        osg::ref_ptr<osg::Program> program;

        PerRendererParam() {
            grp = new osg::Group;

            osg::ref_ptr vertShader = new osg::Shader(osg::Shader::VERTEX, hmr_3d_vert);
            osg::ref_ptr fragShader = new osg::Shader(osg::Shader::FRAGMENT, hmr_3d_frag);
            program = new osg::Program;
            program->addShader(vertShader);
            program->addShader(fragShader);
        }
    };
    PerRendererParam param;

    struct PerVolParam {
        osg::ref_ptr<osg::Uniform> minLatitute;
        osg::ref_ptr<osg::Uniform> maxLatitute;
        osg::ref_ptr<osg::Uniform> minLongtitute;
        osg::ref_ptr<osg::Uniform> maxLongtitute;
        osg::ref_ptr<osg::Uniform> minHeight;
        osg::ref_ptr<osg::Uniform> maxHeight;
        osg::ref_ptr<osg::Uniform> height;

        osg::ref_ptr<osg::ShapeDrawable> sphere;
        osg::ref_ptr<osg::Texture3D> volTex;
        osg::ref_ptr<osg::Texture1D> colTblTex;

        PerVolParam(osg::ref_ptr<osg::Texture3D> volTex, osg::ref_ptr<osg::Texture1D> colTblTex,
                    PerRendererParam *renderer)
            : volTex(volTex), colTblTex(colTblTex) {
            sphere = new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(0.f, 0.f, 0.f), MinHeight),
                                            new osg::TessellationHints);
            sphere->setUseVertexBufferObjects(true);

            auto states = sphere->getOrCreateStateSet();
            auto deg2Rad = [](float deg) {
                return deg * static_cast<float>(std::numbers::pi) / 180.f;
            };
#define STATEMENT(name, val)                                                                       \
    name = new osg::Uniform(#name, val);                                                           \
    states->addUniform(name)
            STATEMENT(minLatitute, deg2Rad(MinLatitute));
            STATEMENT(maxLatitute, deg2Rad(MaxLatitute));
            STATEMENT(minLongtitute, deg2Rad(MinLongtitute));
            STATEMENT(maxLongtitute, deg2Rad(MaxLongtitute));
            STATEMENT(minHeight, MinHeight);
            STATEMENT(maxHeight, MaxHeight);
            STATEMENT(height, .5f * (MinHeight + MaxHeight));
#undef STATEMENT

            states->setTextureAttributeAndModes(0, volTex, osg::StateAttribute::ON);
            states->setTextureAttributeAndModes(1, colTblTex, osg::StateAttribute::ON);

            osg::ref_ptr cf = new osg::CullFace(osg::CullFace::BACK);
            states->setAttributeAndModes(cf);

            states->setAttributeAndModes(renderer->program, osg::StateAttribute::ON);
            states->setMode(GL_BLEND, osg::StateAttribute::ON);
        }
    };
    std::map<std::string, PerVolParam> vols;

  public:
    HeatMap3DRenderer() {}

    osg::Group *GetGroup() { return param.grp.get(); }

    void AddVolume(const std::string &name, osg::ref_ptr<osg::Texture3D> volTex,
                   osg::ref_ptr<osg::Texture1D> colTblTex) {
        if (auto itr = vols.find(name); itr != vols.end()) {
            param.grp->removeChild(itr->second.sphere);
            vols.erase(itr);
        }
        auto opt = vols.emplace(std::piecewise_construct, std::forward_as_tuple(name),
                                std::forward_as_tuple(volTex, colTblTex, &param));
        param.grp->addChild(opt.first->second.sphere);
    }
};

class HeatMap2DRenderer {
  private:
    struct PerRendererParam {
        osg::ref_ptr<osg::Group> grp;
        osg::ref_ptr<osg::Program> program;

        PerRendererParam() {
            grp = new osg::Group;

            osg::ref_ptr vertShader = new osg::Shader(osg::Shader::VERTEX, hmr_2d_vert);
            osg::ref_ptr fragShader = new osg::Shader(osg::Shader::FRAGMENT, hmr_2d_frag);
            program = new osg::Program;
            program->addShader(vertShader);
            program->addShader(fragShader);
        }
    };
    PerRendererParam param;

    struct PerVolParam {
        osg::ref_ptr<osg::Uniform> minHeight;
        osg::ref_ptr<osg::Uniform> maxHeight;
        osg::ref_ptr<osg::Uniform> height;

        osg::ref_ptr<osg::Geode> geode;
        osg::ref_ptr<osg::Geometry> geom;
        osg::ref_ptr<osg::Texture3D> volTex;
        osg::ref_ptr<osg::Texture1D> colTblTex;

        PerVolParam(osg::ref_ptr<osg::Texture3D> volTex, osg::ref_ptr<osg::Texture1D> colTblTex,
                    PerRendererParam *renderer)
            : volTex(volTex), colTblTex(colTblTex) {
            osg::ref_ptr verts = new osg::Vec3Array;
            verts->push_back(osg::Vec3(-1.f, -1.f, 0.f));
            verts->push_back(osg::Vec3(+1.f, -1.f, 0.f));
            verts->push_back(osg::Vec3(+1.f, +1.f, 0.f));
            verts->push_back(osg::Vec3(+1.f, +1.f, 0.f));
            verts->push_back(osg::Vec3(-1.f, +1.f, 0.f));
            verts->push_back(osg::Vec3(-1.f, -1.f, 0.f));
            geom = new osg::Geometry;
            geom->setVertexArray(verts.get());
            geom->addPrimitiveSet(
                new osg::DrawArrays(osg::PrimitiveSet::TRIANGLES, 0, verts->size()));

            geode = new osg::Geode;
            geode->addDrawable(geom);
            auto states = geode->getOrCreateStateSet();
#define STATEMENT(name, val)                                                                       \
    name = new osg::Uniform(#name, val);                                                           \
    states->addUniform(name)
            STATEMENT(minHeight, MinHeight);
            STATEMENT(maxHeight, MaxHeight);
            STATEMENT(height, .5f * (MinHeight + MaxHeight));
#undef STATEMENT

            states->setTextureAttributeAndModes(0, volTex, osg::StateAttribute::ON);
            states->setTextureAttributeAndModes(1, colTblTex, osg::StateAttribute::ON);

            states->setAttributeAndModes(renderer->program, osg::StateAttribute::ON);
            states->setMode(GL_BLEND, osg::StateAttribute::ON);
        }
    };
    std::map<std::string, PerVolParam> vols;

  public:
    HeatMap2DRenderer() {}

    osg::Group *GetGroup() { return param.grp.get(); }

    void AddVolume(const std::string &name, osg::ref_ptr<osg::Texture3D> volTex,
                   osg::ref_ptr<osg::Texture1D> colTblTex) {
        if (auto itr = vols.find(name); itr != vols.end()) {
            param.grp->removeChild(itr->second.geode);
            vols.erase(itr);
        }
        auto opt = vols.emplace(std::piecewise_construct, std::forward_as_tuple(name),
                                std::forward_as_tuple(volTex, colTblTex, &param));
        param.grp->addChild(opt.first->second.geode);
    }
};

} // namespace ScalarViser
} // namespace SciVis

#endif // !SCIVIS_SCALAR_VISER_HMR_H
