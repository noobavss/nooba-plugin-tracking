#pragma once

#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>


#include "cvblob/cvblob.h"

#include <QFile>
#include <QTextStream>

class BlobTracking
{
private:
  bool firstTime;
  int minArea;
  int maxArea;
  
  QFile file;
  int frameIndex;

  bool debugTrack;
  bool debugBlob;
  bool showBlobMask;
  bool showOutput;

  cvb::CvTracks tracks;
  void saveConfig();
  void loadConfig();

public:
  BlobTracking();
  ~BlobTracking();

  void setOutputFile(QString output_location);
  void process(const cv::Mat &img_input, const cv::Mat &img_mask, cv::Mat &img_output);
  const cvb::CvTracks getTracks();
};

