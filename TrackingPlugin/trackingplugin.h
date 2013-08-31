#ifndef TRACKINGPLUGIN_H
#define TRACKINGPLUGIN_H

#include "trackingplugin_global.h"
#include "noobapluginapi.h"

#include <QObject>

class TRACKINGPLUGIN_EXPORT TrackingPlugin: public NoobaPluginAPI
{
    Q_OBJECT
    Q_INTERFACES(NoobaPluginAPI)
#if QT_VERSION >= 0x050000
    Q_PLUGIN_METADATA(IID "nooba.plugins.qt5.tracking-plugin" FILE "trackingPlugin.json")
#endif

public:
    TrackingPlugin();
    ~TrackingPlugin();

    bool procFrame(const cv::Mat &in, cv::Mat &out, ProcParams &params);
    bool init();
    bool release();
    PluginInfo getPluginInfo() const;

private:

};

#endif // TRACKINGPLUGIN_H
