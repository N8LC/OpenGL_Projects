#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <random>
#include <iostream>
#include <chrono>
#include <thread>

static int dropCount = 20000;

// 1️⃣ Create a random device (true entropy seed)
std::random_device rd;

// 2️⃣ Create a random number generator, seeded with rd
std::mt19937 gen(rd());

std::uniform_real_distribution<> dis(-1.0, 1.0);

const char *vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec2 aPos;
layout (location = 1) in vec2 iOffset;
layout (location = 2) in float iRadius;
layout (location = 3) in vec3 iColor;

out vec3 fragColor;

uniform float aspectRatio;  // width / height

void main()
{
    vec2 pos = aPos * iRadius;  // scale the unit circle shape
    pos.x /= aspectRatio;       // correct the shape for aspect ratio
    pos += iOffset;             // then place it in the world
    gl_Position = vec4(pos, 0.0, 1.0);
    fragColor = iColor;
}
)";

const char* fragmentShaderSource = R"(
#version 330 core
in vec3 fragColor;
out vec4 FragColor;

void main()
{
    FragColor = vec4(fragColor, 1.0);
}
)";

struct Drop
{
    float x,y;
    float vx,vy;
    float radius;
    float r,g,b;
};

struct DropInstance
{
        float x,y;
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

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(mode->width, mode->height, "Potato (Full Screen)", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    // Make the window full screen
    //glfwSetWindowMonitor(window, primaryMonitor, 0, 0, mode->width, mode->height, mode->refreshRate);

    int fbWidth, fbHeight;

    /* Make the window's context current */
    glfwMakeContextCurrent(window);
    glewExperimental = GL_TRUE;
    glewInit();

    glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
    glViewport(0, 0, fbWidth, fbHeight);

    // Compile vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    int success; char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) { glGetShaderInfoLog(vertexShader, 512, NULL, infoLog); std::cerr << "Vert error: " << infoLog << std::endl; }

    // Compile fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) { glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog); std::cerr << "Frag error: " << infoLog << std::endl; }

    // Link
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    glUseProgram(shaderProgram);

    float aspectRatio = (float)fbWidth / (float)fbHeight;
    GLint aspectLoc = glGetUniformLocation(shaderProgram, "aspectRatio");
    glUniform1f(aspectLoc, aspectRatio);

    GLuint VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    /* Create Vertex Buffer Object*/
    GLuint instanceVBO;
    glGenBuffers(1, &instanceVBO);                      
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);         
    glBufferData(GL_ARRAY_BUFFER,                       
                dropCount * sizeof(DropInstance), 
                nullptr,                               
                GL_DYNAMIC_DRAW);

    float circleVerts[] = {
        0.0f,  0.0f,   // center
        1.0f,  0.0f,
        0.707f, 0.707f,
        0.0f,  1.0f,
        -0.707f, 0.707f,
        -1.0f, 0.0f,
        -0.707f,-0.707f,
        0.0f, -1.0f,
        0.707f,-0.707f,
        1.0f,  0.0f,   // close the fan
    };

    GLuint circleVBO;
    glGenBuffers(1, &circleVBO);
    glBindBuffer(GL_ARRAY_BUFFER, circleVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(circleVerts), circleVerts, GL_STATIC_DRAW);

    struct Drop *drops = (struct Drop *) malloc(sizeof(Drop) * dropCount);

    for(int i = 0; i < dropCount; ++i) {
        drops[i] = {(float)dis(gen), (float)dis(gen) + 2.0f, 
                    ((float)dis(gen)) * 0.3f, 
                    -0.7f + ((float)dis(gen)) * 0.1f,
                    0.005f + ((float)dis(gen)) * 0.001f,
                    1.0f, 1.0f, 1.0f};
    }

    /* Array for flattening drops */
    std::vector <DropInstance> dropInstances(dropCount);

    // attribute 0: shape geometry — this is fine, circleVBO is bound here
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    // switch to instanceVBO before describing instance attributes
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);

    // attribute 1: x, y (offset 0)
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(DropInstance), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribDivisor(1, 1); // advance once per instance

    // attribute 2: radius (offset 8 bytes)
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(DropInstance), (void*)8);
    glEnableVertexAttribArray(2);
    glVertexAttribDivisor(2, 1);

    // attribute 3: r, g, b (offset 12 bytes)
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(DropInstance), (void*)12);
    glEnableVertexAttribArray(3);
    glVertexAttribDivisor(3, 1);


    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        auto frameStart = std::chrono::high_resolution_clock::now();

        processInput(window);

        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        for (int i = 0; i < dropCount; ++i) {
            std::chrono::duration<float> curElapsed = std::chrono::high_resolution_clock::now() - frameStart;
            updateDrop(drops[i], (float) curElapsed.count());

            dropInstances[i] = {drops[i].x, drops[i].y, drops[i].radius, drops[i].r, drops[i].g, drops[i].b};
            // drawCircle(drops[i].x, drops[i].y, drops[i].radius, drops[i].r, drops[i].g, drops[i].b);
        }

        glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);

        glBufferSubData(GL_ARRAY_BUFFER, 0, dropInstances.size() * sizeof(DropInstance), dropInstances.data());

        glDrawArraysInstanced(GL_TRIANGLE_FAN, 0, 10, dropCount);

        /* Swap front and back buffers */
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    free(drops);
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}