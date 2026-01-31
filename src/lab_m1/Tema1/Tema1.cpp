#include "lab_m1/Tema1/Tema1.h"

#include <vector>
#include <iostream>
#include <set>
#include <queue>
#include <utility> 

#include "lab_m1/Tema1/object2D.h"
#include "lab_m1/Tema1/transform2D.h"



using namespace std;
using namespace m1;

/*
 *  To find out more about `FrameStart`, `Update`, `FrameEnd`
 *  and the order in which they are called, see `world.cpp`.
 */


Tema1::Tema1()
{ 
    dragMesh = new Block();
}


Tema1::~Tema1()
{
}

Mesh* CombineMeshes(const std::string& name, const std::vector<Mesh*>& parts, const std::vector<glm::mat3>& transforms)
{
    Mesh* combined = new Mesh(name);
    int offset = 0;

    for (size_t i = 0; i < parts.size(); i++) {
        Mesh* mesh = parts[i];
        glm::mat3 transform = transforms[i];

        for (auto vertex : mesh->vertices) {
            glm::vec3 pos = vertex.position;
            glm::vec3 transformed = transform * glm::vec3(pos.x, pos.y, 1);
            vertex.position = glm::vec3(transformed.x, transformed.y, 0);
            combined->vertices.push_back(vertex);
        }

        for (auto index : mesh->indices) {
            combined->indices.push_back(index + offset);
        }

        offset += mesh->vertices.size();
    }

    combined->InitFromData(combined->vertices, combined->indices);
    return combined;
}

