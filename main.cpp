#include <osg/PositionAttitudeTransform>
#include <osg/ShapeDrawable>
#include <osg/Texture2D>

#include <osgDB/FileUtils>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>

#include <scalar_viser/direct_volume_renderer.h>
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

int main(int argc, char **argv) {
    osg::ref_ptr viewer = new osgViewer::Viewer;
    viewer->setUpViewInWindow(200, 50, 800, 600);

    osg::ref_ptr grp = new osg::Group;
    grp->addChild(createEarth());

    osg::ref_ptr hints = new osg::TessellationHints;
    osg::ref_ptr shape =
        new osg::ShapeDrawable(new osg::Box(osg::Vec3(0.0f, 0.0f, 0.0f), 2, 2, 2), hints);

    // Create direct volume renderer
    SciVis::ScalarViser::DirectVolumeRenderer renderer;

    // Prepare volume data
    {
        auto volTex = SciVis::VolumeLoader::RawConvertor<uint8_t>::LoadFromFileToTexture(
            "CLOUDf01.bin", {500, 500, 100}, {8, 8, 6},
            [](const uint8_t &src) -> float { return src / 255.f; });
        auto tfTex = SciVis::VolumeLoader::TFLoader<uint8_t>::LoadFromFileToTexture("cloud_tf.txt");
        renderer.AddVolume("cloud05", volTex, tfTex);
    }
    grp->addChild(renderer.GetGroup());

    viewer->setSceneData(grp.get());
    viewer->run();

    return 0;
}