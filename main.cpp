#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include "shader.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

//camera
glm::vec3 camEye = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 camTarget = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 camDir = glm::normalize(camEye-camTarget);
glm::vec3 camFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 camUp = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 camRight = glm::cross(camDir, camUp);

int main()
{
    // glfw: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); //fix compilation on OS X

    // glfw window creation
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Procedural Terrain Gen", NULL, NULL);
    if (window == NULL){
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    glEnable(GL_DEPTH_TEST);

    Shader shader("shaders/vertex.glsl", "shaders/frag.glsl");

    int imgWidth, imgHeight, nChannels;
    unsigned char* data = stbi_load("resources/iceland_heightmap.png", &imgWidth, &imgHeight, &nChannels, 0);
    if(!data) {
        std::cout << "Failed to load image" << std::flush;
    }
    float yScale = 64.0f/256.0f;
    float yOffset = 16.0f;

    std::vector<float> vertices;
    vertices.reserve(imgWidth * imgHeight * 3);
    for(int i=0; i<imgHeight; i++) {
        for(int j=0; j<imgWidth; j++) {
            unsigned char* tex = data+(j+imgWidth*i)*nChannels;
            unsigned char y = tex[0];
            vertices.push_back(i - imgWidth/2.0);
            vertices.push_back((int)y*yScale - yOffset);
            vertices.push_back(j - imgWidth/2.0);
        }
    }
    stbi_image_free(data);

    const int numRows = imgHeight-1;
    const int numVertPerRow = imgWidth*2;
    std::vector<unsigned int> indices;
    for(unsigned int i=0; i<numRows; i++) {
        for(unsigned int j=0; j<imgWidth; j++) {
            for(unsigned int k=0; k<2; k++) {
                indices.push_back(j+imgWidth*(i+k));
            }
        }
    }
    GLuint terrainVAO, terrainVBO, terrainEBO;
    glGenVertexArrays(1, &terrainVAO);
    glBindVertexArray(terrainVAO);

    glGenBuffers(1, &terrainVBO);
    glBindBuffer(GL_ARRAY_BUFFER, terrainVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size()*sizeof(float), &vertices[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &terrainEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, terrainEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    glBindVertexArray(terrainVAO);


    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::mat4(1.0f);
    //view = glm::lookAt(camEye, camTarget, camUp);
    view = glm::lookAt(camEye, camEye+camFront, camUp);
    glm::mat4 projection = glm::mat4(1.0f);
    projection = glm::perspective(glm::radians(65.0f), 800.0f/600.0f, 0.1f, 1000.0f);

    shader.use();
    int modelLoc = glGetUniformLocation(shader.ID, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    int viewLoc = glGetUniformLocation(shader.ID, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    int projLoc = glGetUniformLocation(shader.ID, "proj");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    const float radius = 300.0f;
    // render loop
    while (!glfwWindowShouldClose(window)){
        // input
        processInput(window);


        float camX = sin(glfwGetTime()*0.5)*radius;
        float camZ = cos(glfwGetTime()*0.5)*radius;
        //view = glm::lookAt(camEye, camEye+camFront, camUp);
        view = glm::lookAt(glm::vec3(camX, 100.0f, camZ), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        shader.setMat4("view", view);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();
        for(unsigned int r=0; r<numRows; r++) {
            glDrawElements(GL_TRIANGLE_STRIP, numVertPerRow, GL_UNSIGNED_INT,
                (void*)(sizeof(unsigned int) * numVertPerRow*r));
        }
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    const float cameraSpeed = 0.5f;
    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        camEye += cameraSpeed * camFront;
    }
    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        camEye -= cameraSpeed * glm::normalize(glm::cross(camFront, camUp));
    }
    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        camEye -= cameraSpeed * camFront;
    }
    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        camEye += cameraSpeed * glm::normalize(glm::cross(camFront, camUp));
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}