#include "Horde3D/Bindings/C++/Horde3D.h"
#include "Horde3D/Bindings/C++/Horde3DUtils.h"
#include "project_glfw-prefix/include/GLFW/glfw3.h"

#include <iostream>

constexpr int winWidth = 800;
constexpr int winHeight = 600;

bool gameLoop(H3DNode& model, H3DNode& cam, float fps) {
  static float t = 0;

  // Increase animation time
  t = t + 10.0f * (1 / fps);
  std::cout << "Time is " << t << "\n";
  // Play animation
  h3dSetModelAnimParams(model, 0, t, 1.0f);
  std::cout << "Animation done\n";
  h3dUpdateModel(
      model, H3DModelUpdateFlags::Animation | H3DModelUpdateFlags::Geometry);
  std::cout << "Updated model\n";

  // Set new model position
  h3dSetNodeTransform(model, t * 10, 0, 0, // Translation
                      0, 0, 0,             // Rotation
                      1, 1, 1);            // Scale
  std::cout << "Set position\n";

  // Render scene
  h3dRender(cam);
  std::cout << "Rendered\n";

  // Finish rendering of frame
  h3dFinalizeFrame();
  std::cout << "Finalised\n";
  return t > 200;
}

int main(int argc, char** argv) {
  glfwSetErrorCallback([](int code, const char* description) {
      std::cout << "Code " << code << " " << description << "\n";
    });
  int init = glfwInit();
  auto* window = glfwCreateWindow(winWidth, winHeight, "Test", nullptr, nullptr);
  if (window == nullptr) {
    glfwTerminate();
    return 1;
  }
  glfwMakeContextCurrent(window);

  // Initialize engine.
  if (!h3dInit(H3DRenderDevice::OpenGL4)) {
    std::cout << "Failed to initialise\n";
    glfwTerminate();
    return 1;    
  }

  // Add pipeline resource
  H3DRes pipeRes = h3dAddResource(H3DResTypes::Pipeline,
                                  "C:\\Users\\Rolf\\base\\third_"
                                  "party\\Horde3D\\Horde3D\\Binaries\\Content\\pipel"
                                  "ines\\forward.pipeline.xml",
                                  0);

  // Add model resource
  H3DRes modelRes = h3dAddResource(
      H3DResTypes::SceneGraph,
      "C:\\Users\\Rolf\\base\\third_"
      "party\\Horde3D\\Horde3D\\Binaries\\Content\\models\\man\\man.scene.xml",
      0);
  // Add animation resource
  H3DRes animRes = h3dAddResource(
      H3DResTypes::Animation,
      "C:\\Users\\Rolf\\base\\third_"
      "party\\Horde3D\\Horde3D\\Binaries\\Content\\animations\\man.anim",
      0);
  // Load added resources
  h3dutLoadResourcesFromDisk("");

  // Add model to scene
  H3DNode model = h3dAddNodes(H3DRootNode, modelRes);
  // Apply animation
  h3dSetupModelAnimStage(model, 0, animRes, 0, "", false);

  // Add camera
  H3DNode cam = h3dAddCameraNode(H3DRootNode, "Camera", pipeRes);

  // Add light source
  H3DNode light =
      h3dAddLightNode(H3DRootNode, "Light1", 0, "LIGHTING", "SHADOWMAP");
  if (light == 0) {
    std::cout << "Could not create light\n";
    glfwTerminate();
    return 1;
  }

  // Set light position and radius
  h3dSetNodeTransform(light, 0, 20, 0, 0, 0, 0, 1, 1, 1);
  h3dSetNodeParamF(light, H3DLight::RadiusF, 0, 50.0f);

  // Setup viewport and render target sizes
  h3dSetNodeParamI(cam, H3DCamera::ViewportXI, 0);
  h3dSetNodeParamI(cam, H3DCamera::ViewportYI, 0);
  h3dSetNodeParamI(cam, H3DCamera::ViewportWidthI, winWidth);
  h3dSetNodeParamI(cam, H3DCamera::ViewportHeightI, winHeight);
  h3dSetupCameraView(cam, 45.0f, (float)winWidth / winHeight, 0.5f, 2048.0f);
  h3dResizePipelineBuffers(pipeRes, winWidth, winHeight);

  bool done = false;
  while (!done) {
    done = gameLoop(model, cam, 60.0);
  }
  std::cout << "Reached the end\n";
  // Release engine.
  h3dRelease();
  std::cout << "Released\n";
  glfwDestroyWindow(window);
  std::cout << "Destroyed\n";
  glfwTerminate();
  std::cout << "Terminated\n";
  return 0;
}
