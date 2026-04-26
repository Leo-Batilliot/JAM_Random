#include "UIModule.hpp"
#include "core/AssetManager.hpp"
#include "core/EventBus.hpp"
#include "core/GameEvents.hpp"
#include "core/GameState.hpp"
#include "core/InputManager.hpp"
#include "config/AssetConfig.hpp"
#include "config/ColorConfig.hpp"
#include "config/GameplayConfig.hpp"
#include "config/HUDConfig.hpp"
#include "config/PowerUpConfig.hpp"
#include "config/VisualConfig.hpp"
#include "config/WindowConfig.hpp"
#include <SFML/Graphics.hpp>
#include <algorithm>
#include <cmath>
#include <deque>
#include <numbers>
#include <random>
#include <string>
#include <vector>
#include <array>

namespace {

enum class AnimationState {
    IDLE = 0,
    WALK = 1,
    HIT = 2,
    DEAD = 3
};

struct AnimationFrameSet {
    std::vector<sf::Texture> frames;
    float framesPerSecond = 8.f;
    bool looping = true;
};

struct CharacterAnimation {
    std::array<AnimationFrameSet, 4> animationSet;
    AnimationState state = AnimationState::IDLE;
    int frameIndex = 0;
    float frameTimer = 0.f;
    bool animationDone = false;
    sf::Vector2f deathPosition;

