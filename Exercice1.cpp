#include "GLShader.h" 
#include <iostream>
#include <cmath>
#include <GL/glew.h>
#include <GLFW/glfw3.h>


GLuint vao, vbo, ebo;
GLShader shader; 

// structure pour un vecteur 3D
struct Vec3 {
    float x, y, z;
};

// structure pour une matrice 4x4
struct Mat4 {
    float data[16];
};

// fonction pour creer une matrice identite
Mat4 identityMatrix() {
    Mat4 mat = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };
    return mat;
}

// fonction pour creer une matrice de translation
Mat4 translate(float x, float y, float z) {
    Mat4 mat = identityMatrix();
    mat.data[12] = x;
    mat.data[13] = y;
    mat.data[14] = z;
    return mat;
}

// fonction pour creer une matrice de projection en perspective
Mat4 perspective(float fov, float aspect, float near, float far) {
    Mat4 mat = { 0 };
    float tanHalfFOV = tan(fov / 2.0f);
    mat.data[0] = 1.0f / (aspect * tanHalfFOV);
    mat.data[5] = 1.0f / tanHalfFOV;
    mat.data[10] = -(far + near) / (far - near);
    mat.data[11] = -1.0f;
    mat.data[14] = -(2.0f * far * near) / (far - near);
    return mat;
}

// fonction pour multiplier deux matrices 4x4
Mat4 multiplyMat4(Mat4 a, Mat4 b) {
    Mat4 result = { 0 };
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            result.data[i * 4 + j] =
                a.data[i * 4 + 0] * b.data[0 * 4 + j] +
                a.data[i * 4 + 1] * b.data[1 * 4 + j] +
                a.data[i * 4 + 2] * b.data[2 * 4 + j] +
                a.data[i * 4 + 3] * b.data[3 * 4 + j];
        }
    }
    return result;
}

// fonctions pour creer des matrices de rotation autour des axes Y, X et Z 
Mat4 rotateY(float angle) {
    Mat4 mat = identityMatrix();
    mat.data[0] = cos(angle);
    mat.data[2] = sin(angle);
    mat.data[8] = -sin(angle);
    mat.data[10] = cos(angle);
    return mat;
}

Mat4 rotateX(float angle) {
    Mat4 mat = identityMatrix();
    mat.data[5] = cos(angle);
    mat.data[6] = -sin(angle);
    mat.data[9] = sin(angle);
    mat.data[10] = cos(angle);
    return mat;
}

Mat4 rotateZ(float angle) {
    Mat4 mat = identityMatrix();
    mat.data[0] = cos(angle);
    mat.data[1] = -sin(angle);
    mat.data[4] = sin(angle);
    mat.data[5] = cos(angle);
    return mat;
}

float cube_vertices[] = {
    -1.0f, -1.0f,  1.0f,   1.0f, 0.0f, 0.0f,
     1.0f, -1.0f,  1.0f,   0.0f, 1.0f, 0.0f,
     1.0f,  1.0f,  1.0f,   0.0f, 0.0f, 1.0f,
    -1.0f,  1.0f,  1.0f,   1.0f, 1.0f, 0.0f,
    -1.0f, -1.0f, -1.0f,   1.0f, 0.0f, 1.0f,
     1.0f, -1.0f, -1.0f,   0.0f, 1.0f, 1.0f,
     1.0f,  1.0f, -1.0f,   0.5f, 0.5f, 0.5f,
    -1.0f,  1.0f, -1.0f,   1.0f, 0.5f, 0.5f
};

unsigned int cube_elements[] = {
    0, 1, 2, 2, 3, 0,
    1, 5, 6, 6, 2, 1,
    7, 6, 5, 5, 4, 7,
    4, 0, 3, 3, 7, 4,
    4, 5, 1, 1, 0, 4,
    3, 2, 6, 6, 7, 3
};

bool initialize() {
    if (!glfwInit()) return false;

    GLFWwindow* window = glfwCreateWindow(800, 600, "Cube en rotation", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK) {
        std::cerr << "Erreur d'initialisation de GLEW" << std::endl;
        return false;
    }

    glEnable(GL_DEPTH_TEST);  

    // Init du cube
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_elements), cube_elements, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // charger les shaders
    if (!shader.LoadShaders("Basic.vs", "Basic.fs")) {
        return false;
    }
    shader.Use();

    return true;
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float time = glfwGetTime();

    // creation des matrices de transformation et de la rotation
    Mat4 model = identityMatrix();

    Mat4 rotationY = rotateY(time);
    Mat4 rotationX = rotateX(time * 0.5f);
    Mat4 rotationZ = rotateZ(time * 0.2f);

    model = multiplyMat4(rotationZ, multiplyMat4(rotationX, rotationY));

    Mat4 view = translate(0.0f, 0.0f, -5.0f);

    Mat4 projection = perspective(45.0f * (3.14159f / 180.0f), 800.0f / 600.0f, 0.1f, 100.0f);

    GLuint modelLoc = glGetUniformLocation(shader.m_Program, "model");
    GLuint viewLoc = glGetUniformLocation(shader.m_Program, "view");
    GLuint projLoc = glGetUniformLocation(shader.m_Program, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, model.data);
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, view.data);
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, projection.data);

    // dessiner le cube
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
}

void terminate() {
    shader.Destroy();
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    glfwTerminate();
}

int main() {
    if (!initialize()) return -1;

    while (!glfwWindowShouldClose(glfwGetCurrentContext())) {
        render();
        glfwSwapBuffers(glfwGetCurrentContext());
        glfwPollEvents();
    }

    terminate();
    return 0;
}