void Tema1::Init()
{
    glm::ivec2 resolution = window->GetResolution();
    auto camera = GetSceneCamera();
    camera->SetOrthographic(0, (float)resolution.x, 0, (float)resolution.y, 0.01f, 400);
    camera->SetPosition(glm::vec3(0, 0, 50));
    camera->SetRotation(glm::vec3(0, 0, 0));
    camera->Update();
    GetCameraInput()->SetActive(false);

    windowHeight = window->GetResolution().y;
    windowWidth = window->GetResolution().x;

    // Paddles
    paddleLeftPos.x = 300;
    paddleLeftPos.y = 360;
    paddleRightPos.x = 980;
    paddleRightPos.y = 360;

    constraints = false;

    // Score
    leftScore = 0;
    rightScore = 0;
    leftPlayerLost = true;
    bool draggingExistingBlock = false;


    // Ball
    ballRadius = 12;
    ballSpeed = 600;
    ballPos = glm::vec2(windowWidth / 2, windowHeight / 2);
    ResetBall();

    

    text = new gfxc::TextRenderer(
        window->props.selfDir,
        windowWidth,
        windowHeight
    );

    text->Load("assets/fonts/Hack-Bold.ttf", 50);

    {
        glLineWidth(2.0f);

        // Colours
        glm::vec3 COLOUR = glm::vec3(0, 0, 0);
        glm::vec3 RED = glm::vec3(1, 0, 0);
        glm::vec3 BLUE = glm::vec3(0, 0, 1);
        glm::vec3 GREEN = glm::vec3(0, 1, 0);
        glm::vec3 LIGHT_BLUE = glm::vec3(0.678f, 0.847f, 0.950f);
        glm::vec3 BEIGE = glm::vec3(0.9f, 0.9f, 0.8f);
        glm::vec3 ORANGE = glm::vec3(1.0f, 0.65f, 0.0f);
        glm::vec3 DARK_ORANGE = glm::vec3(0.8f, 0.4f, 0.0f);
        glm::vec3 LIGHT_GRAY = glm::vec3(0.7f, 0.7f, 0.7f);
        glm::vec3 MEDIUM_GRAY = glm::vec3(0.6f, 0.6f, 0.6f);
        glm::vec3 GRAY = glm::vec3(0.5f, 0.5f, 0.5f);
        glm::vec3 DARK_GRAY = glm::vec3(0.1f, 0.1f, 0.1f);
        glm::vec3 DARK_BLUE = glm::vec3(0.0f, 0.0f, 0.545f);

        float RED_BORDER_HEIGHT = 180;
        float RED_BORDER_WIDTH = 256;
        float SCREEN_WIDTH = 1280;
        float SCREEN_HEIGHT = 720;

        // Screen origin
        glm::vec3 ORIGIN = glm::vec3(0, 0, 0);
        // Red squares border
        glm::vec3 ORIGIN2 = ORIGIN + glm::vec3(0, RED_BORDER_HEIGHT * 1, 0);
        glm::vec3 ORIGIN3 = ORIGIN + glm::vec3(0, RED_BORDER_HEIGHT * 2, 0);
        glm::vec3 ORIGIN4 = ORIGIN + glm::vec3(0, RED_BORDER_HEIGHT * 3, 0);
        // Blue border
        glm::vec3 ORIGIN5 = glm::vec3(301, 36, 0);
        // Solid block
        glm::vec3 ORIGIN6 = glm::vec3(107, 70, 0);
        // Bumper block
        glm::vec3 ORIGIN7 = glm::vec3(107, 575, 0);
        // Engine block
        glm::vec3 ORIGIN8 = glm::vec3(107, 270, 0);
        // Canon base square
        glm::vec3 ORIGIN9 = glm::vec3(107, 380, 0);
        // Canon pipe
        glm::vec3 ORIGIN10 = glm::vec3(113.75f, 420, 0);
        // Start button
        glm::vec3 ORIGIN11 = glm::vec3(1215, 620, 0);
        // Engine flame
        glm::vec3 ORIGIN12 = glm::vec3(107, 240, 0);
        // Canon top
        glm::vec3 ORIGIN13 = glm::vec3(112.25f, 500, 0);

        // Red screen border
        Mesh* screenBorder = object2D::CreateRectangle("screenBorder", ORIGIN, SCREEN_WIDTH, SCREEN_HEIGHT, RED, false);

        // Red borders
        Mesh* miniSquare1 = object2D::CreateRectangle("miniSquare1", ORIGIN, RED_BORDER_WIDTH, RED_BORDER_HEIGHT, RED, false);
        Mesh* miniSquare2 = object2D::CreateRectangle("miniSquare2", ORIGIN2, RED_BORDER_WIDTH, RED_BORDER_HEIGHT, RED, false);
        Mesh* miniSquare3 = object2D::CreateRectangle("miniSquare3", ORIGIN3, RED_BORDER_WIDTH, RED_BORDER_HEIGHT, RED, false);
        Mesh* miniSquare4 = object2D::CreateRectangle("miniSquare4", ORIGIN4, RED_BORDER_WIDTH, RED_BORDER_HEIGHT, RED, false);

        // Blue border
        Mesh* blueBorder = object2D::CreateRectangle("blueBorder", ORIGIN5, 934, 510, BLUE, false);

        // Blue squares grid
        float BLUE_SQUARE_SIDE = 30;   
        float SPACING_BLUE = 20;     
        int ROWS = 9;
        int COLS = 17;
        int ORIGIN13x = 351;
        int ORIGIN13y = 73;
        
        for (int i = 0; i < ROWS; i++) {
            for (int j = 0; j < COLS; j++) {
                float x = j * (BLUE_SQUARE_SIDE + SPACING_BLUE) + ORIGIN13x;
                float y = i * (BLUE_SQUARE_SIDE + SPACING_BLUE) + ORIGIN13y;

                std::string name = "blueSquare_" + std::to_string(i) + "_" + std::to_string(j);

                Mesh* square = object2D::CreateSquare(name, glm::vec3(x, y, 0), BLUE_SQUARE_SIDE, LIGHT_BLUE, true);

                AddMeshToList(square);
            }
        }

        // Green squares line
        float MAIN_SQUARE_SIDE = 50;
        float SPACING_GREEN = 40;
        NR_GREEN_SQUARES = 10;

        for (int j = 0; j < NR_GREEN_SQUARES; j++) {
            float x = j * (MAIN_SQUARE_SIDE + SPACING_GREEN) + 315;

            std::string name = "greenSquare_" + std::to_string(j);

            Mesh* square = object2D::CreateSquare(name, glm::vec3(x, 620, 0), MAIN_SQUARE_SIDE, GREEN, true);

            AddMeshToList(square);
        }

        // Solid block
        Mesh* solidBlock = object2D::CreateRectangle("solidBlock", ORIGIN6, MAIN_SQUARE_SIDE, MAIN_SQUARE_SIDE, GRAY, true);

        // Bumper base
        Mesh* bumperBase = object2D::CreateRectangle("bumperBase", ORIGIN7, MAIN_SQUARE_SIDE, MAIN_SQUARE_SIDE, LIGHT_GRAY, true);

        // Bumper semicircle
        glm::vec2 SEMICIRCLE_CENTER1 = glm::vec2(132, 625); 
        float SEMICIRCLE_RADIUS1 = 25;
        int segments = 30; 

        Mesh* bumperSemicircle = object2D::CreateSemiCircle("bumperSemicircle", SEMICIRCLE_CENTER1, SEMICIRCLE_RADIUS1, segments, BEIGE);

        // Bumper
        glm::mat3 bumperBaseMat = glm::mat3(1);
        glm::mat3 bumperSemiMat = glm::mat3(1);
        bumperSemiMat *= transform2D::Translate(132, 625);
        bumperSemiMat *= transform2D::Scale(3.0f, 2.0f);
        bumperSemiMat *= transform2D::Translate(-132, -625);

        Mesh* bumper = CombineMeshes("bumper",
            { bumperBase, bumperSemicircle },
            { bumperBaseMat, bumperSemiMat });

        AddMeshToList(bumper);

        // Canon base block
        Mesh* canonBase = object2D::CreateRectangle("canonBase", ORIGIN9, MAIN_SQUARE_SIDE, 25, MEDIUM_GRAY, true);
        // Canon pipe
        Mesh* canonPipe = object2D::CreateRectangle("canonPipe", ORIGIN10, 36, 2 * MAIN_SQUARE_SIDE - 10, DARK_GRAY, true);
        // Canon top
        Mesh* canonTop = object2D::CreateRectangle("canonTop", ORIGIN13, 40, 10, DARK_GRAY, true);
        // Canon semicircle base
        glm::vec2 center2 = glm::vec2(132, 405);
        float radius2 = 25;
        Mesh* canonSemiCircle = object2D::CreateSemiCircle("canonSemiCircle", center2, radius2, segments, MEDIUM_GRAY);

        // Canon
        glm::mat3 canonMat = glm::mat3(1);
        Mesh* canon = CombineMeshes("canon",
            { canonBase, canonSemiCircle, canonPipe, canonTop },
            { canonMat, canonMat, canonMat, canonMat });

        // Green start button
        std::vector<VertexFormat> verticesGreen;
        std::vector<unsigned int> indicesGreen;

        verticesGreen.push_back(VertexFormat(ORIGIN11 + glm::vec3(0, 0, 0), GREEN));
        verticesGreen.push_back(VertexFormat(ORIGIN11 + glm::vec3(50, 0, 0), GREEN));
        verticesGreen.push_back(VertexFormat(ORIGIN11 + glm::vec3(50, 50, 0), GREEN));
        verticesGreen.push_back(VertexFormat(ORIGIN11 + glm::vec3(0, 50, 0), GREEN));
        verticesGreen.push_back(VertexFormat(ORIGIN11 + glm::vec3(30, 25, 0), GREEN));

        indicesGreen = {
            0, 1, 4,
            0, 4, 3,
            4, 2, 3
        };

        Mesh* greenStartButton = new Mesh("greenStartButton");
        greenStartButton->InitFromData(verticesGreen, indicesGreen);
     
        // Red start button
        std::vector<VertexFormat> vertices;
        std::vector<unsigned int> indices;

        vertices.push_back(VertexFormat(ORIGIN11 + glm::vec3(0, 0, 0), RED));
        vertices.push_back(VertexFormat(ORIGIN11 + glm::vec3(50, 0, 0), RED));
        vertices.push_back(VertexFormat(ORIGIN11 + glm::vec3(50, 50, 0), RED));
        vertices.push_back(VertexFormat(ORIGIN11 + glm::vec3(0, 50, 0), RED));
        vertices.push_back(VertexFormat(ORIGIN11 + glm::vec3(30, 25, 0), RED));

        indices = {
            0, 1, 4,
            0, 4, 3,
            4, 2, 3
        };

        Mesh* redStartButton = new Mesh("redStartButton");
        redStartButton->InitFromData(vertices, indices);

        // Engine flame
        std::vector<VertexFormat> vertices2;
        std::vector<unsigned int> indices2;

        vertices2.push_back(VertexFormat(ORIGIN12 + glm::vec3(0, 0, 0), DARK_ORANGE));
        vertices2.push_back(VertexFormat(ORIGIN12 + glm::vec3(50, 0, 0), DARK_ORANGE));
        vertices2.push_back(VertexFormat(ORIGIN12 + glm::vec3(50, 50, 0), DARK_ORANGE));
        vertices2.push_back(VertexFormat(ORIGIN12 + glm::vec3(0, 50, 0), DARK_ORANGE));

        vertices2.push_back(VertexFormat(ORIGIN12 + glm::vec3(0, 0, 0), DARK_ORANGE));
        vertices2.push_back(VertexFormat(ORIGIN12 + glm::vec3(12.5f, 0, 0), DARK_ORANGE));
        vertices2.push_back(VertexFormat(ORIGIN12 + glm::vec3(6.25f, -15, 0), DARK_ORANGE));

        vertices2.push_back(VertexFormat(ORIGIN12 + glm::vec3(12.5f, 0, 0), DARK_ORANGE));
        vertices2.push_back(VertexFormat(ORIGIN12 + glm::vec3(25, 0, 0), DARK_ORANGE));
        vertices2.push_back(VertexFormat(ORIGIN12 + glm::vec3(18.75f, -15, 0), DARK_ORANGE));

        vertices2.push_back(VertexFormat(ORIGIN12 + glm::vec3(25, 0, 0), DARK_ORANGE));
        vertices2.push_back(VertexFormat(ORIGIN12 + glm::vec3(37.5f, 0, 0), DARK_ORANGE));
        vertices2.push_back(VertexFormat(ORIGIN12 + glm::vec3(31.25f, -15, 0), DARK_ORANGE));

        vertices2.push_back(VertexFormat(ORIGIN12 + glm::vec3(37.5f, 0, 0), DARK_ORANGE));
        vertices2.push_back(VertexFormat(ORIGIN12 + glm::vec3(50, 0, 0), DARK_ORANGE));
        vertices2.push_back(VertexFormat(ORIGIN12 + glm::vec3(43.75f, -15, 0), DARK_ORANGE));

        indices2 = { 0, 1, 2, 0, 2, 3 };
        indices2.insert(indices2.end(), { 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 });

        Mesh* engineFlame = new Mesh("engineFlame");
        engineFlame->InitFromData(vertices2, indices2);

        // Engine block
        Mesh* engineBlock = object2D::CreateRectangle("engineBlock", ORIGIN8, MAIN_SQUARE_SIDE, MAIN_SQUARE_SIDE, ORANGE, true);

        // Engine
        glm::mat3 engineMat = glm::mat3(1);
        Mesh* engine = CombineMeshes("engine",
            { engineBlock, engineFlame},
            { engineMat, engineMat });

        // White border
        glm::vec3 ORIGIN14 = glm::vec3(250, 30, 0);
        float WHITE_BORDER_WIDTH = 780;
        float WHITE_BORDER_HEIGHT = 660;

        Mesh* whiteBorder = object2D::CreateRectangle("whiteBorder", ORIGIN14, WHITE_BORDER_WIDTH, WHITE_BORDER_HEIGHT, WHITE, false);

        // Central line
        float RECTANGLE_WITDTH = 10;
        float RECTANGLE_HEIGHT = 30;
        float SPACING = 20;
        int NR_RECTANGLES = 12;
        for (int j = 0; j < NR_RECTANGLES; j++) {
            float y = j * (RECTANGLE_HEIGHT + SPACING) + 70;

            std::string name = "greyRectangle_" + std::to_string(j);

            Mesh* greyRectangle = object2D::CreateRectangle(name, glm::vec3(635, y, 0), RECTANGLE_WITDTH, RECTANGLE_HEIGHT, GRAY, true);

            AddMeshToList(greyRectangle);
        }

        // Ball
        Mesh* gameBall = CreateCircle("gameBall", glm::vec3(windowWidth/2, windowHeight/2, 0), ballRadius, 50, ORANGE);

        //Ball
        AddMeshToList(gameBall);
        // Green start button
        AddMeshToList(greenStartButton);
        // Red start button
        AddMeshToList(redStartButton);
        // Bumper
        AddMeshToList(bumper);
        // Screen border
        AddMeshToList(screenBorder);
        // Red borders
        AddMeshToList(miniSquare1);
        AddMeshToList(miniSquare2);
        AddMeshToList(miniSquare3);
        AddMeshToList(miniSquare4);
        // Blue border
        AddMeshToList(blueBorder);
        // Solid block
        AddMeshToList(solidBlock);
        // Engine
        AddMeshToList(engine);
        // Canon
        AddMeshToList(canon);
        // White border
        AddMeshToList(whiteBorder);
        //AddMeshToList(combinedMesh);
    }
}

