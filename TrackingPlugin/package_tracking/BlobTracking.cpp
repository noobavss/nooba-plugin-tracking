#include "BlobTracking.h"

BlobTracking::BlobTracking() : firstTime(true),
		minArea(500), maxArea(20000), debugTrack(false), 
		debugBlob(false), showBlobMask(false), showOutput(true),
        frameIndex(0)
{
  std::cout << "BlobTracking()" << std::endl;
  //if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
  //		return;
}

BlobTracking::~BlobTracking()
{
	file.close();
  std::cout << "~BlobTracking()" << std::endl;
}

void BlobTracking::setOutputFile(QString output_location)
{
    file.close();
    file.setFileName(output_location);

  std::cout << "Opening Output File" << std::endl;
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;
}

const cvb::CvTracks BlobTracking::getTracks()
{
  return tracks;
}


void BlobTracking::process(const cv::Mat &img_input, const cv::Mat &img_mask, cv::Mat &img_output)
{
  if(img_input.empty() || img_mask.empty())
    return;

  loadConfig();

  if(firstTime){

    saveConfig();
	
  }
  IplImage* frame = new IplImage(img_input);
  cvConvertScale(frame, frame, 1, 0);

  IplImage* segmentated = new IplImage(img_mask);
  
  IplConvKernel* morphKernel = cvCreateStructuringElementEx(5, 5, 1, 1, CV_SHAPE_RECT, NULL);
  cvMorphologyEx(segmentated, segmentated, NULL, morphKernel, CV_MOP_OPEN, 1);

  //if(showBlobMask)
  //  cvShowImage("Blob Mask", segmentated);

  IplImage* labelImg = cvCreateImage(cvGetSize(frame), IPL_DEPTH_LABEL, 1);

  cvb::CvBlobs blobs;
  unsigned int result = cvb::cvLabel(segmentated, labelImg, blobs);
  
  //cvb::cvFilterByArea(blobs, 500, 1000000);
  cvb::cvFilterByArea(blobs, minArea, maxArea);
  
  QTextStream out_stream(&file);

  for(std::map<cvb::CvLabel,cvb::CvBlob *>::iterator it = blobs.begin() ; it != blobs.end(); it++){
  
	cvb::CvLabel lablel = (*it).first;
    cvb::CvBlob * blob = (*it).second;
	//out_stream << blob;
	out_stream << frameIndex << "," << lablel  << ","<< blob->centroid.x << "," << blob->centroid.y << "|";

  }
	out_stream << "\n";

  

  //cvb::cvRenderBlobs(labelImg, blobs, frame, frame, CV_BLOB_RENDER_BOUNDING_BOX);
  if(debugBlob)
    cvb::cvRenderBlobs(labelImg, blobs, frame, frame, CV_BLOB_RENDER_BOUNDING_BOX|CV_BLOB_RENDER_CENTROID|CV_BLOB_RENDER_ANGLE|CV_BLOB_RENDER_TO_STD);
  else
    cvb::cvRenderBlobs(labelImg, blobs, frame, frame, CV_BLOB_RENDER_BOUNDING_BOX|CV_BLOB_RENDER_CENTROID|CV_BLOB_RENDER_ANGLE);

  cvb::cvUpdateTracks(blobs, tracks, 200., 5);
  
  if(debugTrack)
    cvb::cvRenderTracks(tracks, frame, frame, CV_TRACK_RENDER_ID|CV_TRACK_RENDER_BOUNDING_BOX|CV_TRACK_RENDER_TO_STD);
  else
    cvb::cvRenderTracks(tracks, frame, frame, CV_TRACK_RENDER_ID|CV_TRACK_RENDER_BOUNDING_BOX);
  
  //std::map<CvID, CvTrack *> CvTracks

  //if(showOutput)
   // cvShowImage("Blob Tracking", frame);

  cv::Mat img_result(frame);
  img_result.copyTo(img_output);

  //cvReleaseImage(&frame);
  //cvReleaseImage(&segmentated);
  cvReleaseImage(&labelImg);
  delete frame;
  delete segmentated;
  cvReleaseBlobs(blobs);
  cvReleaseStructuringElement(&morphKernel);

  firstTime = false;
    frameIndex++;
}

void BlobTracking::saveConfig()
{
  CvFileStorage* fs = cvOpenFileStorage("configuration//BlobTrackingTest.xml", 0, CV_STORAGE_WRITE);

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

void BlobTracking::loadConfig()
{
  CvFileStorage* fs = cvOpenFileStorage("configuration//BlobTrackingTest.xml", 0, CV_STORAGE_READ);
  
  if(fs = NULL){
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
  showBlobMask	= cvReadIntByName(fs, 0, "showBlobMask", false);
  showOutput	= cvReadIntByName(fs, 0, "showOutput", true);

  cvReleaseFileStorage(&fs);
}
