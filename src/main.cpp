#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <iostream>
#include "shader_s.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>
#include <vector>
#include <map>

using namespace std;
using namespace glm;

// functions
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);
unsigned int loadTexture(const char *path);
float lerpC(float a, float b, float f);

// screen size
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

// camera
static vec3 camPos = vec3(0, 0, 5.0f);
static vec3 camUp = vec3(0, 1.0f, 0);
static vec3 camFront = vec3(0, 0, -1.0f);
bool firstMouse = true;
float yawC = -90.0f;
float pitchC = 0.0f;
float lastX = 800.0f / 2.0;
float lastY = 600.0 / 2.0;
float fov = 45.0f;

// frame management
static float deltaTime = 0.0f;
static float lastFrame = 0.0f;

// player postion
static vec2 playerPos = vec2(0.0f, 0.0f);

// FSM state for cube side
static char side = 'F';

// prevents the player moving into the area
class BlockBox
{
public:
    BlockBox(vec2 start, vec2 finish)
    {
        bottomLeft = start;
        topRight = finish;
    }

    bool contains(vec2 pos)
    {
        // if pos is in the blocking box
        if (pos.x >= bottomLeft.x && pos.y >= bottomLeft.y && pos.x <= topRight.x && pos.y <= topRight.y)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

private:
    vec2 bottomLeft;
    vec2 topRight;
};

// teleports the player to a different position on collision
class TeleportBox
{
public:
    TeleportBox(vec2 start, vec2 finish, vec2 teleport)
    {
        bottomLeft = start;
        topRight = finish;
        teleportPos = teleport;
    }

private:
    vec2 bottomLeft;
    vec2 topRight;
    vec2 teleportPos;
};

// create block boxes
static BlockBox *houseBox = new BlockBox(vec2(-7.0f, 1.0f), vec2(-2.5f, 4.5f));

int main()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // create window
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glLineWidth(2.0f);

    Shader flatShader("C:\\Users\\Trevor Thomas\\Desktop\\Invert\\src\\flat_vertex_shader.glsl",
                      "C:\\Users\\Trevor Thomas\\Desktop\\Invert\\src\\flat_fragment_shader.glsl");
    Shader ourShader("C:\\Users\\Trevor Thomas\\Desktop\\Invert\\src\\vertex_shader.glsl",
                     "C:\\Users\\Trevor Thomas\\Desktop\\Invert\\src\\fragment_shader.glsl");
    Shader lineShader("C:\\Users\\Trevor Thomas\\Desktop\\Invert\\src\\line_vertex_shader.glsl",
                      "C:\\Users\\Trevor Thomas\\Desktop\\Invert\\src\\line_fragment_shader.glsl",
                      "C:\\Users\\Trevor Thomas\\Desktop\\Invert\\src\\line_geometry_shader.glsl");

