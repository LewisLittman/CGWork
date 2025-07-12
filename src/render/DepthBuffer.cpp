#include "DepthBuffer.h"

DepthBuffer::DepthBuffer(int width, int height) : buffer(width, std::vector<float>(height, 0.0f)) {}

void DepthBuffer::reset(float value) {
    for (auto& row : buffer) {
        std::fill(row.begin(), row.end(), value);
    }
}

float DepthBuffer::get(int x, int y) const {
    return buffer[x][y];
}

void DepthBuffer::set(int x, int y, float depth) {
    buffer[x][y] = depth;
}

