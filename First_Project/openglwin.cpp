#include <GLFW/glfw3.h>
#include <random>
#include <iostream>
#include <chrono>
#include <thread>

static int dropCount = 5000;

// 1️⃣ Create a random device (true entropy seed)
std::random_device rd;

// 2️⃣ Create a random number generator, seeded with rd
std::mt19937 gen(rd());

std::uniform_real_distribution<> dis(-1.0, 1.0);

const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0";

struct Drop
{
    float x,y;
    float vx,vy;
    float radius;
    float r,g,b;
};

void updateDrop(Drop &d, float time)
{
    d.x += d.vx * time;
    d.y += d.vy * time;

    if(d.x > 1.0f || d.x < -1.0f) d.x = -d.x;
    if(d.y < -1.0f) d.y = -d.y;
}

void drawSquare(float x, float y, float size, float r, float g, float b)
{
    float half = size / 2.0f;
    float square_verts[] = {
        x - half, y - half,
        x + half, y - half,
        x + half, y + half,
        x + half, y + half,
        x - half, y + half,
        x - half, y - half
    };

    glColor3f(r, g, b);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, square_verts);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDisableClientState(GL_VERTEX_ARRAY);
}

void drawCircle(float x, float y, float size, float r, float g, float b)
{
    float half = size / 2.0f;
    float oneOverSqrtPI = 1.0f / sqrt(3.14159265f);
    float circle_verts[] = {
        x, y,
        x - (half * oneOverSqrtPI), y - (half * oneOverSqrtPI),
        x - half, y,
        x - (half * oneOverSqrtPI), y + (half * oneOverSqrtPI),
        x, y + half,
        x + (half * oneOverSqrtPI), y + (half * oneOverSqrtPI),
        x + half, y,
        x + (half * oneOverSqrtPI), y - (half * oneOverSqrtPI),
        x, y - half,
        x - (half * oneOverSqrtPI), y - (half * oneOverSqrtPI),
    };

    glColor3f(r, g, b);
    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, circle_verts);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 10);
    glDisableClientState(GL_VERTEX_ARRAY);
}

float vertices[] = {
    -0.5f, -0.5f, 0.0f,
     0.5f, -0.5f, 0.0f,
     0.0f,  0.5f, 0.0f
};  

void processInput(GLFWwindow *window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}


int main(void)
{
    /* buffer stuff */
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit()) return -1;

    // Get the primary monitor
    GLFWmonitor* primaryMonitor = glfwGetPrimaryMonitor();
    if (!primaryMonitor) {
        glfwTerminate();
        return -1;
    }

    // Get the video mode of the monitor (screen resolution)
    const GLFWvidmode* mode = glfwGetVideoMode(primaryMonitor);
    if (!mode) {
        glfwTerminate();
        return -1;
    }

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(mode->width, mode->height, "Potato (Full Screen)", primaryMonitor, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    // Make the window full screen
    //glfwSetWindowMonitor(window, primaryMonitor, 0, 0, mode->width, mode->height, mode->refreshRate);

    int fbWidth, fbHeight;
    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    glViewport(0, 0, fbWidth, fbHeight);

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    Drop drops[dropCount];
    for(int i = 0; i < dropCount; ++i) {
        drops[i] = {(float)dis(gen), (float)dis(gen) + 2.0f, 
                    ((float)dis(gen) - 0.5f) * 0.4f, 
                    -1.0f + ((float)dis(gen)) * 0.25f,
                    0.005f + ((float)dis(gen)) * 0.001f,
                    0.0f, 0.0f, ((float)dis(gen) + 0.5f) / 0.5f};
    }

    const double frames = 60.0;
    const std::chrono::duration<double, std::milli> frameDuration(1000.0 / frames);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        auto frameStart = std::chrono::high_resolution_clock::now();

        processInput(window);

        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        for (auto &drop : drops) {
            std::chrono::duration<float> curElapsed = std::chrono::high_resolution_clock::now() - frameStart;
            updateDrop(drop, (float) curElapsed.count());
            drawCircle(drop.x, drop.y, drop.radius, drop.r, drop.g, drop.b);
        }

        /* Swap front and back buffers */
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();

        std::chrono::duration<double, std::milli> elapsed = std::chrono::high_resolution_clock::now() - frameStart;
        if (elapsed < frameDuration) {
            //std::this_thread::sleep_for(frameDuration - elapsed);
        }
    }

    glfwTerminate();
    return 0;
}