void Tema1::FrameStart()
{
    // Clears the color buffer (using the previously set color) and depth buffer
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::ivec2 resolution = window->GetResolution();
    // Sets the screen area where to draw
    glViewport(0, 0, resolution.x, resolution.y);
}

void Tema1::Update(float deltaTimeSeconds)
{

    glm::vec3 cameraOffset(0);

    if (cameraShaking) {
        cameraShakeTime += deltaTimeSeconds;

        if (cameraShakeTime >= cameraShakeDuration) {
            cameraShaking = false;
        }
        else {
            float offsetX = ((rand() % 200) / 100.0f - 1.0f) * cameraShakeIntensity;
            float offsetY = ((rand() % 200) / 100.0f - 1.0f) * cameraShakeIntensity;
            cameraOffset = glm::vec3(offsetX, offsetY, 0);
        }
    }

    // aplici offsetul
    glm::mat3 viewMatrix = transform2D::Translate(cameraOffset.x, cameraOffset.y);

    modelMatrix = glm::mat3(1); 

    if (editorState) {
        // Dragged mesh
        if (isDragging && dragMesh && dragMesh->mesh) {
            RenderMesh2D(dragMesh->mesh, shaders["VertexColor"], glm::mat3(1));
        }

        // Car blocks
        for (auto* block : placedBlocks)
        {
            glm::mat3 modelMatrix = glm::mat3(1);
            RenderMesh2D(block->mesh, shaders["VertexColor"], modelMatrix);
        }

        // Bumper
        RenderMesh2D(meshes["bumper"], shaders["VertexColor"], modelMatrix);

        // Start button
        if (constraints)
            RenderMesh2D(meshes["greenStartButton"], shaders["VertexColor"], modelMatrix);
        else
            RenderMesh2D(meshes["redStartButton"], shaders["VertexColor"], modelMatrix);

        // Solid block
        RenderMesh2D(meshes["solidBlock"], shaders["VertexColor"], modelMatrix);

        // Canon
        RenderMesh2D(meshes["canon"], shaders["VertexColor"], modelMatrix);

        // Engine 
        RenderMesh2D(meshes["engine"], shaders["VertexColor"], modelMatrix);

        // Screen border
        RenderMesh2D(meshes["screenBorder"], shaders["VertexColor"], modelMatrix);

        // Red borders
        RenderMesh2D(meshes["miniSquare1"], shaders["VertexColor"], modelMatrix);
        RenderMesh2D(meshes["miniSquare2"], shaders["VertexColor"], modelMatrix);
        RenderMesh2D(meshes["miniSquare3"], shaders["VertexColor"], modelMatrix);
        RenderMesh2D(meshes["miniSquare4"], shaders["VertexColor"], modelMatrix);

        // Blue border
        RenderMesh2D(meshes["blueBorder"], shaders["VertexColor"], modelMatrix);

        // Blue squares grid
        for (auto const& meshPair : meshes) {
            if (meshPair.first.find("blueSquare_") == 0) {
                RenderMesh2D(meshPair.second, shaders["VertexColor"], modelMatrix);
            }
        }

        // Green squares line
        int drawnSquares = 0;
        for (auto const& meshPair : meshes) {
            if (meshPair.first.find("greenSquare_") == 0) {
                if (drawnSquares < NR_GREEN_SQUARES) {
                    RenderMesh2D(meshPair.second, shaders["VertexColor"], modelMatrix);
                    drawnSquares++;
                }
            }
        }
    } else {

        glClearColor(0.0f, 0.0f, 0.2f, 1.0f); 
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // White border
        glLineWidth(5.0f);
        RenderMesh2D(meshes["whiteBorder"], shaders["VertexColor"], viewMatrix * modelMatrix);
        glLineWidth(1.0f);

        // Grey line
        for (auto const& meshPair : meshes) {
            if (meshPair.first.find("greyRectangle_") == 0) {
                RenderMesh2D(meshPair.second, shaders["VertexColor"], viewMatrix * modelMatrix);
            }
        }

        glm::vec3 WHITE = glm::vec3(1, 1, 1);
        float spacing = 25.0f;


        glm::vec3 center = GetMeshCenter(combinedMesh);
        glm::vec2 paddlesDims = GetMeshDimensions(combinedMesh);

        glm::mat3 modelLeft = glm::mat3(1);
        glm::mat3 modelRight = glm::mat3(1);

        // Animatie paleta stanga
        float leftScaleY = 1.0f;
        if (leftPaddleAnimating) {
            float progress = leftPaddleAnimTime / paddleAnimDuration;
            leftScaleY = 1.0f + 0.2f * sin(progress * M_PI * 2); 
            leftPaddleAnimTime += deltaTimeSeconds;
            if (leftPaddleAnimTime >= paddleAnimDuration)
                leftPaddleAnimating = false;
        }

        // Animatie paleta dreapta
        float rightScaleY = 1.0f;
        if (rightPaddleAnimating) {
            float progress = rightPaddleAnimTime / paddleAnimDuration;
            rightScaleY = 1.0f + 0.2f * sin(progress * M_PI * 2);
            rightPaddleAnimTime += deltaTimeSeconds;
            if (rightPaddleAnimTime >= paddleAnimDuration)
                rightPaddleAnimating = false;
        }

        // Transform paleta stanga
        modelLeft *= transform2D::Translate(paddleLeftPos.x, paddleLeftPos.y);
        modelLeft *= transform2D::Rotate(-M_PI / 2);
        modelLeft *= transform2D::Scale(0.6f, 0.6f * leftScaleY); 
        modelLeft *= transform2D::Translate(-center.x, -center.y);
        RenderMesh2D(combinedMesh, shaders["VertexColor"], viewMatrix * modelLeft);

        // Transform paleta dreapta
        modelRight *= transform2D::Translate(paddleRightPos.x, paddleRightPos.y);
        modelRight *= transform2D::Rotate(M_PI / 2);
        modelRight *= transform2D::Scale(0.6f, 0.6f * rightScaleY);
        modelRight *= transform2D::Translate(-center.x, -center.y);
        RenderMesh2D(combinedMesh, shaders["VertexColor"], viewMatrix * modelRight);


        float scaledPaddleWidth = paddleWidth * 0.6f;
        float scaledPaddleHeight = paddleHeight * 0.6f;

        // Ball
        Tema1::UpdateBall(deltaTimeSeconds);

        glm::mat3 gameBall = glm::mat3(1);
        gameBall *= transform2D::Translate(ballPos.x - windowWidth / 2, ballPos.y - windowHeight / 2);
        RenderMesh2D(meshes["gameBall"], shaders["VertexColor"], viewMatrix * gameBall);

        if (text) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            float windowCorrectHeight = window->GetResolution().y;

            float y = windowCorrectHeight - 650.0f;

            float textScale = 1.0f;   
            
            // LEFT:
            text->RenderText("LEFT:" + std::to_string(leftScore), 50.0f, y, textScale, WHITE);

            // RIGHT:
            text->RenderText("RIGHT:" + std::to_string(rightScore), 1050.0f, y, textScale, WHITE);

            glDisable(GL_BLEND);
        }
    }
}