    // Loads all animation frames for the character.
    bool load(const std::string& bodyDirectory) {
        struct CharacterAnimations {
            const char* prefix;
            int frameCount;
            float fps;
            bool loop;
        };
        const std::array<CharacterAnimations, 4> specs = {{
            {"idle", 6, 8.f, true},
            {"walk", 8, 10.f, true},
            {"hit", 3, 15.f, false},
            {"death", 10, 12.f, false},
        }};
        for (int animationIndex = 0; animationIndex < 4; ++animationIndex) {
            auto& frameSet = animationSet[animationIndex];
            frameSet.framesPerSecond = specs[animationIndex].fps;
            frameSet.looping = specs[animationIndex].loop;
            frameSet.frames.resize(specs[animationIndex].frameCount);
            for (int frameNumber = 0; frameNumber < specs[animationIndex].frameCount; ++frameNumber) {
                const std::string filename = std::string(specs[animationIndex].prefix) + "_" + std::to_string(frameNumber) + ".png";
                if (!frameSet.frames[frameNumber].loadFromFile(bodyDirectory + "/" + filename))
                    return false;
            }
        }
        return true;
    }
};

// Switches the animation state
void applyAnimationState(CharacterAnimation& character, AnimationState newState) {
    if (character.state == AnimationState::DEAD && newState != AnimationState::IDLE)
        return;
    if (character.state == newState)
        return;
    character.state = newState;
    character.frameIndex = 0;
    character.frameTimer = 0.f;
    character.animationDone = false;
}

// Advances the current animation based on dt
void advanceAnimation(CharacterAnimation& character, float dt) {
    const AnimationFrameSet& frameSet = character.animationSet[(int)character.state];
    if (frameSet.frames.empty())
        return;
    character.frameTimer += dt;
    const float frameStep = 1.f / frameSet.framesPerSecond;
    while (character.frameTimer >= frameStep) {
        character.frameTimer -= frameStep;
        ++character.frameIndex;
        if (character.frameIndex >= (int)frameSet.frames.size()) {
            if (frameSet.looping) {
                character.frameIndex = 0;
            } else {
                character.frameIndex = (int)frameSet.frames.size() - 1;
                character.animationDone = true;
            }
        }
    }
}

// Draws the body sprite
void renderCharacterBody(sf::RenderWindow& window, const CharacterAnimation& character,
                         sf::Vector2f position, bool facingLeft, sf::Color tint)
{
    const AnimationFrameSet& frameSet = character.animationSet[(int)character.state];
    if (frameSet.frames.empty())
        return;
    const int index = std::min(character.frameIndex, (int)frameSet.frames.size() - 1);
    const sf::Texture& texture = frameSet.frames[index];
    const auto textureSize = texture.getSize();
    sf::Sprite bodySprite(texture);
    bodySprite.setOrigin({
        textureSize.x * 0.5f,
        textureSize.y * VisualConfig::CHARACTER_Y_ORIGIN_FACTOR
    });
    bodySprite.setScale({
        facingLeft ? -VisualConfig::CHARACTER_SCALE : VisualConfig::CHARACTER_SCALE,
        VisualConfig::CHARACTER_SCALE
    });
    bodySprite.setPosition(position);
    bodySprite.setColor(tint);
    window.draw(bodySprite);
}

// UI color for each power-up type (waiting for final merge to change if needed)
sf::Color getPowerUpColor(PowerUpType type) {
    switch (type) {
        case PowerUpType::SpeedBoost: return {255, 220, 50};
        case PowerUpType::RapidFire: return {255, 160, 20};
        case PowerUpType::SpreadShot: return {60, 210, 255};
        case PowerUpType::Shield: return {80, 180, 255};
        case PowerUpType::HomingBullets: return {200, 80, 255};
        case PowerUpType::Invincibility: return {255, 255, 100};
        case PowerUpType::TripleDamage: return {255, 50, 50};
    }
    return sf::Color::White;
}

// Short label for HUD icons.
const char* getPowerUpLabel(PowerUpType type) {
    switch (type) {
        case PowerUpType::SpeedBoost: return "SPD";
        case PowerUpType::RapidFire: return "RFR";
        case PowerUpType::SpreadShot: return "SPR";
        case PowerUpType::Shield: return "SHL";
        case PowerUpType::HomingBullets: return "HOM";
        case PowerUpType::Invincibility: return "INV";
        case PowerUpType::TripleDamage: return "3X";
    }
    return "???";
}

// Maps a d6 roll to the same tier logic used by power-up spawning.
int powerUpTierFromRoll(int rollValue) {
    if (rollValue >= 6)
        return 3;
    if (rollValue >= 4)
        return 2;
    return 1;
}

// Draws the HUD panel for one player.
void drawPlayerHUD(sf::RenderWindow& window, const sf::Font& font, const PlayerState& player, int playerIndex, float hitFlashAlpha)
{
    const sf::Color playerColor = (playerIndex == 0) ? ColorConfig::PLAYER0_COLOR : ColorConfig::PLAYER1_COLOR;
    const float windowWidth = static_cast<float>(WindowConfig::WINDOW_WIDTH);
    const float panelX = (playerIndex == 0) ? HUDConfig::PANEL_SIDE_MARGIN : windowWidth - HUDConfig::PANEL_WIDTH - HUDConfig::PANEL_SIDE_MARGIN;

    // Panel background
    sf::RectangleShape panel({HUDConfig::PANEL_WIDTH, HUDConfig::PANEL_HEIGHT});
    panel.setPosition({panelX, HUDConfig::PANEL_Y});
    panel.setFillColor(ColorConfig::HUD_PANEL_BG);
    panel.setOutlineColor(sf::Color(playerColor.r, playerColor.g, playerColor.b, static_cast<uint8_t>(HUDConfig::PANEL_OUTLINE_ALPHA)));
    panel.setOutlineThickness(2.f);
    window.draw(panel);

    // Hit flash overlay
    if (hitFlashAlpha > 0.f) {
        sf::RectangleShape hitFlashRect({HUDConfig::PANEL_WIDTH - 4.f, HUDConfig::PANEL_HEIGHT - 4.f});
        hitFlashRect.setPosition({panelX + 2.f, HUDConfig::PANEL_Y + 2.f});
        hitFlashRect.setFillColor(sf::Color(255, 255, 255, static_cast<uint8_t>(hitFlashAlpha * VisualConfig::HIT_FLASH_MAX_ALPHA)));
        window.draw(hitFlashRect);
    }

    // Labels
    sf::Text playerLabel(font, (playerIndex == 0) ? "P1" : "P2", HUDConfig::FONT_PLAYER_LABEL);
    playerLabel.setFillColor(playerColor);
    playerLabel.setPosition({panelX + 10.f, HUDConfig::PANEL_Y + HUDConfig::PLAYER_LABEL_Y});
    window.draw(playerLabel);

    sf::Text hpLabel(font, "HP", HUDConfig::FONT_HP_LABEL);
    hpLabel.setFillColor(ColorConfig::HUD_LABEL_GRAY);
    hpLabel.setPosition({panelX + 10.f, HUDConfig::PANEL_Y + HUDConfig::HP_ROW_Y});
    window.draw(hpLabel);

    // HP bar
    const float barX = panelX + HUDConfig::HP_BAR_OFFSET_X;
    const float barY = HUDConfig::PANEL_Y + HUDConfig::HP_ROW_Y;
    const float fillWidth = HUDConfig::HP_BAR_WIDTH * (player.hp / static_cast<float>(GameplayConfig::PLAYER_MAX_HP));

    sf::RectangleShape barBackground({HUDConfig::HP_BAR_WIDTH, HUDConfig::HP_BAR_HEIGHT});
    barBackground.setPosition({barX, barY});
    barBackground.setFillColor(ColorConfig::HUD_HP_BAR_BG);
    barBackground.setOutlineColor(ColorConfig::HUD_HP_BAR_BORDER);
    barBackground.setOutlineThickness(1.f);
    window.draw(barBackground);

    if (fillWidth > 0.f) {
        sf::RectangleShape barFill({fillWidth, HUDConfig::HP_BAR_HEIGHT});
        barFill.setPosition({barX, barY});
        barFill.setFillColor(playerColor);
        window.draw(barFill);
    }

    // Score
    sf::Text scoreLabel(font, "SCORE", HUDConfig::FONT_SCORE_LABEL);
    scoreLabel.setFillColor(ColorConfig::HUD_SCORE_LABEL);
    scoreLabel.setPosition({panelX + 10.f, HUDConfig::PANEL_Y + HUDConfig::SCORE_LABEL_Y});
    window.draw(scoreLabel);

    sf::Text killsText(font,
        std::to_string(player.kills) + " / " + std::to_string(GameplayConfig::KILLS_TO_WIN),
        HUDConfig::FONT_SCORE_VALUE);
    killsText.setFillColor(ColorConfig::HUD_SCORE_VALUE);
    killsText.setPosition({panelX + 10.f, HUDConfig::PANEL_Y + HUDConfig::SCORE_VALUE_Y});
    window.draw(killsText);

    // Active effects change after last merge
    float effectX = panelX + 10.f;
    const float effectY = HUDConfig::PANEL_Y + HUDConfig::EFFECT_ROW_Y;
    for (const auto& activeEffect : player.effects) {
        sf::CircleShape effectDot(HUDConfig::EFFECT_DOT_RADIUS);
        effectDot.setOrigin({HUDConfig::EFFECT_DOT_RADIUS, HUDConfig::EFFECT_DOT_RADIUS});
        effectDot.setFillColor(getPowerUpColor(activeEffect.type));
        effectDot.setPosition({effectX + HUDConfig::EFFECT_DOT_RADIUS, effectY + HUDConfig::EFFECT_DOT_RADIUS});
        window.draw(effectDot);
        sf::Text effectLabel(font, getPowerUpLabel(activeEffect.type), HUDConfig::FONT_EFFECT_LABEL);
        effectLabel.setFillColor(ColorConfig::HUD_EFFECT_LABEL);
        effectLabel.setPosition({effectX + 14.f, effectY});
        window.draw(effectLabel);
        effectX += HUDConfig::EFFECT_SPACING;
        if (effectX + HUDConfig::EFFECT_SPACING > panelX + HUDConfig::PANEL_WIDTH - 8.f)
            break;
    }
}

} // namespace

