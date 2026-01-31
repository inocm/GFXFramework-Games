#pragma once

#include "components/simple_scene.h"
#include "components/text_renderer.h"
#include <vector>
#include "components/transform.h"

namespace m1
{
    // Structura pentru o unitate de șină
    enum RailType {
        NORMAL,  // Șine normale pe câmpie
        BRIDGE,  // Pod peste apă
        TUNNEL   // Tunel prin munte
    };

    // Structura pentru o unitate de șină
    struct Rail {
        glm::vec3 startPosition;
        glm::vec3 endPosition;
        Rail* next;
        RailType type;
        float height;

        // Pentru sistemul de avarii îmbunătățit
        bool isDamaged;
        float damageTime;           // Cât timp a fost avariată
        float flickerTimer;
        int damageSeverity;         // 1 = minor, 2 = mediu, 3 = grav
        float repairProgress;       // 0.0 - 1.0
        bool isBeingRepaired;       // Dacă e în proces de reparare
        float baseRepairTime;       // Timpul de bază pentru reparare
        float currentRepairTime;    // Timpul actual necesar (crește cu severitatea)

        Rail(glm::vec3 start, glm::vec3 end, RailType t = NORMAL)
            : startPosition(start), endPosition(end), next(nullptr), type(t),
            isDamaged(false), damageTime(0.0f), flickerTimer(0.0f),
            damageSeverity(1), repairProgress(0.0f), isBeingRepaired(false),
            baseRepairTime(2.0f), currentRepairTime(2.0f) {
            if (t == BRIDGE) {
                height = 0.8f;
            }
            else {
                height = 0.0f;
            }
        }
    };

    // Structură pentru stație
    struct Station {
        glm::vec3 position;
        Rail* attachedRail;
        std::string name;
        float stopPosition; // Poziția (0-1) pe șină unde se oprește trenul

        Station(glm::vec3 pos, Rail* rail, std::string n, float stopPos = 0.5f)
            : position(pos), attachedRail(rail), name(n), stopPosition(stopPos) {
        }
    };

    // Stări pentru semafor
    enum TrafficLightState {
        RED_LIGHT,      // Oprit
        YELLOW_LIGHT,   // Atenție
        GREEN_LIGHT    // Liber (GREEN e deja folosit pentru culoare)
    };

    // Structură pentru semafor
    struct TrafficLight {
        glm::vec3 position;
        Rail* controlledRail;  // Șina pe care o controlează
        TrafficLightState state;
        float stateTimer;
        float redDuration;
        float yellowDuration;
        float greenDuration;

        TrafficLight(glm::vec3 pos, Rail* rail)
            : position(pos), controlledRail(rail), state(GREEN_LIGHT),
            stateTimer(0.0f), redDuration(10.0f), yellowDuration(3.0f), greenDuration(10.0f) {
        }
    };

    // Structura pentru tren
    struct Train {
        Rail* currentRail;
        float progress;
        float speed;
        glm::vec3 position;
        glm::vec3 direction;

        bool isAtStation;
        float stationTimer;
        float stationWaitTime;
        Station* currentStation;

        bool stoppedAtDamage;
        float damageWaitTime;
        float maxDamageWaitTime;

        // Pentru semafoare
        bool stoppedAtTrafficLight;
        TrafficLight* currentTrafficLight;

        Train() : currentRail(nullptr), progress(0.0f), speed(0.3f),
            position(glm::vec3(0)), direction(glm::vec3(1, 0, 0)),
            isAtStation(false), stationTimer(0.0f), stationWaitTime(10.0f),
            currentStation(nullptr), stoppedAtDamage(false), damageWaitTime(0.0f),
            maxDamageWaitTime(30.0f), stoppedAtTrafficLight(false),
            currentTrafficLight(nullptr) {
        }
    };

    struct Handcar {
        Rail* currentRail;
        float progress;
        float speed;
        glm::vec3 position;
        glm::vec3 direction;
        glm::vec3 visualDirection; // Direcția vizuală (unde "privește" drezina)
        bool movingForward;
        float handleRotation;
        float handleRotationSpeed;

        Handcar() : currentRail(nullptr), progress(0.0f), speed(0.5f),
            position(glm::vec3(0)), direction(glm::vec3(1, 0, 0)),
            visualDirection(glm::vec3(1, 0, 0)), movingForward(true),
            handleRotation(0.0f), handleRotationSpeed(2.0f) {
        }
    };

    class Tema2 : public gfxc::SimpleScene
    {
    public:
        Tema2();
        ~Tema2();

        void Init() override;

    private:
        void FrameStart() override;
        void Update(float deltaTimeSeconds) override;
        void FrameEnd() override;

