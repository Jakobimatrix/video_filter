#pragma once

#include <Eigen/Dense>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <vector>
#include <video_filter/RoiSelect.hpp>
#include <video_filter/detail/mask_operations.hpp>
#include <video_filter/frame.hpp>

class Tracker {
 public:
  Tracker(const Frame& frame, cv::Rect2d roi)
      : lastRoi(roi), roi_selected(true) {
    initialize(frame);
  }
  Tracker(const Frame& frame) : roi_selected(false) {
    RoiSelect roiSelector(frame.getImage());
    roi_selected = roiSelector.selectRoi(&lastRoi);
    initialize(frame);
  }

  void initialize(const Frame& frame) {
    if (!roi_selected) {
      return;
    }
    updateReferenceFrame(frame.getImage());
    tracks.push_back({frame.getTimestamp(), getCurrentROICenter()});
  }

  bool track(const Frame& frame) {
    // Define ROI around the last known position
    double radius = std::max(lastRoi.width, lastRoi.height);
    cv::Rect roi = getROI(frame.getImage().size(), getCurrentROICenter(), radius);

    if (!trackLightSource(frame.getImage(), roi)) {
      if (!trackContrastContour(frame.getImage(), roi)) {
        if (!trackColorFingerprint(frame.getImage(), roi)) {
          if (!trackReferenceFrame(frame.getImage(), roi)) {
            if (!manualTracking(frame.getImage())) {
              return false;
            }
          }
        }
      }
    }
    updateReferenceFrame(frame.getImage());
    tracks.push_back({frame.getTimestamp(), getCurrentROICenter()});
    return true;
  }

  const std::vector<std::pair<std::chrono::nanoseconds, cv::Point2d>>& getTracks() const {
    return tracks;
  }

  const std::pair<std::chrono::nanoseconds, cv::Point2d>& getLastTrack() const {
    return tracks.back();
  }

 private:
  cv::Rect2d lastRoi;
  cv::Mat referenceFrame;
  bool roi_selected;
  std::vector<std::pair<std::chrono::nanoseconds, cv::Point2d>> tracks;

  cv::Point2d getCurrentROICenter() {
    return cv::Point2d{lastRoi.br() + lastRoi.tl()} * 0.5;
  }


  cv::Rect getROI(const cv::Size& frameSize, cv::Point2d center, double radius) {
    const cv::Point2d topLeft(std::max(0., center.x - radius),
                              std::max(0., center.y - radius));
    const double size = radius * 2.;
    const cv::Point2d bottomRight(
        std::min(topLeft.x + size, static_cast<double>(frameSize.width)),
        std::min(topLeft.y + size, static_cast<double>(frameSize.height)));
    return cv::Rect(
        topLeft.x, topLeft.y, bottomRight.x - topLeft.x, bottomRight.y - topLeft.y);
  }

  bool trackLightSource(const cv::Mat& frame, const cv::Rect& roi) {
    double minVal, maxVal;
    cv::Point minLoc, maxLoc;
    cv::Mat gray, mask;
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    cv::minMaxLoc(gray, &minVal, &maxVal, &minLoc, &maxLoc);
    const cv::Mat roiGray = gray(roi);
    const double ninetyPercent = maxVal - (maxVal - minVal) * 0.1;
    cv::threshold(roiGray, mask, ninetyPercent, 255, cv::THRESH_BINARY);
    const cv::Point2d mean = getMaskMean(mask) + cv::Point2d(roi.x, roi.y);
    const auto currentCenter = getCurrentROICenter();
    lastRoi.x += mean.x - currentCenter.x;
    lastRoi.y += mean.y - currentCenter.y;

    /*
    cv::Mat debug = roiGray.clone();
    cv::circle(debug, getMaskMean(mask), 10, cv::Scalar(255, 255, 0), 2);
    cv::imshow("debug", debug);
    cv::waitKey(0);
    debug = mask.clone();
    cv::circle(debug, getMaskMean(mask), 10, cv::Scalar(125), 2);
    cv::imshow("debug", debug);
    cv::waitKey(0);
    */

    return true;
  }

  bool trackContrastContour(const cv::Mat& frame, const cv::Rect& roi) {
    // Implement contrast/contour based tracking
    // Update reference data if successful
    return false;
  }

  bool trackColorFingerprint(const cv::Mat& frame, const cv::Rect& roi) {
    // Implement color fingerprint (spectrum) based tracking
    // Update reference data if successful
    return false;
  }

  bool trackReferenceFrame(const cv::Mat& frame, const cv::Rect& roi) {
    // Implement reference frame based tracking
    // Update reference data if successful
    return false;
  }

  bool manualTracking(const cv::Mat& frame) {
    RoiSelect roiSelector(frame);
    return roiSelector.selectRoi(&lastRoi);
  }

  void updateReferenceFrame(const cv::Mat& frame) {
    referenceFrame = frame(lastRoi);
  }
};
