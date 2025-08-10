#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

#include "Points.h"

// Function to read shader source code from a file
std::string readShaderSource(const char* filePath) {
    std::ifstream shaderFile;
    // ensure ifstream objects can throw exceptions:
    shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
        shaderFile.open(filePath);
        std::stringstream shaderStream;
        shaderStream << shaderFile.rdbuf();
        shaderFile.close();
        return shaderStream.str();
    } catch (std::ifstream::failure& e) {
        std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: " << filePath << std::endl;
    }
    return "";
}

// Function to compile a shader and check for errors
unsigned int compileShader(unsigned int type, const std::string& source) {
    unsigned int id = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE) {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char* message = (char*)alloca(length * sizeof(char));
        glGetShaderInfoLog(id, length, &length, message);
        std::cerr << "Failed to compile " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader!" << std::endl;
        std::cerr << message << std::endl;
        glDeleteShader(id);
        return 0;
    }

    return id;
}

// Function to create a shader program
unsigned int createShaderProgram(const std::string& vertexShader, const std::string& fragmentShader) {
    unsigned int program = glCreateProgram();
    unsigned int vs = compileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = compileShader(GL_FRAGMENT_SHADER, fragmentShader);

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

int main() {
    // --- GLFW Initialization ---
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // --- Window Creation ---
    const unsigned int SCR_WIDTH = 800;
    const unsigned int SCR_HEIGHT = 800;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL Shaders", NULL, NULL);
    if (window == NULL) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // --- GLAD Initialization ---
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // --- Load Shaders ---
    std::string screenVertexSource = readShaderSource("shaders/screen.vert");
    std::string screenFragmentSource = readShaderSource("shaders/screen.frag");
    unsigned int screenShaderProgram = createShaderProgram(screenVertexSource, screenFragmentSource);

    std::string conveerVertexSource = readShaderSource("shaders/conveer.vert");
    std::string conveerFragmentSource = readShaderSource("shaders/conveer.frag");
    unsigned int conveerShaderProgram = createShaderProgram(conveerVertexSource, conveerFragmentSource);

    // --- Framebuffer for 'conveer' shader ---
    unsigned int fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    unsigned int textureColorbuffer;
    glGenTextures(1, &textureColorbuffer);
    glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cerr << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // --- Vertex Data for a Quad ---
    float quadVertices[] = {
        // positions   // texCoords
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    unsigned int quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));


    float initialQuad[8] = {
        -1.f, -1.f, // bottom-left
         1.f, -1.f, // bottom-right
         1.f,  1.f, // top-right
        -1.f,  1.f  // top-left
    };
    Points conveerQuad(initialQuad);

    float rot = 0.f;
    float scale[2] = {1.f,.3f};
    float pos[2] = {0.f,.4f};
    float trapecional = 0.f;
    int timer = 0;

    // --- Main Render Loop ---
    while (!glfwWindowShouldClose(window)) {
        // --- Render 'conveer' to framebuffer ---
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glEnable(GL_DEPTH_TEST);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(conveerShaderProgram);

        // conveerQuad.move(0.f, 0.f);
        // conveerQuad.scale(.45f, .8f);
        // conveerQuad.rotate(0.f);
        // conveerQuad.deform(-.25f,.25f,0.f,.2f);
        if (timer < 500) {
            if (timer > 200) {
                if (rot < 90) {
                    rot += .5f;
                    scale[1] = .3f + (rot/90*.2f);
                    pos[1] = .4f - (rot/90*.4f);
                    trapecional = rot/90*.3f;
                }
            }
            timer += 1;
        } else {
            rot = 0.f;
            trapecional = 0.f;
            timer = 0;
            scale[1] = .3f;
            pos[1] = .4f;
        }
        conveerQuad.move(pos[0],pos[1]);
        conveerQuad.rotate(rot);
        conveerQuad.scale(scale[0],scale[1]);
        conveerQuad.deform(0.f,0.f,trapecional,-trapecional);
        glUniform2fv(glGetUniformLocation(conveerShaderProgram, "points"), 4, conveerQuad.getRawData());

        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // --- Render to screen ---
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDisable(GL_DEPTH_TEST);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(screenShaderProgram);
        glUniform2f(glGetUniformLocation(screenShaderProgram, "windowSize"), SCR_WIDTH, SCR_HEIGHT);
        glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
        glDrawArrays(GL_TRIANGLES, 0, 6);


        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // --- Cleanup ---
    glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &quadVBO);
    glDeleteFramebuffers(1, &fbo);
    glDeleteProgram(screenShaderProgram);
    glDeleteProgram(conveerShaderProgram);

    glfwTerminate();
    return 0;
}