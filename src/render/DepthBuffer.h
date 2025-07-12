#ifndef DEPTH_BUFFER_H
#define DEPTH_BUFFER_H

#include <vector>
#include <limits>

class DepthBuffer {
public:
    DepthBuffer(int width, int height);
    void reset(float value = 0.0f);
    float get(int x, int y) const;
    void set(int x, int y, float depth);

private:
    std::vector<std::vector<float>> buffer;
};

#endif