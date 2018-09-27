#include "Horde3D/Bindings/C++/Horde3D.h"
#include "Horde3D/Bindings/C++/Horde3DUtils.h"
#include "project_glfw-prefix/include/GLFW/glfw3.h"

#include <cmath>
#include <iostream>

constexpr int winWidth = 800;
constexpr int winHeight = 600;
constexpr float kPi_f = 3.14159265f;
constexpr float kDegToRad = kPi_f / 180;

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
                      0, 0, 0,             // Rotation
                      //0.1, 0.1, 0.1);            // Scale
                      1, 1, 1);

  // Render scene
  h3dRender(cam);

  // Finish rendering of frame
  h3dFinalizeFrame();
  glfwSwapBuffers(winHandle);
  return false;
  //return t > 60;
}

class Terminator {
 public:
  Terminator() {}
  ~Terminator() {
    glfwTerminate();
  }
};

constexpr float kEarthRadius = 200.0f;

// Creates a geometry resource describing a "square" sector of a sphere, from d1
// degrees west of prime and d2 degrees north, to d3 and d4, with a vertex
// density of one per minute.
H3DRes createSphereSquareGeometry(float d1, float d2, float d3, float d4) {
  constexpr int kVertsPerDegree = 1;
  constexpr float kVertexStep = 1.0f / kVertsPerDegree;

  int numXVertices = 1 + (int) floor((d3 - d1) * kVertsPerDegree);
  if (numXVertices < 2) {
    std::cout << "Bad x coordinates: " << d1 << " - " << d3 << "\n";
    return 0;
  }
  int numYVertices = 1 + (int) floor((d4 - d2) * kVertsPerDegree);
  if (numYVertices < 2) {
    std::cout << "Bad y coordinates: " << d2 << " - " << d4 << "\n";
    return 0;
  }

  float* vertices = new float[3 * numXVertices * numYVertices];
  float* texCoords = new float[2 * numXVertices * numYVertices];
  for (int ii = 0; ii < numXVertices; ++ii) {
    float xDeg = d1 + ii * kVertexStep;
    if (xDeg > d3) xDeg = d3;
    for (int jj = 0; jj < numYVertices; ++jj) {
      float yDeg = d2 + jj * kVertexStep;
      if (yDeg > d4) yDeg = d4;

      float xRad = xDeg * kDegToRad;
      float yRad = yDeg * kDegToRad;

      int startIndex = 3 * (numXVertices * jj + ii);
      vertices[startIndex + 0] = kEarthRadius * cos(yRad) * cos(xRad);
      vertices[startIndex + 1] = kEarthRadius * sin(yRad);
      vertices[startIndex + 2] = -kEarthRadius * cos(yRad) * sin(xRad);
      /*
      vertices[startIndex + 0] = kEarthRadius * cos(yRad);
      vertices[startIndex + 1] = kEarthRadius * sin(yRad);
      vertices[startIndex + 2] = 0;
      */

      std::cout << "(" << vertices[startIndex + 0] << ", " << vertices[startIndex + 1] << ", " << vertices[startIndex + 2] << ")\n";
      startIndex = 2 * (numXVertices * jj + ii);
      texCoords[startIndex + 0] = (xDeg - d1) / (d3 - d1);
      texCoords[startIndex + 1] = (yDeg - d2) / (d4 - d2);
    }
  }

  int numXSquares = (numXVertices - 1);
  int numYSquares = (numYVertices - 1);
  int numTriangles = 2 * numXSquares * numYSquares;
  unsigned int* triangles = new unsigned int[3 * numTriangles];
  for (int ii = 0; ii < numXSquares; ++ii) {
    for (int jj = 0; jj < numYSquares; ++jj) {
      int startIndex = 2 * 3 * (numXSquares * jj + ii);
      unsigned int startVertex = numXVertices * jj + ii;
      triangles[startIndex + 0] = startVertex;
      triangles[startIndex + 1] = startVertex + numXVertices;
      triangles[startIndex + 2] = startVertex + 1;
      triangles[startIndex + 3] = startVertex + 1;
      triangles[startIndex + 4] = startVertex + numXVertices;
      // Maximum value of this works out to (numXVertices * numYVertices - 1).
      triangles[startIndex + 5] = startVertex + 1 + numXVertices;
      std::cout << "("
                << vertices[triangles[startIndex + 0] * 3 + 0] << ", "
                << vertices[triangles[startIndex + 0] * 3 + 1] << ", "
                << vertices[triangles[startIndex + 0] * 3 + 2] << ") ("

                << vertices[triangles[startIndex + 1] * 3 + 0] << ", "
                << vertices[triangles[startIndex + 1] * 3 + 1] << ", "
                << vertices[triangles[startIndex + 1] * 3 + 2] << ") ("

                << vertices[triangles[startIndex + 2] * 3 + 0] << ", "
                << vertices[triangles[startIndex + 2] * 3 + 1] << ", "
                << vertices[triangles[startIndex + 2] * 3 + 2] << ")\n";
    }
  }

  return h3dutCreateGeometryRes(
      "MapSector", numXVertices * numYVertices, numTriangles, vertices,
      triangles, nullptr, nullptr, nullptr, texCoords, nullptr);
}

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
  // Load added resources.
  if (!h3dutLoadResourcesFromDisk("c:\\Users\\Rolf\\base\\naval\\graphics|C:"
                                  "\\Users\\Rolf\\base\\third_"
                                  "party\\Horde3D\\Binaries\\Content")) {
    std::cout << "Could not load resources\n";
    return 1;
  }

  // Add model resource.
  /*
  H3DRes modelRes =
      h3dAddResource(H3DResTypes::SceneGraph, "models\\World.scene.xml", 0);
  */
  //#define MAP 1
