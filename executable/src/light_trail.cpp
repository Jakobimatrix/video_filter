#include <cmath>
#include <iomanip>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <queue>
#include <string>
#include <video_filter/CommandLineParser.hpp>

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


std::string typeToString(int type) {
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

bool areMatsCompatible(const cv::Mat& mat1, const cv::Mat& mat2) {
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


int main(int argc, char** argv) {

  std::unordered_map<std::string, InputParser::Option> options = {
      {"-threshold", {"30", false, false}},
      {"-halo_radius", {"50", false, false}},
      {"-use_region_growing", {"false", false, false}}};
  std::vector<std::string> positionalArgs = {"input_video", "output_video"};

  InputParser input(argc, argv, options, positionalArgs);

  const int threshold = input.getCmdOption<int>("-threshold");
  const int haloPixelSize = input.getCmdOption<int>("-halo_radius");
  const bool useRegionCrowing = input.getCmdOption<bool>("-use_region_growing");
  const std::string inputFile = input.getCmdOption<std::string>("input_video");
  const std::string outputFile =
      input.getCmdOption<std::string>("output_video");

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

  cv::Mat frame, gray, mask, maskColor;
  cv::Mat lightTrail = cv::Mat::zeros(frameHeight, frameWidth, CV_8UC3);
  cv::Mat accumulatedLightColorMask = cv::Mat::zeros(frameHeight, frameWidth, CV_8UC3);

  const int gaussKernalSize = haloPixelSize % 2 == 1 ? haloPixelSize : haloPixelSize + 1;


  int frameCount = 0;
  while (cap.read(frame)) {
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    double minVal, maxVal;
    cv::Point minLoc, maxLoc;
    cv::minMaxLoc(gray, &minVal, &maxVal, &minLoc, &maxLoc);

    if (!useRegionCrowing) {
      cv::threshold(gray, mask, maxVal - threshold, 255, cv::THRESH_BINARY);
    } else {
      // Perform region growing from the brightest point
      mask = regionGrowing(gray, maxLoc, threshold);
    }
    cv::GaussianBlur(mask, mask, cv::Size(gaussKernalSize, gaussKernalSize), 0, 0);
    cv::threshold(mask, mask, 50, 255, cv::THRESH_BINARY);
    cv::GaussianBlur(mask, mask, cv::Size(gaussKernalSize, gaussKernalSize), 0, 0);

    cv::cvtColor(mask, maskColor, cv::COLOR_GRAY2BGR);

    accumulatedLightColorMask = cv::max(accumulatedLightColorMask, maskColor);
    lightTrail = cv::max(lightTrail, maskColor.mul(frame / 255.0));

    // cv::imshow("1", lightTrail);
    // cv::waitKey(0);

    frame = cv::max(frame, lightTrail);
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
