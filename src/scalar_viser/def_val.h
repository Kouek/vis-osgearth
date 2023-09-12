#ifndef SCIVIS_SCALAR_VISER_DEF_VAL_H
#define SCIVIS_SCALAR_VISER_DEF_VAL_H

#include <osg/CoordinateSystemNode>

namespace SciVis {
namespace ScalarViser {

inline const auto MinHeight = static_cast<float>(osg::WGS_84_RADIUS_EQUATOR) * 1.005f;
inline const auto MaxHeight = static_cast<float>(osg::WGS_84_RADIUS_EQUATOR) * 1.205f;
inline const auto MinLongtitute = -20.f;
inline const auto MaxLongtitute = +20.f;
inline const auto MinLatitute = -23.5f;
inline const auto MaxLatitute = +23.5f;

} // namespace ScalarViser
} // namespace SciVis

#endif // !SCIVIS_SCALAR_VISER_DEF_VAL_H
