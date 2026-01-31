#include "lab_m1/Tema2/Tema2.h"
#include <vector>
#include <iostream>
#include <set>
#include <queue>
#include <utility> 
using namespace std;
using namespace m1;

Tema2::Tema2()
{
}

Tema2::~Tema2()
{
    // Eliberăm memoria pentru șine
    for (Rail* rail : railRoad) {
        delete rail;
    }
}

void Tema2::Init()
{
    glm::ivec2 resolution = window->GetResolution();
    auto camera = GetSceneCamera();

    // Setare cameră pentru vizualizare 3D
    camera->SetPerspective(60.0f, (float)resolution.x / resolution.y, 0.1f, 400);
    camera->SetPosition(glm::vec3(0, 10, 20));
    camera->SetRotation(glm::vec3(0, 0, 0));
    camera->Update();
    GetCameraInput()->SetActive(true);

    screenHeight = window->GetResolution().y;
    screenWidth = window->GetResolution().x;

    text = new gfxc::TextRenderer(
        window->props.selfDir,
        screenWidth,
        screenHeight
    );
    text->Load("assets/fonts/Hack-Bold.ttf", 50);

    // Culori
    RED = glm::vec3(1, 0, 0);
    GREEN = glm::vec3(0.20f, 0.75f, 0.25f);
    BLUE = glm::vec3(0, 0, 1);
    GREY = glm::vec3(0.5, 0.5, 0.5);
    BLACK = glm::vec3(0.1, 0.1, 0.1);
    YELLOW = glm::vec3(1, 1, 0);
    BROWN = glm::vec3(0.4f, 0.2f, 0.1f);
    glm::vec3 RED_BRICK = glm::vec3(0.7f, 0.2f, 0.2f);
    glm::vec3 ORANGE = glm::vec3(0.9f, 0.5f, 0.1f);
    glm::vec3 DARK_GREY = glm::vec3(0.2f, 0.2f, 0.2f);

   
    numVagons = 2;


    CreateColoredCube("cube_green", GREEN);   // caroserie
    CreateColoredCube("cube_black", BLACK);   // șasiu
    CreateColoredCube("cube_yellow", YELLOW); // dungi
    CreateColoredCube("cube_brown", BROWN);
    CreateColoredCube("cube_grey", GREY);
    CreateColoredCylinder("cylinder_black", BLACK); // coș
    CreateColoredCylinder("cylinder_blue", BLUE);   // motor

    CreateColoredCube("cube_yellow", YELLOW);
    CreateColoredCylinder("cylinder_black", BLACK);
    CreateColoredCylinder("cylinder_blue", BLUE);

    CreateColoredCube("cube_red", RED_BRICK);

    CreateColoredCube("cube_orange", ORANGE);
    CreateColoredCube("cube_dark_grey", DARK_GREY);

    // Culori pentru semafoare
    glm::vec3 RED_BRIGHT = glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 RED_DARK = glm::vec3(0.3f, 0.0f, 0.0f);
    glm::vec3 YELLOW_BRIGHT = glm::vec3(1.0f, 1.0f, 0.0f);
    glm::vec3 YELLOW_DARK = glm::vec3(0.3f, 0.3f, 0.0f);
    glm::vec3 GREEN_BRIGHT = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 GREEN_DARK = glm::vec3(0.0f, 0.3f, 0.0f);

    CreateColoredCylinder("cylinder_red_bright", RED_BRIGHT);
    CreateColoredCylinder("cylinder_red_dark", RED_DARK);
    CreateColoredCylinder("cylinder_yellow_bright", YELLOW_BRIGHT);
    CreateColoredCylinder("cylinder_yellow_dark", YELLOW_DARK);
    CreateColoredCylinder("cylinder_green_bright", GREEN_BRIGHT);
    CreateColoredCylinder("cylinder_green_dark", GREEN_DARK);
    CreateColoredCube("cube_dark_grey", DARK_GREY);

    glm::vec3 OVERLAY_COLOR = glm::vec3(0.0f, 0.0f, 0.0f);
    CreateColoredCube("cube_overlay", OVERLAY_COLOR);

    // Inițializare minimap
    showMinimap = true;
    minimapSize = 200.0f;  // 200x200 pixeli
    minimapX = screenWidth - minimapSize - 20.0f;  // Colț dreapta-sus
    minimapY = screenHeight - minimapSize - 20.0f;
    minimapScale = 10.0f;  // Scală: 1 unitate world = 10 pixeli
    minimapCenter = glm::vec3(0, 0, 0);  // Centrul circuitului

    // Creăm terenul
    CreateTerrain();

    // Creăm mesh-urile necesare
    CreateTrainMeshes();

    // Creăm șinele
    CreateRailRoad();

    CreateStations();

    CreateTrafficLights();

    // Inițializăm trenul
    if (!railRoad.empty()) {
        train.currentRail = railRoad[0];
        train.progress = 0.0f;
        train.speed = 0.2f;
        train.position = train.currentRail->startPosition;
    }

    if (!railRoad.empty()) {
        handcar.currentRail = railRoad[2];
        handcar.progress = 0.3f;
        handcar.speed = 0.4f;

        Rail* rail = handcar.currentRail;
        glm::vec3 basePosition = glm::mix(rail->startPosition, rail->endPosition, handcar.progress);
        handcar.position = basePosition;
        handcar.direction = glm::normalize(rail->endPosition - rail->startPosition);
        handcar.visualDirection = handcar.direction; // ADAUGĂ ASTA
    }

    cameraFollowHandcar = false;

    // Inițializare sistem avarii
    damageSpawnTimer = 0.0f;
    damageSpawnInterval = 10.0f; // O avarie la fiecare 5 secunde
    gameOver = false;
    gameOverReason = "";
    repairRadius = 3.0f; // Raza în care poți repara
    railBeingRepaired = nullptr;
    severityIncreaseInterval = 10.0f;

    totalRails = railRoad.size();
    damagedRails = 0;

    std::cout << "=== GAME RULES ===" << std::endl;
    std::cout << "Keep trains moving by repairing damaged rails!" << std::endl;
    std::cout << "F - Repair nearby damaged rail" << std::endl;
    std::cout << "Game Over if:" << std::endl;
    std::cout << "  - Train waits >30s at damage" << std::endl;
    std::cout << "  - >50% of rails are damaged" << std::endl;
    std::cout << "==================" << std::endl;
}

void Tema2::CreateRailRoad()
{
    float size = 10.0f;

    // Segment 1: lateral stânga sus - NORMAL
    Rail* rail1 = new Rail(glm::vec3(-size, 0, -size), glm::vec3(-size, 0, 0), TUNNEL);
    railRoad.push_back(rail1);

    // Segment 2: curbă stânga-jos - NORMAL
    Rail* rail2 = new Rail(glm::vec3(-size, 0, 0), glm::vec3(-size, 0, size), BRIDGE);
    railRoad.push_back(rail2);
    rail1->next = rail2;

    // Segment 3: jos (peste apă) - BRIDGE
    Rail* rail3 = new Rail(glm::vec3(-size, 0, size), glm::vec3(0, 0, size), NORMAL);
    railRoad.push_back(rail3);
    rail2->next = rail3;

    // Segment 4: jos continuare - NORMAL
    Rail* rail4 = new Rail(glm::vec3(0, 0, size), glm::vec3(size, 0, size), NORMAL);
    railRoad.push_back(rail4);
    rail3->next = rail4;

    // Segment 5: lateral dreapta jos - NORMAL
    Rail* rail5 = new Rail(glm::vec3(size, 0, size), glm::vec3(size, 0, 0), BRIDGE);
    railRoad.push_back(rail5);
    rail4->next = rail5;

    // Segment 6: lateral dreapta sus (prin munte) - TUNNEL
    Rail* rail6 = new Rail(glm::vec3(size, 0, 0), glm::vec3(size, 0, -size), NORMAL);
    railRoad.push_back(rail6);
    rail5->next = rail6;

    // Segment 7: sus - NORMAL
    Rail* rail7 = new Rail(glm::vec3(size, 0, -size), glm::vec3(0, 0, -size), BRIDGE);
    railRoad.push_back(rail7);
    rail6->next = rail7;

    // Segment 8: sus continuare (închide circuitul) - NORMAL
    Rail* rail8 = new Rail(glm::vec3(0, 0, -size), glm::vec3(-size, 0, -size), TUNNEL);
    railRoad.push_back(rail8);
    rail7->next = rail8;

    // Închide circuitul
    rail8->next = rail1;
}

void Tema2::CreateTrainMeshes()
{
    // Creăm un cub generic pentru paralelipipede
    {
        std::vector<VertexFormat> vertices
        {
            // Fața din față
            VertexFormat(glm::vec3(-0.5, -0.5,  0.5), RED),
            VertexFormat(glm::vec3(0.5, -0.5,  0.5), RED),
            VertexFormat(glm::vec3(0.5,  0.5,  0.5), RED),
            VertexFormat(glm::vec3(-0.5,  0.5,  0.5), RED),
            // Fața din spate
            VertexFormat(glm::vec3(-0.5, -0.5, -0.5), RED),
            VertexFormat(glm::vec3(0.5, -0.5, -0.5), RED),
            VertexFormat(glm::vec3(0.5,  0.5, -0.5), RED),
            VertexFormat(glm::vec3(-0.5,  0.5, -0.5), RED)
        };

        std::vector<unsigned int> indices =
        {
            0, 1, 2,  0, 2, 3, // față
            1, 5, 6,  1, 6, 2, // dreapta
            5, 4, 7,  5, 7, 6, // spate
            4, 0, 3,  4, 3, 7, // stânga
            3, 2, 6,  3, 6, 7, // sus
            4, 5, 1,  4, 1, 0  // jos
        };

        Mesh* cube = new Mesh("cube");
        cube->InitFromData(vertices, indices);
        AddMeshToList(cube);
    }

    // Creăm un cilindru pentru roți și motorul locomotivei
    {
        std::vector<VertexFormat> vertices;
        std::vector<unsigned int> indices;

        int segments = 32;
        float radius = 0.5f;
        float height = 1.0f;

        // Vârfuri pentru cercul de sus
        for (int i = 0; i <= segments; i++) {
            float angle = 2.0f * M_PI * i / segments;
            float x = radius * cos(angle);
            float z = radius * sin(angle);
            vertices.push_back(VertexFormat(glm::vec3(x, height / 2, z), RED));
        }

        // Vârfuri pentru cercul de jos
        for (int i = 0; i <= segments; i++) {
            float angle = 2.0f * M_PI * i / segments;
            float x = radius * cos(angle);
            float z = radius * sin(angle);
            vertices.push_back(VertexFormat(glm::vec3(x, -height / 2, z), RED));
        }

        // Centrul cercului de sus
        vertices.push_back(VertexFormat(glm::vec3(0, height / 2, 0), RED));
        int topCenter = vertices.size() - 1;

        // Centrul cercului de jos
        vertices.push_back(VertexFormat(glm::vec3(0, -height / 2, 0), RED));
        int bottomCenter = vertices.size() - 1;

        // Indici pentru suprafața laterală
        for (int i = 0; i < segments; i++) {
            indices.push_back(i);
            indices.push_back(i + segments + 1);
            indices.push_back(i + 1);

            indices.push_back(i + 1);
            indices.push_back(i + segments + 1);
            indices.push_back(i + segments + 2);
        }

        // Indici pentru capacul de sus
        for (int i = 0; i < segments; i++) {
            indices.push_back(topCenter);
            indices.push_back(i);
            indices.push_back(i + 1);
        }

        // Indici pentru capacul de jos
        for (int i = 0; i < segments; i++) {
            indices.push_back(bottomCenter);
            indices.push_back(i + segments + 2);
            indices.push_back(i + segments + 1);
        }

        Mesh* cylinder = new Mesh("cylinder");
        cylinder->InitFromData(vertices, indices);
        AddMeshToList(cylinder);
    }
}

