#pragma once
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>

class Timer {

public:
    // Return a pointer to the singleton instance of Timer
    static Timer& Instance() {
        static Timer timer;
        return timer;
    }

    void init() {
        currentTime = glfwGetTime();
        lastDeltaTime = glfwGetTime();
        lastFPSTime = glfwGetTime();
    }

    // Update 
    // Should be called every frame
    // Updates delta time and frame counts
    void update() {
        currentTime = glfwGetTime();
        deltaTime = currentTime - lastDeltaTime;
        lastDeltaTime = currentTime;
        frameCount++;
        countdown = countdown - deltaTime;
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

    // Get Countdown
    // Returns countdown as pure seconds
    int getCountdown() {
        return countdown;
    }

    // Get Countdown Minutes
    // Returns the minutes component of a standard clock notation (M:SS)
    int getCountdownMins() {
        return countdown / 60;
    }

    // Get Countdown Seconds
    // Returns the seconds component of a standard clock notation (M:SS)
    int getCountdownSecs() {
        return (int)countdown % 60;
    }

private:
    Timer() {};
    Timer(const Timer&);

    int frameCount = 0;						// Frames rendered since last fps update
    double deltaTime = 0.0;					// Time delta between last and current frame
    double currentTime;
    double lastDeltaTime;
    double lastFPSTime;
    double countdown = 300.f;
};