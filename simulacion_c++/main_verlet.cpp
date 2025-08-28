#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// CONFIGURACIÓN
const float G = 0.001f; // constante gravitatoria pequeña
const float restitution = 1.0f;
const float fixedDt = 0.001f;


// CÁMARA
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
float lastX = 400, lastY = 400;
float yaw = -90.0f, pitch = 0.0f;
bool firstMouse = true;
float sensitivity = 0.001f;

// CLASE OBJETO
class Objeto {
public:
    float xPos, yPos, zPos;
    float xPrev, yPrev, zPrev; // Posición anterior para Verlet
    float radius;
    float masa;
    glm::vec3 color;

    Objeto(float x, float y, float z, float vx_, float vy_, float vz_, float r, float m, glm::vec3 c)
        : xPos(x), yPos(y), zPos(z), radius(r), masa(m), color(c) {
        xPrev = x - vx_ * fixedDt;
        yPrev = y - vy_ * fixedDt;
        zPrev = z - vz_ * fixedDt;
    }
};

// CALLBACK PARA AJUSTAR VIEWPORT
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// MOVERSE CON WASD Y ESC PARA SALIR
void processInput(GLFWwindow* window, float deltaTime) {
    float cameraSpeed = 2.5f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// USAR EL MOUSE PARA MOVER LA CAMARA
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

// FUNCIÓN PARA CALCULAR LAS ACELERACIONES DE TODOS LOS OBJETOS EN UN INSTANTE DE TIEMPO
std::vector<glm::vec3> calcularAceleraciones(const std::vector<Objeto>& objetos) {
    std::vector<glm::vec3> aceleraciones(objetos.size(), glm::vec3(0.0f));

    for (size_t i = 0; i < objetos.size(); ++i) {
        for (size_t j = i + 1; j < objetos.size(); ++j) {
            float dx = objetos[j].xPos - objetos[i].xPos;
            float dy = objetos[j].yPos - objetos[i].yPos;
            float dz = objetos[j].zPos - objetos[i].zPos;
            float distSq = dx * dx + dy * dy + dz * dz;

            if (distSq > 0.00001f) {
                float dist = sqrt(distSq);
                float F = G * objetos[i].masa * objetos[j].masa / distSq;

                float Fx = F * dx / dist;
                float Fy = F * dy / dist;
                float Fz = F * dz / dist;

                aceleraciones[i].x += Fx / objetos[i].masa;
                aceleraciones[i].y += Fy / objetos[i].masa;
                aceleraciones[i].z += Fz / objetos[i].masa;

                aceleraciones[j].x -= Fx / objetos[j].masa;
                aceleraciones[j].y -= Fy / objetos[j].masa;
                aceleraciones[j].z -= Fz / objetos[j].masa;
            }
        }
    }
    return aceleraciones;
}

// FUNCIÓN PRINCOPAL DE ACTUALIZACIÓN DEL MÉTODO DE VERLET
void gravedadVerlet(std::vector<Objeto>& objetos, float dt) {
    std::vector<glm::vec3> aceleraciones = calcularAceleraciones(objetos);

    for (size_t i = 0; i < objetos.size(); ++i) {
        float tempX = objetos[i].xPos;
        float tempY = objetos[i].yPos;
        float tempZ = objetos[i].zPos;

        objetos[i].xPos = 2.0f * objetos[i].xPos - objetos[i].xPrev + aceleraciones[i].x * dt * dt;
        objetos[i].yPos = 2.0f * objetos[i].yPos - objetos[i].yPrev + aceleraciones[i].y * dt * dt;
        objetos[i].zPos = 2.0f * objetos[i].zPos - objetos[i].zPrev + aceleraciones[i].z * dt * dt;

        objetos[i].xPrev = tempX;
        objetos[i].yPrev = tempY;
        objetos[i].zPrev = tempZ;
    }
}

// FUNCIÓN PARA CREAR CUERPOS 
void crearEsfera(std::vector<float>& vertices, std::vector<unsigned int> &indices, int sectorCount, int stackCount) {
    float radius = 1.0f;
    for (int i = 0; i <= stackCount; i++) {
        float stackAngle = M_PI / 2 - i * M_PI / stackCount;
        float xy = radius * cosf(stackAngle);
        float z = radius * sinf(stackAngle);

        for (int j = 0; j <= sectorCount; j++) {
            float sectorAngle = j * 2 * M_PI / sectorCount;
            float x = xy * cosf(sectorAngle);
            float y = xy * sinf(sectorAngle);
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);

            glm::vec3 normal = glm::normalize(glm::vec3(x, y, z));
            vertices.push_back(normal.x);
            vertices.push_back(normal.y);
            vertices.push_back(normal.z);
        }
    }
    for (int i = 0; i < stackCount; ++i){
        for (int j = 0; j < sectorCount; ++j){
            int k1 = i * (sectorCount + 1) + j;
            int k2 = k1 + sectorCount + 1;

            // 2 triangulos por sector
            indices.push_back(k1);
            indices.push_back(k2);
            indices.push_back(k1 + 1);

            indices.push_back(k1 + 1);
            indices.push_back(k2);
            indices.push_back(k2 + 1);
        }
    }
}