void Tema1::FrameEnd()
{
}


/*
 *  These are callback functions. To find more about callbacks and
 *  how they behave, see `input_controller.h`.
 */

void Tema1::OnInputUpdate(float deltaTime, int mods)
{
    float speed = 300.0f;
    float paddleHeight = paddlesDims.y;
    float windowHeight = window->GetResolution().y;

    if (window->KeyHold(GLFW_KEY_W)) {
        paddleLeftPos.y += speed * deltaTime;
        if (paddleLeftPos.y + paddleHeight / 2 > windowHeight) {
            paddleLeftPos.y = windowHeight - paddleHeight / 2;
        }
    }
    if (window->KeyHold(GLFW_KEY_S)) {
        paddleLeftPos.y -= speed * deltaTime;
        if (paddleLeftPos.y - paddleHeight / 2 < 0) {
            paddleLeftPos.y = paddleHeight / 2;
        }
    }

    if (window->KeyHold(GLFW_KEY_UP)) {
        paddleRightPos.y += speed * deltaTime;
        if (paddleRightPos.y + paddleHeight / 2 > windowHeight) {
            paddleRightPos.y = windowHeight - paddleHeight / 2;
        }
    }
    if (window->KeyHold(GLFW_KEY_DOWN)) {
        paddleRightPos.y -= speed * deltaTime;
        if (paddleRightPos.y - paddleHeight / 2 < 0) {
            paddleRightPos.y = paddleHeight / 2;
        }
    }
}


