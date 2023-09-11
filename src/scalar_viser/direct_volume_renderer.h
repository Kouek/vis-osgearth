#ifndef SCIVIS_SCALAR_VISER_DVR_H
#define SCIVIS_SCALAR_VISER_DVR_H

#include <numbers>
#include <string>

#include <map>

#include <osg/CoordinateSystemNode>
#include <osg/CullFace>
#include <osg/ShapeDrawable>
#include <osg/Texture1D>
#include <osg/Texture3D>

#include <scivis/callback.h>

#include "shaders/generated/dvr_sphere_proxy_frag.h"
#include "shaders/generated/dvr_sphere_proxy_vert.h"

namespace SciVis {
namespace ScalarViser {

class DirectVolumeRenderer {
  private:
    struct PerRendererParam {
        osg::ref_ptr<osg::Group> grp;
        osg::ref_ptr<osg::Program> program;

        osg::ref_ptr<osg::Uniform> eyePos;
        osg::ref_ptr<osg::Uniform> dt;

        PerRendererParam() {
            grp = new osg::Group;

            osg::ref_ptr vertShader = new osg::Shader(osg::Shader::VERTEX, dvr_sphere_proxy_vert);
            osg::ref_ptr fragShader = new osg::Shader(osg::Shader::FRAGMENT, dvr_sphere_proxy_frag);
            program = new osg::Program;
            program->addShader(vertShader);
            program->addShader(fragShader);

#define STATEMENT(name, val) name = new osg::Uniform(#name, val)
            STATEMENT(eyePos, osg::Vec3());
            STATEMENT(dt, static_cast<float>(osg::WGS_84_RADIUS_EQUATOR) * .0005f);
#undef STATEMENT
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

        osg::ref_ptr<osg::ShapeDrawable> sphere;
        osg::ref_ptr<osg::Texture3D> volTex;
        osg::ref_ptr<osg::Texture1D> tfTex;

        class Callback : public osg::NodeCallback {
          private:
            PerRendererParam *renderer;

          public:
            Callback(PerRendererParam *renderer) : renderer(renderer) {}
            virtual void operator()(osg::Node *node, osg::NodeVisitor *nv) {
                auto eyePos = nv->getEyePoint();
                renderer->eyePos->setUpdateCallback(new UniformUpdateCallback<osg::Vec3>(eyePos));

                traverse(node, nv);
            }
        };

        PerVolParam(osg::ref_ptr<osg::Texture3D> volTex, osg::ref_ptr<osg::Texture1D> tfTex,
                    PerRendererParam *renderer)
            : volTex(volTex), tfTex(tfTex) {
            static const auto MinHeight = static_cast<float>(osg::WGS_84_RADIUS_EQUATOR) * .5f;
            static const auto MaxHeight = static_cast<float>(osg::WGS_84_RADIUS_EQUATOR) * .7f;

            sphere = new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(0.f, 0.f, 0.f), MaxHeight),
                                            new osg::TessellationHints);
            sphere->setUseVertexBufferObjects(true);

            auto states = sphere->getOrCreateStateSet();
            auto deg2Rad = [](float deg) {
                return deg * static_cast<float>(std::numbers::pi) / 180.f;
            };
#define STATEMENT(name, val)                                                                       \
    name = new osg::Uniform(#name, val);                                                           \
    states->addUniform(name)
            STATEMENT(minLatitute, deg2Rad(-23.f));
            STATEMENT(maxLatitute, deg2Rad(+23.f));
            STATEMENT(minLongtitute, deg2Rad(-20.f));
            STATEMENT(maxLongtitute, deg2Rad(+20.f));
            STATEMENT(minHeight, MinHeight);
            STATEMENT(maxHeight, MaxHeight);
#undef STATEMENT
            states->addUniform(renderer->eyePos);
            states->addUniform(renderer->dt);

            states->setTextureAttributeAndModes(0, volTex, osg::StateAttribute::ON);
            states->setTextureAttributeAndModes(1, tfTex, osg::StateAttribute::ON);

            osg::ref_ptr cf = new osg::CullFace(osg::CullFace::BACK);
            states->setAttributeAndModes(cf);
            sphere->addCullCallback(new Callback(renderer));

            states->setAttributeAndModes(renderer->program, osg::StateAttribute::ON);
            states->setMode(GL_BLEND, osg::StateAttribute::ON);
        }
    };
    std::map<std::string, PerVolParam> vols;

  public:
    DirectVolumeRenderer() {}

    osg::Group *GetGroup() { return param.grp.get(); }

    void AddVolume(const std::string &name, osg::ref_ptr<osg::Texture3D> volTex,
                   osg::ref_ptr<osg::Texture1D> tfTex) {
        if (auto itr = vols.find(name); itr != vols.end()) {
            param.grp->removeChild(itr->second.sphere);
            vols.erase(itr);
        }
        auto opt = vols.emplace(std::piecewise_construct, std::forward_as_tuple(name),
                                std::forward_as_tuple(volTex, tfTex, &param));
        param.grp->addChild(opt.first->second.sphere);
    }
};

} // namespace ScalarViser
} // namespace SciVis

#endif // !SCIVIS_SCALAR_VISER_DVR_H
