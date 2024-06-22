#pragma once
#include <opencv2/opencv.hpp>

inline cv::Point2d getMaskMean(const cv::Mat& mask) {
  std::vector<cv::Point2d> points;
  for (int y = 0; y < mask.rows; y++) {
    for (int x = 0; x < mask.cols; x++) {
      if (mask.at<uchar>(y, x) > 0) {
        points.push_back(cv::Point2d(x, y));
      }
    }
  }
  cv::Point2d mean(0, 0);
  for (const auto& p : points) mean += p;
  mean *= (1.0 / points.size());
  return mean;
}
