#ifndef UTILITIES_H
#define UTILITIES_H

#include <vector>
#include <chrono>
#include <queue>
#ifndef __gl_h_
#include <glad/glad.h>
#endif

GLuint createProgram(const std::string& vertexShaderSource,
                     const std::string& fragmentShaderSource,
                     const std::vector<std::string>& attributes);

std::string getColormapImplementation(const std::string& colormapName);

class FPSCounter {
public:
  void tick();
  double getFramerate() const;
private:
  int _max_n = 60;
  std::chrono::duration<double> _n_frame_duration = std::chrono::duration<double>::zero();
  std::chrono::steady_clock::time_point _previous_frame_time_point;
  std::queue<std::chrono::duration<double>> _frame_durations;
};

#endif