void Tema2::UpdateTrainPosition(float deltaTime)
{
    if (train.currentRail == nullptr || gameOver) return;

    if (train.stoppedAtTrafficLight) {
        return; // Nu ne mișcăm
    }

    if (train.stoppedAtDamage) {
        return; // Nu ne mișcăm
    }

    // Dacă trenul este la stație, așteptăm
    if (train.isAtStation) {
        train.stationTimer += deltaTime;

        // Când a trecut timpul de așteptare, plecăm
        if (train.stationTimer >= train.stationWaitTime) {
            train.isAtStation = false;
            train.stationTimer = 0.0f;
            train.currentStation = nullptr;
            std::cout << "Train departing from station" << std::endl;
        }
        else {
            // Rămânem în loc
            return;
        }
    }

    // Actualizăm progresul pe șina curentă (doar dacă nu suntem la stație)
    float oldProgress = train.progress;
    train.progress += deltaTime * train.speed;

    // Verificăm dacă trecem prin poziția unei stații pe șina curentă
    for (Station& station : stations) {
        if (station.attachedRail == train.currentRail) {
            // Verificăm dacă am trecut prin poziția stației
            if (oldProgress < station.stopPosition && train.progress >= station.stopPosition) {
                // Oprim trenul exact la poziția stației
                train.progress = station.stopPosition;
                train.isAtStation = true;
                train.stationTimer = 0.0f;
                train.currentStation = &station;
                std::cout << "Train arrived at " << station.name << std::endl;

                // Actualizăm poziția și ieșim
                Rail* rail = train.currentRail;
                glm::vec3 basePosition = glm::mix(rail->startPosition, rail->endPosition, train.progress);
                train.position = basePosition;
                train.direction = glm::normalize(rail->endPosition - rail->startPosition);
                return;
            }
        }
    }

    // Dacă am ajuns la finalul șinei
    if (train.progress >= 1.0f) {
        train.progress = 0.0f;
        train.currentRail = train.currentRail->next;

        // Verificăm dacă mai avem șine
        if (train.currentRail == nullptr) {
            train.currentRail = railRoad[0];
        }
    }

    // Interpolăm liniar poziția
    Rail* rail = train.currentRail;
    glm::vec3 basePosition = glm::mix(rail->startPosition, rail->endPosition, train.progress);

    // Adăugăm înălțimea șinei
    train.position = basePosition;

    // Calculăm direcția
    train.direction = glm::normalize(rail->endPosition - rail->startPosition);
}

glm::vec3 Tema2::GetVagonPosition(int vagonIndex)
{
    if (train.currentRail == nullptr) return glm::vec3(0);

    float vagonSpacing = 3.0f; // Distanța între vagoane
    float totalDistance = vagonSpacing * (vagonIndex + 1);

    // Calculăm poziția pe care ar trebui să fie vagonul
    Rail* rail = train.currentRail;
    float progress = train.progress;

    // Parcurgem înapoi pe șine pentru a găsi poziția vagonului
    while (totalDistance > 0 && rail != nullptr) {
        float segmentLength = glm::length(rail->endPosition - rail->startPosition);
        float remainingOnSegment = segmentLength * progress;

        if (totalDistance <= remainingOnSegment) {
            // Vagonul este pe acest segment
            float vagonProgress = (remainingOnSegment - totalDistance) / segmentLength;
            return glm::mix(rail->startPosition, rail->endPosition, vagonProgress);
        }

        totalDistance -= remainingOnSegment;

        // Trecem la segmentul anterior
        Rail* prevRail = nullptr;
        for (Rail* r : railRoad) {
            if (r->next == rail) {
                prevRail = r;
                break;
            }
        }

        if (prevRail == nullptr) {
            // Am ajuns la începutul circuitului, mergem la sfârșit
            prevRail = railRoad[railRoad.size() - 1];
        }

        rail = prevRail;
        progress = 1.0f;
    }

    return train.position;
}

glm::vec3 Tema2::GetVagonDirection(int vagonIndex)
{
    if (train.currentRail == nullptr) return glm::vec3(1, 0, 0);

    float vagonSpacing = 3.0f;
    float totalDistance = vagonSpacing * (vagonIndex + 1);

    Rail* rail = train.currentRail;
    float progress = train.progress;

    while (totalDistance > 0 && rail != nullptr) {
        float segmentLength = glm::length(rail->endPosition - rail->startPosition);
        float remainingOnSegment = segmentLength * progress;

        if (totalDistance <= remainingOnSegment) {
            return glm::normalize(rail->endPosition - rail->startPosition);
        }

        totalDistance -= remainingOnSegment;

        Rail* prevRail = nullptr;
        for (Rail* r : railRoad) {
            if (r->next == rail) {
                prevRail = r;
                break;
            }
        }

        if (prevRail == nullptr) {
            prevRail = railRoad[railRoad.size() - 1];
        }

        rail = prevRail;
        progress = 1.0f;
    }

    return train.direction;
}

void Tema2::DrawLocomotive(glm::vec3 position, glm::vec3 direction, glm::vec3 color)
{
    glm::mat4 modelMatrix = glm::mat4(1);

    // Normalizăm direcția
    glm::vec3 forward = glm::normalize(direction);
    glm::vec3 up = glm::vec3(0, 1, 0);
    glm::vec3 right = glm::normalize(glm::cross(up, forward));
    glm::vec3 actualUp = glm::cross(forward, right);

    // Creăm matricea de rotație bazată pe vectorii de direcție
    glm::mat4 rotationMatrix = glm::mat4(1);
    rotationMatrix[0] = glm::vec4(right, 0);
    rotationMatrix[1] = glm::vec4(actualUp, 0);
    rotationMatrix[2] = glm::vec4(forward, 0);

    // Încăperea vatmanului (paralelipiped vertical)
    modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, position + glm::vec3(0, 0.8f, 0));
    modelMatrix = modelMatrix * rotationMatrix;
    modelMatrix = glm::scale(modelMatrix, glm::vec3(0.8f, 1.0f, 1.2f));
    RenderMesh(meshes["cube_green"], shaders["VertexColor"], modelMatrix);

    // Zona motorului (cilindru orizontal) - în față
    modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, position + glm::vec3(0, 0.5f, 0));
    modelMatrix = modelMatrix * rotationMatrix;
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0, 0, 0.6f));
    modelMatrix = glm::rotate(modelMatrix, float(M_PI / 2), glm::vec3(1, 0, 0));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(0.6f, 1.0f, 0.6f));
    RenderMesh(meshes["cylinder_blue"], shaders["VertexColor"], modelMatrix);

    // Roțile locomotivei
    float wheelRadius = 0.25f;
    float wheelWidth = 0.15f;
    float wheelSpacing = 0.35f;

    for (int i = 0; i < 7; i++) {
        float zOffset = -1.0f + i * wheelSpacing;

        // Roată stânga
        modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, position);
        modelMatrix = modelMatrix * rotationMatrix;
        modelMatrix = glm::translate(modelMatrix, glm::vec3(-0.5f, wheelRadius - 0.1f, zOffset));
        modelMatrix = glm::rotate(modelMatrix, float(M_PI / 2), glm::vec3(1, 0, 0));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(wheelRadius, wheelWidth, wheelRadius));
        RenderMesh(meshes["cylinder"], shaders["VertexColor"], modelMatrix);

        // Roată dreapta
        modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, position);
        modelMatrix = modelMatrix * rotationMatrix;
        modelMatrix = glm::translate(modelMatrix, glm::vec3(0.5f, wheelRadius - 0.1f, zOffset));
        modelMatrix = glm::rotate(modelMatrix, float(M_PI / 2), glm::vec3(1, 0, 0));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(wheelRadius, wheelWidth, wheelRadius));
        RenderMesh(meshes["cylinder"], shaders["VertexColor"], modelMatrix);
    }

    // Coș locomotivă (în partea din față)
    modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, position);
    modelMatrix = modelMatrix * rotationMatrix;
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0, 1.3f, 0.8f));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(0.3f, 0.4f, 0.3f));
    RenderMesh(meshes["cylinder_black"], shaders["VertexColor"], modelMatrix);
}

void Tema2::DrawVagon(glm::vec3 position, glm::vec3 direction, glm::vec3 color, int index)
{
    glm::mat4 modelMatrix = glm::mat4(1);

    // Normalizăm direcția
    glm::vec3 forward = glm::normalize(direction);
    glm::vec3 up = glm::vec3(0, 1, 0);
    glm::vec3 right = glm::normalize(glm::cross(up, forward));
    glm::vec3 actualUp = glm::cross(forward, right);

    // Creăm matricea de rotație bazată pe vectorii de direcție
    glm::mat4 rotationMatrix = glm::mat4(1);
    rotationMatrix[0] = glm::vec4(right, 0);
    rotationMatrix[1] = glm::vec4(actualUp, 0);
    rotationMatrix[2] = glm::vec4(forward, 0);

    // Corpul vagonului
    modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, position + glm::vec3(0, 0.7f, 0));
    modelMatrix = modelMatrix * rotationMatrix;
    modelMatrix = glm::scale(modelMatrix, glm::vec3(1.0f, 0.8f, 2.0f));
    RenderMesh(meshes["cube_green"], shaders["VertexColor"], modelMatrix);

    // Linie galbenă decorativă
    modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, position + glm::vec3(0, 0.25f, 0));
    modelMatrix = modelMatrix * rotationMatrix;
    modelMatrix = glm::scale(modelMatrix, glm::vec3(1.05f, 0.05f, 2.0f));
    RenderMesh(meshes["cube_yellow"], shaders["VertexColor"], modelMatrix);

    // Roțile vagonului
    float wheelRadius = 0.25f;
    float wheelWidth = 0.15f;

    // Roată stânga-față
    modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, position);
    modelMatrix = modelMatrix * rotationMatrix;
    modelMatrix = glm::translate(modelMatrix, glm::vec3(-0.6f, wheelRadius - 0.1f, 0.7f));
    modelMatrix = glm::rotate(modelMatrix, float(M_PI / 2), glm::vec3(1, 0, 0));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(wheelRadius, wheelWidth, wheelRadius));
    RenderMesh(meshes["cylinder"], shaders["VertexColor"], modelMatrix);

    // Roată dreapta-față
    modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, position);
    modelMatrix = modelMatrix * rotationMatrix;
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0.6f, wheelRadius - 0.1f, 0.7f));
    modelMatrix = glm::rotate(modelMatrix, float(M_PI / 2), glm::vec3(1, 0, 0));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(wheelRadius, wheelWidth, wheelRadius));
    RenderMesh(meshes["cylinder"], shaders["VertexColor"], modelMatrix);

    // Roată stânga-spate
    modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, position);
    modelMatrix = modelMatrix * rotationMatrix;
    modelMatrix = glm::translate(modelMatrix, glm::vec3(-0.6f, wheelRadius - 0.1f, -0.7f));
    modelMatrix = glm::rotate(modelMatrix, float(M_PI / 2), glm::vec3(1, 0, 0));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(wheelRadius, wheelWidth, wheelRadius));
    RenderMesh(meshes["cylinder"], shaders["VertexColor"], modelMatrix);

    // Roată dreapta-spate
    modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, position);
    modelMatrix = modelMatrix * rotationMatrix;
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0.6f, wheelRadius - 0.1f, -0.7f));
    modelMatrix = glm::rotate(modelMatrix, float(M_PI / 2), glm::vec3(1, 0, 0));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(wheelRadius, wheelWidth, wheelRadius));
    RenderMesh(meshes["cylinder"], shaders["VertexColor"], modelMatrix);
}

