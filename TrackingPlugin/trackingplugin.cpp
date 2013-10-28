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
    output_file = "/home/chathuranga/Programming/FYP/data/text/2013-10-28-sample-blobs.txt";
    createStringParam("output_file",output_file,false);

    blobEventWriterNode.openFile(output_file);
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
        setProperty("output_location",val);
        output_file = val;
        blobEventWriterNode.openFile(output_file);

        debugMsg("output_file set to "  + val);
    }
}

void TrackingPlugin::onCaptureEvent(QList<DetectedEvent> captured_event){

    PluginPassData eventData;

    foreach(DetectedEvent e, captured_event){
        //debugMsg(QString(e.getIdentifier() + " " + e.getMessage() + " %1").arg(e.getConfidence()));
        eventData.appendStrList(QString(e.getIdentifier() + " " + e.getMessage() + " %1").arg(e.getConfidence()));
    }
    //out_stream << frameIndex << "," << (*track).first << ","<< blob->centroid.x << "," << blob->centroid.y << "|";

    //eventQueue.append(event);
    outputDataRequest(&eventData);

}

// see qt4 documentation for details on the macro (Qt Assistant app)
// Mandatory  macro for plugins in qt4. Made obsolete in qt5
#if QT_VERSION < 0x050000
    Q_EXPORT_PLUGIN2(TrackingPlugin, TrackingPlugin);
#endif