struct UIModule::UIRenderState {
    std::array<CharacterAnimation, 2> characters;
    std::array<sf::Vector2f, 2> facingDirection = {{sf::Vector2f{1.f, 0.f}, sf::Vector2f{-1.f, 0.f}}};
    std::array<bool, 2> facingLeft = {false, true};
    std::array<float, 2> muzzleFlashTimer = {};
    std::array<float, 2> muzzleFireCooldown = {};
    std::array<float, 2> muzzleFireAngle = {};
    sf::Texture muzzleTexture;

    struct HitEffect {
        sf::Vector2f position;
        float timeRemaining;
    };
    std::vector<HitEffect> hitEffects;

    std::array<float, 2> hudFlashTimers = {};
    std::array<sf::Texture, 2> weaponTextures;
    std::array<sf::Texture, 6> diceFaceTextures;

    struct DicePopup {
        sf::Vector2f position;
        int finalValue;
        int displayedValue;
        int tier;
        float timeRemaining;
        float rollAnimationRemaining;
        float nextFaceChangeIn;
    };
    std::deque<int> pendingPowerUpRolls;
    std::vector<DicePopup> dicePopups;
    std::mt19937 rng{std::random_device{}()};

    [[nodiscard]] bool loadAssets() {
        if (!characters[0].load(AssetConfig::CHAR1_DIRECTORY))
            return false;
        if (!characters[1].load(AssetConfig::CHAR4_DIRECTORY))
            return false;
        if (!weaponTextures[0].loadFromFile(AssetConfig::WEAPON1_SPRITE))
            return false;
        if (!weaponTextures[1].loadFromFile(AssetConfig::WEAPON2_SPRITE))
            return false;
        for (std::size_t i = 0; i < diceFaceTextures.size(); ++i) {
            if (!diceFaceTextures[i].loadFromFile(std::string(PowerUpConfig::DICE_FACE_TEXTURE_PATHS[i])))
                return false;
        }
        return muzzleTexture.loadFromFile(AssetConfig::MUZZLE_TEXTURE);
    }
};

