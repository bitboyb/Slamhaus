#include "slam.hpp"
#include <iostream>
#include <string>
#include <vector>

const int gridSize = 16;
static int spiralStep = 0;
static bool reverse = false;
static std::vector<std::pair<int, int>> spiralPath;

SFUNCTION(const char*, Greet, const char* name)
{
    static std::string greeting;
    greeting = "Hello, " + std::string(name) + "!";
    return greeting.c_str();
}

SFUNCTION(int, Add, int a, int b)
{
    return a + b;
}

SFUNCTION(int, Subtract, int a, int b)
{
    return a - b;
}

SFUNCTION(int, Multiply, int a, int b)
{
    return a * b;
}

SFUNCTION(float, Divide, int a, int b)
{
    if (b == 0) return 0.0f; 
    return static_cast<float>(a) / b;
}

SFUNCTION(const char*, Concat, const char* a, const char* b)
{
    static std::string result;
    result = std::string(a) + std::string(b);
    return result.c_str();
}

SFUNCTION(void, LogMessage, const char* msg)
{
    std::cout << "[WASM Log] " << msg << std::endl;
}

void GenerateSpiralPath()
{
    spiralPath.clear();
    int n = gridSize;
    int left = 0, right = n - 1, top = 0, bottom = n - 1;

    while (left <= right && top <= bottom) 
    {
        for (int i = left; i <= right; ++i) spiralPath.emplace_back(top, i);
        ++top;
        for (int i = top; i <= bottom; ++i) spiralPath.emplace_back(i, right);
        --right;
        for (int i = right; i >= left; --i) spiralPath.emplace_back(bottom, i);
        --bottom;
        for (int i = bottom; i >= top; --i) spiralPath.emplace_back(i, left);
        ++left;
    }
}

SFUNCTION(const char*, RenderLoop)
{
    if (spiralPath.empty()) GenerateSpiralPath();

    static std::string frame;
    frame.clear();

    std::vector<std::string> grid(gridSize, std::string(gridSize, ' '));

    int maxStep = static_cast<int>(spiralPath.size()) - 1;

    if (!reverse) 
    {
        if (spiralStep >= maxStep) 
        {
            spiralStep = maxStep;
            reverse = true;
        }
    } 
    else 
    {
        if (spiralStep <= 0) 
        {
            spiralStep = 0;
            reverse = false;
        }
    }

    auto [y, x] = spiralPath[spiralStep];
    grid[y][x] = '@';

    if (reverse) spiralStep--;
    else spiralStep++;

    for (int i = 0; i < gridSize; ++i) 
    {
        frame += grid[i];
        if (i < gridSize - 1) frame += '\n';
    }

    return frame.c_str();
}
