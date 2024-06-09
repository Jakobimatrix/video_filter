#include <cmath>
#include <iomanip>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <queue>
#include <string>

std::string getExtension(const std::string& filename) {
  size_t dotPos = filename.find_last_of(".");
  if (dotPos == std::string::npos) {
    return "";
  }
  return filename.substr(dotPos + 1);
}

// Function to perform region growing from the brightest pixel
cv::Mat regionGrowing(const cv::Mat& gray, cv::Point seed, double threshold) {
  cv::Mat mask = cv::Mat::zeros(gray.size(), CV_8U);
  std::queue<cv::Point> points;
  points.push(seed);
  mask.at<uchar>(seed) = 255;

  int dx[] = {1, -1, 0, 0, 1, 1, -1, -1};
  int dy[] = {0, 0, 1, -1, 1, -1, 1, -1};

  while (!points.empty()) {
    cv::Point p = points.front();
    points.pop();

    for (int i = 0; i < 8; ++i) {
      int nx = p.x + dx[i];
      int ny = p.y + dy[i];

      if (nx >= 0 && nx < gray.cols && ny >= 0 && ny < gray.rows &&
          mask.at<uchar>(ny, nx) == 0) {
        double diff = std::abs(gray.at<uchar>(ny, nx) - gray.at<uchar>(seed));
        if (diff <= threshold) {
          mask.at<uchar>(ny, nx) = 255;
          points.push(cv::Point(nx, ny));
        }
      }
    }
  }

  return mask;
}

void displayProgressBar(float progress) {
  int barWidth = 50;
  int pos = static_cast<int>(std::round(progress * barWidth));
  std::cout << "[";
  for (int i = 0; i < barWidth; ++i) {
    if (i < pos)
      std::cout << "#";
    else
      std::cout << " ";
  }
  std::cout << "] " << std::setw(3)
            << static_cast<int>(std::round(progress * 100.0)) << "%\r";
  std::cout.flush();
}

bool areMatsCompatible(const cv::Mat& mat1, const cv::Mat& mat2) {
  if (mat1.size() != mat2.size() || mat1.type() != mat2.type()) {
    std::cerr << "Matrix size or type mismatch!" << std::endl;
    std::cerr << "Mat1 - Size: " << mat1.size() << ", Type: " << mat1.type() << std::endl;
    std::cerr << "Mat2 - Size: " << mat2.size() << ", Type: " << mat2.type() << std::endl;
    return false;
  }
  return true;
}



int main(int argc, char** argv) {

#ifndef NDEBUG
  std::string inputFile = "/home/jakob/projects/video_filter/test/in.mp4";
  std::string outputFile = "/home/jakob/projects/video_filter/test/out.mp4";
#else
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <input_video> <output_video>" << std::endl;
    return -1;
  }
  std::string inputFile = argv[1];
  std::string outputFile = argv[2];
#endif

  std::string fileExtension = getExtension(outputFile);

  cv::VideoCapture cap(inputFile);
  if (!cap.isOpened()) {
    std::cerr << "Error opening video stream or file" << std::endl;
    return -1;
  }

  const int frameWidth = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
  const int frameHeight = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));
  const int fps = static_cast<int>(cap.get(cv::CAP_PROP_FPS));
  const int totalFrames = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_COUNT));

  int codec;
  if (fileExtension == "mp4") {
    codec = cv::VideoWriter::fourcc('m', 'p', '4', 'v');
  } else if (fileExtension == "avi") {
    codec = cv::VideoWriter::fourcc('M', 'J', 'P', 'G');
  } else if (fileExtension == "mov") {
    codec = cv::VideoWriter::fourcc('m', 'p', '4', 'v');
  } else {
    std::cerr << "Unsupported output video format: " << fileExtension << std::endl;
    return -1;
  }

  cv::VideoWriter writer(outputFile, codec, fps, cv::Size(frameWidth, frameHeight));

  if (!writer.isOpened()) {
    std::cerr << "Could not open the output video file for write" << std::endl;
    return -1;
  }

  std::vector<cv::Mat> masks;
  cv::Mat frame, gray, mask, maskColor;
  cv::Mat lightTrail = cv::Mat::zeros(frameHeight, frameWidth, CV_8UC3);
  cv::Mat accumulatedLightColorMask = cv::Mat::zeros(frameHeight, frameWidth, CV_8UC3);

  double threshold = 30.0;
  int haloPixelSize = 50;

  constexpr bool useRegionCrowing = false;

  int frameCount = 0;
  while (cap.read(frame)) {
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    double minVal, maxVal;
    cv::Point minLoc, maxLoc;
    cv::minMaxLoc(gray, &minVal, &maxVal, &minLoc, &maxLoc);

    if constexpr (!useRegionCrowing) {
      cv::threshold(gray, mask, maxVal - threshold, 255, cv::THRESH_BINARY);
    } else {
      // Perform region growing from the brightest point
      mask = regionGrowing(gray, maxLoc, threshold);
    }

    cv::cvtColor(mask, maskColor, cv::COLOR_GRAY2BGR);


    cv::bitwise_or(accumulatedLightColorMask, maskColor, accumulatedLightColorMask);
    cv::bitwise_or(lightTrail, maskColor & frame, lightTrail);

    cv::bitwise_or(frame, accumulatedLightColorMask & lightTrail, frame);
    writer.write(frame);

    // Update and display progress bar
    frameCount++;
    float progress = static_cast<float>(frameCount) / totalFrames;
    displayProgressBar(progress);
  }

  cap.release();
  writer.release();
  cv::destroyAllWindows();

  return 0;
}
