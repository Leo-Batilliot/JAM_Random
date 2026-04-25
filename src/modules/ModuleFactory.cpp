#include "ModuleFactory.hpp"
#include "combat/CombatModule.hpp"
#include "powerup/PowerUpModule.hpp"
#include "ui/UIModule.hpp"

std::vector<std::unique_ptr<IGameModule>> createGameModules(ModuleContext& context) {
    std::vector<std::unique_ptr<IGameModule>> modules;
    modules.push_back(std::make_unique<CombatModule>(context));
    modules.push_back(std::make_unique<PowerUpModule>(context));
    modules.push_back(std::make_unique<UIModule>(context));
    return modules;
}
