#pragma once

#include <functional>
#include <opencv2/opencv.hpp>
#include <vector>

class RoiSelect {
  const cv::Mat& frameRef;
  cv::Point2d mousePos;
  std::vector<cv::Point2d> corners;

  static void onMouse(int event, int x, int y, int flags, void* userdata) {
    RoiSelect* self = reinterpret_cast<RoiSelect*>(userdata);
    self->handleMouse(event, x, y);
  }

  void handleMouse(int event, int x, int y) {
    if (event == cv::EVENT_LBUTTONDOWN) {
      if (corners.size() == 2) {
        corners[1] = cv::Point2d(x, y);
      } else {
        corners.push_back(cv::Point2d(x, y));
      }
    } else if (event == cv::EVENT_MOUSEMOVE) {
      mousePos = cv::Point2d(x, y);
    }
  }

 public:
  RoiSelect(const cv::Mat& frame) : frameRef(frame) {}

  bool selectRoi(cv::Rect2d* roi) {
    cv::namedWindow("Select ROI", cv::WINDOW_AUTOSIZE);
    cv::setMouseCallback("Select ROI", onMouse, this);

    bool enterPressed = false;

    while (true) {
      cv::Mat displayFrame;
      frameRef.copyTo(displayFrame);

      if (corners.size() == 1) {
        cv::rectangle(displayFrame, corners[0], mousePos, cv::Scalar(0, 255, 0), 2);
      } else if (corners.size() == 2) {
        cv::rectangle(displayFrame, corners[0], corners[1], cv::Scalar(0, 255, 0), 2);
      }

      cv::imshow("Select ROI", displayFrame);
      char key = cv::waitKey(16);

      if (key == 13) {  // Enter key
        enterPressed = true;
      } else if (key == 8 || key == 127) {  // backspace / del
        if (!corners.empty())
          corners.pop_back();
      } else if (key == 27) {  // ESC
        cv::destroyWindow("Select ROI");
        return false;
      }

      if (enterPressed && corners.size() == 2) {
        cv::destroyWindow("Select ROI");
        *roi = cv::Rect2d(corners[0], corners[1]);
        return true;
      } else if (enterPressed) {
        cv::destroyWindow("Select ROI");
        return false;
      }
    }
    cv::destroyWindow("Select ROI");
    return false;
  }
};
