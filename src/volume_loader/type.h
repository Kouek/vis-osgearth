#ifndef SCIVIS_VOL_LOADER_TYPE_H
#define SCIVIS_VOL_LOADER_TYPE_H

#include <type_traits>

#include <osg/GL>

namespace SciVis {
namespace VolumeLoader {

template <typename... T> constexpr bool AlwaysFalse = false;

template <typename Ty> constexpr GLenum VoxTy2GLPxFmt() {
    if constexpr (std::is_same_v<Ty, uint8_t> || std::is_same_v<Ty, float>)
        return GL_RED;
    else
        static_assert(AlwaysFalse<Ty>);
}

template <typename Ty> constexpr GLenum VoxTy2GLTy() {
    if constexpr (std::is_same_v<Ty, uint8_t>)
        return GL_UNSIGNED_BYTE;
    else if constexpr (std::is_same_v<Ty, float>)
        return GL_FLOAT;
    else
        static_assert(AlwaysFalse<Ty>);
}

} // namespace VolumeLoader
} // namespace SciVis

#endif // !SCIVIS_VOL_LOADER_TYPE_H
