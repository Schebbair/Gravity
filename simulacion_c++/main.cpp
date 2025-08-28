#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// CONFIGURACIÓN
const float G = 0.0001f; // constante gravitatoria pequeña
const float restitution = 1.0f;

// CÁMARA
glm::vec3 cameraPos = glm::vec3(0.0f,0.0f,3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f,  0.0f);
float lastX = 400, lastY = 400;
float yaw = -90.0f, pitch = 0.0f;
bool firstMouse = true;
float sensitivity = 0.001f;

// CLASE OBJETO
class Objeto {
public:
    float xPos, yPos, zPos;
    float vx, vy, vz;
    float radius;
    float masa;
    glm::vec3 color;

    Objeto(float x, float y, float z, float vx_, float vy_, float vz_, float r, float m, glm::vec3 c)
        : xPos(x), yPos(y), zPos(z), vx(vx_), vy(vy_), vz(vz_), radius(r), masa(m), color(c) {}

    void update(float dt) {
        xPos += vx * dt;
        yPos += vy * dt;
        zPos += vz * dt;

    }
};

// CALLBACK PARA AJUSTAR VIEWPORT
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// MOVERSE CON WASD Y ESC PARA SALIR
void processInput(GLFWwindow *window, float deltaTime) {
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
void mouse_callback(GLFWwindow* window, double xpos, double ypos){
    if (firstMouse){
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

    yaw   += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)  pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

// COLISIÓN DE LAS ESFERAS
void Collision(Objeto &a, Objeto &b){
    float dx = b.xPos - a.xPos;
    float dy = b.yPos - a.yPos;
    float dz = b.zPos - a.zPos;
    float dist = sqrt(dx*dx + dy*dy + dz*dz);
    float minDist = a.radius + b.radius;

    if (dist < minDist && dist >0.0f){
        float nx = dx / dist;
        float ny = dy / dist;
        float nz = dz / dist;

        float p = 2 * (a.vx*nx + a.vy*ny + a.vz*nz - b.vx*nx - b.vy*ny - b.vz*nz) / 2.0f;

        a.vx -= p * nx;
        a.vy -= p * ny;
        a.vz -= p * nz;
        b.vx += p * nx;
        b.vy += p * ny;
        b.vz += p * nz;

        float overlap = 0.5f * (minDist - dist);
        a.xPos -= overlap * nx;
        a.yPos -= overlap * ny;
        a.zPos -= overlap * nz;
        b.xPos += overlap * nx;
        b.yPos += overlap * ny;
        b.zPos += overlap * nz;
    }
}

// ATRACCIÓN ENTRE LAS ESFERAS
void gravedadMutua(std::vector<Objeto> &objetos, float G, float dt){
    for (size_t i = 0; i < objetos.size(); i++){
        for (size_t j = i+1; j < objetos.size(); j++){
            float dx = objetos[j].xPos - objetos[i].xPos;
            float dy = objetos[j].yPos - objetos[i].yPos;
            float dz = objetos[j].zPos - objetos[i].zPos;
            float distSq = dx*dx + dy*dy + dz*dz;

            if (distSq > 0.001f){
                float dist = sqrt(distSq);
                float F = G * objetos[i].masa * objetos[j].masa / distSq;

                float Fx = F * dx / dist;
                float Fy = F * dy / dist;
                float Fz = F * dz / dist;

                float ax_i = Fx / objetos[i].masa;
                float ay_i = Fy / objetos[i].masa;
                float az_i = Fz / objetos[i].masa;

                float ax_j = -Fx / objetos[j].masa;
                float ay_j = -Fy / objetos[j].masa;
                float az_j = -Fz / objetos[j].masa;

                objetos[i].vx += ax_i * dt;
                objetos[i].vy += ay_i * dt;
                objetos[i].vz += az_i * dt;

                objetos[j].vx += ax_j * dt;
                objetos[j].vy += ay_j * dt;
                objetos[j].vz += az_j * dt;
            }
        }
    }
}

// FUNCIÓN PARA CREAR CUERPOS 
void crearEsfera(std::vector<float> &vertices, std::vector<unsigned int> &indices, int sectorCount, int stackCount){
    float radius = 1.0f;
    for (int i = 0; i <= stackCount; i++){
        float stackAngle = M_PI/2 -i *M_PI / stackCount;
        float xy = radius * cosf(stackAngle);
        float z = radius * sinf(stackAngle);

        for(int j= 0; j <= sectorCount; j++){
            float sectorAngle = j * 2 * M_PI / sectorCount;
            float x = xy * cosf(sectorAngle);
            float y = xy * sinf(sectorAngle);
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);

            glm::vec3 normal = glm::normalize(glm::vec3(x,y,z));
            vertices.push_back(normal.x);
            vertices.push_back(normal.y);
            vertices.push_back(normal.z);
        }
    }
    for (int i = 0; i < stackCount; ++i){
        for (int j = 0; j < sectorCount; ++j){
            int k1 = i * (sectorCount + 1) + j;
            int k2 = k1 + sectorCount + 1;

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
    glfwSetCursorPosCallback(window,mouse_callback);
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
    glShaderSource(vertexShader,1,&vertexShaderSource,NULL);
    glCompileShader(vertexShader);
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader,1,&fragmentShaderSource,NULL);
    glCompileShader(fragmentShader);

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram,vertexShader);
    glAttachShader(shaderProgram,fragmentShader);
    glLinkProgram(shaderProgram);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // ------------------------------------GENERAR LAS ESFERAS------------------------------------------
    std::vector<float> sphereVertices;
    std::vector<unsigned int> sphereIndices;
    crearEsfera(sphereVertices,sphereIndices,36,18);

    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1,&VAO);
    glGenBuffers(1,&VBO);
    glGenBuffers(1,&EBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER,VBO);
    glBufferData(GL_ARRAY_BUFFER,sphereVertices.size()*sizeof(float),&sphereVertices[0],GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO); // Nuevo
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereIndices.size()*sizeof(unsigned int), &sphereIndices[0], GL_STATIC_DRAW); // Nuevo


    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,6*sizeof(float),(void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,6*sizeof(float),(void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    // Planeta central (masivo)
    std::vector<Objeto> objetos;
    objetos.push_back(Objeto(0.0f,0.0f,0.0f, 0.0f,0.0f,0.0f, 0.2f, 1000.0f, glm::vec3(1.0f,0.8f,0.2f)));

    // Planetas satelites
    float r = 0.6f;
    float v = sqrt(G * 1000.0f / r);
    objetos.push_back(Objeto(r,0.0f,0.0f, 0.0f,v,0.0f, 0.05f, 1.0f, glm::vec3(0.2f,0.6f,1.0f)));



    float lastFrame = 0.0f;

    // -------------------------------LOOP DE LA VENTANA-------------------------------------------
    while(!glfwWindowShouldClose(window)){
        float currentFrame = glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window, deltaTime);

        glClearColor(0.1f,0.1f,0.1f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        gravedadMutua(objetos,G,deltaTime);
        for(auto &obj:objetos) obj.update(deltaTime);


        glm::mat4 projection = glm::perspective(glm::radians(45.0f),800.0f/800.0f,0.1f,100.0f);
        glm::mat4 view = glm::lookAt(cameraPos,cameraPos+cameraFront,cameraUp);

        glUseProgram(shaderProgram);
        glUniform3f(glGetUniformLocation(shaderProgram,"lightPos"),1.2f,1.0f,2.0f);
        glUniform3f(glGetUniformLocation(shaderProgram,"lightColor"),1.0f,1.0f,1.0f);
        glUniform3f(glGetUniformLocation(shaderProgram,"viewPos"),cameraPos.x,cameraPos.y,cameraPos.z);

        for(auto &obj:objetos){
            glm::mat4 model = glm::translate(glm::mat4(1.0f),glm::vec3(obj.xPos,obj.yPos,obj.zPos));
            model = glm::scale(model,glm::vec3(obj.radius));
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram,"model"),1,GL_FALSE,glm::value_ptr(model));
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram,"view"),1,GL_FALSE,glm::value_ptr(view));
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram,"projection"),1,GL_FALSE,glm::value_ptr(projection));
            glUniform3fv(glGetUniformLocation(shaderProgram,"objectColor"),1,glm::value_ptr(obj.color));

            glBindVertexArray(VAO);
            glDrawElements(GL_TRIANGLES, sphereIndices.size(), GL_UNSIGNED_INT, 0); 
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1,&VAO);
    glDeleteBuffers(1,&VBO);
    glfwTerminate();
    return 0;
}