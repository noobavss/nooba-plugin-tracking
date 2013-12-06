#ifndef TRACKINGPLUGIN_H
#define TRACKINGPLUGIN_H

#include "trackingplugin_global.h"
#include "noobapluginapi.h"

#include <QObject>
#include <QImage>

#include <detectedevent.h>
#include <featurenode.h>

#include "filewriternode.h"
#include "package_tracking/BlobTrackingNode.h"


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

public slots:
    void onStringParamChanged(const QString& varName, const QString& val);
    void onIntParamChanged(const QString &varName, int val);
    void onDoubleParamChanged(const QString &varName, double val);
    void onMultiValParamChanged(const QString &varName, const QString &val);
    void onCaptureEvent(QList<DetectedEvent> captured_event);
    //void onCaptureEvent(QList<DetectedEvent> captured_event,QImage image);
    void onCaptureEvent(QList<DetectedEvent> captured_event,QList<QImage> images);

    void inputData(const QStringList &strList, QList<QImage> imageList);

private:
    QString output_file;
    cv::Mat img_blob;
    cv::Mat img_mask;
    BlobTrackingNode blobTrackingNode;
    FileWriterNode blobEventWriterNode;

    bool showOutputBlobMessages;


};

#endif // TRACKINGPLUGIN_H