UIModule::UIModule(ModuleContext& context)
    : _context(context), _uiState(std::make_unique<UIRenderState>())
{
    if (!_uiState->loadAssets())
        throw std::runtime_error("UIModule: asset load failed");

    _context.bus.subscribe<PlayerHitEvent>([this](const PlayerHitEvent& event) {
        _uiState->hudFlashTimers[event.victimId] = VisualConfig::HIT_FLASH_DURATION;
        _uiState->hitEffects.push_back({_context.state.players[event.victimId].position, VisualConfig::HIT_EFFECT_DURATION});
        auto& character = _uiState->characters[event.victimId];
        if (character.state != AnimationState::DEAD) {
            character.state = AnimationState::HIT;
            character.frameIndex = 0;
            character.frameTimer = 0.f;
            character.animationDone = false;
        }
    });

    _context.bus.subscribe<PlayerDiedEvent>([this](const PlayerDiedEvent& event) {
        auto& character = _uiState->characters[event.victimId];
        character.deathPosition = event.position;
        character.state = AnimationState::DEAD;
        character.frameIndex = 0;
        character.frameTimer = 0.f;
        character.animationDone = false;
    });

    _context.bus.subscribe<PlayerRespawnEvent>([this](const PlayerRespawnEvent& event) {
        auto& character = _uiState->characters[event.playerId];
        character.state = AnimationState::IDLE;
        character.frameIndex = 0;
        character.frameTimer = 0.f;
        character.animationDone = false;
    });

    _context.bus.subscribe<DiceRolledEvent>([this](const DiceRolledEvent& event) {
        if (event.playerId < 0)
            _uiState->pendingPowerUpRolls.push_back(event.value);
    });

    _context.bus.subscribe<PowerUpSpawnedEvent>([this](const PowerUpSpawnedEvent& event) {
        int rollValue = 1;
        if (!_uiState->pendingPowerUpRolls.empty()) {
            rollValue = _uiState->pendingPowerUpRolls.front();
            _uiState->pendingPowerUpRolls.pop_front();
        }

        _uiState->dicePopups.push_back(UIRenderState::DicePopup{
            event.position + sf::Vector2f{0.f, VisualConfig::DICE_POPUP_OFFSET_Y},
            rollValue,
            1,
            powerUpTierFromRoll(rollValue),
            VisualConfig::DICE_POPUP_DURATION,
            VisualConfig::DICE_ROLL_ANIMATION_DURATION,
            0.f
        });
    });
}

UIModule::~UIModule() = default;
void UIModule::handleEvent(const ModuleContext&, const sf::Event&) {}

