#ifndef SCIVIS_SCALAR_VISER_MCR_H
#define SCIVIS_SCALAR_VISER_MCR_H

#include <algorithm>
#include <numeric>
#include <string>

#include <array>
#include <map>
#include <optional>

#include <osg/CoordinateSystemNode>
#include <osg/Geometry>
#include <osg/Texture3D>

#include <scivis/callback.h>

#include "def_val.h"
#include "marching_cube_table.h"

#include "shaders/generated/mc_frag.h"
#include "shaders/generated/mc_vert.h"

namespace SciVis {
namespace ScalarViser {

class MarchingCubeCPURenderer {
  private:
    struct PerRendererParam {
        osg::ref_ptr<osg::Group> grp;
        osg::ref_ptr<osg::Program> program;

        PerRendererParam() {
            grp = new osg::Group;
            osg::ref_ptr vertShader = new osg::Shader(osg::Shader::VERTEX, mc_vert);
            osg::ref_ptr fragShader = new osg::Shader(osg::Shader::FRAGMENT, mc_frag);
            program = new osg::Program;
            program->addShader(vertShader);
            program->addShader(fragShader);
        }
    };
    PerRendererParam param;

    class PerVolumeParam {
        uint8_t rndrVertsBufIdx = 0;
        std::array<int, 3> volDim;
        osg::Vec3 voxSz;

        std::shared_ptr<std::vector<float>> volDat;

        osg::ref_ptr<osg::Geometry> geom;
        osg::ref_ptr<osg::Geode> geode;
        std::array<osg::ref_ptr<osg::Vec3Array>, 2> vertsBuf;

        osg::ref_ptr<osg::Uniform> minLatitute;
        osg::ref_ptr<osg::Uniform> maxLatitute;
        osg::ref_ptr<osg::Uniform> minLongtitute;
        osg::ref_ptr<osg::Uniform> maxLongtitute;
        osg::ref_ptr<osg::Uniform> minHeight;
        osg::ref_ptr<osg::Uniform> maxHeight;

      private:
        void swapVertsBuf() {
            rndrVertsBufIdx = (rndrVertsBufIdx + 1) & 1;

            geom->setVertexArray(vertsBuf[rndrVertsBufIdx]);

            geom->getPrimitiveSetList().clear();
            geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLES, 0,
                                                      vertsBuf[rndrVertsBufIdx]->size()));
        }

      public:
        PerVolumeParam(decltype(volDat) volDat, const std::array<int, 3> &volDim,
                       PerRendererParam *renderer)
            : volDat(volDat), volDim(volDim) {
            voxSz = osg::Vec3(1.f / volDim[0], 1.f / volDim[1], 1.f / volDim[2]);

            vertsBuf[0] = new osg::Vec3Array;
            vertsBuf[1] = new osg::Vec3Array;

            geom = new osg::Geometry;
            geode = new osg::Geode;
            geode->addDrawable(geom);

            auto states = geode->getOrCreateStateSet();
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
#undef STATEMENT

            states->setAttributeAndModes(renderer->program, osg::StateAttribute::ON);
        }

