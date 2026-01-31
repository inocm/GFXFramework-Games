#include "lab_m1/Tema1/Scena2.h"
#include "lab_m1/Tema1/Tema1.h"
#include <vector>
#include <iostream>


#include "lab_m1/Tema1/object2D.h"
#include "lab_m1/Tema1/transform2D.h"

using namespace std;
using namespace m1;

/*
 *  To find out more about `FrameStart`, `Update`, `FrameEnd`
 *  and the order in which they are called, see `world.cpp`.
 */


Scena2::Scena2()
{ 
   
}


Scena2::~Scena2()
{
}

void Scena2::Init()
{
    glm::ivec2 resolution = window->GetResolution();
    auto camera = GetSceneCamera();
    camera->SetOrthographic(0, (float)resolution.x, 0, (float)resolution.y, 0.01f, 400);
    camera->SetPosition(glm::vec3(0, 0, 50));
    camera->SetRotation(glm::vec3(0, 0, 0));
    camera->Update();
    GetCameraInput()->SetActive(false);

    {
        glLineWidth(2.0f);

        // Colours
        glm::vec3 COLOUR = glm::vec3(0, 0, 0);
        glm::vec3 RED = glm::vec3(1, 0, 0);
        glm::vec3 BLUE = glm::vec3(0, 0, 1);
        glm::vec3 GREEN = glm::vec3(0, 1, 0);
        glm::vec3 BEIGE = glm::vec3(0.9f, 0.9f, 0.8f);
        glm::vec3 ORANGE = glm::vec3(1.0f, 0.65f, 0.0f);
        glm::vec3 DARK_ORANGE = glm::vec3(0.8f, 0.4f, 0.0f);
        glm::vec3 WHITE = glm::vec3(255, 255, 255);

        float SCREEN_WIDTH = 1280;
        float SCREEN_HEIGHT = 720;
        float RECTANGLE_WIDTH = 104;
        float RECTANGLE_HEIGHT = 60;
        float HALF_SCREEN_HEIGHT = 360;
        float LEFT_ARRANGEMENT = 5;
        float SPACING = 2;

        int WIDTH_RECTANGLES = 12;
        int HEIGHT_RECTANGLES = 5;

        for (int i = 0; i < WIDTH_RECTANGLES; i++) {
            float x = i * (RECTANGLE_WIDTH + SPACING) + LEFT_ARRANGEMENT;
            for (int j = 0; j < HEIGHT_RECTANGLES; j++) {
                switch (j) {
                case 0:
                    COLOUR = BEIGE;
                    break;
                case 1:
                    COLOUR = ORANGE;
                    break;
                case 2:
                    COLOUR = RED;
                    break;
                case 3:
                    COLOUR = GREEN;
                    break;
                case 4:
                    COLOUR = BLUE;
                    break;
                }
                float y = j * (RECTANGLE_HEIGHT + SPACING) + HALF_SCREEN_HEIGHT;
                std::string name = "rectangle_" + std::to_string(i) + std::to_string(j);
                Mesh* rectangle = object2D::CreateRectangle(name, glm::vec3(x, y, 0), RECTANGLE_WIDTH, RECTANGLE_HEIGHT, COLOUR, true);

                Block* block = new Block();
                block->mesh = rectangle;
                block->position.x = x;
                block->position.y = y;
                AddMeshToList(rectangle);
            }
        }

    }
}


void Scena2::FrameStart()
{
    // Clears the color buffer (using the previously set color) and depth buffer
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::ivec2 resolution = window->GetResolution();
    // Sets the screen area where to draw
    glViewport(0, 0, resolution.x, resolution.y);
}

void Scena2::Update(float deltaTimeSeconds)
{
    modelMatrix = glm::mat3(1);

    // Blue squares grid
    for (auto const& meshPair : meshes) {
        if (meshPair.first.find("rectangle_") == 0) {
            RenderMesh2D(meshPair.second, shaders["VertexColor"], modelMatrix);
        }
    }

    if (Tema1::combinedMesh)
        RenderMesh2D(Tema1::combinedMesh, shaders["VertexColor"], glm::mat3(1));
}


void Scena2::FrameEnd()
{
}


/*
 *  These are callback functions. To find more about callbacks and
 *  how they behave, see `input_controller.h`.
 */

void Scena2::OnInputUpdate(float deltaTime, int mods)
{

}


void Scena2::OnKeyPress(int key, int mods)
{
    
}


void Scena2::OnKeyRelease(int key, int mods)
{
    // Add key release event
}


void Scena2::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
   
}


void Scena2::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) 
{
   
}

void Scena2::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) {
    
}

void Scena2::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) 
{

}


void Scena2::OnWindowResize(int width, int height)
{

}


