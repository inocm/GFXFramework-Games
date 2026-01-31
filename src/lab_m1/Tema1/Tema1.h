#pragma once

#include "components/simple_scene.h"
#include "components/text_renderer.h"

namespace m1
{
    class Tema1 : public gfxc::SimpleScene
    {
    public:
        Tema1();
        ~Tema1();
        void Init() override;
        static Mesh* combinedMesh;
        bool startPressed = false;

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

        void Tema1::CombinePlacedBlocks();

        
        struct Block {
            Mesh* mesh;
            std::string type;
            glm::vec3 position;
        };

        std::vector<Block*> gameBlocks;
        glm::mat3 modelMatrix;


        std::vector<Block*> placedBlocks;
        bool Tema1::CheckConstraints(const std::vector<Block*>& placedBlocks, Block* newBlock);

        bool isDragging = false;         
        std::string copyName;
        std::string selectedBlock;
        glm::vec2 dragOffset;          
        glm::vec2 cursorPos;
        int placedBlocksCount = 0;
        int dragCount = 0;
        int canonCount = 0;
        int bumperCount = 0;
        int engineCount = 0;
        int solidBlockCount = 0;
        float newBlockX;
        float newBlockY;
        int NR_GREEN_SQUARES;
        Block* dragMesh = nullptr;
        Mesh* original = nullptr;
        Mesh* copy = nullptr;
        bool constraits = true;
        bool editorState = true; 
        bool draggingExistingBlock;

        glm::vec3 WHITE = glm::vec3(255, 255, 255);

        Mesh* Tema1::CreateCircle(const std::string& name, glm::vec3 center, float radius, int segments, glm::vec3 color);
        glm::vec3 Tema1::GetMeshCenter(Mesh* mesh);

        gfxc::TextRenderer* text;

        glm::vec2 paddleLeftPos;
        glm::vec2 paddleRightPos;
        glm::vec2 paddlesDims;

        float paddleWidth = 20.0f;   
        float paddleHeight = 100.0f; 

        glm::vec2 Tema1::GetMeshDimensions(Mesh* mesh);

        glm::vec2 ballPos;
        glm::vec2 ballVel; 
        float ballRadius;
        float ballSpeed;

        float windowHeight;
        float windowWidth;

        bool leftPlayerLost;
        int leftScore;
        int rightScore;

        float leftPaddleAnimTime = 0.0f;   
        float rightPaddleAnimTime = 0.0f;  
        float paddleAnimDuration = 1.0f;   
        bool leftPaddleAnimating = false;
        bool rightPaddleAnimating = false;

        bool constraints;

        bool cameraShaking = false;
        float cameraShakeTime = 0.0f;
        float cameraShakeDuration = 1.0f;   
        float cameraShakeIntensity = 10.0f; 


        void Tema1::UpdateBall(float deltaTime);
        void Tema1::HandlePaddleCollision(glm::vec2& ballPos, glm::vec2& ballVel, glm::vec2 paddleCenter, float paddleWidth, float paddleHeight, bool isLeftPaddle, float ballSpeed);
        bool Tema1::CircleAABBIntersect(glm::vec2 circlePos, float ballRadius, glm::vec2 aabbCenter, float aabbWidth, float aabbHeight);
        void Tema1::ResetBall();
        bool Tema1::CheckBumperCollision(const glm::vec2& ballPos, float radius, const Block& bumper);
         
    };
}   // namespace m1
