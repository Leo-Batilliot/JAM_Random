#pragma once
#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <stdexcept>
#include <string>
#include <unordered_map>

// Asset cache. First call loads from disk, next calls reuse memory.
// No create or load on repeat !
class AssetManager {
public:
    static AssetManager& instance() {
        static AssetManager instance;
        return instance;
    }

    // Use full path from project root (ex: "assets/textures/player.png").
    const sf::Texture& texture(const std::string& path) {
        auto file = _textures.find(path);
        if (file != _textures.end())
            return file->second;
        auto& texture = _textures[path];
        if (!texture.loadFromFile(path))
            throw std::runtime_error("cannot load texture: " + path);
        return texture;
    }

    const sf::Font& font(const std::string& path) {
        auto file = _fonts.find(path);
        if (file != _fonts.end())
            return file->second;
        auto& font = _fonts[path];
        if (!font.openFromFile(path))
            throw std::runtime_error("cannot load font: " + path);
        return font;
    }

    const sf::SoundBuffer& sound(const std::string& path) {
        auto file = _sounds.find(path);
        if (file != _sounds.end())
            return file->second;
        auto& sound = _sounds[path];
        if (!sound.loadFromFile(path))
            throw std::runtime_error("cannot load sound: " + path);
        return sound;
    }

private:
    AssetManager() = default;
    std::unordered_map<std::string, sf::Texture> _textures;
    std::unordered_map<std::string, sf::Font> _fonts;
    std::unordered_map<std::string, sf::SoundBuffer> _sounds;
};
