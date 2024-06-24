#pragma once

#include <opencv2/opencv.hpp>
#include <string>

namespace dbg {
inline std::string typeToString(int type) {
  switch (type) {
    case CV_8UC1:
      return "CV_8UC1";
    case CV_8UC2:
      return "CV_8UC2";
    case CV_8UC3:
      return "CV_8UC3";
    case CV_8UC4:
      return "CV_8UC4";
    case CV_16SC1:
      return "CV_16SC1";
    case CV_16SC2:
      return "CV_16SC2";
    case CV_16SC3:
      return "CV_16SC3";
    case CV_16SC4:
      return "CV_16SC4";
    case CV_16UC1:
      return "CV_16UC1";
    case CV_16UC2:
      return "CV_16UC2";
    case CV_16UC3:
      return "CV_16UC3";
    case CV_16UC4:
      return "CV_16UC4";
    case CV_32SC1:
      return "CV_32SC1";
    case CV_32SC2:
      return "CV_32SC2";
    case CV_32SC3:
      return "CV_32SC3";
    case CV_32SC4:
      return "CV_32SC4";
    case CV_32FC1:
      return "CV_32FC1";
    case CV_32FC2:
      return "CV_32FC2";
    case CV_32FC3:
      return "CV_32FC3";
    case CV_32FC4:
      return "CV_32FC4";
    case CV_64FC1:
      return "CV_64FC1";
    case CV_64FC2:
      return "CV_64FC2";
    case CV_64FC3:
      return "CV_64FC3";
    case CV_64FC4:
      return "CV_64FC4";
    default:
      return "Unknown type";
  }
}

inline bool areMatsCompatible(const cv::Mat& mat1, const cv::Mat& mat2) {
  if (mat1.size() != mat2.size() || mat1.type() != mat2.type()) {
    std::cerr << "Matrix size or type mismatch!" << std::endl;
    std::cerr << "Mat1 - Size: " << mat1.size()
              << ", Type: " << typeToString(mat1.type()) << std::endl;
    std::cerr << "Mat2 - Size: " << mat2.size()
              << ", Type: " << typeToString(mat2.type()) << std::endl;
    return false;
  }
  return true;
}

}  // namespace dbg
