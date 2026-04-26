#pragma once
#include <functional>
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <vector>

// Looks complicated but its easier to use than to create *bruh*
// a system emits an event (ex: PlayerDiedEvent) and any other system
// can subscribe to it (HUD, VFX, audio, etc.). What this allows for exemple :
// in the combat system you can have PlayerDiedEvent without adding anything else
// and in the sfx system you can add what the PlayerDiedEvent should do (sounds, images...)
class EventBus {
public:
    template<typename Event>
    // Example: bus.subscribe<PlayerDiedEvent>([](const PlayerDiedEvent& e){ ... });
    void subscribe(std::function<void(const Event&)> handler) {
        _handlers[typeid(Event)].push_back(
            std::make_shared<Handler<Event>>(std::move(handler))
        );
    }

    void clear() { _handlers.clear(); }

    template<typename Event>
    // Example: bus.emit(PlayerDiedEvent{victimId, killerId, pos});
    void emit(const Event& event) {
        auto it = _handlers.find(typeid(Event));
        if (it == _handlers.end())
            return;
        for (auto& handler : it->second)
            static_cast<Handler<Event>*>(handler.get())->call(event);
    }

private:
    struct HandlerBase { virtual ~HandlerBase() = default; };

    template<typename Event>
    struct Handler : HandlerBase {
        std::function<void(const Event&)> fn;
        explicit Handler(std::function<void(const Event&)> f) : fn(std::move(f)) {}
        void call(const Event& e) { fn(e); }
    };

    std::unordered_map<std::type_index, std::vector<std::shared_ptr<HandlerBase>>> _handlers;
};
