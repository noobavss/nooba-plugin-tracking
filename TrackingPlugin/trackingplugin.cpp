#include "trackingplugin.h"
#include <QtCore>

// opencv includes
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>

#include <QDebug>
#include <QImage>

TrackingPlugin::TrackingPlugin()
{
    connect(&blobTrackingNode, SIGNAL(generateEvent(QList<DetectedEvent>,QList<QImage>)), this, SLOT(onCaptureEvent(QList<DetectedEvent>,QList<QImage>)));

    //connect(&blobTrackingNode, SIGNAL(generateEvent(QList<DetectedEvent>)), this, SLOT(onCaptureEvent(QList<DetectedEvent>)));
    connect(&blobTrackingNode, SIGNAL(generateEvent(QList<DetectedEvent>)), &blobEventWriterNode, SLOT(captureEvent(QList<DetectedEvent>)));

}

TrackingPlugin::~TrackingPlugin()
{
    blobEventWriterNode.closeFile();
}

bool TrackingPlugin::procFrame( const cv::Mat &in, cv::Mat &out, ProcParams &params )
{
    Q_UNUSED(params)
    img_mask = in.clone();

        staticBGS.process(in, img_mask);

    cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT,cv::Size(2,2));
    cv::dilate(img_mask,img_mask, element,cv::Point(-1,-1),1);
    cv::erode(img_mask,img_mask, element,cv::Point(-1,-1),2);

    //cv::imshow("BGS",img_mask);
    blobTrackingNode.process(in, img_mask, img_blob);

    in.copyTo(out);
    //cv::cvtColor(img_blob, out, CV_BGR2GRAY);

    return true;
}

bool TrackingPlugin::init()
{

    QDateTime timestamp = QDateTime::currentDateTime();
    QStringList enable_disable_list;
    QStringList bgsList;

    enable_disable_list.append("Enable");
    enable_disable_list.append("Disable");

    bgsList.append("StaticFrameDifference");
    bgsList.append("MixtureOfGaussianV2");

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
    createMultiValParam("enable_file_output",enable_disable_list);
    createMultiValParam("show_blob_mask",enable_disable_list);
    createMultiValParam("show_blob_output",enable_disable_list);

    createMultiValParam("Background Subtractor",bgsList);

    blobEventWriterNode.openFile(output_file);
    blobTrackingNode.setInactiveTimeThreshold(10);
    blobTrackingNode.setDistanceThreshold(100.0);
    blobTrackingNode.setParent(this);


    background_subtractor = "StaticFrameDifference";
    blobTrackingNode.saveConfig();

    createFrameViewer("Tracking Output");
    createFrameViewer("Tracking Mask");

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
        blobTrackingNode.saveConfig();

        debugMsg("output_file set to "  + val);
    }
}

void TrackingPlugin::onIntParamChanged(const QString &varName, int val){
    if(varName == "inactive_time"){

        blobTrackingNode.setInactiveTimeThreshold(val);
        blobTrackingNode.saveConfig();

        debugMsg("inactive_time set to "  + QString("%1").arg(val));
    }
    else if(varName == "min_area"){

        blobTrackingNode.setMinArea(val);
        blobTrackingNode.saveConfig();
        debugMsg("min_area set to "  + QString("%1").arg(val));
    }
    else if(varName == "max_area"){

        blobTrackingNode.setMaxArea(val);
        blobTrackingNode.saveConfig();
        debugMsg("max_area set to "  + QString("%1").arg(val));
    }

}

void TrackingPlugin::onDoubleParamChanged(const QString &varName, double val){

    if(varName == "distance_threshold"){
            blobTrackingNode.setDistanceThreshold(val);
            blobTrackingNode.saveConfig();

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
            setFrameViewerVisibility("Tracking Mask",false);
        }
        else{
            blobTrackingNode.toggleBlobMaskOutput(false);
            setFrameViewerVisibility("Tracking Mask",false);
        }
        blobTrackingNode.saveConfig();

        debugMsg("show_blob_mask set to " + val);
    }
    else if(varName == "show_blob_output"){
        if(val == "Enable"){
            blobTrackingNode.toggleShowOutput(true);
        }
        else{
            blobTrackingNode.toggleShowOutput(false);
            cv::destroyWindow("Blob Tracking");
        }
        blobTrackingNode.saveConfig();

        debugMsg("show_blob_mask set to " + val);
    }
    else if(varName == "Background Subtractor"){
        if(val == "StaticFrameDifference"){
            background_subtractor = "StaticFrameDifference";
        }
        else if(val == "MixtureOfGaussianV2"){
            background_subtractor = "MixtureOfGaussianV2";
        }
        else{
            background_subtractor = "StaticFrameDifference";
        }
        blobTrackingNode.saveConfig();

        debugMsg("Background Subtractor set to " + val);
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

//void TrackingPlugin::onCaptureEvent(QList<DetectedEvent> captured_event,QImage image){

//    Q_UNUSED(image);
//    PluginPassData eventData;
//    foreach(DetectedEvent e, captured_event){
//        //debugMsg(QString(e.getIdentifier() + " " + e.getMessage() + " %1").arg(e.getConfidence()));
//        eventData.appendStrList(QString(e.getIdentifier() + " " + e.getMessage() + " %1").arg(e.getConfidence()));
//    }
//    //out_stream << frameIndex << "," << (*track).first << ","<< blob->centroid.x << "," << blob->centroid.y << "|";

//    eventData.setImage(image);
//    outputData(eventData);

////    QList<QImage> images;
////    QStringList strings;

////    images.append(convertToQImage(image));
////    images.append(convertToQImage(output));
////    emit outputData(strings,images);

//}
void TrackingPlugin::onCaptureEvent(QList<DetectedEvent> captured_event,QList<QImage> images){

        QStringList strings;

    foreach(DetectedEvent e, captured_event){
        //debugMsg(QString(e.getIdentifier() + " " + e.getMessage() + " %1").arg(e.getConfidence()));
        strings.append(QString(e.getIdentifier() + " " + e.getMessage() + " %1").arg(e.getConfidence()));
    }
    //out_stream << frameIndex << "," << (*track).first << ","<< blob->centroid.x << "," << blob->centroid.y << "|";

    emit outputData(strings,images);

}

void TrackingPlugin::inputData(const QStringList &strList, QList<QImage> imageList){
    Q_UNUSED(strList)

    cv::Mat frame(imageList.at(0).height(),imageList.at(0).width(),CV_8UC3,(uchar*)imageList.at(0).bits(),imageList.at(0).bytesPerLine());
    cv::Mat bgmask(imageList.at(1).height(),imageList.at(1).width(),CV_8UC1,(uchar*)imageList.at(1).bits(),imageList.at(1).bytesPerLine());
    //cv::imshow("test",bgmask);
    //cv::imshow("test2",frame);

    blobTrackingNode.process(frame, bgmask, img_blob);

}


// see qt4 documentation for details on the macro (Qt Assistant app)
// Mandatory  macro for plugins in qt4. Made obsolete in qt5
#if QT_VERSION < 0x050000
    Q_EXPORT_PLUGIN2(TrackingPlugin, TrackingPlugin);
#endif