void Tema2::DrawNormalRail(Rail* rail)
{
    if (rail->isDamaged) {
        DrawDamagedRail(rail);
        return;
    }

    glm::vec3 start = rail->startPosition;
    glm::vec3 end = rail->endPosition;
    glm::vec3 center = (start + end) / 2.0f;
    float length = glm::length(end - start);
    glm::vec3 direction = glm::normalize(end - start);
    float angle = atan2(direction.z, direction.x);

    glm::mat4 modelMatrix;

    // Șină stânga
    modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, center + glm::vec3(0, 0.05f, 0));
    modelMatrix = glm::rotate(modelMatrix, angle, glm::vec3(0, 1, 0));
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0, 0, -0.3f));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(length, 0.1f, 0.1f));
    RenderMesh(meshes["cube"], shaders["VertexColor"], modelMatrix);

    // Șină dreapta
    modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, center + glm::vec3(0, 0.05f, 0));
    modelMatrix = glm::rotate(modelMatrix, angle, glm::vec3(0, 1, 0));
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0, 0, 0.3f));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(length, 0.1f, 0.1f));
    RenderMesh(meshes["cube"], shaders["VertexColor"], modelMatrix);

    // Traverse (4-5 traverse per segment)
    int numTraverse = 5;
    for (int i = 0; i < numTraverse; i++) {
        float t = (float)i / (numTraverse - 1);
        glm::vec3 pos = glm::mix(start, end, t);

        modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, pos + glm::vec3(0, 0.03f, 0));
        modelMatrix = glm::rotate(modelMatrix, angle, glm::vec3(0, 1, 0));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.1f, 0.08f, 0.8f));
        RenderMesh(meshes["cube_brown"], shaders["VertexColor"], modelMatrix);
    }
}

void Tema2::DrawBridgeRail(Rail* rail)
{

    if (rail->isDamaged) {
        DrawDamagedRail(rail);
        return;
    }

    glm::vec3 start = rail->startPosition;
    glm::vec3 end = rail->endPosition;
    glm::vec3 center = (start + end) / 2.0f;
    float length = glm::length(end - start);
    glm::vec3 direction = glm::normalize(end - start);
    float angle = atan2(direction.z, direction.x);

    glm::mat4 modelMatrix;

    // Platforma podului (mai înaltă)
    float bridgeHeight = 0.0f;

    // Platformă solidă
    modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, center + glm::vec3(0, bridgeHeight - 0.1f, 0));
    modelMatrix = glm::rotate(modelMatrix, angle, glm::vec3(0, 1, 0));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(length, 0.15f, 1.0f));
    RenderMesh(meshes["cube_brown"], shaders["VertexColor"], modelMatrix);

    // Șină stânga (pe platformă)
    modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, center + glm::vec3(0, bridgeHeight, 0));
    modelMatrix = glm::rotate(modelMatrix, angle, glm::vec3(0, 1, 0));
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0, 0, -0.3f));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(length, 0.1f, 0.1f));
    RenderMesh(meshes["cube"], shaders["VertexColor"], modelMatrix);

    // Șină dreapta (pe platformă)
    modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, center + glm::vec3(0, bridgeHeight, 0));
    modelMatrix = glm::rotate(modelMatrix, angle, glm::vec3(0, 1, 0));
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0, 0, 0.3f));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(length, 0.1f, 0.1f));
    RenderMesh(meshes["cube"], shaders["VertexColor"], modelMatrix);

    // Stâlpi de susținere (6-8 stâlpi)
    int numPillars = 6;
    for (int i = 0; i < numPillars; i++) {
        float t = (float)i / (numPillars - 1);
        glm::vec3 pos = glm::mix(start, end, t);

        // Stâlp stânga
        modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, pos + glm::vec3(0, bridgeHeight + 0.4f, 0));
        modelMatrix = glm::rotate(modelMatrix, angle, glm::vec3(0, 1, 0));
        modelMatrix = glm::translate(modelMatrix, glm::vec3(0, 0, -0.4f));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.12f, bridgeHeight - 0.2f, 0.12f));
        RenderMesh(meshes["cube_brown"], shaders["VertexColor"], modelMatrix);

        // Stâlp dreapta
        modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, pos + glm::vec3(0, bridgeHeight + 0.4f, 0));
        modelMatrix = glm::rotate(modelMatrix, angle, glm::vec3(0, 1, 0));
        modelMatrix = glm::translate(modelMatrix, glm::vec3(0, 0, 0.4f));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.12f, bridgeHeight - 0.2f, 0.12f));
        RenderMesh(meshes["cube_brown"], shaders["VertexColor"], modelMatrix);
    }

    // Balustrade laterale
    modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, center + glm::vec3(0, bridgeHeight + 0.5f, 0));
    modelMatrix = glm::rotate(modelMatrix, angle, glm::vec3(0, 1, 0));
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0, 0, -0.5f));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(length, 0.08f, 0.08f));
    RenderMesh(meshes["cube_brown"], shaders["VertexColor"], modelMatrix);

    modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, center + glm::vec3(0, bridgeHeight + 0.5f, 0));
    modelMatrix = glm::rotate(modelMatrix, angle, glm::vec3(0, 1, 0));
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0, 0, 0.5f));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(length, 0.08f, 0.08f));
    RenderMesh(meshes["cube_brown"], shaders["VertexColor"], modelMatrix);
}

void Tema2::DrawTunnelRail(Rail* rail)
{
    if (rail->isDamaged) {
        DrawDamagedRail(rail);
        return;
    }

    glm::vec3 start = rail->startPosition;
    glm::vec3 end = rail->endPosition;
    glm::vec3 center = (start + end) / 2.0f;
    float length = glm::length(end - start);
    glm::vec3 direction = glm::normalize(end - start);
    float angle = atan2(direction.z, direction.x);

    glm::mat4 modelMatrix;

    // Șină stânga
    modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, center + glm::vec3(0, 0.05f, 0));
    modelMatrix = glm::rotate(modelMatrix, angle, glm::vec3(0, 1, 0));
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0, 0, -0.3f));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(length, 0.1f, 0.1f));
    RenderMesh(meshes["cube"], shaders["VertexColor"], modelMatrix);

    // Șină dreapta
    modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, center + glm::vec3(0, 0.05f, 0));
    modelMatrix = glm::rotate(modelMatrix, angle, glm::vec3(0, 1, 0));
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0, 0, 0.3f));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(length, 0.1f, 0.1f));
    RenderMesh(meshes["cube"], shaders["VertexColor"], modelMatrix);

    //// Pereți laterali ai tunelului (gri închis - stâncă)
    float tunnelHeight = 2.5f;
    float tunnelWidth = 1.5f;

    // Perete stânga
    modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, center + glm::vec3(0, tunnelHeight / 2, 0));
    modelMatrix = glm::rotate(modelMatrix, angle, glm::vec3(0, 1, 0));
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0, 0, -tunnelWidth / 2));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(length, tunnelHeight, 0.2f));
    RenderMesh(meshes["cube_grey"], shaders["VertexColor"], modelMatrix);

    // Perete dreapta
    modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, center + glm::vec3(0, tunnelHeight / 2, 0));
    modelMatrix = glm::rotate(modelMatrix, angle, glm::vec3(0, 1, 0));
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0, 0, tunnelWidth / 2));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(length, tunnelHeight, 0.2f));
    RenderMesh(meshes["cube_grey"], shaders["VertexColor"], modelMatrix);

    // Tavan tunel (mai îngust, formă de arc)
    modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, center + glm::vec3(0, tunnelHeight, 0));
    modelMatrix = glm::rotate(modelMatrix, angle, glm::vec3(0, 1, 0));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(length, 0.2f, tunnelWidth));
    RenderMesh(meshes["cube_grey"], shaders["VertexColor"], modelMatrix);

}

void Tema2::DrawRails()
{
    for (Rail* rail : railRoad) {
        switch (rail->type) {
        case NORMAL:
            DrawNormalRail(rail);
            break;
        case BRIDGE:
            DrawBridgeRail(rail);
            break;
        case TUNNEL:
            DrawTunnelRail(rail);
            break;
        }
    }
}

