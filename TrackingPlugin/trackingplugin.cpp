#include "trackingplugin.h"
#include <QtCore>
#include <opencv2/core/core.hpp>

// opencv includes
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>

#include <QDebug>

TrackingPlugin::TrackingPlugin()
{

    connect(&blobTrackingNode, SIGNAL(generateEvent(QList<DetectedEvent>)), this, SLOT(onCaptureEvent(QList<DetectedEvent>)));
    connect(&blobTrackingNode, SIGNAL(generateEvent(QList<DetectedEvent>)), &blobEventWriterNode, SLOT(captureEvent(QList<DetectedEvent>)));

}

TrackingPlugin::~TrackingPlugin()
{
    blobEventWriterNode.closeFile();
}

bool TrackingPlugin::procFrame( const cv::Mat &in, cv::Mat &out, ProcParams &params )
{

    img_mask = in.clone();

    // bgs->process(...) method internally shows the foreground mask image
    bgs.process(in, img_mask);

    cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT,cv::Size(2,2));
    cv::dilate(img_mask,img_mask, element,cv::Point(-1,-1),1);
    cv::erode(img_mask,img_mask, element,cv::Point(-1,-1),2);

    //cv::imshow("BGS",img_mask);
    blobTrackingNode.process(in, img_mask, img_blob);

    cv::cvtColor(img_blob, out, CV_BGR2GRAY);

    //cv::imshow("mask", img_mask);
    //cv::imshow("blob", img_blob);
    // Perform blob counting
    blobCounting.setInput(img_blob);
    blobCounting.setTracks(blobTrackingNode.getTracks());
    blobCounting.process();

    return true;
}

bool TrackingPlugin::init()
{
    QDateTime timestamp = QDateTime::currentDateTime();
    QStringList enable_disable_list;

    enable_disable_list.append("Enable");
    enable_disable_list.append("Disable");

    QDir dir(QDir::home());
    if(!dir.exists("NoobaVSS")){
        dir.mkdir("NoobaVSS");
    }
    dir.cd("NoobaVSS");
    if(!dir.exists("data")){
        dir.mkdir("data");
    }
    dir.cd("data");
    if(!dir.exists("text")){
        dir.mkdir("text");
    }
    dir.cd("text");



    output_file = dir.absoluteFilePath(timestamp.currentDateTime().toString("yyyy-MM-dd-hhmm") + "-blobs.txt").toLocal8Bit();
    createStringParam("output_file",output_file,false);
    createIntParam("inactive_time",10,1000,0);
    createDoubleParam("distance_threshold",100.0,1000.0,1.0);
    createIntParam("min_area",100,100000,0);
    createIntParam("max_area",20000,100000,0);

    //createMultiValParam("enable_file_output",enable_disable_list);
    createMultiValParam("show_blob_mask",enable_disable_list);

    blobEventWriterNode.openFile(output_file);
    blobTrackingNode.setInactiveTimeThreshold(10);
    blobTrackingNode.setDistanceThreshold(100.0);
    debugMsg("TrackingPlugin initialized");
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
        "Blob tracking and counting using cvBlob and BGS library",
        "Chathuranga Hettiarachchi");
    return pluginInfo;
}

void TrackingPlugin:: onStringParamChanged(const QString& varName, const QString& val){
    if(varName == "output_file"){
        output_file = val;
        blobEventWriterNode.openFile(output_file);
        debugMsg("output_file set to "  + val);
    }
}

void TrackingPlugin::onIntParamChanged(const QString &varName, int val){
    if(varName == "inactive_time"){

        blobTrackingNode.setInactiveTimeThreshold(val);
        debugMsg("inactive_time set to "  + QString("%1").arg(val));
    }
    else if(varName == "min_area"){

        blobTrackingNode.setMinArea(val);
        debugMsg("min_area set to "  + QString("%1").arg(val));
    }
    else if(varName == "max_area"){

        blobTrackingNode.setMaxArea(val);
        debugMsg("max_area set to "  + QString("%1").arg(val));
    }

}

void TrackingPlugin::onDoubleParamChanged(const QString &varName, double val){

    if(varName == "distance_threshold"){
            blobTrackingNode.setDistanceThreshold(val);
            debugMsg("distance_threshold set to "  + QString("%1").arg(val));
    }
}

void TrackingPlugin::onMultiValParamChanged(const QString &varName, const QString &val){
    if(varName == "enable_file_output"){

            debugMsg("enable_file_output set to " + val);
    }
    else if(varName == "show_blob_mask"){
        if(val == "Enable"){
        blobTrackingNode.toggleBlobMaskOutput(true);
        }
        else{
            blobTrackingNode.toggleBlobMaskOutput(false);
        }
        debugMsg("show_blob_mask set to " + val);
    }
}

void TrackingPlugin::onCaptureEvent(QList<DetectedEvent> captured_event){

    PluginPassData eventData;
    foreach(DetectedEvent e, captured_event){
        //debugMsg(QString(e.getIdentifier() + " " + e.getMessage() + " %1").arg(e.getConfidence()));
        eventData.appendStrList(QString(e.getIdentifier() + " " + e.getMessage() + " %1").arg(e.getConfidence()));
    }
    //out_stream << frameIndex << "," << (*track).first << ","<< blob->centroid.x << "," << blob->centroid.y << "|";

    outputData(eventData);
    //outputDataRequest(eventData);

}


// see qt4 documentation for details on the macro (Qt Assistant app)
// Mandatory  macro for plugins in qt4. Made obsolete in qt5
#if QT_VERSION < 0x050000
    Q_EXPORT_PLUGIN2(TrackingPlugin, TrackingPlugin);
#endif
