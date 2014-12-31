#include "BlobTrackingNode.h"

BlobTrackingNode::BlobTrackingNode(FeatureNode *parent) :
    FeatureNode(parent),
    firstTime(true),
    minArea(500),
    maxArea(20000),
    debugTrack(false),
    debugBlob(false),
    showBlobMask(true),
    showOutput(true),
    showFrameID(true),
    frameIndex(0),
    threshold_distance(100.0),
    threshold_inactive(10)
{

}

BlobTrackingNode::~BlobTrackingNode()
{

}

const cvb::CvTracks BlobTrackingNode::getTracks()
{
    return tracks;
}


void BlobTrackingNode::process(const cv::Mat &img_input, const cv::Mat &img_mask, cv::Mat &img_output)
{
    //This is output event
    QList<DetectedEvent> blobEvent;
cv::Mat newInput;
img_input.copyTo(newInput);

    if(img_input.empty() || img_mask.empty()){
        return;
    }
    loadConfig();

    if(firstTime){
        saveConfig();
    }
    IplImage* frame = new IplImage(img_input);
    cvConvertScale(frame, frame, 1, 0);

    IplImage* segmentated = new IplImage(img_mask);

    IplConvKernel* morphKernel = cvCreateStructuringElementEx(5, 5, 1, 1, CV_SHAPE_RECT, NULL);
    cvMorphologyEx(segmentated, segmentated, NULL, morphKernel, CV_MOP_OPEN, 1);

    cv::Mat temp = cv::Mat(segmentated);
    if(showBlobMask){
        //cv::imshow("test",temp);
        ownerPlugin->updateFrameViewer("Tracking Mask",convertToQImage(temp));
    }
    IplImage* labelImg = cvCreateImage(cvGetSize(frame), IPL_DEPTH_LABEL, 1);

    cvb::CvBlobs blobs;
    cvb::cvLabel(segmentated, labelImg, blobs);

    cvb::cvFilterByArea(blobs, minArea, maxArea);
  
    if(debugBlob){
        cvb::cvRenderBlobs(labelImg, blobs, frame, frame, CV_BLOB_RENDER_BOUNDING_BOX|CV_BLOB_RENDER_CENTROID|CV_BLOB_RENDER_ANGLE|CV_BLOB_RENDER_TO_STD);
    }
    else{
        cvb::cvRenderBlobs(labelImg, blobs, frame, frame, CV_BLOB_RENDER_BOUNDING_BOX|CV_BLOB_RENDER_CENTROID|CV_BLOB_RENDER_ANGLE);
    }

    cvb::cvUpdateTracks(blobs, tracks,threshold_distance, threshold_inactive);



    //At this point, we have assigned each blob in current frame in to a track. so we can iterate through active tracks, and
    // find out out blobs within one of those tracks. Following loop does that. This is helpfull to have more generalized idea about
    // relationship between blobs with increasing time.

    for (cvb::CvTracks::const_iterator track = tracks.begin(); track!=tracks.end(); ++track)
    {
        if((*track).second->active != 0){
            for(std::map<cvb::CvLabel,cvb::CvBlob *>::iterator it = blobs.begin() ; it != blobs.end(); it++){

                cvb::CvLabel label = (*it).first;
                cvb::CvBlob * blob = (*it).second;

                if((*track).second->label == label){
                    //qDebug()<< blob->minx <<","<<blob->miny;
                    //This is smoothed time tracked blob lables
                    blobEvent.append(DetectedEvent("blob",QString("%1,%2,%3,%4,%5,%6,%7,%8").arg(frameIndex).arg((*track).first).arg(blob->centroid.x).arg(blob->centroid.y).arg(blob->minx).arg(blob->miny).arg(blob->maxx).arg(blob->maxy),1.0));
                }
            }
        }
    }

    if(debugTrack){
        cvb::cvRenderTracks(tracks, frame, frame, CV_TRACK_RENDER_ID|CV_TRACK_RENDER_BOUNDING_BOX|CV_TRACK_RENDER_TO_STD);
    }
    else{
        cvb::cvRenderTracks(tracks, frame, frame, CV_TRACK_RENDER_ID|CV_TRACK_RENDER_BOUNDING_BOX);
    }
    if(showFrameID){
        cv::Mat temp = cv::Mat(frame);
        cv::putText(temp,QString("%1").arg(frameIndex).toStdString(),cv::Point(40,120),cv::FONT_HERSHEY_PLAIN,2,cv::Scalar(0,255,0),2);
    }
    if(showOutput){
        cv::Mat temp = cv::Mat(frame);
        ownerPlugin->updateFrameViewer("Tracking Output",convertToQImage(temp));

        //cvShowImage("Blob Tracking", frame);
    }
    cv::Mat img_result(frame);

    cv::putText(img_result,QString("%1").arg(QString::number(frameIndex)).toUtf8().constData(), cv::Point(10,30), CV_FONT_HERSHEY_PLAIN,1.0, CV_RGB(255,255,255));

    img_result.copyTo(img_output);
    cvReleaseImage(&labelImg);
    delete frame;
    delete segmentated;
    cvReleaseBlobs(blobs);
    cvReleaseStructuringElement(&morphKernel);

    firstTime = false;
    frameIndex++;

    QImage input((uchar*)newInput.data, newInput.cols, newInput.rows, newInput.step1(), QImage::Format_RGB888);
    QImage output((uchar*)img_output.data, img_output.cols, img_output.rows, img_output.step1(), QImage::Format_RGB888);
    QList<QImage> images;

    images.append(input);
    images.append(output);
    emit generateEvent(blobEvent,images);

    //emit generateEvent(blobEvent);
}

