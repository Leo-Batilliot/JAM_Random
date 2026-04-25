#pragma once

class Game;
class EventBus;
class GameState;
class InputManager;

// Shared context passed to modules so they can access core systems.
struct ModuleContext {
    Game& game;
    EventBus& bus;
    GameState& state;
    InputManager& input;
};
