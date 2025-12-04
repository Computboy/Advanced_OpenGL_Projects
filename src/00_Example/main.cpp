#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "camera.h"
#include "model.h"

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
// 帧缓冲窗口大小调整函数
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
// 鼠标移动函数
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
// 鼠标滚轮反馈
void processInput(GLFWwindow* window);
// 响应用户键盘输入的函数
void togglePolygonModeWithSpace(GLFWwindow* window);  // 切换绘制模式（线框或填充模式）
GLuint loadCubemap(vector<std::string> faces);
// 加载天空盒纹理的封装函数

// 视口大小
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// 设置摄像机的初始配置属性
float StandardYAxis = -0.0f;  // [Y轴基准设定]
Camera camera(glm::vec3(0.0f, StandardYAxis + 0.5f, 4.0f));

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// 设置deltaTime以平衡不同帧率的电脑体验
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// 绘制模式[按空格键改变线框模式/填充模式]
bool isWireframe = false;  // 初始为填充模式

vector<string> textures_faces;

// 全局缩放比例
float scaleBound = 2.0f;

int main()
{
    // 初始化glfw并配置
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // 窗口创建与配置
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Model_Importing", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // 隐藏鼠标并始终捕捉光标
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // GLAD: 用来管理GLFW函数指针的，因此在调用任何GLFW函数之前都应该先初始化GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // 开启多重采样抗锯齿
    glEnable(GL_MULTISAMPLE);

    // 在模型model.h中删去aiProcess_FlipUVs这一句，保留.cpp文件中的全局翻转
    stbi_set_flip_vertically_on_load(true);

    // 开启深度测试
    glEnable(GL_DEPTH_TEST);

    // 一句话构建并编译自己的着色器
    Shader ModelShader("Shader/00_Example/Shader.vs", "Shader/00_Example/Shader.fs");
    Shader skyboxShader("Shader/00_Example/SkyBox.vs", "Shader/00_Example/SkyBox.fs");

    // 加载网格与模型
    Model ourModel("Model/hotdog_car/HotDog_Car.obj");
    Model CottageHouse("Model/Cottage/cottage_obj.obj");
    Model GrassLand("Model/GrassLand/10450_Rectangular_Grass_Patch_v1_iterations-2.obj");

    // 绘制天空盒[纹理设置+贴图绑定与设置]
    textures_faces.push_back("Texture/skybox/right.jpg");
    textures_faces.push_back("Texture/skybox/left.jpg");
    textures_faces.push_back("Texture/skybox/top.jpg");
    textures_faces.push_back("Texture/skybox/bottom.jpg");
    textures_faces.push_back("Texture/skybox/front.jpg");
    textures_faces.push_back("Texture/skybox/back.jpg");
    GLuint cubemapTexture = loadCubemap(textures_faces);

    float skyboxVertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };
    // 天空盒的顶点属性
    GLuint skyboxVAO, skyboxVBO;
    glGenBuffers(1, &skyboxVBO);
    glGenVertexArrays(1, &skyboxVAO);

    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    // 配置SkyBox的VAO, VBO

    // 使用天空盒着色器绘制，并设置Uniform纹理
    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);

    // 渲染循环
    while (!glfwWindowShouldClose(window))
    {
        // ===== 1. 每帧运行时间逻辑 =====
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // ===== 2. 处理用户键入 =====
        processInput(window);

        // ===== 3. 渲染 =====
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        togglePolygonModeWithSpace(window);
        // 按空格键来调整绘制模式 → 线框/填充模式

        // ===== 4. 使用模型着色器 =====
        ModelShader.use();

        // ===== 5. 向着色器中传递变换矩阵信息 =====
        glm::mat4 projection = glm::perspective(
            glm::radians(camera.Zoom),
            (float)SCR_WIDTH / (float)SCR_HEIGHT,
            0.1f, 500.0f
        );
        glm::mat4 view = camera.GetViewMatrix();
        ModelShader.setMat4("projection", projection);
        ModelShader.setMat4("view", view);

        // ===== 6. 以摄像机为手电筒 =====
        ModelShader.setVec3("viewPos", camera.Position);

        // 摄像机位置 & 方向 = 手电筒
        ModelShader.setVec3("spot.position", camera.Position);
        ModelShader.setVec3("spot.direction", camera.Front);

        // 聚光内部 & 外部夹角（带柔性边缘）
        ModelShader.setFloat("spot.cutOff", glm::cos(glm::radians(7.5f)));
        ModelShader.setFloat("spot.outerCutOff", glm::cos(glm::radians(12.5f)));

        // 光照颜色
        ModelShader.setVec3("spot.ambient", glm::vec3(0.05f));
        ModelShader.setVec3("spot.diffuse", glm::vec3(0.9f));
        ModelShader.setVec3("spot.specular", glm::vec3(1.0f));

        // 衰减系数
        ModelShader.setFloat("spot.constant", 1.0f);
        ModelShader.setFloat("spot.linear", 0.09f);
        ModelShader.setFloat("spot.quadratic", 0.032f);

        // 方向光系数设置
        ModelShader.setVec3("dirLight.direction", -1.0f, -1.0f, 0.0f);
        ModelShader.setVec3("dirLight.ambient", glm::vec3(0.4f));
        ModelShader.setVec3("dirLight.diffuse", glm::vec3(0.6f));
        ModelShader.setVec3("dirLight.specular", glm::vec3(0.8f));

        // ===== 7. 渲染外部模型 =====

        // 热狗车 HotDog_Car
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-2.0f, StandardYAxis + 0.7f, 0.0f));
        model = glm::scale(model, glm::vec3(1.0f * scaleBound));
        ModelShader.setMat4("model", model);
        glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(model)));
        ModelShader.setMat3("NormalMatrix", normalMatrix);

        ourModel.Draw(ModelShader);

        // 残破的木屋 Cottage House
        glm::mat4 model2 = glm::mat4(1.0f);
        model2 = glm::translate(model2, glm::vec3(2.0f, StandardYAxis - 1.3f, 0.0f));
        model2 = glm::rotate(model2, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model2 = glm::rotate(model2, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        model2 = glm::scale(model2, glm::vec3(0.15f * scaleBound));
        ModelShader.setMat4("model", model2);
        normalMatrix = glm::transpose(glm::inverse(glm::mat3(model2)));
        ModelShader.setMat3("NormalMatrix", normalMatrix);

        CottageHouse.Draw(ModelShader);

        // 草皮 GrassLand
        glm::mat4 model3 = glm::mat4(1.0f);
        model3 = glm::translate(model3, glm::vec3(0.0f, StandardYAxis - 0.7f, -1.0f));
        model3 = glm::rotate(model3, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model3 = glm::scale(model3, glm::vec3(0.2f * scaleBound, 0.2f * scaleBound, 0.07f * scaleBound));
        ModelShader.setMat4("model", model3);
        normalMatrix = glm::transpose(glm::inverse(glm::mat3(model3)));
        ModelShader.setMat3("NormalMatrix", normalMatrix);

        GrassLand.Draw(ModelShader);

        // ===== 8. 最后进行天空盒渲染 =====
        glDepthFunc(GL_LEQUAL);  // 深度缓冲函数
        skyboxShader.use();
        glm::mat4 modelofsky = glm::mat4(1.0f);
        modelofsky = glm::translate(modelofsky, camera.Position * -0.02f);
        skyboxShader.setMat4("model", modelofsky);
        view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // remove translation from the view matrix
        skyboxShader.setMat4("view", view);
        skyboxShader.setMat4("projection", projection);
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); // 还原为默认值

        // ===== 9. 交换缓冲区+处理事件 =====
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteBuffers(1, &skyboxVBO);
    glDeleteVertexArrays(1, &skyboxVAO);
    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

void togglePolygonModeWithSpace(GLFWwindow* window) {
    static bool pressed = false;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        // 使用静态变量避免按键期间重复切换（简单的按键防抖）
        if (!pressed) {
            isWireframe = !isWireframe;  // 切换模式状态
            if (isWireframe) {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);  // 线框模式
            }
            else {
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);  // 填充模式
            }
            pressed = true;
        }
    }
    else {
        // 按键释放时重置状态
        pressed = false;
    }
}

GLuint loadCubemap(vector<std::string> faces) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    unsigned char* data;

    for (unsigned int i = 0; i < faces.size(); i++)
    {    
        stbi_set_flip_vertically_on_load(false);  
        data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
            );
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    stbi_set_flip_vertically_on_load(true);

    return textureID;
}