void Tema2::FrameStart()
{
    glClearColor(0.53f, 0.81f, 0.92f, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glm::ivec2 resolution = window->GetResolution();
    glViewport(0, 0, resolution.x, resolution.y);
}

void Tema2::Update(float deltaTimeSeconds)
{
    if (!gameOver) {
        UpdateDamageSystem(deltaTimeSeconds);
        UpdateTrafficLights(deltaTimeSeconds);
        CheckTrainDamageCollision();
        CheckTrainTrafficLightCollision();
        CheckGameOver();
        UpdateTrainPosition(deltaTimeSeconds);
        UpdateHandcarPosition(deltaTimeSeconds);
        UpdateHandcarCamera();
    }

    DrawTerrain();

    for (Rail* rail : railRoad) {
        if (rail->type == NORMAL) {
            DrawNormalRail(rail);
        }
        else if (rail->type == BRIDGE) {
            DrawBridgeRail(rail);
        }
    }

    DrawStations();

    for (const TrafficLight& light : trafficLights) {
        DrawTrafficLight(light);
    }

    DrawLocomotive(train.position, train.direction, GREEN);

    for (int i = 0; i < numVagons; i++) {
        glm::vec3 vagonPos = GetVagonPosition(i);
        glm::vec3 vagonDir = GetVagonDirection(i);
        DrawVagon(vagonPos, vagonDir, GREEN, i);
    }

    DrawHandcar(handcar.position, handcar.visualDirection, handcar.handleRotation);

    for (Rail* rail : railRoad) {
        if (rail->type == TUNNEL) {
            DrawTunnelRail(rail);
        }
    }

    // Desenăm minimap-ul
    DrawMinimap();

    if (gameOver) {
        DrawGameOverScreen();
    }
}

void Tema2::FrameEnd()
{
}

void Tema2::OnInputUpdate(float deltaTime, int mods)
{
    // Controlăm viteza trenului
    if (!cameraFollowHandcar) {
        if (window->KeyHold(GLFW_KEY_UP)) {
            train.speed += 0.1f * deltaTime;
        }
        if (window->KeyHold(GLFW_KEY_DOWN)) {
            train.speed -= 0.1f * deltaTime;
            if (train.speed < 0) train.speed = 0;
        }
    }

    // Reparare continuă dacă ții apăsat F
    if (window->KeyHold(GLFW_KEY_F) && cameraFollowHandcar) {
        if (railBeingRepaired && railBeingRepaired->isBeingRepaired) {
            // Deja se repară, UpdateRepair se ocupă automat
        }
        else {
            // Începe reparare
            RepairNearbyRail();
        }
    }
}

Mesh* Tema2::CreateColoredCube(const std::string& name, glm::vec3 color)
{
    std::vector<VertexFormat> vertices =
    {
        VertexFormat(glm::vec3(-0.5, -0.5,  0.5), color),
        VertexFormat(glm::vec3(0.5, -0.5,  0.5), color),
        VertexFormat(glm::vec3(0.5,  0.5,  0.5), color),
        VertexFormat(glm::vec3(-0.5,  0.5,  0.5), color),

        VertexFormat(glm::vec3(-0.5, -0.5, -0.5), color),
        VertexFormat(glm::vec3(0.5, -0.5, -0.5), color),
        VertexFormat(glm::vec3(0.5,  0.5, -0.5), color),
        VertexFormat(glm::vec3(-0.5,  0.5, -0.5), color)
    };

    std::vector<unsigned int> indices =
    {
        0,1,2, 0,2,3,
        1,5,6, 1,6,2,
        5,4,7, 5,7,6,
        4,0,3, 4,3,7,
        3,2,6, 3,6,7,
        4,5,1, 4,1,0
    };

    Mesh* cube = new Mesh(name);
    cube->InitFromData(vertices, indices);
    AddMeshToList(cube);
    return cube;
}

Mesh* Tema2::CreateColoredCylinder(const std::string& name, glm::vec3 color)
{
    std::vector<VertexFormat> vertices;
    std::vector<unsigned int> indices;

    int segments = 32;
    float radius = 0.5f;
    float height = 1.0f;

    // Cerc sus
    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f * M_PI * i / segments;
        float x = radius * cos(angle);
        float z = radius * sin(angle);
        vertices.push_back(VertexFormat(glm::vec3(x, height / 2, z), color));
    }

    // Cerc jos
    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f * M_PI * i / segments;
        float x = radius * cos(angle);
        float z = radius * sin(angle);
        vertices.push_back(VertexFormat(glm::vec3(x, -height / 2, z), color));
    }

    // Centre capace
    vertices.push_back(VertexFormat(glm::vec3(0, height / 2, 0), color));
    int topCenter = vertices.size() - 1;

    vertices.push_back(VertexFormat(glm::vec3(0, -height / 2, 0), color));
    int bottomCenter = vertices.size() - 1;

    // Suprafață laterală
    for (int i = 0; i < segments; i++) {
        indices.push_back(i);
        indices.push_back(i + segments + 1);
        indices.push_back(i + 1);

        indices.push_back(i + 1);
        indices.push_back(i + segments + 1);
        indices.push_back(i + segments + 2);
    }

    // Capac sus
    for (int i = 0; i < segments; i++) {
        indices.push_back(topCenter);
        indices.push_back(i);
        indices.push_back(i + 1);
    }

    // Capac jos
    for (int i = 0; i < segments; i++) {
        indices.push_back(bottomCenter);
        indices.push_back(i + segments + 2);
        indices.push_back(i + segments + 1);
    }

    Mesh* cylinder = new Mesh(name);
    cylinder->InitFromData(vertices, indices);
    AddMeshToList(cylinder);

    return cylinder;
}

Mesh* Tema2::CreateColoredQuad(const std::string& name, glm::vec3 color)
{
    std::vector<VertexFormat> vertices =
    {
        VertexFormat(glm::vec3(-0.5, 0, -0.5), color),
        VertexFormat(glm::vec3(0.5, 0, -0.5), color),
        VertexFormat(glm::vec3(0.5, 0, 0.5), color),
        VertexFormat(glm::vec3(-0.5, 0, 0.5), color)
    };

    std::vector<unsigned int> indices =
    {
        0, 1, 2,
        0, 2, 3
    };

    Mesh* quad = new Mesh(name);
    quad->InitFromData(vertices, indices);
    AddMeshToList(quad);
    return quad;
}


void Tema2::CreateTerrain()
{
    glm::vec3 GRASS = glm::vec3(0.6f, 0.9f, 0.4f);
    glm::vec3 WATER = glm::vec3(0.3f, 0.6f, 0.9f);
    glm::vec3 MOUNT = glm::vec3(0.8f, 0.7f, 0.5f);

    // Zonă mare de iarbă (fundalul)
    terrainTiles.push_back(TerrainTile(glm::vec3(0, -0.1f, 0), GRASS, glm::vec2(50, 50)));

    // Canal de apă (forma L)
    terrainTiles.push_back(TerrainTile(glm::vec3(0, -0.05f, 5), WATER, glm::vec2(40, 2)));
    terrainTiles.push_back(TerrainTile(glm::vec3(5, -0.05f, -10), WATER, glm::vec2(2, 30)));

    // Zonă de nisip (în colț)
    terrainTiles.push_back(TerrainTile(glm::vec3(-8, 0, -8), MOUNT, glm::vec2(12, 12)));
}

void Tema2::DrawTerrain()
{
    for (const TerrainTile& tile : terrainTiles) {
        glm::mat4 modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, tile.position);
        modelMatrix = glm::scale(modelMatrix, glm::vec3(tile.size.x, 1, tile.size.y));

        // Creăm un mesh temporar cu culoarea corespunzătoare
        // Sau folosim un mesh generic și îi schimbăm culoarea
        std::string meshName = "quad_" + std::to_string((int)(tile.color.r * 100)) +
            "_" + std::to_string((int)(tile.color.g * 100)) +
            "_" + std::to_string((int)(tile.color.b * 100));

        if (meshes.find(meshName) == meshes.end()) {
            CreateColoredQuad(meshName, tile.color);
        }

        RenderMesh(meshes[meshName], shaders["VertexColor"], modelMatrix);
    }
}

void Tema2::CreateStations()
{
    // Selectăm 3 șine pentru stații
    // De exemplu: șina 1, șina 4 și șina 6

    if (railRoad.size() >= 6) {
        // Stația 1 - lângă prima șină
        Rail* rail1 = railRoad[1];
        glm::vec3 center1 = (rail1->startPosition + rail1->endPosition) / 2.0f;
        glm::vec3 dir1 = glm::normalize(rail1->endPosition - rail1->startPosition);
        glm::vec3 perpendicular1 = glm::vec3(-dir1.z, 0, dir1.x); // Perpendicular pe stânga
        glm::vec3 stationPos1 = center1 + perpendicular1 * 3.0f; // 3 unități la stânga șinei
        stations.push_back(Station(stationPos1, rail1, "Station Alpha"));

        // Stația 2 - lângă a patra șină
        Rail* rail2 = railRoad[4];
        glm::vec3 center2 = (rail2->startPosition + rail2->endPosition) / 2.0f;
        glm::vec3 dir2 = glm::normalize(rail2->endPosition - rail2->startPosition);
        glm::vec3 perpendicular2 = glm::vec3(-dir2.z, 0, dir2.x);
        glm::vec3 stationPos2 = center2 + perpendicular2 * 3.0f;
        stations.push_back(Station(stationPos2, rail2, "Station Beta"));

        // Stația 3 - lângă a șasea șină
        Rail* rail3 = railRoad[6];
        glm::vec3 center3 = (rail3->startPosition + rail3->endPosition) / 2.0f;
        glm::vec3 dir3 = glm::normalize(rail3->endPosition - rail3->startPosition);
        glm::vec3 perpendicular3 = glm::vec3(dir3.z, 0, -dir3.x); // Perpendicular pe dreapta
        glm::vec3 stationPos3 = center3 + perpendicular3 * 3.0f;
        stations.push_back(Station(stationPos3, rail3, "Station Gamma"));
    }
}

void Tema2::DrawStation(const Station& station)
{
    glm::mat4 modelMatrix;

    // Calculăm orientarea stației bazată pe șina atașată
    glm::vec3 railDir = glm::normalize(station.attachedRail->endPosition -
        station.attachedRail->startPosition);
    float angle = atan2(railDir.z, railDir.x);

	float stationHeight = 0.0f;

    // Platforma stației (pardoseală)
    modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, station.position + glm::vec3(0, stationHeight + 0.05f, 0));
    modelMatrix = glm::rotate(modelMatrix, angle, glm::vec3(0, 1, 0));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(4.0f, 0.1f, 2.5f));
    RenderMesh(meshes["cube_grey"], shaders["VertexColor"], modelMatrix);

    // Clădirea stației (corp principal)
    modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, station.position + glm::vec3(0, stationHeight + 0.8f, 0));
    modelMatrix = glm::rotate(modelMatrix, angle, glm::vec3(0, 1, 0));
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0, 0, -0.8f)); // Mutăm în spate
    modelMatrix = glm::scale(modelMatrix, glm::vec3(3.5f, 1.5f, 1.0f));
    RenderMesh(meshes["cube_red"], shaders["VertexColor"], modelMatrix);

    // Acoperiș
    modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, station.position + glm::vec3(0, stationHeight + 1.6f, 0));
    modelMatrix = glm::rotate(modelMatrix, angle, glm::vec3(0, 1, 0));
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0, 0, -0.8f));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(3.8f, 0.2f, 1.3f));
    RenderMesh(meshes["cube_brown"], shaders["VertexColor"], modelMatrix);

    // Copertină deasupra platformei
    modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, station.position + glm::vec3(0, stationHeight + 1.8f, 0));
    modelMatrix = glm::rotate(modelMatrix, angle, glm::vec3(0, 1, 0));
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0, 0, 0.3f));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(3.5f, 0.1f, 1.8f));
    RenderMesh(meshes["cube_yellow"], shaders["VertexColor"], modelMatrix);

    // Stâlpi de susținere pentru copertină (4 stâlpi)
    for (int i = 0; i < 4; i++) {
        float xOffset = -1.5f + i * 1.0f;

        modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, station.position + glm::vec3(0, stationHeight + 0.9f, 0));
        modelMatrix = glm::rotate(modelMatrix, angle, glm::vec3(0, 1, 0));
        modelMatrix = glm::translate(modelMatrix, glm::vec3(xOffset, 0, 0.3f));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.1f, 1.7f, 0.1f));
        RenderMesh(meshes["cube_grey"], shaders["VertexColor"], modelMatrix);
    }

    // Bancă pentru pasageri
    modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, station.position + glm::vec3(0, stationHeight + 0.3f, 0));
    modelMatrix = glm::rotate(modelMatrix, angle, glm::vec3(0, 1, 0));
    modelMatrix = glm::translate(modelMatrix, glm::vec3(-1.0f, 0, 0.5f));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(1.0f, 0.3f, 0.4f));
    RenderMesh(meshes["cube_brown"], shaders["VertexColor"], modelMatrix);

    // Semn/Indicator (stâlp cu panou)
    modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, station.position + glm::vec3(0, stationHeight + 1.2f, 0));
    modelMatrix = glm::rotate(modelMatrix, angle, glm::vec3(0, 1, 0));
    modelMatrix = glm::translate(modelMatrix, glm::vec3(1.5f, 0, 0.8f));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(0.08f, 1.0f, 0.08f));
    RenderMesh(meshes["cube_black"], shaders["VertexColor"], modelMatrix);

    // Panou indicator
    modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, station.position + glm::vec3(0, stationHeight + 1.7f, 0));
    modelMatrix = glm::rotate(modelMatrix, angle, glm::vec3(0, 1, 0));
    modelMatrix = glm::translate(modelMatrix, glm::vec3(1.5f, 0, 0.8f));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(0.6f, 0.4f, 0.05f));
    RenderMesh(meshes["cube_blue"], shaders["VertexColor"], modelMatrix);
}

