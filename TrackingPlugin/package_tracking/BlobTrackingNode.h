#ifndef BLOBTRACKINGNODE_H
#define BLOBTRACKINGNODE_H

#include <iostream>
#include "featurenode.h"
#include "detectedevent.h"
#include "noobapluginapi.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "cvblob/cvblob.h"

#include <QFile>
#include <QTextStream>
#include <QCoreApplication>
#include <QDir>
#include <QDebug>
#include <QImage>

class BlobTrackingNode : public FeatureNode
{

public:
    explicit BlobTrackingNode(FeatureNode* parent = 0);
    ~BlobTrackingNode();

    void process(const cv::Mat &img_input, const cv::Mat &img_mask, cv::Mat &img_output);

    const cvb::CvTracks getTracks();

    //Not used. only signal will be used.
    void processEvents(const QList<DetectedEvent> event);

    //Handling Parameters
    //Setters
    void setMinArea(int area){ minArea = area;}
    void setMaxArea(int area){ maxArea = area;}
    void toggleBlobMaskOutput(bool show){ showBlobMask = show;}
    void toggleShowOutput(bool show){ showOutput = show;}
    void setDistanceThreshold(float threshold){ threshold_distance = threshold;}
    void setInactiveTimeThreshold(int threshold){ threshold_inactive = threshold;}

    //Getters
    int getMinArea(){return minArea;}
    int getMaxArea(){return maxArea;}
    int getInactiveTimeThreshold(){return threshold_inactive;}
    bool getBlobMaskOutputStatus(){return showBlobMask;}
    bool getShowOutputStatus(){return showOutput;}
    float getDistanceThreshold(){return threshold_distance;}

    void setParent(NoobaPluginAPI *ownerPlugin){this->ownerPlugin = ownerPlugin;}
    void saveConfig();
    void loadConfig();

    QImage convertToQImage(cv::Mat &cvImg);


    bool getShowFrameID() const;
    void setShowFrameID(bool value);

private:
    bool firstTime;
    int minArea;
    int maxArea;


    bool debugTrack;
    bool debugBlob;
    bool showBlobMask;
    bool showOutput;
    bool showFrameID;
    int frameIndex;

    float threshold_distance;
    int threshold_inactive;


    cvb::CvTracks tracks;
    NoobaPluginAPI *ownerPlugin;
};

#endif // BLOBTRACKINGNODE_H