    float flatVertices[] = {
        1.0f, 1.0f, 0.0f, 1.0f, 1.0f,   // top right
        1.0f, -1.0f, 0.0f, 1.0f, 0.0f,  // bottom right
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, // bottom left
        -1.0f, 1.0f, 0.0f, 0.0f, 1.0f   // top left
    };
    unsigned int indices[] = {
        0, 1, 3, // first Triangle
        1, 2, 3  // second Triangle
    };
    unsigned int fVBO, fVAO, fEBO;
    glGenVertexArrays(1, &fVAO);
    glGenBuffers(1, &fVBO);
    glGenBuffers(1, &fEBO);
    glBindVertexArray(fVAO);
    glBindBuffer(GL_ARRAY_BUFFER, fVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(flatVertices), flatVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    // texture coords attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // load data for background
    stbi_set_flip_vertically_on_load(true);
    int width, height, nrChannels;
    unsigned char *dataBackground = stbi_load("C:\\Users\\Trevor Thomas\\Desktop\\Invert\\assets\\cubemap.png", &width, &height, &nrChannels, 0);

    // background texture
    unsigned int background;
    glGenTextures(1, &background);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, background);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 576, 432, 0, GL_RGBA, GL_UNSIGNED_BYTE, dataBackground);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    stbi_image_free(dataBackground);

    // load data for player sprite
    unsigned char *dataPlayer = stbi_load("C:\\Users\\Trevor Thomas\\Desktop\\Invert\\assets\\idle.png", &width, &height, &nrChannels, 0);

    // player texture
    unsigned int player;
    glGenTextures(1, &player);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, player);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 32, 32, 0, GL_RGBA, GL_UNSIGNED_BYTE, dataPlayer);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    stbi_image_free(dataPlayer);

    float vertices[] = {
        // back
        -0.5f, -0.5f, -0.5f, 1.0f, 0.33f, // A
        0.5f, -0.5f, -0.5f, 0.75f, 0.33f, // B
        0.5f, 0.5f, -0.5f, 0.75f, 0.66f,  // C
        0.5f, 0.5f, -0.5f, 0.75f, 0.66f,  // C
        -0.5f, 0.5f, -0.5f, 1.0f, 0.66f,  // D
        -0.5f, -0.5f, -0.5f, 1.0f, 0.33f, // A

        // front
        -0.5f, -0.5f, 0.5f, 0.25f, 0.33f, // A
        0.5f, -0.5f, 0.5f, 0.5f, 0.33f,   // B
        0.5f, 0.5f, 0.5f, 0.5f, 0.66f,    // C
        0.5f, 0.5f, 0.5f, 0.5f, 0.66f,    // C
        -0.5f, 0.5f, 0.5f, 0.25f, 0.66f,  // D
        -0.5f, -0.5f, 0.5f, 0.25f, 0.33f, // A

        // left
        -0.5f, 0.5f, 0.5f, 0.25f, 0.66f,  // A
        -0.5f, 0.5f, -0.5f, 0.0f, 0.66f,  // B
        -0.5f, -0.5f, -0.5f, 0.0f, 0.33f, // C
        -0.5f, -0.5f, -0.5f, 0.0f, 0.33f, // C
        -0.5f, -0.5f, 0.5f, 0.25f, 0.33f, // D
        -0.5f, 0.5f, 0.5f, 0.25f, 0.66f,  // A

        // right
        0.5f, 0.5f, 0.5f, 0.5f, 0.66f,    // A
        0.5f, 0.5f, -0.5f, 0.75f, 0.66f,  // B
        0.5f, -0.5f, -0.5f, 0.75f, 0.33f, // C
        0.5f, -0.5f, -0.5f, 0.75f, 0.33f, // C
        0.5f, -0.5f, 0.5f, 0.5f, 0.33f,   // D
        0.5f, 0.5f, 0.5f, 0.5f, 0.66f,    // A

        // down
        -0.5f, -0.5f, -0.5f, 0.25f, 0.0f, // A
        0.5f, -0.5f, -0.5f, 0.5f, 0.0f,   // B
        0.5f, -0.5f, 0.5f, 0.5f, 0.33f,   // C
        0.5f, -0.5f, 0.5f, 0.5f, 0.33f,   // C
        -0.5f, -0.5f, 0.5f, 0.25f, 0.33f, // D
        -0.5f, -0.5f, -0.5f, 0.25f, 0.0f, // A

        // up
        -0.5f, 0.5f, -0.5f, 0.25f, 1.0f,  // A
        0.5f, 0.5f, -0.5f, 0.5f, 1.0f,    // B
        0.5f, 0.5f, 0.5f, 0.5f, 0.66f,    // C
        0.5f, 0.5f, 0.5f, 0.5f, 0.66f,    // C
        -0.5f, 0.5f, 0.5f, 0.25f, 0.66f,  // D
        -0.5f, 0.5f, -0.5f, 0.25f, 1.0f}; // A

    unsigned int VBO,
        VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    // texture attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // light
    vec3 lightPos = vec3(10.0f, 13.0f, 20.0f);

    // create framebuffer
    unsigned int fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    // create texture to bind framebuffer to
    unsigned int texture;
    glGenTextures(1, &texture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 576, 432, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // render loop
    while (!glfwWindowShouldClose(window))
    {
        // frame management
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        processInput(window);

        // fix player position

        // adjust camera

        // render to framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glEnable(GL_BLEND);
        glEnable(GL_ALPHA_TEST);
        glViewport(0, 0, 576, 432);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        flatShader.use();
        glBindVertexArray(fVAO);

        // draw background
        glm::mat4 model = glm::mat4(1.0f);
        flatShader.setInt("text", 0);
        flatShader.setMat4("model", model);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        model = scale(model, vec3(0.066f, 0.066f, 0.066f));
        model = translate(model, vec3(playerPos, 0.0f));
        flatShader.setInt("text", 2);
        flatShader.setMat4("model", model);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glBindVertexArray(0);

        // render to screen
        model = glm::mat4(1.0f);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glEnable(GL_DEPTH_TEST);
        glViewport(0, 0, 1920, 1080);
        glClearColor(0.85f, 0.89f, 0.86f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ourShader.use();
        glBindVertexArray(VAO);
        glBindTexture(GL_TEXTURE_2D, texture);
        ourShader.setInt("text", 1);
        ourShader.setVec3("lightPos", lightPos);

        // create transformations
        glm::mat4 view;
        view = lookAt(camPos, camPos + camFront, camUp);

        glm::mat4 projection = glm::mat4(1.0f);
        projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 400.0f);

        // pass transformation matrices to the shader
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        // calculate the model matrix for each object and pass it to shader before drawing
        model = translate(model, vec3(0, sin(glfwGetTime()) / 5.0f, 0));

        // draw starting cube
        ourShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // draw starting cube outline
        lineShader.use();
        lineShader.setMat4("projection", projection);
        lineShader.setMat4("view", view);
        lineShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // draw distance cubes
        ourShader.use();
        model = glm::mat4(1.0f);
        model = scale(model, vec3(3.0f, 3.0f, 3.0f));
        model = translate(model, vec3(1.5f, 0.3f, -3.0f));
        ourShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        lineShader.use();
        lineShader.setMat4("model", model);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // garbage collection
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &fVAO);
    glDeleteBuffers(1, &fVBO);
    glDeleteFramebuffers(1, &fbo);
    glfwTerminate();
    return 0;
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f; // change this value to your liking
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yawC += xoffset;
    pitchC += yoffset;

    // make sure that when pitch is out of bounds, screen doesn't get flipped
    if (pitchC > 89.0f)
        pitchC = 89.0f;
    if (pitchC < -89.0f)
        pitchC = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yawC)) * cos(glm::radians(pitchC));
    front.y = sin(glm::radians(pitchC));
    front.z = sin(glm::radians(yawC)) * cos(glm::radians(pitchC));
    camFront = glm::normalize(front);
}