void Tema2::DrawStations()
{
    for (const Station& station : stations) {
        DrawStation(station);
    }
}

bool Tema2::IsNearStation(Station* station)
{
    if (train.currentRail != station->attachedRail) {
        return false;
    }

    // Verificăm dacă progresul trenului este aproape de poziția de oprire a stației
    float threshold = 0.05f; // Toleranță de 5%
    return std::abs(train.progress - station->stopPosition) < threshold;
}

Station* Tema2::CheckIfAtStation()
{
    for (Station& station : stations) {
        if (IsNearStation(&station)) {
            return &station;
        }
    }
    return nullptr;
}

void Tema2::DrawHandcar(glm::vec3 position, glm::vec3 direction, float handleRotation)
{
    glm::mat4 modelMatrix = glm::mat4(1);

    // Folosim visualDirection pentru orientarea drezinei
    glm::vec3 forward = glm::normalize(handcar.visualDirection);
    glm::vec3 up = glm::vec3(0, 1, 0);
    glm::vec3 right = glm::normalize(glm::cross(up, forward));
    glm::vec3 actualUp = glm::cross(forward, right);

    glm::mat4 rotationMatrix = glm::mat4(1);
    rotationMatrix[0] = glm::vec4(right, 0);
    rotationMatrix[1] = glm::vec4(actualUp, 0);
    rotationMatrix[2] = glm::vec4(forward, 0);

    // Platforma drezinei
    modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, position + glm::vec3(0, 0.3f, 0));
    modelMatrix = modelMatrix * rotationMatrix;
    modelMatrix = glm::scale(modelMatrix, glm::vec3(0.8f, 0.2f, 1.2f));
    RenderMesh(meshes["cube_orange"], shaders["VertexColor"], modelMatrix);

    // Suport pentru mâner
    modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, position + glm::vec3(0, 0.6f, 0));
    modelMatrix = modelMatrix * rotationMatrix;
    modelMatrix = glm::scale(modelMatrix, glm::vec3(0.1f, 0.6f, 0.1f));
    RenderMesh(meshes["cube_brown"], shaders["VertexColor"], modelMatrix);

    // Mânerul (animat)
    modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, position + glm::vec3(0, 0.9f, 0));
    modelMatrix = modelMatrix * rotationMatrix;
    modelMatrix = glm::rotate(modelMatrix, handleRotation, glm::vec3(0, 1, 0));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(0.8f, 0.08f, 0.08f));
    RenderMesh(meshes["cube_green"], shaders["VertexColor"], modelMatrix);

    // Capete mâner stânga
    modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, position + glm::vec3(0, 0.9f, 0));
    modelMatrix = modelMatrix * rotationMatrix;
    modelMatrix = glm::rotate(modelMatrix, handleRotation, glm::vec3(0, 1, 0));
    modelMatrix = glm::translate(modelMatrix, glm::vec3(-0.4f, 0, 0));
    modelMatrix = glm::rotate(modelMatrix, float(M_PI / 2), glm::vec3(0, 0, 1));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(0.1f, 0.15f, 0.1f));
    RenderMesh(meshes["cylinder"], shaders["VertexColor"], modelMatrix);

    // Capete mâner dreapta
    modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, position + glm::vec3(0, 0.9f, 0));
    modelMatrix = modelMatrix * rotationMatrix;
    modelMatrix = glm::rotate(modelMatrix, handleRotation, glm::vec3(0, 1, 0));
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0.4f, 0, 0));
    modelMatrix = glm::rotate(modelMatrix, float(M_PI / 2), glm::vec3(0, 0, 1));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(0.1f, 0.15f, 0.1f));
    RenderMesh(meshes["cylinder"], shaders["VertexColor"], modelMatrix);

    // Roțile
    float wheelRadius = 0.15f;
    float wheelWidth = 0.12f;

    // Roată față-stânga
    modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, position);
    modelMatrix = modelMatrix * rotationMatrix;
    modelMatrix = glm::translate(modelMatrix, glm::vec3(-0.35f, wheelRadius, 0.45f));
    modelMatrix = glm::rotate(modelMatrix, float(M_PI / 2), glm::vec3(1, 0, 0));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(wheelRadius, wheelWidth, wheelRadius));
    RenderMesh(meshes["cylinder"], shaders["VertexColor"], modelMatrix);

    // Roată față-dreapta
    modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, position);
    modelMatrix = modelMatrix * rotationMatrix;
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0.35f, wheelRadius, 0.45f));
    modelMatrix = glm::rotate(modelMatrix, float(M_PI / 2), glm::vec3(1, 0, 0));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(wheelRadius, wheelWidth, wheelRadius));
    RenderMesh(meshes["cylinder"], shaders["VertexColor"], modelMatrix);

    // Roată spate-stânga
    modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, position);
    modelMatrix = modelMatrix * rotationMatrix;
    modelMatrix = glm::translate(modelMatrix, glm::vec3(-0.35f, wheelRadius, -0.45f));
    modelMatrix = glm::rotate(modelMatrix, float(M_PI / 2), glm::vec3(1, 0, 0));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(wheelRadius, wheelWidth, wheelRadius));
    RenderMesh(meshes["cylinder"], shaders["VertexColor"], modelMatrix);

    // Roată spate-dreapta
    modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, position);
    modelMatrix = modelMatrix * rotationMatrix;
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0.35f, wheelRadius, -0.45f));
    modelMatrix = glm::rotate(modelMatrix, float(M_PI / 2), glm::vec3(1, 0, 0));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(wheelRadius, wheelWidth, wheelRadius));
    RenderMesh(meshes["cylinder"], shaders["VertexColor"], modelMatrix);
}

void Tema2::UpdateHandcarPosition(float deltaTime)
{
    if (handcar.currentRail == nullptr) return;

    if (!cameraFollowHandcar) {
        return;
    }

    // Animația mânerului
    if (window->KeyHold(GLFW_KEY_W) || window->KeyHold(GLFW_KEY_S)) {
        handcar.handleRotation += deltaTime * handcar.handleRotationSpeed;
        if (handcar.handleRotation > 2 * M_PI) {
            handcar.handleRotation -= 2 * M_PI;
        }
    }

    // Control înainte/înapoi
    float movement = 0.0f;
    if (window->KeyHold(GLFW_KEY_S)) {
        movement = deltaTime * handcar.speed;
        handcar.movingForward = true;
    }
    if (window->KeyHold(GLFW_KEY_W)) {
        movement = -deltaTime * handcar.speed;
        handcar.movingForward = false;
    }

    // Actualizăm progresul
    handcar.progress += movement;

    // Gestionăm schimbarea șinei
    if (handcar.progress >= 1.0f) {
        handcar.progress -= 1.0f;
        handcar.currentRail = handcar.currentRail->next;
        if (handcar.currentRail == nullptr) {
            handcar.currentRail = railRoad[0];
        }
    }
    else if (handcar.progress < 0.0f) {
        Rail* prevRail = nullptr;
        for (Rail* r : railRoad) {
            if (r->next == handcar.currentRail) {
                prevRail = r;
                break;
            }
        }
        if (prevRail == nullptr) {
            prevRail = railRoad[railRoad.size() - 1];
        }
        handcar.currentRail = prevRail;
        handcar.progress = 1.0f + handcar.progress;
    }

    // Calculăm poziția
    Rail* rail = handcar.currentRail;
    glm::vec3 basePosition = glm::mix(rail->startPosition, rail->endPosition, handcar.progress);
    handcar.position = basePosition;

    // Direcția șinei
    glm::vec3 railDirection = glm::normalize(rail->endPosition - rail->startPosition);

    // Actualizăm visualDirection DOAR când mergem înainte
    // Când mergem înapoi, păstrăm direcția anterioară
    if (handcar.movingForward) {
        handcar.visualDirection = railDirection;
    }
    // Când mergem înapoi (S), visualDirection rămâne neschimbat!

    // Direction pentru calcule interne
    handcar.direction = railDirection;
}

void Tema2::UpdateHandcarCamera()
{
    auto camera = GetSceneCamera();

    if (cameraFollowHandcar) {
        // Parametri optimizați pentru TPS
        float distance = 3.0f;      // Distanță în spate (înainte era 1.0f)
        float height = 2.0f;         // Înălțime deasupra (înainte era 1.0f)
        float pitchAngle = -20.0f;   // Unghi de privire în jos (înainte era -10.0f)

        // Poziția camerei în spatele și deasupra drezinei
        glm::vec3 cameraPosition = handcar.position
            - handcar.visualDirection * distance
            + glm::vec3(0, height, 0);

        // CORECȚIE CRITICĂ: calculăm yaw corect!
        // Pentru direcția în plan XZ: atan2(z, x) și ajustăm cu -90 grade
        float yaw = atan2(handcar.visualDirection.z, handcar.visualDirection.x) - M_PI / 2.0f;

        // Convertim pitch din grade în radiani
        float pitch = glm::radians(pitchAngle);

        camera->SetPosition(cameraPosition);
        camera->SetRotation(glm::vec3(pitch, yaw, 0.0f));
        camera->Update();
    }
}

void Tema2::OnKeyPress(int key, int mods)
{
    if (key == GLFW_KEY_R && gameOver) {
        RestartGame();
        return;
    }

    // Exit game
    if (key == GLFW_KEY_ESCAPE) {
        if (gameOver) {
            // Închide aplicația
            window->Close();
        }
    }

    if (gameOver) return;

    if (key == GLFW_KEY_F && cameraFollowHandcar) {
        RepairNearbyRail();
    }

    if (key == GLFW_KEY_C) {
        cameraFollowHandcar = !cameraFollowHandcar;

        if (cameraFollowHandcar) {
            // Activăm modul TPS - dezactivăm controlul manual
            GetCameraInput()->SetActive(false);
            std::cout << "Camera: TPS mode (following handcar)" << std::endl;
            std::cout << "Controls: W/S to move handcar, C to toggle camera" << std::endl;
        }
        else {
            // Activăm modul liber - permitem controlul manual
            GetCameraInput()->SetActive(true);
            std::cout << "Camera: Free mode" << std::endl;
            std::cout << "Controls: Mouse to look, WASD/QE to move camera, C to toggle" << std::endl;
        }
    }

    if (key == GLFW_KEY_M) {
        showMinimap = !showMinimap;
        std::cout << "Minimap: " << (showMinimap ? "ON" : "OFF") << std::endl;
    }
}