int main() {

    //--------------- INICIALIZACIÓN DE LA VENTANA ---------------------------------
    if (!glfwInit()) {
        std::cerr << "Error al inicializar GLFW\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1000, 1000, "Simulador 3D", NULL, NULL);
    if (!window) {
        std::cerr << "Error al crear la ventana\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Error al inicializar GLAD\n";
        return -1;
    }

    glViewport(0, 0, 1000, 1000);
    glEnable(GL_DEPTH_TEST);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);


    //----------------------------SHADERS DE LAS ESFERAS----------------------------------
    const char* vertexShaderSource = R"(
        #version 330 core
        layout(location = 0) in vec3 aPos;
        layout(location = 1) in vec3 aNormal;

        out vec3 FragPos;
        out vec3 Normal;

        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;

        void main(){
            FragPos = vec3(model * vec4(aPos,1.0));
            Normal = mat3(transpose(inverse(model))) * aNormal;
            gl_Position = projection * view * vec4(FragPos,1.0);
        }
    )";

    const char* fragmentShaderSource = R"(
        #version 330 core
        out vec4 FragColor;

        in vec3 FragPos;
        in vec3 Normal;

        uniform vec3 objectColor;
        uniform vec3 lightColor;
        uniform vec3 lightPos;
        uniform vec3 viewPos;

        void main(){
            // Ambient
            float ambientStrength = 0.2;
            vec3 ambient = ambientStrength * lightColor;

            // Diffuse
            vec3 norm = normalize(Normal);
            vec3 lightDir = normalize(lightPos - FragPos);
            float diff = max(dot(norm, lightDir), 0.0);
            vec3 diffuse = diff * lightColor;

            // Specular
            float specularStrength = 0.5;
            vec3 viewDir = normalize(viewPos - FragPos);
            vec3 reflectDir = reflect(-lightDir, norm);
            float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
            vec3 specular = specularStrength * spec * lightColor;

            vec3 result = (ambient + diffuse + specular) * objectColor;
            FragColor = vec4(result, 1.0);
        }
    )";

    // Compilar shaders
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // ------------------------------------GENERAR LAS ESFERAS------------------------------------------
    std::vector<float> sphereVertices;
    std::vector<unsigned int> sphereIndices;
    crearEsfera(sphereVertices,sphereIndices, 36, 18);

    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sphereVertices.size() * sizeof(float), &sphereVertices[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereIndices.size()*sizeof(unsigned int), &sphereIndices[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Planeta central (masivo)
    std::vector<Objeto> objetos;
    objetos.push_back(Objeto(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.2f, 1000.0f, glm::vec3(1.0f, 0.8f, 0.2f)));

    // Planetas satelites
    float r1 = 0.6f;
    float v1 = sqrt(G * 1000.0f / r1);
    objetos.push_back(Objeto(r1, 0.0f, 0.0f, 0.0f, v1, 0.0f, 0.05f, 1.0f, glm::vec3(0.2f, 0.6f, 1.0f)));

    float r2 = 0.8f;
    float v2 = sqrt(G * 1000.0f / r2);
    objetos.push_back(Objeto(0.0f, r2, 0.0f, -v2, 0.0f, 0.0f, 0.03f, 0.5f, glm::vec3(1.0f, 0.2f, 0.2f)));

    float r3 = 1.0f;
    float v3 = sqrt(G * 1000.0f / r3);
    objetos.push_back(Objeto(0.0f, 0.0f, r3, v3, 0.0f, 0.0f, 0.04f, 2.0f, glm::vec3(0.8f, 0.8f, 0.8f)));


    
    float lastFrame = 0.0f;
    static float acumulador = 0.0f;

    // -------------------------------LOOP DE LA VENTANA-------------------------------------------
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        acumulador += deltaTime;

        while(acumulador >= fixedDt){
            gravedadVerlet(objetos, fixedDt);
            acumulador -= fixedDt;
        }

        processInput(window, deltaTime);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 800.0f, 0.1f, 100.0f);
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);

        glUseProgram(shaderProgram);
        glUniform3f(glGetUniformLocation(shaderProgram, "lightPos"), 1.2f, 1.0f, 2.0f);
        glUniform3f(glGetUniformLocation(shaderProgram, "lightColor"), 1.0f, 1.0f, 1.0f);
        glUniform3f(glGetUniformLocation(shaderProgram, "viewPos"), cameraPos.x, cameraPos.y, cameraPos.z);

        for (auto& obj : objetos) {
            glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(obj.xPos, obj.yPos, obj.zPos));
            model = glm::scale(model, glm::vec3(obj.radius));
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
            glUniform3fv(glGetUniformLocation(shaderProgram, "objectColor"), 1, glm::value_ptr(obj.color));

            glBindVertexArray(VAO);
            glDrawElements(GL_TRIANGLES, sphereIndices.size(), GL_UNSIGNED_INT, 0); 
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glfwTerminate();
    return 0;
}