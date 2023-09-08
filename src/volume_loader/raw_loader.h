#ifndef SCIVIS_VOL_LOADER_RAW_LOADER_H
#define SCIVIS_VOL_LOADER_RAW_LOADER_H

#include <algorithm>
#include <format>
#include <fstream>
#include <optional>
#include <source_location>
#include <string>

#include <array>
#include <vector>

#include <osg/Texture3D>

#include "type.h"

namespace SciVis {
namespace VolumeLoader {

template <typename SrcTy, typename DstTy> class RawLoader {
  public:
    template <typename Src2DstFuncTy>
    static std::vector<DstTy> LoadFromFile(const std::string &filePath,
                                           const std::array<int, 3> &dim, Src2DstFuncTy funcSrc2Dst,
                                           std::string *errMsg = nullptr) {
        std::ifstream is(filePath, std::ios::binary | std::ios::out | std::ios::ate);
        if (!is.is_open()) {
            if (errMsg)
                *errMsg = std::format("File:{} => Func:{} => Err: Cannot open file {}",
                                      std::source_location::current().file_name(),
                                      std::source_location::current().function_name(), filePath);
            return std::vector<DstTy>();
        }

        auto voxNum = static_cast<size_t>(is.tellg()) / sizeof(SrcTy);
        {
            auto _voxNum = (size_t)dim[0] * dim[1] * dim[2];
            if (voxNum < _voxNum) {
                if (errMsg)
                    *errMsg = std::format("File:{} => Func:{} => Err: Volume in file {} is smaller "
                                          "than dim ({},{},{})",
                                          std::source_location::current().file_name(),
                                          std::source_location::current().function_name(), filePath,
                                          dim[0], dim[1], dim[2]);
                is.close();
                return std::vector<DstTy>();
            }
            voxNum = std::min(voxNum, _voxNum);
        }

        std::vector<SrcTy> src(voxNum);
        is.seekg(0);
        is.read(reinterpret_cast<char *>(src.data()), sizeof(SrcTy) * voxNum);
        is.close();

        std::vector<DstTy> dst(voxNum);
        std::transform(src.begin(), src.end(), dst.begin(), funcSrc2Dst);

        return dst;
    }
};

template <typename SrcTy> class RawConvertor {
  public:
    template <typename Src2DstFuncTy>
    static osg::ref_ptr<osg::Texture3D>
    LoadFromFileToTexture(const std::string &filePath, const std::array<int, 3> &srcDim,
                          const std::array<uint8_t, 3> &logDstDim, Src2DstFuncTy funcSrc2Dst,
                          std::string *errMsg = nullptr) {
        auto volDat = RawLoader<SrcTy, float>::LoadFromFile(filePath, srcDim, funcSrc2Dst, errMsg);
        if (volDat.empty())
            return nullptr;

        std::array dstDim{1 << logDstDim[0], 1 << logDstDim[1], 1 << logDstDim[2]};
        std::array scaleDst2Src{static_cast<float>(srcDim[0] - 1) / (dstDim[0] - 1),
                                static_cast<float>(srcDim[1] - 1) / (dstDim[1] - 1),
                                static_cast<float>(srcDim[2] - 1) / (dstDim[2] - 1)};

        osg::ref_ptr img = new osg::Image;
        img->allocateImage(dstDim[0], dstDim[1], dstDim[2], GL_RED, GL_FLOAT);
        img->setInternalTextureFormat(GL_RED);
        auto *pxPtr = reinterpret_cast<float *>(img->data());

        for (int z = 0; z < dstDim[2]; ++z)
            for (int y = 0; y < dstDim[1]; ++y)
                for (int x = 0; x < dstDim[0]; ++x) {
                    auto srcZ = static_cast<int>(z * scaleDst2Src[2]);
                    auto srcY = static_cast<int>(y * scaleDst2Src[1]);
                    auto srcX = static_cast<int>(x * scaleDst2Src[0]);
                    if (srcX >= srcDim[0])
                        srcX = srcDim[0] - 1;
                    if (srcY >= srcDim[1])
                        srcY = srcDim[1] - 1;
                    if (srcZ >= srcDim[2])
                        srcZ = srcDim[2] - 1;

                    auto XYZ2Offs = [&](int x, int y, int z) {
                        return static_cast<size_t>(z) * srcDim[1] * srcDim[0] + y * srcDim[0] + x;
                    };

                    if (srcX != 0)
                        --srcX;
                    if (srcY != 0)
                        --srcY;
                    if (srcZ != 0)
                        --srcZ;
                    auto srcX1 = srcX == srcDim[0] - 1 ? srcX : srcX + 1;
                    auto srcY1 = srcY == srcDim[1] - 1 ? srcY : srcY + 1;
                    auto srcZ1 = srcZ == srcDim[2] - 1 ? srcZ : srcZ + 1;

                    auto max = 0.f;
                    for (int subZ = srcZ; subZ <= srcZ1; ++subZ)
                        for (int subY = srcY; subY <= srcY1; ++subY)
                            for (int subX = srcX; subX <= srcX1; ++subX)
                                max = std::max(volDat[XYZ2Offs(subX, subY, subZ)], max);

                    *pxPtr = max;
                    ++pxPtr;
                }

        osg::ref_ptr tex = new osg::Texture3D;
        tex->setFilter(osg::Texture::MAG_FILTER, osg::Texture::FilterMode::LINEAR);
        tex->setFilter(osg::Texture::MIN_FILTER, osg::Texture::FilterMode::NEAREST);
        tex->setWrap(osg::Texture::WRAP_S, osg::Texture::WrapMode::CLAMP);
        tex->setWrap(osg::Texture::WRAP_T, osg::Texture::WrapMode::CLAMP);
        tex->setWrap(osg::Texture::WRAP_R, osg::Texture::WrapMode::CLAMP);
        tex->setInternalFormatMode(osg::Texture::InternalFormatMode::USE_IMAGE_DATA_FORMAT);
        tex->setImage(img);

        return tex;
    }
};

} // namespace VolumeLoader
} // namespace SciVis

#endif // !SCIVIS_VOL_LOADER_RAW_LOADER_H
