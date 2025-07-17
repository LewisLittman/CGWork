#include "DrawingWindow.h"
#include "Scene.h"
#include "SceneLoader.h"
#include "Renderer.h"
#include "PointCloudRenderer.h"
#include "WireFrameRenderer.h"
#include "RasterisedRenderer.h"
#include "DepthBuffer.h"
#include "RayTraceRenderer.h"
#include "./other/Constants.h"
#include <memory>

void handleEvent(SDL_Event event, Scene &scene, std::unique_ptr<Renderer>& renderer) {
    if (event.type == SDL_QUIT) {
        std::cout << "Exiting the 3D Renderer..." << std::endl;
        exit(0);
    }
    
    // Handle keyboard events
  if (event.type == SDL_KEYDOWN) {
    if (event.key.keysym.sym == SDLK_LEFT) std::cout << "LEFT" << std::endl;
    else if (event.key.keysym.sym == SDLK_RIGHT) std::cout << "RIGHT" << std::endl;
    else if (event.key.keysym.sym == SDLK_UP) scene.camera.position.y += 0.1;
    else if (event.key.keysym.sym == SDLK_DOWN) scene.camera.position.y -= 0.1;
    else if (event.key.keysym.sym == SDLK_w) scene.camera.position.z -= 0.1;
    else if (event.key.keysym.sym == SDLK_s) scene.camera.position.z += 0.1;
    else if (event.key.keysym.sym == SDLK_a) scene.camera.position.x -= 0.1;
    else if (event.key.keysym.sym == SDLK_d) scene.camera.position.x += 0.1;
    else if (event.key.keysym.sym == SDLK_r) scene.camera.reset();
    else if (event.key.keysym.sym == SDLK_1) scene.camera.position = scene.camera.rot_y_axis(-PI/180) * scene.camera.position;
    else if (event.key.keysym.sym == SDLK_2) scene.camera.position = scene.camera.rot_y_axis(PI/180) * scene.camera.position;
    else if (event.key.keysym.sym == SDLK_3) scene.camera.position = scene.camera.rot_x_axis(-PI/180) * scene.camera.position;
    else if (event.key.keysym.sym == SDLK_4) scene.camera.position = scene.camera.rot_x_axis(PI/180) * scene.camera.position;
    else if (event.key.keysym.sym == SDLK_q) scene.camera.orientation = scene.camera.rot_y_axis(PI/180) * scene.camera.orientation;
    else if (event.key.keysym.sym == SDLK_e) scene.camera.orientation = scene.camera.rot_y_axis(-PI/180) * scene.camera.orientation;
    else if (event.key.keysym.sym == SDLK_z) scene.camera.orientation = scene.camera.rot_x_axis(PI/180) * scene.camera.orientation;
    else if (event.key.keysym.sym == SDLK_x) scene.camera.orientation = scene.camera.rot_x_axis(-PI/180) * scene.camera.orientation;
    // else if (event.key.keysym.sym == SDLK_o) orbiting = !orbiting;
    else if (event.key.keysym.sym == SDLK_v) { renderer = std::make_unique<PointCloudRenderer>(); std::cout << "Renderer: Point Cloud" << std::endl; }
    else if (event.key.keysym.sym == SDLK_b) { renderer = std::make_unique<WireFrameRenderer>(); std::cout << "Renderer: Wire Frame" << std::endl; }
    else if (event.key.keysym.sym == SDLK_n) { renderer = std::make_unique<RasterisedRenderer>(); std::cout << "Renderer: Rasterised" << std::endl; }
    else if (event.key.keysym.sym == SDLK_m) { renderer = std::make_unique<RayTraceRenderer>(); std::cout << "Renderer: Ray Trace" << std::endl; }
    // else if (event.key.keysym.sym == SDLK_n) { renderMode = 2; cout << "RenderMode: Ray Tracing" << endl; }
    // else if (event.key.keysym.sym == SDLK_y) { printCamValues(); }
    // else if (event.key.keysym.sym == SDLK_u) { lights[0] = vec3(0, 1.2, 0); }
    // else if (event.key.keysym.sym == SDLK_i) { lights[0] = vec3(1, 1.2, 0); }
//   } else if (event.type == SDL_MOUSEBUTTONDOWN) {
//     window.savePPM("output.ppm");
//     window.saveBMP("output.bmp");
  }
    
}

int main(int argc, char *argv[]) {
    std::cout << "Starting the 3D Renderer..." << std::endl;
    DrawingWindow window(640, 480, false);
    SDL_Event event;
    std::cout << "Window created with dimensions: " << window.width << "x" << window.height << std::endl;
    Scene scene = SceneLoader::loadScene();
    std::cout << "Scene loaded with " << scene.triangles.size() << " triangles." << std::endl;
    std::unique_ptr<Renderer> renderer = std::make_unique<WireFrameRenderer>();

    DepthBuffer depthBuffer(window.width, window.height);

    while (true) {
        if (window.pollForInputEvents(event)) handleEvent(event, scene, renderer);
        window.clearPixels();

        if (auto rasterisedRenderer = dynamic_cast<RasterisedRenderer*>(renderer.get())) {
            depthBuffer.reset();
            rasterisedRenderer->render(2.0f, window, scene, depthBuffer);
        } else {
            renderer->render(2.0f, window, scene);
        }
        window.renderFrame();
    }
    return 0;
}