void Tema2::UpdateDamageSystem(float deltaTime)
{
    if (gameOver) return;

    // Spawn avarii noi
    damageSpawnTimer += deltaTime;
    if (damageSpawnTimer >= damageSpawnInterval) {
        damageSpawnTimer = 0.0f;
        SpawnRandomDamage();
    }

    // Actualizăm severitatea avariilor
    UpdateDamageSeverity(deltaTime);

    // Actualizăm procesul de reparare
    UpdateRepair(deltaTime);
}

void Tema2::SpawnRandomDamage()
{
    if (gameOver) return;

    // Găsim șine care nu sunt deja avariate
    std::vector<Rail*> healthyRails;
    for (Rail* rail : railRoad) {
        if (!rail->isDamaged) {
            healthyRails.push_back(rail);
        }
    }

    if (healthyRails.empty()) return;

    // Alegem o șină random
    int randomIndex = rand() % healthyRails.size();
    Rail* targetRail = healthyRails[randomIndex];

    targetRail->isDamaged = true;
    targetRail->damageTime = 0.0f;
    targetRail->flickerTimer = 0.0f;
    damagedRails++;

    std::cout << "Rail damaged! Total damaged: " << damagedRails << "/" << totalRails << std::endl;
}

void Tema2::CheckTrainDamageCollision()
{
    if (gameOver) return;

    // Verificăm dacă trenul se apropie de o șină avariată
    Rail* nextRail = train.currentRail->next;

    if (nextRail && nextRail->isDamaged && train.progress > 0.8f) {
        // Trenul se apropie de o avarie
        if (!train.stoppedAtDamage) {
            train.stoppedAtDamage = true;
            train.damageWaitTime = 0.0f;
            std::cout << "Train stopped at damaged rail!" << std::endl;
        }

        // Oprim trenul
        train.progress = 0.8f;

        // Cronometrăm cât timp așteaptă
        train.damageWaitTime += GetLastFrameTime();

    }
    else if (train.currentRail->isDamaged && train.progress < 0.2f) {
        // Trenul este pe o șină care tocmai s-a avariat
        if (!train.stoppedAtDamage) {
            train.stoppedAtDamage = true;
            train.damageWaitTime = 0.0f;
        }
        train.progress = 0.2f;
        train.damageWaitTime += GetLastFrameTime();

    }
    else {
        // Trenul poate circula normal
        if (train.stoppedAtDamage) {
            train.stoppedAtDamage = false;
            train.damageWaitTime = 0.0f;
            std::cout << "Train resumed movement!" << std::endl;
        }
    }
}

Rail* Tema2::GetNearestDamagedRail()
{
    Rail* nearest = nullptr;
    float minDistance = repairRadius;

    for (Rail* rail : railRoad) {
        if (rail->isDamaged) {
            // Calculăm centrul șinei
            glm::vec3 railCenter = (rail->startPosition + rail->endPosition) / 2.0f;
            railCenter.y += rail->height;

            float distance = glm::length(handcar.position - railCenter);

            if (distance < minDistance) {
                minDistance = distance;
                nearest = rail;
            }
        }
    }

    return nearest;
}

void Tema2::RepairNearbyRail()
{
    if (gameOver) return;

    Rail* damagedRail = GetNearestDamagedRail();

    if (damagedRail) {
        // Începem sau continuăm repararea
        if (!damagedRail->isBeingRepaired) {
            StartRepair(damagedRail);
        }
        // Dacă deja e în reparare, continuăm automat în UpdateRepair
    }
    else {
        std::cout << "No damaged rail nearby to repair!" << std::endl;
    }
}

void Tema2::CheckGameOver()
{
    if (gameOver) return;

    // Condiție 1: Trenul așteaptă prea mult
    if (train.damageWaitTime >= train.maxDamageWaitTime) {
        gameOver = true;
        gameOverReason = "Train waited too long at damaged rail!";
        std::cout << "GAME OVER: " << gameOverReason << std::endl;
        return;
    }

    // Condiție 2: Prea multe șine avariate
    float damagePercentage = (float)damagedRails / (float)totalRails;
    if (damagePercentage > 0.5f) {
        gameOver = true;
        gameOverReason = "Too many damaged rails (>50%)!";
        std::cout << "GAME OVER: " << gameOverReason << std::endl;
        return;
    }
}

void Tema2::DrawDamagedRail(Rail* rail)
{
    glm::vec3 start = rail->startPosition;
    glm::vec3 end = rail->endPosition;
    glm::vec3 center = (start + end) / 2.0f;
    float length = glm::length(end - start);
    glm::vec3 direction = glm::normalize(end - start);
    float angle = atan2(direction.z, direction.x);

    glm::mat4 modelMatrix;

    // Culoare în funcție de severitate cu flicker
    float flickerIntensity = 0.5f + 0.5f * sin(rail->flickerTimer);
    glm::vec3 baseColor = GetDamageColor(rail->damageSeverity);
    glm::vec3 damageColor = baseColor * flickerIntensity;

    // Deformare crește cu severitatea
    float deformAmount = 0.2f + 0.15f * rail->damageSeverity;
    float twist = sin(rail->flickerTimer) * (0.1f + 0.1f * rail->damageSeverity);

    // Creăm mesh cu culoarea avariată
    std::string meshName = "cube_damaged_" + std::to_string(rail->damageSeverity) +
        "_" + std::to_string((int)(flickerIntensity * 10));
    if (meshes.find(meshName) == meshes.end()) {
        CreateColoredCube(meshName, damageColor);
    }

    // Șină stânga (deformată)
    modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, center + glm::vec3(0, rail->height + 0.05f + deformAmount * sin(rail->flickerTimer * 2), 0));
    modelMatrix = glm::rotate(modelMatrix, angle + twist, glm::vec3(0, 1, 0));
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0, 0, -0.3f));
    modelMatrix = glm::rotate(modelMatrix, twist * 2, glm::vec3(1, 0, 0));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(length * (1.0f - 0.05f * rail->damageSeverity), 0.12f, 0.08f));
    RenderMesh(meshes[meshName], shaders["VertexColor"], modelMatrix);

    // Șină dreapta (deformată)
    modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, center + glm::vec3(0, rail->height + 0.05f + deformAmount * sin(rail->flickerTimer * 2 + M_PI), 0));
    modelMatrix = glm::rotate(modelMatrix, angle - twist, glm::vec3(0, 1, 0));
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0, 0, 0.3f));
    modelMatrix = glm::rotate(modelMatrix, -twist * 2, glm::vec3(1, 0, 0));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(length * (1.0f - 0.05f * rail->damageSeverity), 0.12f, 0.08f));
    RenderMesh(meshes[meshName], shaders["VertexColor"], modelMatrix);

    // Traverse sparte (mai puține pentru severitate mai mare)
    int numTraverse = 4 - rail->damageSeverity;
    for (int i = 0; i < numTraverse; i++) {
        float t = (float)(i * 2) / 8.0f;
        glm::vec3 pos = glm::mix(start, end, t);

        modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, pos + glm::vec3(0, rail->height + 0.03f, 0));
        modelMatrix = glm::rotate(modelMatrix, angle + sin(rail->flickerTimer + i) * 0.5f, glm::vec3(0, 1, 0));
        modelMatrix = glm::rotate(modelMatrix, sin(rail->flickerTimer * 3 + i) * 0.3f, glm::vec3(1, 0, 0));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.08f, 0.06f, 0.7f));
        RenderMesh(meshes[meshName], shaders["VertexColor"], modelMatrix);
    }

    // Desenăm bara de progres dacă e în reparare
    if (rail->isBeingRepaired) {
        DrawRepairProgress(rail);
    }
}

void Tema2::DrawGameOverScreen()
{
    // Desenăm un overlay semi-transparent peste toată scena
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Overlay întunecat (negru semi-transparent)
    glm::mat4 modelMatrix = glm::mat4(1);

    // Trebuie să desenăm în spațiul 2D al ecranului
    // Salvăm starea depth test
    GLboolean depthTestEnabled;
    glGetBooleanv(GL_DEPTH_TEST, &depthTestEnabled);
    glDisable(GL_DEPTH_TEST);

    // Desenăm background-ul întunecat
    // (Opțional - dacă vrei un overlay vizual)

    glDisable(GL_BLEND);
    if (depthTestEnabled) {
        glEnable(GL_DEPTH_TEST);
    }

    // Desenăm textul
    float centerX = screenWidth / 2.0f;
    float centerY = screenHeight / 2.0f;

    // Titlu GAME OVER
    text->RenderText("GAME OVER", centerX - 200, centerY + 100, 1.5f, glm::vec3(1.0f, 0.0f, 0.0f));

    // Motivul
    text->RenderText(gameOverReason, centerX - 300, centerY + 20, 0.8f, glm::vec3(1.0f, 1.0f, 1.0f));

    // Statistici
    std::string stats = "Damaged Rails: " + std::to_string(damagedRails) + "/" + std::to_string(totalRails);
    text->RenderText(stats, centerX - 200, centerY - 40, 0.6f, glm::vec3(0.8f, 0.8f, 0.8f));

    if (train.stoppedAtDamage) {
        std::string waitTime = "Train waited: " + std::to_string((int)train.damageWaitTime) + "s";
        text->RenderText(waitTime, centerX - 180, centerY - 80, 0.6f, glm::vec3(0.8f, 0.8f, 0.8f));
    }

    // Instrucțiuni
    text->RenderText("Press R to Restart", centerX - 180, centerY - 140, 0.7f, glm::vec3(0.0f, 1.0f, 0.0f));
    text->RenderText("Press ESC to Exit", centerX - 160, centerY - 180, 0.7f, glm::vec3(1.0f, 1.0f, 0.0f));
}

void Tema2::CreateTrafficLights()
{
    // Semafor 1 - Colț stânga-sus (controlează șina din acea zonă)
    if (railRoad.size() > 0) {
        Rail* rail1 = railRoad[1]; // Alege o șină din colțul stânga-sus
        glm::vec3 railCenter1 = (rail1->startPosition + rail1->endPosition) / 2.0f;
        glm::vec3 pos1 = railCenter1 + glm::vec3(-2.0f, 0, 0); // Lângă șină

        TrafficLight light1(pos1, rail1);
        light1.state = GREEN_LIGHT;
        trafficLights.push_back(light1);

        std::cout << "Traffic Light 1 created at position: ("
            << pos1.x << ", " << pos1.y << ", " << pos1.z << ")" << std::endl;
    }

    // Semafor 2 - Colț dreapta-jos (opus primului)
    if (railRoad.size() > 5) {
        Rail* rail2 = railRoad[5]; // Alege o șină din colțul dreapta-jos
        glm::vec3 railCenter2 = (rail2->startPosition + rail2->endPosition) / 2.0f;
        glm::vec3 pos2 = railCenter2 + glm::vec3(2.0f, 0, 0); // Lângă șină

        TrafficLight light2(pos2, rail2);
        light2.state = RED_LIGHT; // Începe pe roșu (opus primului)
        trafficLights.push_back(light2);

        std::cout << "Traffic Light 2 created at position: ("
            << pos2.x << ", " << pos2.y << ", " << pos2.z << ")" << std::endl;
    }
}

