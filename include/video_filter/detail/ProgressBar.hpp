#pragma once

#include <cmath>
#include <iomanip>
#include <iostream>


class ProgressBar {
  size_t m_max_loop_count;
  size_t m_progress{0};

 public:
  ProgressBar(size_t max_loop_count) : m_max_loop_count(max_loop_count) {}

  void operator++() { ++m_progress; }

  void display() {
    constexpr int barWidth = 50;
    const float progress = static_cast<float>(m_progress) / m_max_loop_count;
    const int pos = static_cast<int>(std::round(progress * barWidth));
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
};