void Tema1::OnKeyPress(int key, int mods)
{
    
}


void Tema1::OnKeyRelease(int key, int mods)
{
    // Add key release event
}


void Tema1::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
    glm::ivec2 resolution = window->GetResolution();
    int mouseYcorrected = resolution.y - mouseY;
    glm::vec2 clickPos(mouseX, mouseYcorrected);

    if (button == 1) {
        isDragging = 1;

        draggingExistingBlock = false;

        float blockSize = 50.0f;

        for (auto* block : placedBlocks) {
            glm::vec3 pos = block->position;
            if (clickPos.x >= pos.x && clickPos.x <= pos.x + blockSize &&
                clickPos.y >= pos.y && clickPos.y <= pos.y + blockSize) {

                // Mutam un bloc deja existent
                dragMesh = block;
                draggingExistingBlock = true;

                glm::vec3 blockPos = dragMesh->mesh->vertices[0].position;
                dragOffset = glm::vec2(mouseX, mouseYcorrected) - glm::vec2(blockPos.x, blockPos.y);

                return;
            }
        }

        if (!draggingExistingBlock && mouseX < 256)
        {
            if (mouseYcorrected < 180) {
                dragMesh->mesh = meshes["solidBlock"];
                dragMesh->type = "solidBlock";
                copyName = "solidBlock_copy_" + std::to_string(solidBlockCount);
            }
            else if (mouseYcorrected >= 180 && mouseYcorrected < 360) {
                dragMesh->mesh = meshes["engine"];
                dragMesh->type = "engine";
                copyName = "engine_copy_" + std::to_string(engineCount);
            }
            else if (mouseYcorrected >= 360 && mouseYcorrected < 540) {
                dragMesh->mesh = meshes["canon"];
                dragMesh->type = "canon";
                copyName = "canon_copy_" + std::to_string(canonCount);
            }
            else if (mouseYcorrected >= 540 && mouseYcorrected < 720) {
                dragMesh->mesh = meshes["bumper"];
                dragMesh->type = "bumper";
                copyName = "bumper_copy_" + std::to_string(bumperCount);
            }

            Mesh* copy = new Mesh(copyName);

            copy->vertices = dragMesh->mesh->vertices;
            copy->indices = dragMesh->mesh->indices;
            copy->InitFromData(copy->vertices, copy->indices);

            dragMesh->mesh = copy;
            isDragging = true;

            glm::vec3 blockPos = copy->vertices[0].position;
            dragOffset = glm::vec2(mouseX, mouseYcorrected) - glm::vec2(blockPos.x, blockPos.y);
        }

        glm::vec3 ORIGIN11 = glm::vec3(1215, 620, 0);
        int buttonSize = 50;

        if (clickPos.x >= ORIGIN11.x && clickPos.x <= ORIGIN11.x + buttonSize &&
            clickPos.y >= ORIGIN11.y && clickPos.y <= ORIGIN11.y + buttonSize && constraints == true) {
            Tema1::CombinePlacedBlocks();
            editorState = false;
        }
        else if (clickPos.x >= ORIGIN11.x && clickPos.x <= ORIGIN11.x + buttonSize &&
            clickPos.y >= ORIGIN11.y && clickPos.y <= ORIGIN11.y + buttonSize && constraints == false) {
            return;
        }
    }
     
    if (button == 2) {
        float blockSize = 50.0f;
        NR_GREEN_SQUARES++;

        bool deleted = false;

        for (auto it = placedBlocks.begin(); it != placedBlocks.end(); ++it) {
            Block* block = *it;
            glm::vec3 pos = block->position;

            if (clickPos.x >= pos.x && clickPos.x <= pos.x + blockSize &&
                clickPos.y >= pos.y && clickPos.y <= pos.y + blockSize) {

                if (block->type == "solidBlock") solidBlockCount--;
                else if (block->type == "engine") engineCount--;
                else if (block->type == "canon") canonCount--;
                else if (block->type == "bumper") bumperCount--;

                delete block;
                placedBlocks.erase(it);
                deleted = true;
                break;
            }
        }

        if (deleted) {
            constraints = true; 
            for (auto& block : placedBlocks) {
                constraints = constraints && Tema1::CheckConstraints(placedBlocks, block);
            }
        }
    }
}