void BlobTrackingNode::saveConfig()
{

    QDir dir(QDir::home());
    if(!dir.exists("NoobaVSS")){
        dir.mkdir("NoobaVSS");
    }
    dir.cd("NoobaVSS");
    if(!dir.exists("config")){
        dir.mkdir("config");
    }
    dir.cd("config");


    CvFileStorage* fs = cvOpenFileStorage(dir.absoluteFilePath("blobtrackingconfig.xml").toLocal8Bit(), 0, CV_STORAGE_WRITE);

    if(fs == NULL){
      return;
    }
    cvWriteInt(fs, "minArea", minArea);
    cvWriteInt(fs, "maxArea", maxArea);

    cvWriteInt(fs, "debugTrack", debugTrack);
    cvWriteInt(fs, "debugBlob", debugBlob);
    cvWriteInt(fs, "showBlobMask", showBlobMask);
    cvWriteInt(fs, "showOutput", showOutput);

    cvReleaseFileStorage(&fs);
}

void BlobTrackingNode::loadConfig()
{
    QDir dir(QDir::home());
    if(!dir.exists("NoobaVSS")){
        dir.mkdir("NoobaVSS");
    }
    dir.cd("NoobaVSS");
    if(!dir.exists("config")){
        dir.mkdir("config");
    }
    dir.cd("config");

    CvFileStorage* fs = cvOpenFileStorage(dir.absoluteFilePath("blobtrackingconfig.xml").toLocal8Bit(), 0, CV_STORAGE_READ);
  
    if(fs == NULL){
        std::cout << "Error opening file";
        minArea		= 100;
        maxArea		= 20000;
        debugTrack	= false;
        debugBlob	= false;
        showBlobMask= false;
        showOutput	= false;
        return;
    }
    minArea = cvReadIntByName(fs, 0, "minArea", 100);
    maxArea = cvReadIntByName(fs, 0, "maxArea", 20000);

    debugTrack	= cvReadIntByName(fs, 0, "debugTrack", false);
    debugBlob		= cvReadIntByName(fs, 0, "debugBlob", false);
    showBlobMask	= cvReadIntByName(fs, 0, "showBlobMask", true);
    showOutput	= cvReadIntByName(fs, 0, "showOutput", true);

    cvReleaseFileStorage(&fs);
}


QImage BlobTrackingNode::convertToQImage(cv::Mat &cvImg)
{
    if (cvImg.channels()== 1){
        QImage img((uchar*)cvImg.data, cvImg.cols, cvImg.rows, cvImg.step1(), QImage::Format_Indexed8);
        return img;
    }
    else{
        QImage img((uchar*)cvImg.data, cvImg.cols, cvImg.rows, cvImg.step1(), QImage::Format_RGB888);
        return img;
    }
}
bool BlobTrackingNode::getShowFrameID() const
{
    return showFrameID;
}

void BlobTrackingNode::setShowFrameID(bool value)
{
    showFrameID = value;
}



void BlobTrackingNode::processEvents(const QList<DetectedEvent> event){
    Q_UNUSED(event)
    //This is not suppose to recieve any event. Input is an image
}
