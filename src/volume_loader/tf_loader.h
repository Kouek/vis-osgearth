#ifndef SCIVIS_VOL_LOADER_TF_LOADER_H
#define SCIVIS_VOL_LOADER_TF_LOADER_H

#include <algorithm>
#include <format>
#include <fstream>
#include <optional>
#include <source_location>
#include <string>

#include <array>
#include <vector>

#include <osg/Texture1D>

#include "type.h"

namespace SciVis {
namespace VolumeLoader {

template <typename KeyTy> class TFLoader {
  public:
    static osg::ref_ptr<osg::Texture1D> LoadFromFileToTexture(const std::string &filePath,
                                                              std::string *errMsg = nullptr) {
        static_assert(std::is_unsigned_v<KeyTy> && std::is_integral_v<KeyTy>);

        static constexpr auto TexW = static_cast<int>(std::numeric_limits<KeyTy>::max()) + 1;

        std::ifstream is(filePath, std::ios::in);
        if (!is.is_open()) {
            if (errMsg)
                *errMsg = std::format("File:{} => Func:{} => Err: Cannot open file {}",
                                      std::source_location::current().file_name(),
                                      std::source_location::current().function_name(), filePath);
            return nullptr;
        }

        osg::ref_ptr img = new osg::Image;
        img->allocateImage(TexW, 1, 1, GL_RGBA, GL_FLOAT);
        img->setInternalTextureFormat(GL_RGBA);

        std::string buf;
        std::vector<std::pair<KeyTy, std::array<double, 4>>> tfPnts;
        while (std::getline(is, buf)) {
            double key;
            std::array<double, 4> col;

            auto cnt =
                sscanf(buf.c_str(), "%lf%lf%lf%lf%lf", &key, &col[0], &col[1], &col[2], &col[3]);
            if (cnt != 5)
                continue;

            for (auto &v : col)
                v /= 255.0;
            tfPnts.emplace_back(std::make_pair(static_cast<KeyTy>(key), col));
        }

        auto pntItr = tfPnts.begin();
        auto lftPntItr = pntItr;
        double lft2Rht = 1.0;
        auto *pxPtr = reinterpret_cast<osg::Vec4 *>(img->data());
        for (int i = 0; i < TexW; ++i) {
            auto assign = [&](double t = 1.0) {
                pxPtr[i].r() = (1.0 - t) * lftPntItr->second[0] + t * pntItr->second[0];
                pxPtr[i].g() = (1.0 - t) * lftPntItr->second[1] + t * pntItr->second[1];
                pxPtr[i].b() = (1.0 - t) * lftPntItr->second[2] + t * pntItr->second[2];
                pxPtr[i].a() = (1.0 - t) * lftPntItr->second[3] + t * pntItr->second[3];
            };

            if (pntItr == tfPnts.end())
                assign();
            else if (i == static_cast<int>(pntItr->first)) {
                assign();
                lftPntItr = pntItr;
                ++pntItr;
                if (pntItr != tfPnts.end())
                    lft2Rht = pntItr->first - lftPntItr->first;
                else
                    lft2Rht = 1.f;
            } else
                assign((i - lftPntItr->first) / lft2Rht);
        }

        osg::ref_ptr tex = new osg::Texture1D;
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

#endif // !SCIVIS_VOL_LOADER_TF_LOADER_H
