#include <memory>

#include <osg/PositionAttitudeTransform>
#include <osg/ShapeDrawable>
#include <osg/Texture2D>

#include <osgDB/FileUtils>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>

#include <graph_viser/graph_layout.h>
#include <scalar_viser/direct_volume_renderer.h>
#include <scalar_viser/heat_map_renderer.h>
#include <scalar_viser/marching_cube_renderer.h>
#include <volume_loader/raw_loader.h>
#include <volume_loader/tf_loader.h>

static inline osg::Node *createEarth() {
    auto *hints = new osg::TessellationHints;
    hints->setDetailRatio(5.0f);

    auto *sd = new osg::ShapeDrawable(
        new osg::Sphere(osg::Vec3(0.0, 0.0, 0.0), osg::WGS_84_RADIUS_POLAR), hints);
    sd->setUseVertexBufferObjects(true);
    sd->setUseVertexArrayObject(true);

    auto *geode = new osg::Geode;
    geode->addDrawable(sd);

    auto filename = osgDB::findDataFile("land_shallow_topo_2048.jpg");
    geode->getOrCreateStateSet()->setTextureAttributeAndModes(
        0, new osg::Texture2D(osgDB::readImageFile(filename)));

    auto *csn = new osg::CoordinateSystemNode;
    csn->setEllipsoidModel(new osg::EllipsoidModel);
    csn->addChild(geode);

    return csn;
}

#define TEST_DVR
// #define TEST_MARCHING_CUBE
// #define TEST_HEAT_MAP_2D
// #define TEST_HEAT_MAP_3D
// #define TEST_GRAPH_LAYOUT_2D
// #define TEST_GRAPH_LAYOUT_3D

#ifdef TEST_DVR
int main(int argc, char **argv) {
    osg::ref_ptr viewer = new osgViewer::Viewer;
    viewer->setUpViewInWindow(200, 50, 800, 600);

    osg::ref_ptr grp = new osg::Group;
    grp->addChild(createEarth());

    SciVis::ScalarViser::DirectVolumeRenderer renderer;

    {
        auto volTex = SciVis::VolumeLoader::RawConvertor<uint8_t>::LoadFromFileToTexture(
            "CLOUDf01.bin", {500, 500, 100}, {8, 8, 6},
            [](const uint8_t &src) -> float { return src / 255.f; });
        auto tfTex = SciVis::VolumeLoader::TFLoader<uint8_t>::LoadFromFileToTexture("cloud_tf.txt");
        renderer.AddVolume("cloud01", volTex, tfTex);
    }
    grp->addChild(renderer.GetGroup());

    viewer->setSceneData(grp);
    viewer->run();

    return 0;
}
#elif defined(TEST_MARCHING_CUBE)
int main(int argc, char **argv) {
    osg::ref_ptr viewer = new osgViewer::Viewer;
    viewer->setUpViewInWindow(200, 50, 800, 600);

    osg::ref_ptr grp = new osg::Group;
    grp->addChild(createEarth());

    SciVis::ScalarViser::MarchingCubeCPURenderer renderer;

    {
        std::array<int, 3> volDim{500, 500, 100};
        auto volDat = SciVis::VolumeLoader::RawLoader<uint8_t, float>::LoadFromFile(
            "CLOUDf01.bin", volDim, [](const uint8_t &src) -> float { return src / 255.f; });
        auto volDatShared = std::make_shared<decltype(volDat)>();
        (*volDatShared) = std::move(volDat);
        renderer.AddVolume("cloud01", volDatShared, volDim);
    }
    grp->addChild(renderer.GetGroup());

    if (auto opt = renderer.GetVolume("cloud01"); opt.has_value())
        opt.value()->second.MarchingCube(5.f / 255.f);

    viewer->setSceneData(grp);
    viewer->run();

    return 0;
}
#elif defined(TEST_HEAT_MAP_2D)
int main(int argc, char **argv) {
    osg::ref_ptr viewer = new osgViewer::Viewer;
    viewer->setUpViewInWindow(200, 50, 800, 600);

    osg::ref_ptr grp = new osg::Group;

    SciVis::ScalarViser::HeatMap2DRenderer renderer;

    {
        auto volTex = SciVis::VolumeLoader::RawConvertor<uint8_t>::LoadFromFileToTexture(
            "CLOUDf01.bin", {500, 500, 100}, {8, 8, 6},
            [](const uint8_t &src) -> float { return src / 255.f; });
        auto colTblTex =
            SciVis::VolumeLoader::TFLoader<uint8_t>::LoadFromFileToTexture("cloud_color_tbl.txt");
        renderer.AddVolume("cloud01", volTex, colTblTex);
    }
    grp->addChild(renderer.GetGroup());

    viewer->setSceneData(grp);
    viewer->run();

    return 0;
}
#elif defined(TEST_HEAT_MAP_3D)
int main(int argc, char **argv) {
    osg::ref_ptr viewer = new osgViewer::Viewer;
    viewer->setUpViewInWindow(200, 50, 800, 600);

    osg::ref_ptr grp = new osg::Group;
    grp->addChild(createEarth());

    SciVis::ScalarViser::HeatMap3DRenderer renderer;

    {
        auto volTex = SciVis::VolumeLoader::RawConvertor<uint8_t>::LoadFromFileToTexture(
            "CLOUDf01.bin", {500, 500, 100}, {8, 8, 6},
            [](const uint8_t &src) -> float { return src / 255.f; });
        auto colTblTex =
            SciVis::VolumeLoader::TFLoader<uint8_t>::LoadFromFileToTexture("cloud_color_tbl.txt");
        renderer.AddVolume("cloud01", volTex, colTblTex);
    }
    grp->addChild(renderer.GetGroup());

    viewer->setSceneData(grp);
    viewer->run();

    return 0;
}
#elif defined(TEST_GRAPH_LAYOUT_2D)
int main(int argc, char **argv) {
    osg::ref_ptr viewer = new osgViewer::Viewer;
    viewer->setUpViewInWindow(200, 50, 800, 600);

    osg::ref_ptr grp = new osg::Group;

    SciVis::GraphViser::GraphLayout2D graphLayout;
    grp->addChild(graphLayout.GetGroup());

    viewer->setSceneData(grp);
    viewer->run();

    return 0;
}
#elif defined(TEST_GRAPH_LAYOUT_3D)
int main(int argc, char **argv) {
    osg::ref_ptr viewer = new osgViewer::Viewer;
    viewer->setUpViewInWindow(200, 50, 800, 600);

    osg::ref_ptr grp = new osg::Group;

    SciVis::GraphViser::GraphLayout3D graphLayout;
    grp->addChild(graphLayout.GetGroup());

    viewer->setSceneData(grp);
    viewer->run();

    return 0;
}
#endif // TEST_DVR