void Tema1::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) {
    if (button == 1) {
        isDragging = false;

        glm::ivec2 resolution = window->GetResolution();
        int mouseYcorrected = resolution.y - mouseY;
        glm::vec3 releasePos = glm::vec3(mouseX, mouseYcorrected, 0);

        float gridX = 351;
        float gridY = 73;
        float squareSize = 30;
        float spacing = 20;
        int cols = 17;
        int rows = 9;
        float gridWidth = cols * squareSize + (cols - 1) * spacing;
        float gridHeight = rows * squareSize + (rows - 1) * spacing;

        if (releasePos.x >= gridX && releasePos.x <= gridX + gridWidth &&
            releasePos.y >= gridY && releasePos.y <= gridY + gridHeight) {

            int col = (releasePos.x - gridX) / (squareSize + spacing);
            int row = (releasePos.y - gridY) / (squareSize + spacing);

            float cellCenterX = gridX + col * (squareSize + spacing) + squareSize / 2.0f;
            float cellCenterY = gridY + row * (squareSize + spacing) + squareSize / 2.0f;

            float blockSize = 50.0f;
            float newBlockX = cellCenterX - blockSize / 2.0f;
            float newBlockY = cellCenterY - blockSize / 2.0f;


            placedBlocksCount++;

            if (!draggingExistingBlock) {
                if (NR_GREEN_SQUARES > 0)
                    NR_GREEN_SQUARES--;
            }


            if (dragMesh->type == "solidBlock") {
                solidBlockCount++;
            }
            else if (dragMesh->type == "engine") {
                engineCount++;
            }
            else if (dragMesh->type == "canon") {
                canonCount++;
            }
            else if (dragMesh->type == "bumper") {
                bumperCount++;
            }

            Block* newBlock = new Block();
            newBlock->mesh = new Mesh(*dragMesh->mesh);
            newBlock->type = dragMesh->type;
            newBlock->position = glm::vec3(newBlockX, newBlockY, 0);
            placedBlocks.push_back(newBlock);
            constraints = Tema1::CheckConstraints(placedBlocks, newBlock);

            std::vector<VertexFormat> newVertices = newBlock->mesh->vertices;
            glm::vec3 firstVertexPos = newBlock->mesh->vertices[0].position;

            for (auto& vertex : newVertices) {
                glm::vec3 offset = vertex.position - firstVertexPos;
                vertex.position = glm::vec3(newBlockX + offset.x, newBlockY + offset.y, 1.0f);
            }

            newBlock->mesh->InitFromData(newVertices, newBlock->mesh->indices);
        }
        else {
            if (dragMesh) {
                dragMesh->mesh = nullptr;
                dragMesh->type.clear();
            }
            isDragging = false;
        }

        draggingExistingBlock = false;
    }
}

