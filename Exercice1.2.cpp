/*#include "GLShader.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>

// Déclaration des variables globales (pour les buffers)
GLuint vao3D, vbo3D, ebo3D, vao2D, vbo2D, ebo2D;
GLuint shaderProgram;

// Structure pour un vecteur 3D
struct Vec3 {
    float x, y, z;
};

// Structure pour une matrice 4x4
struct Mat4 {
    float data[16];
};

// Fonction pour créer une matrice identité
Mat4 identityMatrix() {
    Mat4 mat = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };
    return mat;
}

// Fonction pour créer une matrice de projection orthographique en 2D (pixels)
Mat4 ortho2D(float left, float right, float bottom, float top) {
    Mat4 mat = { 0 };
    mat.data[0] = 2.0f / (right - left);
    mat.data[5] = 2.0f / (top - bottom);
    mat.data[10] = -1.0f;
    mat.data[12] = -(right + left) / (right - left);
    mat.data[13] = -(top + bottom) / (top - bottom);
    mat.data[15] = 1.0f;
    return mat;
}

// Fonction pour créer une matrice de projection en perspective
Mat4 perspective(float fov, float aspectRatio, float nearPlane, float farPlane) {
    Mat4 mat = { 0 };
    float tanHalfFOV = tan(fov / 2.0f);
    mat.data[0] = 1.0f / (aspectRatio * tanHalfFOV);
    mat.data[5] = 1.0f / tanHalfFOV;
    mat.data[10] = -(farPlane + nearPlane) / (farPlane - nearPlane);
    mat.data[11] = -1.0f;
    mat.data[14] = -(2.0f * farPlane * nearPlane) / (farPlane - nearPlane);
    return mat;
}

// Fonction pour multiplier deux matrices 4x4
Mat4 multiplyMat4(const Mat4& a, const Mat4& b) {
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

// Fonction pour créer un shader OpenGL à partir du code source
GLuint createShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    // Vérification des erreurs de compilation
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        std::cerr << "Erreur de compilation du shader : " << infoLog << std::endl;
    }
    return shader;
}

// Fonction pour créer le programme de shaders
GLuint createShaderProgram() {
    const char* vertexShaderSource = R"(
        #version 330 core
        layout(location = 0) in vec3 position;
        uniform mat4 projection;
        uniform mat4 model;
        void main() {
            gl_Position = projection * model * vec4(position, 1.0);
        }
    )";

    const char* fragmentShaderSource = R"(
        #version 330 core
        out vec4 fragColor;
        void main() {
            fragColor = vec4(0.2, 0.7, 0.3, 1.0);  // Couleur verte
        }
    )";

    GLuint vertexShader = createShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint fragmentShader = createShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    // Vérification des erreurs de linkage
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        std::cerr << "Erreur de linkage du programme de shaders : " << infoLog << std::endl;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}

// Données de vertices pour le cube 3D
float cube_vertices[] = {
    // Front face
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    // Back face
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f
};

unsigned int cube_elements[] = {
    // Front face
    0, 1, 2, 2, 3, 0,
    // Right face
    1, 5, 6, 6, 2, 1,
    // Back face
    7, 6, 5, 5, 4, 7,
    // Left face
    4, 0, 3, 3, 7, 4,
    // Bottom face
    4, 5, 1, 1, 0, 4,
    // Top face
    3, 2, 6, 6, 7, 3
};

// Données de vertices pour un rectangle 2D (en pixels)
float rect_vertices[] = {
    // Positions en pixels
    100.0f, 100.0f, 0.0f,  // coin haut gauche
    200.0f, 100.0f, 0.0f,  // coin haut droit
    200.0f, 200.0f, 0.0f,  // coin bas droit
    100.0f, 200.0f, 0.0f   // coin bas gauche
};

unsigned int rect_indices[] = {
    0, 1, 2,  // Premier triangle
    2, 3, 0   // Deuxième triangle
};

bool initialize() {
    // Initialisation de GLFW
    if (!glfwInit()) return false;

    // Créer une fenêtre OpenGL
    GLFWwindow* window = glfwCreateWindow(800, 600, "Exercice 1.2 - Affichage 2D/3D", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(window);

    // Initialisation de GLEW
    if (glewInit() != GLEW_OK) {
        std::cerr << "Erreur d'initialisation de GLEW" << std::endl;
        return false;
    }

    // Activer le test de profondeur pour le 3D
    glEnable(GL_DEPTH_TEST);

    // Générer les VAO et VBO pour le cube 3D
    glGenVertexArrays(1, &vao3D);
    glGenBuffers(1, &vbo3D);
    glGenBuffers(1, &ebo3D);

    glBindVertexArray(vao3D);
    glBindBuffer(GL_ARRAY_BUFFER, vbo3D);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo3D);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_elements),
        cube_elements, GL_STATIC_DRAW);

    // Spécifier les attributs des vertices (position)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Générer les VAO et VBO pour le rectangle 2D
    glGenVertexArrays(1, &vao2D);
    glGenBuffers(1, &vbo2D);
    glGenBuffers(1, &ebo2D);

    glBindVertexArray(vao2D);
    glBindBuffer(GL_ARRAY_BUFFER, vbo2D);
    glBufferData(GL_ARRAY_BUFFER, sizeof(rect_vertices), rect_vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo2D);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(rect_indices), rect_indices, GL_STATIC_DRAW);

    // Spécifier les attributs des vertices pour le rectangle 2D
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Créer les shaders
    shaderProgram = createShaderProgram();
    glUseProgram(shaderProgram);

    return true;
}

void render3D() {
    // Effacer l'écran et le Z-buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Matrice de transformation du modèle (rotation simple avec le temps)
    Mat4 model = identityMatrix();

    // Matrice de vue (placer la caméra à -5 unités sur l'axe Z)
    Mat4 view = identityMatrix();
    view.data[14] = -5.0f;

    // Matrice de projection en perspective
    Mat4 projection = perspective(45.0f * (3.14159f / 180.0f), 800.0f / 600.0f, 0.1f, 100.0f);

    // Envoyer les matrices au shader
    GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
    GLuint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLuint projLoc = glGetUniformLocation(shaderProgram, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, model.data);
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, view.data);
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, projection.data);

    // Dessiner le cube 3D
    glBindVertexArray(vao3D);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
}

void render2D() {
    // Désactiver le Depth Test et le Culling pour le rendu 2D
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    // Créer la matrice de projection orthographique pour le 2D
    Mat4 ortho = ortho2D(0.0f, 800.0f, 600.0f, 0.0f);  // Origine en haut à gauche

    // Envoyer la matrice orthographique au shader
    GLuint projLoc = glGetUniformLocation(shaderProgram, "projection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, ortho.data);

    // Dessiner le rectangle 2D
    glBindVertexArray(vao2D);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    // Réactiver le Depth Test et le Culling
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
}

void terminate() {
    // Libérer les ressources OpenGL
    glDeleteVertexArrays(1, &vao3D);
    glDeleteBuffers(1, &vbo3D);
    glDeleteBuffers(1, &ebo3D);
    glDeleteVertexArrays(1, &vao2D);
    glDeleteBuffers(1, &vbo2D);
    glDeleteBuffers(1, &ebo2D);
    glDeleteProgram(shaderProgram);

    // Fermer GLFW
    glfwTerminate();
}

int main() {
    if (!initialize()) return -1;

    // Boucle principale
    while (!glfwWindowShouldClose(glfwGetCurrentContext())) {
        render3D();  // Rendu 3D du cube
        render2D();  // Rendu 2D du rectangle
        glfwSwapBuffers(glfwGetCurrentContext());
        glfwPollEvents();
    }

    // Nettoyage
    terminate();
    return 0;
}
*/