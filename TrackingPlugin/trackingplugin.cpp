#include "trackingplugin.h"
#include <QtCore>
#include <opencv2/core/core.hpp>

TrackingPlugin::TrackingPlugin()
{

}

TrackingPlugin::~TrackingPlugin()
{

}

bool TrackingPlugin::procFrame( const cv::Mat &in, cv::Mat &out, ProcParams &params )
{

    return true;
}

bool TrackingPlugin::init()
{
    return true;
}

bool TrackingPlugin::release()
{
    return true;
}

PluginInfo TrackingPlugin::getPluginInfo() const
{
    PluginInfo pluginInfo(
        "Tracking Plugin",
        0,
        1,
        "Plugin Description goes here",
        "Plugin Creator");
    return pluginInfo;
}


// see qt4 documentation for details on the macro (Qt Assistant app)
// Mandatory  macro for plugins in qt4. Made obsolete in qt5
#if QT_VERSION < 0x050000
    Q_EXPORT_PLUGIN2(TrackingPlugin, TrackingPlugin);
#endif