void Tema2::UpdateTrafficLights(float deltaTime)
{
    for (TrafficLight& light : trafficLights) {
        light.stateTimer += deltaTime;

        // Schimbăm starea în funcție de timp
        switch (light.state) {
        case GREEN_LIGHT:
            if (light.stateTimer >= light.greenDuration) {
                light.state = YELLOW_LIGHT;
                light.stateTimer = 0.0f;
                std::cout << "Traffic light switching to YELLOW" << std::endl;
            }
            break;

        case YELLOW_LIGHT:
            if (light.stateTimer >= light.yellowDuration) {
                light.state = RED_LIGHT;
                light.stateTimer = 0.0f;
                std::cout << "Traffic light switching to RED" << std::endl;
            }
            break;

        case RED_LIGHT:
            if (light.stateTimer >= light.redDuration) {
                light.state = GREEN_LIGHT;
                light.stateTimer = 0.0f;
                std::cout << "Traffic light switching to GREEN" << std::endl;
            }
            break;
        }
    }
}

void Tema2::CheckTrainTrafficLightCollision()
{
    if (gameOver || train.stoppedAtDamage || train.isAtStation) return;

    bool shouldStop = false;
    TrafficLight* blockingLight = nullptr;

    // Verificăm fiecare semafor
    for (TrafficLight& light : trafficLights) {
        // Verificăm dacă trenul se apropie de șina controlată de semafor
        Rail* nextRail = train.currentRail->next;

        if (train.currentRail == light.controlledRail || nextRail == light.controlledRail) {
            // Suntem pe sau aproape de șina controlată

            if (light.state == RED_LIGHT) {
                // Semafor roșu - STOP
                shouldStop = true;
                blockingLight = &light;

                // Oprim trenul înainte să intre pe șină
                if (train.currentRail == light.controlledRail && train.progress < 0.3f) {
                    train.progress = 0.3f;
                }
                else if (nextRail == light.controlledRail && train.progress > 0.7f) {
                    train.progress = 0.7f;
                }
                break;

            }
            else if (light.state == YELLOW_LIGHT) {
                // Semafor galben - Încetinește
                if (train.progress > 0.5f) {
                    // Dacă suntem aproape, oprim
                    shouldStop = true;
                    blockingLight = &light;
                    if (train.progress > 0.7f) {
                        train.progress = 0.7f;
                    }
                }
                break;
            }
        }
    }

    if (shouldStop) {
        if (!train.stoppedAtTrafficLight) {
            train.stoppedAtTrafficLight = true;
            train.currentTrafficLight = blockingLight;
            std::cout << "Train stopped at traffic light!" << std::endl;
        }
    }
    else {
        if (train.stoppedAtTrafficLight) {
            train.stoppedAtTrafficLight = false;
            train.currentTrafficLight = nullptr;
            std::cout << "Train can proceed (green light)!" << std::endl;
        }
    }
}

void Tema2::DrawTrafficLight(const TrafficLight& light)
{
    glm::mat4 modelMatrix;
    float height = light.controlledRail->height;

    // Stâlpul semaforului (negru)
    modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, light.position + glm::vec3(0, height + 1.5f, 0));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(0.15f, 3.0f, 0.15f));
    RenderMesh(meshes["cube_black"], shaders["VertexColor"], modelMatrix);

    // Cutia semaforului (gri închis)
    modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, light.position + glm::vec3(0, height + 3.2f, 0));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(0.4f, 1.2f, 0.3f));
    RenderMesh(meshes["cube_dark_grey"], shaders["VertexColor"], modelMatrix);

    // Lămpile semaforului (3 poziții)
    float lampRadius = 0.2f;  // Mărește puțin pentru vizibilitate

    // Lumină ROȘIE (sus)
    modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, light.position + glm::vec3(0.2f, height + 3.6f, 0)); // Mut în față
    modelMatrix = glm::rotate(modelMatrix, float(M_PI / 2), glm::vec3(1, 0, 0)); // Rotesc pe X
    modelMatrix = glm::scale(modelMatrix, glm::vec3(lampRadius, 0.1f, lampRadius)); // Disc plat
    if (light.state == RED_LIGHT) {
        RenderMesh(meshes["cylinder_red_bright"], shaders["VertexColor"], modelMatrix);
    }
    else {
        RenderMesh(meshes["cylinder_red_dark"], shaders["VertexColor"], modelMatrix);
    }

    // Lumină GALBENĂ (mijloc)
    modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, light.position + glm::vec3(0.2f, height + 3.2f, 0));
    modelMatrix = glm::rotate(modelMatrix, float(M_PI / 2), glm::vec3(1, 0, 0));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(lampRadius, 0.1f, lampRadius));
    if (light.state == YELLOW_LIGHT) {
        RenderMesh(meshes["cylinder_yellow_bright"], shaders["VertexColor"], modelMatrix);
    }
    else {
        RenderMesh(meshes["cylinder_yellow_dark"], shaders["VertexColor"], modelMatrix);
    }

    // Lumină VERDE (jos)
    modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, light.position + glm::vec3(0.2f, height + 2.8f, 0));
    modelMatrix = glm::rotate(modelMatrix, float(M_PI / 2), glm::vec3(1, 0, 0));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(lampRadius, 0.1f, lampRadius));
    if (light.state == GREEN_LIGHT) {
        RenderMesh(meshes["cylinder_green_bright"], shaders["VertexColor"], modelMatrix);
    }
    else {
        RenderMesh(meshes["cylinder_green_dark"], shaders["VertexColor"], modelMatrix);
    }
}

void Tema2::UpdateDamageSeverity(float deltaTime)
{
    if (gameOver) return;

    for (Rail* rail : railRoad) {
        if (rail->isDamaged) {
            // Incrementăm timpul de avarie
            rail->damageTime += deltaTime;

            // Verificăm dacă trebuie să creștem severitatea
            int newSeverity = 1 + (int)(rail->damageTime / severityIncreaseInterval);

            if (newSeverity > rail->damageSeverity && newSeverity <= 3) {
                rail->damageSeverity = newSeverity;

                // Recalculăm timpul de reparare
                rail->currentRepairTime = rail->baseRepairTime * rail->damageSeverity;

                std::cout << "Rail damage severity increased to " << rail->damageSeverity
                    << " (repair time: " << rail->currentRepairTime << "s)" << std::endl;
            }

            // Actualizăm flicker-ul (mai rapid pentru severitate mai mare)
            rail->flickerTimer += deltaTime * (3.0f + rail->damageSeverity);
            if (rail->flickerTimer > 2 * M_PI) {
                rail->flickerTimer -= 2 * M_PI;
            }
        }
    }
}

void Tema2::StartRepair(Rail* rail)
{
    if (!rail || !rail->isDamaged) return;

    if (!rail->isBeingRepaired) {
        rail->isBeingRepaired = true;
        rail->repairProgress = 0.0f;
        railBeingRepaired = rail;

        std::cout << "Started repairing rail (Severity: " << rail->damageSeverity
            << ", Time needed: " << rail->currentRepairTime << "s)" << std::endl;
    }
}

void Tema2::UpdateRepair(float deltaTime)
{
    if (!railBeingRepaired || !railBeingRepaired->isBeingRepaired) {
        railBeingRepaired = nullptr;
        return;
    }

    // Verificăm dacă drezina este încă în raza de reparare
    glm::vec3 railCenter = (railBeingRepaired->startPosition + railBeingRepaired->endPosition) / 2.0f;
    railCenter.y += railBeingRepaired->height;
    float distance = glm::length(handcar.position - railCenter);

    if (distance > repairRadius) {
        // Drezina a ieșit din rază - anulăm repararea
        railBeingRepaired->isBeingRepaired = false;
        railBeingRepaired->repairProgress = 0.0f;
        railBeingRepaired = nullptr;

        std::cout << "Repair cancelled - moved out of range!" << std::endl;
        return;
    }

    // Continuăm repararea
    railBeingRepaired->repairProgress += deltaTime / railBeingRepaired->currentRepairTime;

    // Verificăm dacă am terminat
    if (railBeingRepaired->repairProgress >= 1.0f) {
        // Reparare completă!
        railBeingRepaired->isDamaged = false;
        railBeingRepaired->damageTime = 0.0f;
        railBeingRepaired->damageSeverity = 1;
        railBeingRepaired->repairProgress = 0.0f;
        railBeingRepaired->isBeingRepaired = false;
        railBeingRepaired->currentRepairTime = railBeingRepaired->baseRepairTime;

        damagedRails--;

        std::cout << "Rail repaired successfully! Remaining damaged: "
            << damagedRails << "/" << totalRails << std::endl;

        // Dacă trenul aștepta, îl repornim
        if (train.stoppedAtDamage) {
            if (train.currentRail->next == railBeingRepaired || train.currentRail == railBeingRepaired) {
                train.stoppedAtDamage = false;
                train.damageWaitTime = 0.0f;
            }
        }

        railBeingRepaired = nullptr;
    }
}

glm::vec3 Tema2::GetDamageColor(int severity)
{
    switch (severity) {
    case 1: // Minor - Portocaliu
        return glm::vec3(1.0f, 0.6f, 0.0f);
    case 2: // Mediu - Roșu
        return glm::vec3(1.0f, 0.2f, 0.0f);
    case 3: // Grav - Roșu închis
        return glm::vec3(0.8f, 0.0f, 0.0f);
    default:
        return glm::vec3(1.0f, 0.0f, 0.0f);
    }
}

void Tema2::DrawRepairProgress(Rail* rail)
{
    if (!rail || !rail->isBeingRepaired) return;

    glm::vec3 railCenter = (rail->startPosition + rail->endPosition) / 2.0f;
    railCenter.y += rail->height;

    glm::mat4 modelMatrix;

    // Fundal bară (roșu)
    modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, railCenter + glm::vec3(0, 2.0f, 0));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(2.0f, 0.2f, 0.1f));
    RenderMesh(meshes["cube_red"], shaders["VertexColor"], modelMatrix);

    // Progres bară (verde)
    if (rail->repairProgress > 0.0f) {
        modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, railCenter + glm::vec3(-1.0f + rail->repairProgress, 2.0f, 0));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(2.0f * rail->repairProgress, 0.25f, 0.15f));
        RenderMesh(meshes["cube_green"], shaders["VertexColor"], modelMatrix);
    }

    // Text indicator severitate (cuburi colorate)
    for (int i = 0; i < rail->damageSeverity; i++) {
        modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, railCenter + glm::vec3(-0.6f + i * 0.6f, 2.5f, 0));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.3f, 0.3f, 0.1f));

        glm::vec3 severityColor = GetDamageColor(rail->damageSeverity);
        std::string meshName = "cube_severity_" + std::to_string(rail->damageSeverity);
        if (meshes.find(meshName) == meshes.end()) {
            CreateColoredCube(meshName, severityColor);
        }
        RenderMesh(meshes[meshName], shaders["VertexColor"], modelMatrix);
    }
}