        void MarchingCube(float isoVal) {
            using DimTy = decltype(volDim)::value_type;

            auto volDimYxX = static_cast<size_t>(volDim[1]) * volDim[0];
            auto sample = [&](DimTy x, DimTy y, DimTy z) {
                x = std::min(x, volDim[0] - 1);
                y = std::min(y, volDim[1] - 1);
                z = std::min(z, volDim[2] - 1);
                auto i = z * volDimYxX + y * volDim[0] + x;
                return (*volDat)[i];
            };
            auto cmptField = [&](DimTy x, DimTy y, DimTy z) {
                std::array<float, 8> field;
                field[0] = sample(x + 0, y + 0, z + 0);
                field[1] = sample(x + 1, y + 0, z + 0);
                field[2] = sample(x + 1, y + 1, z + 0);
                field[3] = sample(x + 0, y + 1, z + 0);
                field[4] = sample(x + 0, y + 0, z + 1);
                field[5] = sample(x + 1, y + 0, z + 1);
                field[6] = sample(x + 1, y + 1, z + 1);
                field[7] = sample(x + 0, y + 1, z + 1);
                return field;
            };
            auto cmptCubeIdx = [&](const std::array<float, 8> &field) {
                uint32_t cubeIdx = 0;
                cubeIdx |= field[0] < isoVal ? (1 << 0) : 0;
                cubeIdx |= field[1] < isoVal ? (1 << 1) : 0;
                cubeIdx |= field[2] < isoVal ? (1 << 2) : 0;
                cubeIdx |= field[3] < isoVal ? (1 << 3) : 0;
                cubeIdx |= field[4] < isoVal ? (1 << 4) : 0;
                cubeIdx |= field[5] < isoVal ? (1 << 5) : 0;
                cubeIdx |= field[6] < isoVal ? (1 << 6) : 0;
                cubeIdx |= field[7] < isoVal ? (1 << 7) : 0;
                return cubeIdx;
            };

            std::vector<size_t> voxVertNums(volDat->size(), 0);
            for (size_t i = 0; i < volDat->size(); ++i) {
                DimTy z = i / volDimYxX;
                DimTy y = (i - z * volDimYxX) / volDim[0];
                DimTy x = i - z * volDimYxX - y * volDim[0];

                auto field = cmptField(x, y, z);
                auto cubeIdx = cmptCubeIdx(field);

                voxVertNums[i] = VertNumTable[cubeIdx];
            }

            auto vertNum = std::accumulate(voxVertNums.begin(), voxVertNums.end(), 0);

            auto cmptVerts = vertsBuf[(rndrVertsBufIdx + 1) & 1];
            cmptVerts->clear();
            cmptVerts->reserve(vertNum);

            for (size_t i = 0; i < volDat->size(); ++i) {
                if (voxVertNums[i] == 0)
                    continue;

                DimTy z = i / volDimYxX;
                DimTy y = (i - z * volDimYxX) / volDim[0];
                DimTy x = i - z * volDimYxX - y * volDim[0];

                std::array<osg::Vec3, 8> v;
                {
                    osg::Vec3f p(x * voxSz.x(), y * voxSz.y(), z * voxSz.z());
                    v[0] = p;
                    v[1] = p + osg::Vec3(voxSz.x(), 0.f, 0.f);
                    v[2] = p + osg::Vec3(voxSz.x(), voxSz.y(), 0.f);
                    v[3] = p + osg::Vec3(0.f, voxSz.y(), 0.f);
                    v[4] = p + osg::Vec3(0.f, 0.f, voxSz.z());
                    v[5] = p + osg::Vec3(voxSz.x(), 0.f, voxSz.z());
                    v[6] = p + osg::Vec3(voxSz.x(), voxSz.y(), voxSz.z());
                    v[7] = p + osg::Vec3(0.f, voxSz.y(), voxSz.z());
                }
                auto field = cmptField(x, y, z);
                auto cubeIdx = cmptCubeIdx(field);

                std::array<osg::Vec3, 12> vertList;
                auto vertInterp = [&](const osg::Vec3 &p0, const osg::Vec3 &p1, float f0,
                                      float f1) {
                    float t = (isoVal - f0) / (f1 - f0);
                    auto dlt = p1 - p0;
                    return osg::Vec3(p0.x() + t * dlt.x(), p0.y() + t * dlt.y(),
                                     p0.z() + t * dlt.z());
                };
                vertList[0] = vertInterp(v[0], v[1], field[0], field[1]);
                vertList[1] = vertInterp(v[1], v[2], field[1], field[2]);
                vertList[2] = vertInterp(v[2], v[3], field[2], field[3]);
                vertList[3] = vertInterp(v[3], v[0], field[3], field[0]);

                vertList[4] = vertInterp(v[4], v[5], field[4], field[5]);
                vertList[5] = vertInterp(v[5], v[6], field[5], field[6]);
                vertList[6] = vertInterp(v[6], v[7], field[6], field[7]);
                vertList[7] = vertInterp(v[7], v[4], field[7], field[4]);

                vertList[8] = vertInterp(v[0], v[4], field[0], field[4]);
                vertList[9] = vertInterp(v[1], v[5], field[1], field[5]);
                vertList[10] = vertInterp(v[2], v[6], field[2], field[6]);
                vertList[11] = vertInterp(v[3], v[7], field[3], field[7]);

                auto cmptNorm = [](const osg::Vec3 &p0, const osg::Vec3 &p1, const osg::Vec3 &p2) {
                    auto e0 = p1 - p0;
                    auto e1 = p2 - p1;
                    auto n = e0 ^ e1;
                    n.normalize();
                    return n;
                };
                for (uint32_t j = 0; j < voxVertNums[i]; j += 3) {
                    auto edge = TriangleTable[cubeIdx][j];
                    cmptVerts->push_back(vertList[edge]);
                    edge = TriangleTable[cubeIdx][j + 1];
                    cmptVerts->push_back(vertList[edge]);
                    edge = TriangleTable[cubeIdx][j + 2];
                    cmptVerts->push_back(vertList[edge]);
                }
            }

            swapVertsBuf();
        }

        friend class MarchingCubeCPURenderer;
    };
    std::map<std::string, PerVolumeParam> vols;

  public:
    MarchingCubeCPURenderer() {}

    osg::Group *GetGroup() { return param.grp.get(); }

    void AddVolume(const std::string &name, decltype(PerVolumeParam::volDat) volDat,
                   const std::array<int, 3> &volDim) {
        if (auto itr = vols.find(name); itr != vols.end()) {
            param.grp->removeChild(itr->second.geode);
            vols.erase(itr);
        }
        auto opt = vols.emplace(std::piecewise_construct, std::forward_as_tuple(name),
                                std::forward_as_tuple(volDat, volDim, &param));
        param.grp->addChild(opt.first->second.geode);
    }

    std::optional<decltype(vols)::iterator> GetVolume(const std::string &name) {
        auto itr = vols.find(name);
        if (itr == vols.end())
            return {};
        return itr;
    }
};

} // namespace ScalarViser
} // namespace SciVis

#endif // !SCIVIS_SCALAR_VISER_MCR_H
