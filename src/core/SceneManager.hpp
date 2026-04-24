#pragma once
#include "Scene.hpp"
#include <memory>
#include <vector>

class SceneManager {
public:
    void push(std::unique_ptr<Scene> scene) {
        _commands.push_back({Action::Push, std::move(scene)});
    }

    void pop() {
        _commands.push_back({Action::Pop, nullptr});
    }

    void replace(std::unique_ptr<Scene> scene) {
        _commands.push_back({Action::Replace, std::move(scene)});
    }

    // call at end of frame so a scene doesn't delete itself while it's still running
    void applyPending() {
        for (auto& command : _commands) {
            switch (command.action) {
                case Action::Push:
                    _stack.push_back(std::move(command.scene));
                    break;
                case Action::Pop:
                    if (!_stack.empty())
                        _stack.pop_back();
                    if (!_stack.empty())
                        _stack.back()->onResume();
                    break;
                case Action::Replace:
                    if (!_stack.empty())
                        _stack.pop_back();
                    _stack.push_back(std::move(command.scene));
                    break;
            }
        }
        _commands.clear();
    }

    Scene* current() { return _stack.empty() ? nullptr : _stack.back().get(); }
    bool empty() const { return _stack.empty(); }

private:
    enum class Action {
        Push,
        Pop,
        Replace
    };
    struct Command {
        Action action;
        std::unique_ptr<Scene> scene;
    };
    std::vector<std::unique_ptr<Scene>> _stack;
    std::vector<Command> _commands;
};
