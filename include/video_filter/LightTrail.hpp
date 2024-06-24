#pragma once

#include <cmath>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <queue>
#include <string>
#include <video_filter/CommandLineParser.hpp>
#include <video_filter/RoiSelect.hpp>
#include <video_filter/detail/ProgressBar.hpp>
#include <video_filter/detail/stringUtils.hpp>
#include <video_filter/frame.hpp>
#include <video_filter/tracker.hpp>

class LightTrail {
 public:
  LightTrail(const std::string& inputFile,
             const std::string& outputFile,
             int threshold,
             int haloPixelSize,
             bool useRegionGrowing)
      : inputFile(inputFile),
        outputFile(outputFile),
        threshold(threshold),
        haloPixelSize(haloPixelSize),
        useRegionGrowing(useRegionGrowing) {}

  void processVideo() {
    cv::VideoCapture cap(inputFile);
    if (!cap.isOpened()) {
      std::cerr << "Error opening video stream or file" << std::endl;
      return;
    }

    int codec = getCodec();
    if (codec == -1) {
      std::cerr << "Unsupported output video format" << std::endl;
      return;
    }

    int frameWidth = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
    int frameHeight = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));
    int fps = static_cast<int>(cap.get(cv::CAP_PROP_FPS));
    int totalFrames = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_COUNT));

    cv::VideoWriter writer(outputFile, codec, fps, cv::Size(frameWidth, frameHeight));
    if (!writer.isOpened()) {
      std::cerr << "Could not open the output video file for write" << std::endl;
      return;
    }

    cv::Mat lightTrail = cv::Mat::zeros(frameHeight, frameWidth, CV_8UC3);
    int gaussKernalSize = haloPixelSize % 2 == 1 ? haloPixelSize : haloPixelSize + 1;
    cv::Point2d prevLight(-1., -1.);
    bool prevLightSet = false;

    ProgressBar progress_bar(totalFrames);
    int frameCount = 0;
    double roiRadius = 0;
    std::unique_ptr<Tracker> tracker = nullptr;

    cv::namedWindow("LightTrail", cv::WINDOW_AUTOSIZE);
    cv::setMouseCallback("LightTrail", onMouse, this);

    while (cap.read(frame)) {
      Frame f{frame, std::chrono::nanoseconds(frameCount)};
      if (tracker == nullptr) {
        RoiSelect roiSelector(frame);
        cv::Rect2d roi;
        if (roiSelector.selectRoi(&roi)) {
          tracker = std::make_unique<Tracker>(f, roi);
          roiRadius = std::max(roi.width, roi.height);
        }
        frameCount++;
        continue;
      }

      if (!tracker->track(f)) {
        break;
      }

      const cv::Point2d lightPos = tracker->getLastTrack().second;
      const cv::Rect roi = getROI(frame.size(), lightPos, roiRadius);
      cv::Mat light = frame(roi).clone();

      cv::Point2f translation(0.0);
      if (prevLightSet) {
        translation = prevLight - lightPos;
      }
      prevLightSet = true;
      prevLight = lightPos;

      if (!stopTrail) {
        applyTranslationIncrementally(light, roi, translation, lightTrail);
      }

      frame = cv::max(frame, lightTrail);
      debugDisplay(frame, lightPos);

      writer.write(frame);

      frameCount++;
      ++progress_bar;
      progress_bar.display();
    }

    cap.release();
    writer.release();
    cv::destroyAllWindows();
  }

 private:
  std::string inputFile;
  std::string outputFile;
  int threshold;
  int haloPixelSize;
  bool useRegionGrowing;
  cv::Mat frame;
  bool stopTrail = false;

  static void onMouse(int event, int x, int y, int flags, void* userdata) {
    LightTrail* self = reinterpret_cast<LightTrail*>(userdata);
    self->handleMouse(event, x, y);
  }

  void handleMouse(int event, int x, int y) {
    if (event == cv::EVENT_LBUTTONDOWN) {
      stopTrail = true;
    }
  }

  int getCodec() {
    std::string fileExtension = getExtension(outputFile);
    if (fileExtension == "mp4" || fileExtension == "MP4") {
      return cv::VideoWriter::fourcc('m', 'p', '4', 'v');
    } else if (fileExtension == "avi") {
      return cv::VideoWriter::fourcc('M', 'J', 'P', 'G');
    } else if (fileExtension == "mov" || fileExtension == "MOV") {
      return cv::VideoWriter::fourcc('m', 'p', '4', 'v');
    } else {
      return -1;
    }
  }

  cv::Rect getROI(const cv::Size& frameSize, cv::Point2d center, double radius) {
    cv::Point2d topLeft(std::max(0., center.x - radius), std::max(0., center.y - radius));
    double size = radius * 2.;
    cv::Point2d bottomRight(
        std::min(topLeft.x + size, static_cast<double>(frameSize.width)),
        std::min(topLeft.y + size, static_cast<double>(frameSize.height)));
    return cv::Rect(
        topLeft.x, topLeft.y, bottomRight.x - topLeft.x, bottomRight.y - topLeft.y);
  }

  void debugDisplay(const cv::Mat& frame, const cv::Point2d& lightPos) {
    cv::Mat debug = frame.clone();
    cv::circle(debug, lightPos, 30, cv::Scalar(0, 255, 0), 2);
    cv::circle(debug, lightPos, 60, cv::Scalar(0, 255, 0), 2);
    cv::circle(debug, lightPos, 90, cv::Scalar(0, 255, 0), 2);
    cv::resize(debug, debug, debug.size() / 5);
    cv::imshow("LightTrail", debug);
    cv::waitKey(1);
  }

  void applyTranslationIncrementally(const cv::Mat& light,
                                     const cv::Rect& roi,
                                     const cv::Point2f& translation,
                                     cv::Mat& lightTrail) {
    int steps = std::ceil(
        std::sqrt(translation.x * translation.x + translation.y * translation.y));
    for (int i = 1; i <= steps; ++i) {
      cv::Mat translatedLight = cv::Mat::zeros(light.size(), light.type());
      float alpha = static_cast<double>(i) / steps;

      cv::warpAffine(
          light, translatedLight, getAffine(translation * alpha, 0), light.size());
      cv::Rect roiTransformed = roi & cv::Rect(0, 0, lightTrail.cols, lightTrail.rows);
      if (roiTransformed.area() <= 0) {
        std::cerr << "Invalid transformed ROI, skipping frame" << std::endl;
        continue;
      }
      lightTrail(roi) = cv::max(lightTrail(roi), translatedLight);
    }
    if (steps == 0) {
      lightTrail(roi) = cv::max(lightTrail(roi), light);
    }
  }

  cv::Mat getAffine(const cv::Point2f& translation, float angle) {
    return (cv::Mat_<float>(2, 3) << std::cos(angle),
            -std::sin(angle),
            translation.x,
            std::sin(angle),
            std::cos(angle),
            translation.y);
  }
};
