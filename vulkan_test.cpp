// Illtyd Wynn, 8/26/2024, Vulkan Learning

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

#include <iostream>

using namespace std;

int main() {
  GLFWwindow* window;
  uint32_t extension_count;
  glm::mat4 matrix;
  glm::vec4 vec;

  glfwInit();
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  window = glfwCreateWindow(
    800,
    600,
    "Vulkan Window",
    NULL,
    NULL
  );

  extension_count = 0;
  vkEnumerateInstanceExtensionProperties(
    NULL,
    &extension_count,
    NULL
  );

  auto test = matrix * vec;
  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
