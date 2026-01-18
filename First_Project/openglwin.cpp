#include <GLFW/glfw3.h>

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
};

void updateDrop(Drop &d)
{
    d.x += d.vx;
    d.y += d.vy;

    if(d.x > 1.0f || d.x < -1.0f) d.vx = -d.vx;
    if(d.y > 1.0f || d.y < -1.0f) d.vy = -d.vy;
}

void drawSquare(float x, float y, float size)
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

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(2, GL_FLOAT, 0, square_verts);
    glDrawArrays(GL_TRIANGLES, 0, 6);
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
    window = glfwCreateWindow(mode->width, mode->height, "Potato (Full Screen)", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    // Optionally, make the window full screen
    glfwSetWindowMonitor(window, primaryMonitor, 0, 0, mode->width, mode->height, mode->refreshRate);

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    Drop drop = {-1.0f, -1.0f, 0.0001f, 0.0002f, 0.1f};
    

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        updateDrop(drop);
        drawSquare(drop.x, drop.y, drop.radius);

        /* Swap front and back buffers */
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}