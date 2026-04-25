#pragma once
#include "IGameModule.hpp"
#include <memory>
#include <vector>

// Creates the modules in order: combat -> powerups -> UI.
std::vector<std::unique_ptr<IGameModule>> createGameModules(ModuleContext& context);