bool Tema1::CheckConstraints(const std::vector<Block*>& placedBlocks, Block* newBlock) {
    if (!newBlock) return false;

    const float blockSize = 50.0f;
    const int maxBlocks = 10;


    // No more than 10 blocks vehicle
    if (placedBlocks.size() > maxBlocks)
        return false;

    float engineY = -1;
    float cannonY = -1;
    glm::vec3 bumperPos(-1, -1, -1);

    for (auto* block : placedBlocks) {
        if (block->type == "engine") engineY = block->position.y;
        else if (block->type == "canon") cannonY = block->position.y;
        else if (block->type == "bumper") bumperPos = block->position;
    }

    // No block lower than engine
    if (engineY >= 0 && newBlock->position.y < engineY)
        return false;

    // No block higher than canon
    if (cannonY >= 0 && newBlock->position.y > cannonY)
        return false;

    // No blocks higher than bumper
    if (bumperPos.x >= 0) {
        float dx = fabs(newBlock->position.x - bumperPos.x);
        float dy = newBlock->position.y - bumperPos.y;
        if (dy > 0 && dx <= 2 * blockSize)
            return false;
    }

    // No canon near bumper
    if (newBlock->type == "canon" && bumperPos.x >= 0) {
        float dx = fabs(newBlock->position.x - bumperPos.x);
        float dy = fabs(newBlock->position.y - bumperPos.y);
        if (dx == blockSize && dy == 0)
            return false;
    }

    // No neighbouring bumpers
    if (newBlock->type == "bumper") {
        for (auto* block : placedBlocks) {
            if (block->type == "bumper") {
                float dx = fabs(block->position.x - newBlock->position.x);
                float dy = fabs(block->position.y - newBlock->position.y);
                if ((dx == blockSize && dy == 0) || (dx == 0 && dy == blockSize))
                    return false;
            }
        }
    }


    // Connected vehicle
    std::set<std::pair<int, int>> positions;
    for (auto* block : placedBlocks) {
        int gx = static_cast<int>(block->position.x / blockSize);
        int gy = static_cast<int>(block->position.y / blockSize);
        positions.insert({ gx, gy });
    }
 
    int newX = static_cast<int>(newBlock->position.x / blockSize);
    int newY = static_cast<int>(newBlock->position.y / blockSize);
    positions.insert({ newX, newY });

    std::set<std::pair<int, int>> visited;
    std::queue<std::pair<int, int>> q;
    q.push(*positions.begin());
    visited.insert(*positions.begin());

    std::vector<std::pair<int, int>> dirs = { {1,0},{-1,0},{0,1},{0,-1} };

    while (!q.empty()) {
        auto cur = q.front(); q.pop();
        for (auto d : dirs) {
            std::pair<int, int> neighbor = { cur.first + d.first, cur.second + d.second };
            if (positions.count(neighbor) && !visited.count(neighbor)) {
                visited.insert(neighbor);
                q.push(neighbor);
            }
        }
    }

    if (visited.size() != positions.size())
        return false;

    return true;
}


void Tema1::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) {
    if (!isDragging || !dragMesh) return;

    glm::ivec2 resolution = window->GetResolution();
    int mouseYcorrected = resolution.y - mouseY;

    glm::vec2 newPos = glm::vec2(mouseX, mouseYcorrected) - dragOffset;

    std::vector<VertexFormat> newVertices = dragMesh->mesh->vertices;
    glm::vec3 firstVertexPos = dragMesh->mesh->vertices[0].position;

    for (auto& vertex : newVertices) {
        glm::vec3 offset = vertex.position - firstVertexPos;
        vertex.position = glm::vec3(newPos.x + offset.x, newPos.y + offset.y, 0);
    }

    dragMesh->mesh->InitFromData(newVertices, dragMesh->mesh->indices);
}

void Tema1::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) 
{

}


void Tema1::OnWindowResize(int width, int height)
{

}

Mesh* Tema1::combinedMesh = nullptr;

void Tema1::CombinePlacedBlocks()
{
    if (placedBlocks.empty())
        return;

    std::vector<Mesh*> meshesToCombine;
    std::vector<glm::mat3> transforms;

    for (auto* block : placedBlocks) {
        meshesToCombine.push_back(block->mesh);

        glm::mat3 transform = glm::mat3(1); 
        transforms.push_back(transform);

    }

    if (combinedMesh)
        delete combinedMesh;

    combinedMesh = CombineMeshes("combinedBlocks", meshesToCombine, transforms);
    AddMeshToList(combinedMesh);
}

Mesh* Tema1::CreateCircle(const std::string& name, glm::vec3 center, float radius, int segments, glm::vec3 color) {
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    vertices.push_back(VertexFormat(center, color));

    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f * M_PI * i / segments;
        float x = center.x + radius * cos(angle);
        float y = center.y + radius * sin(angle);
        vertices.push_back(VertexFormat(glm::vec3(x, y, 0), color));
    }

    for (int i = 1; i <= segments; i++) {
        indices.push_back(0);     
        indices.push_back(i);      
        indices.push_back(i + 1);  
    }

    Mesh* circle = new Mesh(name);
    circle->InitFromData(vertices, indices);
    return circle;
}

glm::vec3 Tema1::GetMeshCenter(Mesh* mesh) {
    if (!mesh || mesh->vertices.empty())
        return glm::vec3(0.0f);

    float minX = mesh->vertices[0].position.x;
    float maxX = mesh->vertices[0].position.x;
    float minY = mesh->vertices[0].position.y;
    float maxY = mesh->vertices[0].position.y;

    for (auto& v : mesh->vertices) {
        if (v.position.x < minX) minX = v.position.x;
        if (v.position.x > maxX) maxX = v.position.x;
        if (v.position.y < minY) minY = v.position.y;
        if (v.position.y > maxY) maxY = v.position.y;
    }

    float centerX = (minX + maxX) / 2.0f;
    float centerY = (minY + maxY) / 2.0f;

    return glm::vec3(centerX, centerY, 0.0f);
}

glm::vec2 Tema1::GetMeshDimensions(Mesh* mesh) {
    float minX = FLT_MAX, maxX = -FLT_MAX;
    float minY = FLT_MAX, maxY = -FLT_MAX;

    for (auto& vertex : mesh->vertices) {
        if (vertex.position.x < minX) minX = vertex.position.x;
        if (vertex.position.x > maxX) maxX = vertex.position.x;
        if (vertex.position.y < minY) minY = vertex.position.y;
        if (vertex.position.y > maxY) maxY = vertex.position.y;
    }

    return glm::vec2(maxX - minX, maxY - minY);
}

