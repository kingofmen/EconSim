#include "Horde3D/Bindings/C++/Horde3D.h"
#include "Horde3D/Bindings/C++/Horde3DUtils.h"
#include "project_glfw-prefix/include/GLFW/glfw3.h"

#include <iostream>

constexpr int winWidth = 800;
constexpr int winHeight = 600;

bool gameLoop(H3DNode& model, H3DNode& cam, float fps, GLFWwindow* winHandle) {
  static float t = 0;

  // Increase animation time
  t = t + 10.0f * (1 / fps);
  // Play animation
  /*
  h3dSetModelAnimParams(model, 0, t, 1.0f);
  h3dUpdateModel(
      model, H3DModelUpdateFlags::Animation | H3DModelUpdateFlags::Geometry);
  */
  // Set new model position
  h3dSetNodeTransform(model, 0, 0, 0, // Translation
                      0, t, 0,             // Rotation
                      0.1, 0.1, 0.1);            // Scale

  // Render scene
  h3dRender(cam);

  // Finish rendering of frame
  h3dFinalizeFrame();
  glfwSwapBuffers(winHandle);
  //return false;
  return t > 60;
}

class Terminator {
 public:
  Terminator() {}
  ~Terminator() {
    glfwTerminate();
  }
};

int main(int argc, char** argv) {
  glfwSetErrorCallback([](int code, const char* description) {
      std::cout << "Code " << code << " " << description << "\n";
    });
  int init = glfwInit();
  Terminator terminator;
  auto* window = glfwCreateWindow(winWidth, winHeight, "Test", nullptr, nullptr);
  if (window == nullptr) {
    std::cout << "Could not create window\n";
    return 1;
  }
  glfwMakeContextCurrent(window);

  // Initialize engine.
  if (!h3dInit(H3DRenderDevice::OpenGL4)) {
    std::cout << "Failed to initialise\n";
    return 1;    
  }
  h3dSetOption(H3DOptions::LoadTextures, 1);

  // Add pipeline resource.
  H3DRes pipeRes = h3dAddResource(H3DResTypes::Pipeline,
                                  "pipelines\\forward.pipeline.xml", 0);
  H3DRes materialRes = h3dAddResource(H3DResTypes::Material,
                                      "materials/worldmap.material.xml", 0);
  // Add model resource.
  /*
  H3DRes modelRes =
      h3dAddResource(H3DResTypes::SceneGraph, "models\\World.scene.xml", 0);
  */
  float vertices[] = {
    0, 0, 0,
    20, 0, 0,
    0, 20, 0,
    20, 20, 0
  };
  unsigned int triangles[] = {0, 1, 2, 1, 3, 2};
  float texCoords[] = {0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 1.0, 1.0};
  H3DRes modelRes = h3dutCreateGeometryRes(
      "World", 4, 6, vertices, triangles, nullptr, nullptr, nullptr, texCoords, nullptr);
  if (modelRes == 0) {
    std::cout << "Could not create model resource\n";
    return 1;    
  }

  // Load added resources
  if (!h3dutLoadResourcesFromDisk("c:\\Users\\Rolf\\base\\naval\\graphics|C:"
                                  "\\Users\\Rolf\\base\\third_"
                                  "party\\Horde3D\\Binaries\\Content")) {
    std::cout << "Could not load resources\n";
    return 1;
  }

  // Add model to scene
  //H3DNode model = h3dAddNodes(H3DRootNode, modelRes);
  H3DNode model = h3dAddModelNode(H3DRootNode, "World", modelRes);
  if (model == 0) {
    std::cout << "Could not add model node\n";
    return 1;
  }
  H3DNode mesh = h3dAddMeshNode(model, "sphere", materialRes, 0, 6, 0, 3);
  if (mesh == 0) {
    std::cout << "Could not add mesh node\n";
    return 1;
  }


  // Draw at origin.
  h3dSetNodeTransform( model, 0, -3, 0, 0, 0, 0, 0.1, 0.1, 0.1 );

  // Add camera
  H3DNode cam = h3dAddCameraNode(H3DRootNode, "Camera", pipeRes);
  // 20 units out of the screen, look straight at origin.
  h3dSetNodeTransform(cam, 0, 0, 20, 0, 0, 0, 1, 1, 1);


  /*
  // Add light source
  H3DNode light =
      h3dAddLightNode(H3DRootNode, "Light1", 0, "LIGHTING", "SHADOWMAP");
  if (light == 0) {
    std::cout << "Could not create light\n";
    return 1;
  }

  // Set light position and radius
  h3dSetNodeTransform(light, 0, 20, 0, 0, 0, 0, 1, 1, 1);
  h3dSetNodeParamF(light, H3DLight::RadiusF, 0, 50.0f);
  */

  // Setup viewport and render target sizes
  h3dSetNodeParamI(cam, H3DCamera::ViewportXI, 0);
  h3dSetNodeParamI(cam, H3DCamera::ViewportYI, 0);
  h3dSetNodeParamI(cam, H3DCamera::ViewportWidthI, winWidth);
  h3dSetNodeParamI(cam, H3DCamera::ViewportHeightI, winHeight);
  h3dSetupCameraView(cam, 45.0f, (float)winWidth / winHeight, 0.5f, 2048.0f);
  h3dResizePipelineBuffers(pipeRes, winWidth, winHeight);

  bool done = false;
  while (!done) {
    done = gameLoop(model, cam, 60.0, window);
  }
  if (!h3dutDumpMessages()) {
    std::cout << "Could not dump log\n";
  }
  // Release engine.
  h3dRelease();
  glfwDestroyWindow(window);
  return 0;
}