#ifdef MAP
  float vertices[] = {
    0, 0, 0,
    20, 0, 0,
    0, 20, 0,
    20, 20, 0
  };
  unsigned int triangles[] = {0, 1, 2, 1, 3, 2};
  float texCoords[] = {0.0, 0.0, 1.0, 0.0, 0.0, 1.0, 1.0, 1.0};
  H3DRes mapRes = h3dutCreateGeometryRes(
      "World", 4, 6, vertices, triangles, nullptr, nullptr, nullptr, texCoords, nullptr);
  if (mapRes == 0) {
    std::cout << "Could not create map resource\n";
    return 1;    
  }
  H3DNode map = h3dAddModelNode(H3DRootNode, "World", mapRes);
  H3DNode map_mesh = h3dAddMeshNode(map, "sphere", materialRes, 0, 6, 0, 3);
  h3dSetNodeTransform( map, 0, 0, 0, 0, 0, 0, 1, 1, 1 );
#endif

  // Add model to scene
  //H3DNode model = h3dAddNodes(H3DRootNode, modelRes);
#define SPHERE 1
#ifdef SPHERE
  H3DNode modelRes = createSphereSquareGeometry(270, 0, 271, 1);
  if (modelRes == 0) {
    std::cout << "Could not create geometry\n";
    return 1;
  }
  H3DNode model = h3dAddModelNode(H3DRootNode, "World", modelRes);
  if (model == 0) {
    std::cout << "Could not add model node\n";
    return 1;
  }
  H3DNode mesh = h3dAddMeshNode(model, "sphere", materialRes, 0, 7200, 0, 3720);
  if (mesh == 0) {
    std::cout << "Could not add mesh node\n";
    return 1;
  }
#endif
  // Draw at origin.
  //h3dSetNodeTransform( model, 0, 0, 0, 0, 0, 0, 1, 1, 1 );

  // Add camera
  H3DNode cam = h3dAddCameraNode(H3DRootNode, "Camera", pipeRes);
  // 20 units out of the screen, look straight at origin.
  h3dSetNodeTransform(cam, 0, 0, 201, 0, 0, 0, 1, 1, 1);


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
#ifdef MAP
    done = gameLoop(map, cam, 60.0, window);
#endif
#ifdef SPHERE
    done = gameLoop(model, cam, 60.0, window);
#endif
  }
  if (!h3dutDumpMessages()) {
    std::cout << "Could not dump log\n";
  }
  // Release engine.
  h3dRelease();
  glfwDestroyWindow(window);
  return 0;
}