bool Tema1::CircleAABBIntersect(glm::vec2 circlePos, float ballradius, glm::vec2 aabbCenter, float aabbWidth, float aabbHeight) {
    glm::vec2 halfSize = glm::vec2(aabbWidth / 2.0f, aabbHeight / 2.0f);
    glm::vec2 min = aabbCenter - halfSize;
    glm::vec2 max = aabbCenter + halfSize;

    float closestX = glm::clamp(circlePos.x, min.x, max.x);
    float closestY = glm::clamp(circlePos.y, min.y, max.y);
   
    float dx = circlePos.x - closestX;
    float dy = circlePos.y - closestY;

    return (dx * dx + dy * dy) < (ballRadius * ballRadius);
}

void Tema1::HandlePaddleCollision(glm::vec2& ballPos, glm::vec2& ballVel, glm::vec2 paddleCenter,
    float paddleWidth, float paddleHeight, bool isLeftPaddle, float ballSpeed)
{
    if (!CircleAABBIntersect(ballPos, ballRadius, paddleCenter, paddleWidth, paddleHeight))
        return;

    float relativeY = ballPos.y - paddleCenter.y;
    float normalizedY = relativeY / (paddleHeight / 2.0f);
    float bounceAngle = normalizedY * (M_PI / 4.0f);

    if (isLeftPaddle) {
        ballVel.x = ballSpeed * cos(bounceAngle);
        ballVel.y = ballSpeed * sin(bounceAngle);

        leftPaddleAnimating = true;
        leftPaddleAnimTime = 0.0f;
    }
    else {
        ballVel.x = -ballSpeed * cos(bounceAngle);
        ballVel.y = ballSpeed * sin(bounceAngle);

        rightPaddleAnimating = true;
        rightPaddleAnimTime = 0.0f;
    }
}

void Tema1::UpdateBall(float deltaTime) 
{
    ballPos += ballVel * deltaTime;

    HandlePaddleCollision(ballPos, ballVel, paddleLeftPos, paddleWidth, paddleHeight, true, glm::length(ballVel));
    HandlePaddleCollision(ballPos, ballVel, paddleRightPos, paddleWidth, paddleHeight, false, glm::length(ballVel));

    float bottom = 30.0f + ballRadius;
    float top = 30.0f + 660.0f - ballRadius; 

    if (ballPos.y - ballRadius < bottom) {
        float penetration = bottom - (ballPos.y - ballRadius);
        ballPos.y += 2 * penetration; 
        ballVel.y *= -1;
    }

    if (ballPos.y + ballRadius > top) {
        float penetration = (ballPos.y + ballRadius) - top;
        ballPos.y -= 2 * penetration;
        ballVel.y *= -1;
    }

    if (ballPos.x < 250) {
        leftPlayerLost = true;
        rightScore++;
        ResetBall();
    }

    if (ballPos.x > 1030) {
        leftScore++;
        leftPlayerLost = false;
        ResetBall();
    }

    for (auto* block : placedBlocks) {
        if (block->type == "bumper") {
            if (CheckBumperCollision(ballPos, ballRadius, *block)) {
                ballVel.y = -ballVel.y;
                ballVel *= 1.5f;
                break;
            }
        }
    }

    if (leftScore % 5 == 0 && rightScore != 0) {
        cameraShaking = true;
        cameraShakeTime = 0.0f;
    }

    if (rightScore % 5 == 0 && rightScore != 0) {
        cameraShaking = true;
        cameraShakeTime = 0.0f;
    }
}

void Tema1::ResetBall()
{
    ballPos = glm::vec2(windowWidth / 2, windowHeight / 2);

    if (leftPlayerLost) 
        ballVel = glm::vec2(-ballSpeed, 0.0f);
    else
        ballVel = glm::vec2(ballSpeed, 0.0f);
}

bool Tema1::CheckBumperCollision(const glm::vec2& ballPos, float radius, const Block& bumper) {
    float bumperSize = 50.0f;
    float closestX = glm::clamp(ballPos.x, bumper.position.x, bumper.position.x + bumperSize);
    float closestY = glm::clamp(ballPos.y, bumper.position.y, bumper.position.y + bumperSize);

    float dx = ballPos.x - closestX;
    float dy = ballPos.y - closestY;

    return (dx * dx + dy * dy) < (radius * radius);
}

Mesh* CreateRectangle(
    const std::string& name,
    glm::vec3 leftBottomCorner,
    float width,
    float height,
    glm::vec3 color,
    bool fill)
{
    glm::vec3 corner = leftBottomCorner;

    std::vector<VertexFormat> vertices = {
        VertexFormat(corner, color),
        VertexFormat(corner + glm::vec3(width, 0, 0), color),
        VertexFormat(corner + glm::vec3(width, height, 0), color),
        VertexFormat(corner + glm::vec3(0, height, 0), color)
    };

    Mesh* rectangle = new Mesh(name);
    std::vector<unsigned int> indices = { 0, 1, 2, 3 };

    if (!fill) {
        rectangle->SetDrawMode(GL_LINE_LOOP); // doar contur
    }
    else {
        indices.push_back(0);
        indices.push_back(2); // douã triunghiuri pentru interior
    }

    rectangle->InitFromData(vertices, indices);
    return rectangle;
}

Mesh* CreateSemiCircle(const std::string& name, glm::vec2 center, float radius, int segments, glm::vec3 color) {
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    vertices.push_back(VertexFormat(glm::vec3(center.x, center.y, 0), color));

    for (int i = 0; i <= segments; i++) {
        float angle = glm::pi<float>() * i / segments;
        float x = center.x + radius * cos(angle);
        float y = center.y + radius * sin(angle);
        vertices.push_back(VertexFormat(glm::vec3(x, y, 0), color));
    }

    for (int i = 1; i <= segments; i++) {
        indices.push_back(0);
        indices.push_back(i);
        indices.push_back(i + 1);
    }

    Mesh* semicircle = new Mesh(name);
    semicircle->InitFromData(vertices, indices);
    return semicircle;
}