        void OnInputUpdate(float deltaTime, int mods) override;
        void OnKeyPress(int key, int mods) override;
        void OnKeyRelease(int key, int mods) override;
        void OnMouseMove(int mouseX, int mouseY, int deltaX, int deltaY) override;
        void OnMouseBtnPress(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseBtnRelease(int mouseX, int mouseY, int button, int mods) override;
        void OnMouseScroll(int mouseX, int mouseY, int offsetX, int offsetY) override;
        void OnWindowResize(int width, int height) override;

        // Funcții pentru desenarea trenului
        void CreateTrainMeshes();
        void DrawLocomotive(glm::vec3 position, glm::vec3 direction, glm::vec3 color);
        void DrawVagon(glm::vec3 position, glm::vec3 direction, glm::vec3 color, int index);

        // Funcții pentru șine
        void CreateRailRoad();
        void DrawRails();
        void DrawNormalRail(Rail* rail);
        void DrawBridgeRail(Rail* rail);
        void DrawTunnelRail(Rail* rail);
        void UpdateTrainPosition(float deltaTime);
        glm::vec3 GetVagonPosition(int vagonIndex);
        glm::vec3 GetVagonDirection(int vagonIndex);

        Mesh* Tema2::CreateColoredCube(const std::string& name, glm::vec3 color);
        Mesh* Tema2::CreateColoredCylinder(const std::string& name, glm::vec3 color);
        Mesh* CreateColoredQuad(const std::string& name, glm::vec3 color);
        void CreateTerrain();
        void DrawTerrain();

        // Structură pentru dale de teren
        struct TerrainTile {
            glm::vec3 position;
            glm::vec3 color;
            glm::vec2 size;

            TerrainTile(glm::vec3 pos, glm::vec3 col, glm::vec2 sz)
                : position(pos), color(col), size(sz) {
            }
        };

        std::vector<TerrainTile> terrainTiles;

        void CreateStations();
        void DrawStation(const Station& station);
        void DrawStations();

        Station* CheckIfAtStation();
        bool IsNearStation(Station* station);

    protected:
        gfxc::TextRenderer* text;
        float screenHeight;
        float screenWidth;

        std::vector<Station> stations;

        // Culori
        glm::vec3 RED;
        glm::vec3 GREEN;
        glm::vec3 BLUE;
        glm::vec3 GREY;
        glm::vec3 BLACK;
        glm::vec3 YELLOW;
        glm::vec3 BROWN;

        // Șine și tren
        std::vector<Rail*> railRoad;
        Train train;
        int numVagons;

    protected:
        Handcar handcar;
        bool cameraFollowHandcar; // Pentru a comuta între moduri de cameră

    private:
        void DrawHandcar(glm::vec3 position, glm::vec3 direction, float handleRotation);
        void UpdateHandcarPosition(float deltaTime);
        void UpdateHandcarCamera();
        glm::vec3 GetHandcarDirection(int directionSign);

    protected:
        // Sistem de avarii
        float damageSpawnTimer;
        float damageSpawnInterval;
        bool gameOver;
        std::string gameOverReason;
        float repairRadius;

        // Statistici
        int totalRails;
        int damagedRails;

    private:
        void UpdateDamageSystem(float deltaTime);
        void SpawnRandomDamage();
        void CheckTrainDamageCollision();
        void RepairNearbyRail();
        Rail* GetNearestDamagedRail();
        void DrawDamagedRail(Rail* rail);
        void CheckGameOver();
        void DrawGameOverScreen();

    protected:
        std::vector<TrafficLight> trafficLights;

    private:
        void CreateTrafficLights();
        void UpdateTrafficLights(float deltaTime);
        void DrawTrafficLight(const TrafficLight& light);
        void CheckTrainTrafficLightCollision();

    protected:
        Rail* railBeingRepaired;
        float severityIncreaseInterval;  // Cât timp până crește severitatea

    private:
        void UpdateDamageSeverity(float deltaTime);
        void StartRepair(Rail* rail);
        void UpdateRepair(float deltaTime);
        void DrawRepairProgress(Rail* rail);
        glm::vec3 GetDamageColor(int severity);

    private:
        void RestartGame();

    protected:
        // Minimap
        bool showMinimap;
        float minimapSize;
        float minimapX, minimapY;  // Poziția pe ecran
        float minimapScale;         // Scara hărții
        glm::vec3 minimapCenter;    // Centrul hărții

    private:
        void DrawMinimap();
        void DrawMinimapElement(glm::vec3 worldPos, glm::vec3 color, float size);
        glm::vec2 WorldToMinimapPosition(glm::vec3 worldPos);
    };
}