// Updates timers, animation state, and muzzle flash state.
void UIModule::update(const ModuleContext& context, float dt) {
    UIRenderState& renderState = *_uiState;

    // Timers
    for (auto& hudFlashTimer : renderState.hudFlashTimers)
        hudFlashTimer = std::max(0.f, hudFlashTimer - dt);
    for (auto& muzzleFlashTimer : renderState.muzzleFlashTimer)
        muzzleFlashTimer = std::max(0.f, muzzleFlashTimer - dt);
    for (auto& muzzleCooldown : renderState.muzzleFireCooldown)
        muzzleCooldown = std::max(0.f, muzzleCooldown - dt);
    for (auto& hitEffect : renderState.hitEffects)
        hitEffect.timeRemaining -= dt;
    std::erase_if(renderState.hitEffects, [](const auto& hitEffect) {return hitEffect.timeRemaining <= 0.f;});

    for (auto& dicePopup : renderState.dicePopups) {
        dicePopup.timeRemaining -= dt;
        dicePopup.position.y -= VisualConfig::DICE_POPUP_RISE_SPEED * dt;

        if (dicePopup.rollAnimationRemaining > 0.f) {
            dicePopup.rollAnimationRemaining = std::max(0.f, dicePopup.rollAnimationRemaining - dt);
            dicePopup.nextFaceChangeIn -= dt;

            if (dicePopup.nextFaceChangeIn <= 0.f) {
                std::uniform_int_distribution<int> d6(1, 6);
                int face = d6(renderState.rng);
                if (face == dicePopup.displayedValue)
                    face = (face % 6) + 1;
                dicePopup.displayedValue = face;

                const float progress = 1.f - (dicePopup.rollAnimationRemaining / VisualConfig::DICE_ROLL_ANIMATION_DURATION);
                const float easing = progress * progress;
                const float minInterval = VisualConfig::DICE_ROLL_FRAME_MIN_INTERVAL;
                const float maxInterval = VisualConfig::DICE_ROLL_FRAME_MAX_INTERVAL;
                dicePopup.nextFaceChangeIn = minInterval + (maxInterval - minInterval) * easing;
            }

            if (dicePopup.rollAnimationRemaining <= 0.f)
                dicePopup.displayedValue = dicePopup.finalValue;
        }
    }
    std::erase_if(renderState.dicePopups, [](const auto& dicePopup) { return dicePopup.timeRemaining <= 0.f; });

    // Facing + animation state
    for (int playerIndex = 0; playerIndex < GameplayConfig::PLAYER_COUNT; ++playerIndex) {
        const auto& player = context.state.players[playerIndex];
        auto& character = renderState.characters[playerIndex];
        const auto& playerInput = context.input.get(playerIndex);

        if (playerInput.moveDir.x != 0.f || playerInput.moveDir.y != 0.f)
            renderState.facingDirection[playerIndex] = playerInput.moveDir;
        if (playerInput.moveDir.x > 0.f)
            renderState.facingLeft[playerIndex] = false;
        if (playerInput.moveDir.x < 0.f)
            renderState.facingLeft[playerIndex] = true;

        if (player.alive) {
            const bool isMoving = playerInput.moveDir.x != 0.f || playerInput.moveDir.y != 0.f;
            if (character.state == AnimationState::HIT && character.animationDone)
                applyAnimationState(character, isMoving ? AnimationState::WALK : AnimationState::IDLE);
            else if (character.state != AnimationState::HIT)
                applyAnimationState(character, isMoving ? AnimationState::WALK : AnimationState::IDLE);
        }

        // Muzzle flash
        if (playerInput.shoot && renderState.muzzleFireCooldown[playerIndex] <= 0.f && player.alive) {
            renderState.muzzleFlashTimer[playerIndex] = VisualConfig::MUZZLE_FLASH_DURATION;
            renderState.muzzleFireCooldown[playerIndex] = GameplayConfig::FIRE_COOLDOWN;
            renderState.muzzleFireAngle[playerIndex] =
                std::atan2(renderState.facingDirection[playerIndex].y, renderState.facingDirection[playerIndex].x) * (180.f / std::numbers::pi_v<float>);
        }

        // Advance animation
        advanceAnimation(character, dt);
    }
}

