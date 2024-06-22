#pragma once

#include <chrono>
#include <opencv2/opencv.hpp>

class Frame {
  cv::Mat image;
  std::chrono::nanoseconds timestamp;

 public:
  Frame(const cv::Mat& frame, const std::chrono::nanoseconds& time)
      : image(frame), timestamp(time) {}

  std::chrono::nanoseconds getTimestamp() const { return timestamp; }

  const cv::Mat& getImage() const { return image; }

  cv::Mat& getImage() { return image; }
};