/**
 * Handle user input.
 */
void processInput(GLFWwindow *window)
{
    // exit window on esc key
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float cameraSpeed = 15.0f * deltaTime; // adjust accordingly
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camPos += cameraSpeed * camFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camPos -= cameraSpeed * camFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camPos -= normalize(cross(camFront, camUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camPos += normalize(cross(camFront, camUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camPos += cameraSpeed * camUp;
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        camPos -= cameraSpeed * camUp;

    // player movement
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
    {
        if (!houseBox->contains(vec2(playerPos.x - 4 * deltaTime, playerPos.y)))
        {
            playerPos.x -= 4 * deltaTime;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    {
        if (!houseBox->contains(vec2(playerPos.x + 4 * deltaTime, playerPos.y)))
        {
            playerPos.x += 4 * deltaTime;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    {
        if (!houseBox->contains(vec2(playerPos.x, playerPos.y + 4 * deltaTime)))
        {
            playerPos.y += 4 * deltaTime;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        if (!houseBox->contains(vec2(playerPos.x, playerPos.y - 4 * deltaTime)))
        {
            playerPos.y -= 4 * deltaTime;
        }
    }
}

float lerpC(float a, float b, float f)
{
    return a + f * (b - a);
}

/**
 * Called when window changes size.
 */
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}