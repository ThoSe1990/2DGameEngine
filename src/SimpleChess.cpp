
#include <iostream>
#include <sstream>

#include "Constants.hpp"
#include "SimpleChess.hpp"

#include "glm/glm.hpp"

#include "AssetManager.hpp"
#include "Components/TransformComponent.hpp"
#include "Components/SpriteComponent.hpp"
#include "Components/ChesspieceComponent.hpp"
#include "ChessBoard.hpp"

#include "Log.hpp"

#include "Statemachine/Statemachine.hpp"
#include "Statemachine/WhiteTurn.hpp"


EntityManager manager;

std::shared_ptr<Statemachine> statemachine;
std::shared_ptr<ChessBoard> chessBoard;
std::shared_ptr<AssetManager> SimpleChess::assetManager = std::make_shared<AssetManager>(&manager);

SDL_Event SimpleChess::event;
SDL_Renderer* SimpleChess::renderer;


SimpleChess::SimpleChess()
{
    this->isRunning = false;
}

SimpleChess::~SimpleChess()
{

}

bool SimpleChess::IsRunning() const 
{
    return this->isRunning;
}


void SimpleChess::Initialize(int width, int height)
{
    if (SDL_Init(SDL_INIT_EVERYTHING) != 0) 
    {
        std::cerr << "Error initializing SDL." << std::endl;
        return;
    }
   window = SDL_CreateWindow(
        NULL,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        width,
        height,
        SDL_WINDOW_BORDERLESS
    );

    if (!window) 
    {
        std::cerr << "Error creating SDL window." << std::endl;
        return;
    }
    renderer = SDL_CreateRenderer(window, -1, 0);

    if (!renderer) 
    {
        std::cerr << "Error creating SDL renderer." << std::endl;
        return;
    }

    InitializeLua();

    LoadAssets();   

    chessBoard = std::make_shared<ChessBoard>("board_squares", 1, Constants::square_sidelength, Constants::chessboard_offset);
    chessBoard->LoadBoard();

    LoadBoardSetup();

    statemachine = std::make_shared<Statemachine>(new (WhiteTurn), manager.GetEntities(Layer::chess_piece));
    
    isRunning = true;
    return;

}


void SimpleChess::InitializeLua()
{
    lua.open_libraries(sol::lib::base, sol::lib::os, sol::lib::math);
    lua.script_file("./assets/scripts/config.lua");

    sol::optional<int> chespieces_velocity_exists = lua["config"]["chespieces_velocity"];
    Constants::chespieces_velocity = (chespieces_velocity_exists == sol::nullopt) ? Constants::chespieces_velocity : lua["config"]["chespieces_velocity"];
    Logger::Log(logging::trivial::debug, log_location, "loading chespieces_velocity: " , Constants::chespieces_velocity);

    sol::optional<int> chessboard_offset_exists = lua["config"]["chessboard_offset"];
    Constants::chessboard_offset = (chessboard_offset_exists == sol::nullopt) ? Constants::chessboard_offset : lua["config"]["chessboard_offset"];
    Logger::Log(logging::trivial::debug, log_location, "loading chessboard_offset: " , Constants::chessboard_offset);
    
}
void SimpleChess::LoadAssets()
{
    lua.script_file("./assets/scripts/assets.lua");
    sol::table chessboardAssets = lua["chessboard_assets"];

    unsigned int index = 0;
    while (true)
    {
        sol::optional<sol::table> exists = chessboardAssets[index];
        if (exists == sol::nullopt)
            break;
        sol::table asset = chessboardAssets[index];
        std::string type = asset["type"];
        if (type.compare("texture") == 0)
        {
            std::string id = asset["id"];
            std::string file = asset["file"];  
            assetManager->AddTexture(id, file.c_str());
            Logger::Log(logging::trivial::debug, log_location, "ading asset: \n    type: " , type , "\n    id: ", id , "\n    file: ", file);
        }
        index++;
    }
}
void SimpleChess::LoadBoardSetup()
{
    lua.script_file("./assets/scripts/chessboard_setup.lua");
    sol::table chessboardSetup = lua["chessboard_setup"];

    unsigned int index = 0;
    while (true)
    {
        sol::optional<sol::table> exists = chessboardSetup[index];
        if (exists == sol::nullopt)
            break;
        sol::table chesspiece = chessboardSetup[index];
        std::string name = chesspiece["name"];
        std::string type = chesspiece["type"];
        std::string color = chesspiece["color"];
        bool killed = chesspiece["killed"];
        std::stringstream asset_id;
        asset_id << color << '_' << type;
        std::string position = chesspiece["position"];

        auto& newEntity(manager.AddEntity(name, Layer::chess_piece));
        newEntity.AddComponent<TransformComponent>(position, Constants::chesspiece_sidelength, Constants::chesspiece_sidelength, 1);
        newEntity.AddComponent<SpriteComponent>(asset_id.str().c_str());
        newEntity.AddComponent<ChesspieceComponent>(type, color, killed);
        index++;
    }
}



void SimpleChess::ProcessInput() 
{
    while (SDL_PollEvent(&event))
    {
        switch (event.type) 
        {
            case SDL_QUIT: 
            {
                isRunning = false;
                break;
            }
            case SDL_KEYDOWN:
            {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    isRunning = false;
                }
                break;
            }
            case SDL_MOUSEBUTTONDOWN:
            {
                statemachine->SetMousebutton(true);
                break;
            }
            case SDL_MOUSEBUTTONUP:
            {
                statemachine->UpdateStatemachine();
                statemachine->SetMousebutton(false);
                break;
            }
            case SDL_MOUSEMOTION:
            {
                statemachine->SetMousePosition( static_cast<int>(event.motion.x), static_cast<int>(event.motion.y) );
                break;
            }
            default: 
            {
                break;
            }
        }
    }
}


void SimpleChess::Update() 
{
    int timeToWait = FRAME_TARGET_TIME - (SDL_GetTicks() - ticksLastFrame);

    if (timeToWait > 0 && timeToWait <= FRAME_TARGET_TIME) 
        SDL_Delay(timeToWait);
    

    float deltaTime = (SDL_GetTicks() - ticksLastFrame) / 1000.0f;
    deltaTime = (deltaTime > 0.05f) ? 0.05f : deltaTime;

    ticksLastFrame = SDL_GetTicks();

    manager.Update(deltaTime);

}


void SimpleChess::Render() 
{
    SDL_SetRenderDrawColor(renderer, 21, 21, 21, 255);
    SDL_RenderClear(renderer);

    if (manager.HasNoEntities()) 
        return;

    manager.Render();

    SDL_RenderPresent(renderer);
}

void SimpleChess::Destroy() 
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}