// Renders characters, weapons, VFX, and HUD overlays.
void UIModule::render(const ModuleContext& context, sf::RenderWindow& window) {
    const auto& font = AssetManager::instance().font(AssetConfig::FONT);
    UIRenderState& renderState = *_uiState;

    // Character bodies
    for (int playerIndex = 0; playerIndex < GameplayConfig::PLAYER_COUNT; ++playerIndex) {
        const auto& player = context.state.players[playerIndex];
        const auto& character = renderState.characters[playerIndex];

        const sf::Vector2f renderPosition = (character.state == AnimationState::DEAD) ? character.deathPosition : player.position;

        sf::Color tint = sf::Color::White;
        if (player.invincible) {
            const int blinkPhase = static_cast<int>(player.invincibleTimer * 10.f) % 2;
            tint.a = (blinkPhase == 0) ? VisualConfig::INVINCIBLE_BLINK_LOW_ALPHA : 255;
        }

        if (!player.alive && character.state != AnimationState::DEAD)
            continue;

        renderCharacterBody(window, character, renderPosition, renderState.facingLeft[playerIndex], tint);
    }

    // Weapons
    for (int playerIndex = 0; playerIndex < GameplayConfig::PLAYER_COUNT; ++playerIndex) {
        const auto& player = context.state.players[playerIndex];
        if (!player.alive)
            continue;
        if (renderState.characters[playerIndex].state == AnimationState::DEAD)
            continue;

        sf::Vector2f weaponDirection = renderState.facingDirection[playerIndex];
        const bool weaponLeft = weaponDirection.x < 0.f;
        float weaponAngle = std::atan2(weaponDirection.y, weaponDirection.x) * (180.f / std::numbers::pi_v<float>);
        if (weaponLeft)
            weaponAngle += 180.f;

        const auto& weaponTexture = renderState.weaponTextures[playerIndex];
        const auto weaponTextureSize = weaponTexture.getSize();
        sf::Sprite weaponSprite(weaponTexture);
        weaponSprite.setOrigin({weaponTextureSize.x * 0.5f, weaponTextureSize.y * 0.5f});
        sf::Vector2f weaponScale{VisualConfig::WEAPON_SCALE, VisualConfig::WEAPON_SCALE};
        if (weaponLeft)
            weaponScale.x = -weaponScale.x;
        weaponSprite.setScale(weaponScale);
        weaponSprite.setRotation(sf::degrees(weaponAngle));
        weaponSprite.setPosition({player.position.x + VisualConfig::WEAPON_OFFSET_X, player.position.y + VisualConfig::WEAPON_OFFSET_Y});
        window.draw(weaponSprite);
    }

    // Muzzle flashes
    {
        const auto muzzleTextureSize = renderState.muzzleTexture.getSize();
        for (int playerIndex = 0; playerIndex < GameplayConfig::PLAYER_COUNT; ++playerIndex) {
            if (renderState.muzzleFlashTimer[playerIndex] <= 0.f)
                continue;
            if (!context.state.players[playerIndex].alive)
                continue;

            const float angleRadians = renderState.muzzleFireAngle[playerIndex] * (std::numbers::pi_v<float> / 180.f);
            sf::Vector2f shotDirection{std::cos(angleRadians), std::sin(angleRadians)};
            const bool shotLeft = shotDirection.x < 0.f;

            const sf::Vector2f muzzlePosition = context.state.players[playerIndex].position + shotDirection * VisualConfig::MUZZLE_OFFSET_DISTANCE;
            sf::Sprite muzzleSprite(renderState.muzzleTexture);
            muzzleSprite.setOrigin({muzzleTextureSize.x * 0.5f, muzzleTextureSize.y * 0.5f});
            sf::Vector2f muzzleScale{VisualConfig::MUZZLE_SCALE, VisualConfig::MUZZLE_SCALE};
            if (shotLeft)
                muzzleScale.x = -muzzleScale.x;
            muzzleSprite.setScale(muzzleScale);

            float muzzleAngle = renderState.muzzleFireAngle[playerIndex];
            if (shotLeft)
                muzzleAngle += 180.f;
            muzzleSprite.setRotation(sf::degrees(muzzleAngle));
            muzzleSprite.setPosition({muzzlePosition.x + VisualConfig::WEAPON_OFFSET_X, muzzlePosition.y + VisualConfig::WEAPON_OFFSET_Y});
            const float fadeAlpha = renderState.muzzleFlashTimer[playerIndex] / VisualConfig::MUZZLE_FLASH_DURATION;
            muzzleSprite.setColor(sf::Color( ColorConfig::MUZZLE_FLASH_COLOR.r, ColorConfig::MUZZLE_FLASH_COLOR.g, ColorConfig::MUZZLE_FLASH_COLOR.b,
                static_cast<uint8_t>(fadeAlpha * VisualConfig::MUZZLE_FLASH_MAX_ALPHA)));
            window.draw(muzzleSprite);
        }
    }

    // Hit effects
    for (const auto& hitEffect : renderState.hitEffects) {
        const float progress = hitEffect.timeRemaining / VisualConfig::HIT_EFFECT_DURATION;
        const float sparkRadius = VisualConfig::HIT_EFFECT_MAX_RADIUS * progress;
        sf::CircleShape sparkShape(sparkRadius);
        sparkShape.setOrigin({sparkRadius, sparkRadius});
        sparkShape.setPosition(hitEffect.position);
        sparkShape.setFillColor(sf::Color( ColorConfig::HIT_SPARK_COLOR.r, ColorConfig::HIT_SPARK_COLOR.g, ColorConfig::HIT_SPARK_COLOR.b,
            static_cast<uint8_t>(progress * VisualConfig::HIT_SPARK_MAX_ALPHA)));
        window.draw(sparkShape);
    }

    // Dice roll popups for power-up randomness feedback.
    for (const auto& dicePopup : renderState.dicePopups) {
        const float lifeRatio = std::clamp(dicePopup.timeRemaining / VisualConfig::DICE_POPUP_DURATION, 0.f, 1.f);
        const float lifeProgress = 1.f - lifeRatio;
        const float fadeStart = std::clamp(VisualConfig::DICE_POPUP_FADE_START_RATIO, 0.f, 0.95f);
        float fadeRatio = 1.f;
        if (lifeProgress > fadeStart)
            fadeRatio = 1.f - ((lifeProgress - fadeStart) / (1.f - fadeStart));
        fadeRatio = std::clamp(fadeRatio, 0.f, 1.f);
        const uint8_t alpha = static_cast<uint8_t>(VisualConfig::DICE_POPUP_MAX_ALPHA * fadeRatio);
        const std::size_t tierIndex = static_cast<std::size_t>(std::clamp(dicePopup.tier, 1, 3) - 1);
        sf::Color popupColor = PowerUpConfig::TIER_COLORS[tierIndex];
        popupColor.a = alpha;

        sf::Vector2f drawPos = dicePopup.position;
        if (dicePopup.rollAnimationRemaining > 0.f) {
            const float phase = dicePopup.rollAnimationRemaining * 42.f;
            drawPos.x += std::sin(phase) * VisualConfig::DICE_ROLL_SHAKE_AMPLITUDE;
        }

        sf::CircleShape aura(VisualConfig::DICE_POPUP_SPRITE_SIZE * 0.46f);
        aura.setOrigin({aura.getRadius(), aura.getRadius()});
        aura.setPosition(drawPos);
        aura.setFillColor(sf::Color(popupColor.r, popupColor.g, popupColor.b,
            static_cast<uint8_t>(VisualConfig::DICE_POPUP_AURA_MAX_ALPHA * fadeRatio)));
        window.draw(aura);

        const int faceIndex = std::clamp(dicePopup.displayedValue, 1, 6) - 1;
        const sf::Texture& faceTexture = renderState.diceFaceTextures[static_cast<std::size_t>(faceIndex)];
        sf::Sprite diceSprite(faceTexture);
        diceSprite.setOrigin({faceTexture.getSize().x * 0.5f, faceTexture.getSize().y * 0.5f});
        const float scale = VisualConfig::DICE_POPUP_SPRITE_SIZE / static_cast<float>(faceTexture.getSize().x);
        diceSprite.setScale({scale, scale});
        diceSprite.setPosition(drawPos);
        diceSprite.setColor(sf::Color(255, 255, 255, alpha));
        window.draw(diceSprite);

        sf::Text diceValueText(font, std::to_string(dicePopup.displayedValue), VisualConfig::DICE_POPUP_FONT_SIZE - 2);
        diceValueText.setFillColor(sf::Color(25, 25, 25, alpha));
        diceValueText.setOutlineColor(sf::Color(255, 255, 255, alpha));
        diceValueText.setOutlineThickness(1.f);
        const sf::FloatRect textBounds = diceValueText.getLocalBounds();
        diceValueText.setOrigin({textBounds.position.x + textBounds.size.x * 0.5f, textBounds.position.y + textBounds.size.y * 0.5f});
        diceValueText.setPosition({drawPos.x, drawPos.y + VisualConfig::DICE_POPUP_SPRITE_SIZE * 0.78f});
        window.draw(diceValueText);
    }

    // HUD
    for (int playerIndex = 0; playerIndex < GameplayConfig::PLAYER_COUNT; ++playerIndex)
        drawPlayerHUD(window, font, context.state.players[playerIndex], playerIndex, renderState.hudFlashTimers[playerIndex]);
}
