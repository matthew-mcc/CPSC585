#pragma once
#include <iostream>
#include <glad/glad.h>
#include <glm.hpp>

class Timer {

public:
    // Return a pointer to the singleton instance of Timer
    static Timer& Instance() {
        static Timer timer;
        return timer;
    }

    // Update 
    // Should be called every frame
    // Updates delta time and frame counts
    void update() {
        currentTime = glfwGetTime();
        deltaTime = currentTime - lastDeltaTime;
        lastDeltaTime = currentTime;
        frameCount++;
    }

    // Get Delta Time
    // Returns time delta between current and last frame
    double getDeltaTime() {
        return deltaTime;
    }

    // Get FPS
    // Returns FPS (frames-per-second) at <rate> times per second
    // Lower values of <rate> will display more frequent updates, higher values will display less frequent updates
    // Returns NULL if time between last update and now is less than <rate>
    int getFPS(double rate) {
        if (currentTime - lastFPSTime >= rate) {
            int outFPS = (int)((double)frameCount * (1.0 / rate));
            lastFPSTime = currentTime;
            frameCount = 0;
            return outFPS;
        }
        return NULL;
    }

private:
    Timer() {};
    Timer(const Timer&);

    int frameCount = 0;						// Frames rendered since last fps update
    double deltaTime = 0.0;					// Time delta between last and current frame
    double currentTime = glfwGetTime();
    double lastDeltaTime = glfwGetTime();
    double lastFPSTime = glfwGetTime();
};