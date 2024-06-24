#include <string>
#include <video_filter/CommandLineParser.hpp>
#include <video_filter/LightTrail.hpp>

int main(int argc, char** argv) {
  std::unordered_map<std::string, InputParser::Option> options = {
      {"-threshold", {"30", false, false}},
      {"-halo_radius", {"50", false, false}},
      {"-use_region_growing", {"false", false, false}}};
  std::vector<std::string> positionalArgs = {"input_video", "output_video"};

  InputParser input(argc, argv, options, positionalArgs);

  int threshold = input.getCmdOption<int>("-threshold");
  int haloPixelSize = input.getCmdOption<int>("-halo_radius");
  bool useRegionGrowing = input.getCmdOption<bool>("-use_region_growing");
  std::string inputFile = input.getCmdOption<std::string>("input_video");
  std::string outputFile = input.getCmdOption<std::string>("output_video");

  LightTrail lightTrail(inputFile, outputFile, threshold, haloPixelSize, useRegionGrowing);
  lightTrail.processVideo();

  return 0;
}