void Tema2::RestartGame()
{
    std::cout << "Restarting game..." << std::endl;

    // Resetăm game over
    gameOver = false;
    gameOverReason = "";

    // Resetăm toate șinele
    for (Rail* rail : railRoad) {
        rail->isDamaged = false;
        rail->damageTime = 0.0f;
        rail->flickerTimer = 0.0f;
        rail->damageSeverity = 1;
        rail->repairProgress = 0.0f;
        rail->isBeingRepaired = false;
        rail->currentRepairTime = rail->baseRepairTime;
    }

    // Resetăm contorul de avarii
    damagedRails = 0;
    damageSpawnTimer = 0.0f;
    railBeingRepaired = nullptr;

    // Resetăm trenul
    if (!railRoad.empty()) {
        train.currentRail = railRoad[0];
        train.progress = 0.0f;
        train.speed = 0.2f;
        train.position = train.currentRail->startPosition;
        train.isAtStation = false;
        train.stationTimer = 0.0f;
        train.currentStation = nullptr;
        train.stoppedAtDamage = false;
        train.damageWaitTime = 0.0f;
        train.stoppedAtTrafficLight = false;
        train.currentTrafficLight = nullptr;
    }

    // Resetăm drezina
    if (!railRoad.empty()) {
        handcar.currentRail = railRoad[2];
        handcar.progress = 0.3f;
        handcar.speed = 0.4f;

        Rail* rail = handcar.currentRail;
        glm::vec3 basePosition = glm::mix(rail->startPosition, rail->endPosition, handcar.progress);
        handcar.position = basePosition + glm::vec3(0, rail->height, 0);
        handcar.direction = glm::normalize(rail->endPosition - rail->startPosition);
        handcar.visualDirection = handcar.direction;

    }

    // Resetăm semafoarele
    for (TrafficLight& light : trafficLights) {
        light.state = GREEN_LIGHT;
        light.stateTimer = 0.0f;
    }

    // Opțional: randomizăm starea inițială a unui semafor
    if (trafficLights.size() > 1) {
        trafficLights[1].state = RED_LIGHT;
    }

    std::cout << "Game restarted successfully!" << std::endl;
}

glm::vec2 Tema2::WorldToMinimapPosition(glm::vec3 worldPos)
{
    // Convertim coordonatele world (3D) în coordonate minimap (2D)
    float relativeX = (worldPos.x - minimapCenter.x) * minimapScale;
    float relativeZ = (worldPos.z - minimapCenter.z) * minimapScale;

    // Centrăm pe minimap
    float minimapCenterX = minimapX + minimapSize / 2.0f;
    float minimapCenterY = minimapY + minimapSize / 2.0f;

    return glm::vec2(
        minimapCenterX + relativeX,
        minimapCenterY - relativeZ  // Inversăm Z pentru că Y crește în jos pe ecran
    );
}

void Tema2::DrawMinimapElement(glm::vec3 worldPos, glm::vec3 color, float size)
{
    glm::vec2 screenPos = WorldToMinimapPosition(worldPos);

    // Verificăm dacă e în cadrul minimap-ului
    if (screenPos.x < minimapX || screenPos.x > minimapX + minimapSize ||
        screenPos.y < minimapY || screenPos.y > minimapY + minimapSize) {
        return;  // În afara minimap-ului
    }

    // Desenăm un pătrat mic la acea poziție
    // Folosim text renderer pentru a desena primitive 2D simple
    // Sau putem folosi mesh-uri simple
}

void Tema2::DrawMinimap()
{
    if (!showMinimap) return;

    // Salvăm stările OpenGL
    GLboolean depthTestEnabled;
    glGetBooleanv(GL_DEPTH_TEST, &depthTestEnabled);
    glDisable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Obținem camera
    auto camera = GetSceneCamera();
    glm::mat4 view = camera->GetViewMatrix();
    glm::mat4 projection = camera->GetProjectionMatrix();

    // Salvăm viewport-ul curent
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);

    // Setăm viewport pentru minimap (colț dreapta-jos)
    int minimapPixelSize = (int)minimapSize;
    int minimapPosX = screenWidth - minimapPixelSize - 20;
    int minimapPosY = 20; // Jos
    glViewport(minimapPosX, minimapPosY, minimapPixelSize, minimapPixelSize);

    // Creăm o proiecție ortografică pentru minimap
    float mapRange = 15.0f; // Câte unități world să vedem
    glm::mat4 minimapProjection = glm::ortho(-mapRange, mapRange, -mapRange, mapRange, -10.0f, 10.0f);

    // View de sus (top-down)
    glm::vec3 mapCenter = glm::vec3(0, 0, 0); // Centrul circuitului
    glm::mat4 minimapView = glm::lookAt(
        mapCenter + glm::vec3(0, 10, 0),  // Camera deasupra
        mapCenter,                         // Privește în jos
        glm::vec3(0, 0, -1)               // Up vector
    );

    // Desenăm fundalul minimap-ului
    glm::mat4 modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, mapCenter);
    modelMatrix = glm::rotate(modelMatrix, glm::radians(90.0f), glm::vec3(1, 0, 0));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(mapRange * 2, mapRange * 2, 1));

    glm::vec3 BG_COLOR = glm::vec3(0.1f, 0.1f, 0.15f);
    if (meshes.find("minimap_bg") == meshes.end()) {
        CreateColoredCube("minimap_bg", BG_COLOR);
    }

    // Folosim shader-ul și aplicăm matricele minimap
    Shader* shader = shaders["VertexColor"];
    shader->Use();
    glUniformMatrix4fv(shader->GetUniformLocation("View"), 1, GL_FALSE, glm::value_ptr(minimapView));
    glUniformMatrix4fv(shader->GetUniformLocation("Projection"), 1, GL_FALSE, glm::value_ptr(minimapProjection));
    glUniformMatrix4fv(shader->GetUniformLocation("Model"), 1, GL_FALSE, glm::value_ptr(modelMatrix));

    meshes["minimap_bg"]->Render();

    // Desenăm șinele
    for (Rail* rail : railRoad) {
        glm::vec3 start = rail->startPosition;
        glm::vec3 end = rail->endPosition;
        glm::vec3 center = (start + end) / 2.0f;
        center.y = 0; // Flatten

        float length = glm::length(end - start);
        glm::vec3 dir = glm::normalize(end - start);
        float angle = atan2(dir.z, dir.x);

        modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, center);
        modelMatrix = glm::rotate(modelMatrix, glm::radians(90.0f), glm::vec3(1, 0, 0));
        modelMatrix = glm::rotate(modelMatrix, angle, glm::vec3(0, 0, 1));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(length, 0.4f, 1));

        glm::vec3 railColor = rail->isDamaged ? glm::vec3(1.0f, 0.0f, 0.0f) : glm::vec3(0.5f, 0.5f, 0.5f);
        std::string meshName = rail->isDamaged ? "minimap_rail_damaged" : "minimap_rail";

        if (meshes.find(meshName) == meshes.end()) {
            CreateColoredCube(meshName, railColor);
        }

        glUniformMatrix4fv(shader->GetUniformLocation("Model"), 1, GL_FALSE, glm::value_ptr(modelMatrix));
        meshes[meshName]->Render();
    }

    // Desenăm stațiile (pătrate albastre)
    for (const Station& station : stations) {
        glm::vec3 pos = station.position;
        pos.y = 0.1f;

        modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, pos);
        modelMatrix = glm::rotate(modelMatrix, glm::radians(90.0f), glm::vec3(1, 0, 0));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.8f, 0.8f, 1));

        if (meshes.find("minimap_station") == meshes.end()) {
            CreateColoredCube("minimap_station", glm::vec3(0.3f, 0.6f, 1.0f));
        }

        glUniformMatrix4fv(shader->GetUniformLocation("Model"), 1, GL_FALSE, glm::value_ptr(modelMatrix));
        meshes["minimap_station"]->Render();
    }

    // Desenăm semafoarele
    for (const TrafficLight& light : trafficLights) {
        glm::vec3 pos = light.position;
        pos.y = 0.15f;

        modelMatrix = glm::mat4(1);
        modelMatrix = glm::translate(modelMatrix, pos);
        modelMatrix = glm::rotate(modelMatrix, glm::radians(90.0f), glm::vec3(1, 0, 0));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.5f, 0.5f, 1));

        glm::vec3 lightColor;
        std::string meshName;
        if (light.state == RED_LIGHT) {
            lightColor = glm::vec3(1.0f, 0.0f, 0.0f);
            meshName = "minimap_light_red";
        }
        else if (light.state == YELLOW_LIGHT) {
            lightColor = glm::vec3(1.0f, 1.0f, 0.0f);
            meshName = "minimap_light_yellow";
        }
        else {
            lightColor = glm::vec3(0.0f, 1.0f, 0.0f);
            meshName = "minimap_light_green";
        }

        if (meshes.find(meshName) == meshes.end()) {
            CreateColoredCube(meshName, lightColor);
        }

        glUniformMatrix4fv(shader->GetUniformLocation("Model"), 1, GL_FALSE, glm::value_ptr(modelMatrix));
        meshes[meshName]->Render();
    }

    // Desenăm trenul (triunghi verde)
    glm::vec3 trainPos = train.position;
    trainPos.y = 0.2f;

    float trainAngle = atan2(train.direction.z, train.direction.x);

    modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, trainPos);
    modelMatrix = glm::rotate(modelMatrix, glm::radians(90.0f), glm::vec3(1, 0, 0));
    modelMatrix = glm::rotate(modelMatrix, trainAngle - glm::radians(90.0f), glm::vec3(0, 0, 1));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(0.6f, 1.0f, 1));

    if (meshes.find("minimap_train") == meshes.end()) {
        CreateColoredCube("minimap_train", glm::vec3(0.2f, 0.8f, 0.2f));
    }

    glUniformMatrix4fv(shader->GetUniformLocation("Model"), 1, GL_FALSE, glm::value_ptr(modelMatrix));
    meshes["minimap_train"]->Render();

    // Desenăm drezina (triunghi portocaliu)
    glm::vec3 handcarPos = handcar.position;
    handcarPos.y = 0.25f;

    float handcarAngle = atan2(handcar.visualDirection.z, handcar.visualDirection.x);

    modelMatrix = glm::mat4(1);
    modelMatrix = glm::translate(modelMatrix, handcarPos);
    modelMatrix = glm::rotate(modelMatrix, glm::radians(90.0f), glm::vec3(1, 0, 0));
    modelMatrix = glm::rotate(modelMatrix, handcarAngle - glm::radians(90.0f), glm::vec3(0, 0, 1));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(0.5f, 0.8f, 1));

    if (meshes.find("minimap_handcar") == meshes.end()) {
        CreateColoredCube("minimap_handcar", glm::vec3(1.0f, 0.6f, 0.0f));
    }

    glUniformMatrix4fv(shader->GetUniformLocation("Model"), 1, GL_FALSE, glm::value_ptr(modelMatrix));
    meshes["minimap_handcar"]->Render();

    // Restaurăm viewport-ul original
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

    // Restaurăm matricele originale
    glUniformMatrix4fv(shader->GetUniformLocation("View"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(shader->GetUniformLocation("Projection"), 1, GL_FALSE, glm::value_ptr(projection));

    // Restaurăm stările
    glDisable(GL_BLEND);
    if (depthTestEnabled) {
        glEnable(GL_DEPTH_TEST);
    }
}

void Tema2::OnKeyRelease(int key, int mods)
{
}

void Tema2::OnMouseBtnPress(int mouseX, int mouseY, int button, int mods)
{
}

void Tema2::OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods)
{
}

void Tema2::OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY)
{
}

void Tema2::OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY)
{
}

void Tema2::OnWindowResize(int width, int height)
{
}