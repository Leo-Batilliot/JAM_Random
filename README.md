# Jam Random — DICE DUEL

Un jeu de duel d'arène local en 1v1 écrit en **C++20** avec **SFML 3.0**. Deux
joueurs s'affrontent au clavier, ramassent des power-ups dont la puissance
dépend d'un lancé de dé, et le premier à atteindre 3 éliminations remporte le
match.

---

## Sommaire

- [Aperçu](#aperçu)
- [Fonctionnalités](#fonctionnalités)
- [Contrôles](#contrôles)
- [Power-ups](#power-ups)
- [Prérequis](#prérequis)
- [Compilation et exécution](#compilation-et-exécution)
- [Architecture](#architecture)
- [Arborescence du projet](#arborescence-du-projet)
- [Configuration](#configuration)
- [Assets](#assets)

---

## Aperçu

**Jam Random** (titre interne *DICE DUEL*) est un jeu d'action multijoueur
local. Chaque partie se joue dans une arène fermée avec décor procédural
(rochers placés aléatoirement). Des power-ups apparaissent à intervalles
aléatoires et leur tier (durée et impact) est tiré au dé lors de la
récupération, ajoutant une couche de hasard façon *jam*.

- Résolution : **1920 × 1080** @ **60 FPS**
- Mode : **2 joueurs locaux**, même clavier
- Objectif : atteindre **3 éliminations** pour gagner

## Fonctionnalités

- Duel local 1v1 sur clavier partagé
- Système de **power-ups à 3 tiers** (durées × 1.0 / 1.5 / 2.0)
- 7 effets différents (vitesse, cadence de tir, projectiles téléguidés, etc.)
- Système de scènes (Menu → Partie → Écran de victoire)
- Architecture modulaire (combat / power-ups / UI séparés)
- Système d'événements (EventBus) découplant les modules
- Décor d'arène généré aléatoirement à chaque partie
- Animations de personnages (idle, walk, hit, death) et VFX (muzzle flash)

## Contrôles

| Action      | Joueur 1     | Joueur 2          |
|-------------|--------------|-------------------|
| Haut        | `Z`          | `↑`               |
| Bas         | `S`          | `↓`               |
| Gauche      | `Q`          | `←`               |
| Droite      | `D`          | `→`               |
| Tirer       | `Espace`     | `Entrée` / `RShift` |

**Menu / Écran de victoire :** `Entrée` pour continuer, `Échap` pour quitter.

## Power-ups

Chaque power-up apparaît avec un type, et au moment de la collecte un dé
détermine son **tier** (qualité). Les durées sont multipliées par
`1.0`, `1.5` ou `2.0` selon le tier.

| Type             | Effet                                                          |
|------------------|----------------------------------------------------------------|
| Speed Boost      | Vitesse de déplacement × 1.35                                  |
| Rapid Fire       | Cadence de tir augmentée (cooldown × 0.55)                     |
| Spread Shot      | Tir en éventail à 3 projectiles (±14°)                         |
| Shield           | Bouclier qui absorbe un coup                                   |
| Homing Bullets   | Projectiles à tête chercheuse                                  |
| Invincibility    | Invincibilité totale temporaire                                |
| Triple Damage    | Dégâts par balle × 3                                           |

Paramètres modifiables dans `src/config/PowerUpConfig.hpp`.

## Prérequis

- **CMake** ≥ 3.22
- Compilateur **C++20** (g++, clang++)
- **Make**
- Connexion internet pour la première compilation (SFML 3.0 est récupéré
  automatiquement via `FetchContent`)
- Bibliothèques système requises par SFML (X11, OpenGL, audio…)

Sur Debian / Ubuntu :

```bash
sudo apt install build-essential cmake \
    libxrandr-dev libxcursor-dev libxi-dev libudev-dev \
    libfreetype-dev libflac-dev libvorbis-dev \
    libgl1-mesa-dev libegl1-mesa-dev
```

## Compilation et exécution

Un `Makefile` enrobe les commandes CMake :

```bash
make            # compile en Release dans build/, binaire à la racine
make run        # compile puis lance le jeu
make debug      # build en mode Debug
make clean      # nettoie les artefacts CMake
make fclean     # supprime build/ et le binaire
make re         # fclean + build
```

Le binaire produit s'appelle `JamRandom` à la racine du projet. Il **doit**
être lancé depuis la racine pour que les chemins relatifs vers `assets/`
fonctionnent.

Build manuel sans Make :

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
./JamRandom
```

## Architecture

Le projet repose sur trois piliers :

### 1. Scènes (`src/world/`, `src/core/SceneManager.hpp`)

Une pile de scènes gère le flux du jeu :

```
MenuScene ──Entrée──▶ GameScene ──GameWonEvent──▶ WinScene ──Entrée──▶ MenuScene
```

`SceneManager` gère push / pop / replace en différé (`applyPending`) pour
éviter les invalidations en plein update.

### 2. Modules (`src/modules/`)

`GameScene` ne contient pas de logique de gameplay : elle instancie une
liste de modules via `ModuleFactory`. Chaque module implémente
`IGameModule` (`handleEvent`, `update`, `render`) :

- **CombatModule** — déplacements, tirs, balles, collisions, kills, respawn
- **PowerUpModule** — spawn aléatoire, dé, application des effets actifs
- **UIModule** — HUD, scores, indicateurs de power-ups

Ajouter un module = créer une classe `IGameModule` et l'enregistrer dans
`ModuleFactory.cpp`. Aucun module ne dépend d'un autre directement.

### 3. EventBus (`src/core/EventBus.hpp`)

Communication inter-modules par publication/abonnement typée :

```cpp
bus.subscribe<PlayerDiedEvent>([](const PlayerDiedEvent& e) { /* ... */ });
bus.emit(PlayerDiedEvent{victimId, killerId, position});
```

Événements définis dans `src/core/GameEvents.hpp` :
`PlayerHitEvent`, `PlayerDiedEvent`, `PlayerRespawnEvent`,
`DiceRolledEvent`, `PowerUpSpawnedEvent`, `PowerUpPickedEvent`,
`PowerUpExpiredEvent`, `GameWonEvent`, `VFXRequestEvent`.

### Contexte partagé

Chaque module reçoit un `ModuleContext` qui agrège :
`Game&`, `EventBus&`, `GameState&`, `InputManager&`. L'état global
(joueurs, scores, effets actifs) vit dans `GameState`.

## Arborescence du projet

```
JAM_Random/
├── CMakeLists.txt
├── Makefile
├── assets/
│   ├── characters/{char1,char4}/   # sprites idle / walk / hit / death
│   ├── environment/                # rochers
│   ├── fonts/PressStart2P.ttf
│   ├── textures/                   # sols, murs, icônes power-ups
│   ├── vfx/muzzle.png
│   └── weapons/{weapon1,weapon2}/
└── src/
    ├── main.cpp
    ├── config/                     # constantes (tunables)
    │   ├── ArenaConfig.hpp
    │   ├── AssetConfig.hpp
    │   ├── ColorConfig.hpp
    │   ├── DecorConfig.hpp
    │   ├── GameplayConfig.hpp
    │   ├── HUDConfig.hpp
    │   ├── InputConfig.hpp
    │   ├── MenuConfig.hpp
    │   ├── PowerUpConfig.hpp
    │   ├── VisualConfig.hpp
    │   ├── WindowConfig.hpp
    │   └── WorldConfig.hpp
    ├── core/                       # plomberie moteur
    │   ├── AssetManager.hpp
    │   ├── EventBus.hpp
    │   ├── Game.{hpp,cpp}
    │   ├── GameEvents.hpp
    │   ├── GameState.hpp
    │   ├── InputManager.hpp
    │   ├── Scene.hpp
    │   └── SceneManager.hpp
    ├── modules/
    │   ├── IGameModule.hpp
    │   ├── ModuleContext.hpp
    │   ├── ModuleFactory.{hpp,cpp}
    │   ├── combat/CombatModule.{hpp,cpp}
    │   ├── powerup/PowerUpModule.{hpp,cpp}
    │   └── ui/UIModule.{hpp,cpp}
    └── world/                      # scènes
        ├── MenuScene.{hpp,cpp}
        ├── GameScene.{hpp,cpp}
        └── WinScene.{hpp,cpp}
```

## Configuration

Toutes les valeurs ajustables sont rassemblées dans `src/config/`. Les plus
utiles pour bidouiller la sensation du jeu :

| Fichier               | À régler                                                 |
|-----------------------|----------------------------------------------------------|
| `GameplayConfig.hpp`  | vitesse joueur, HP, vitesse balle, cadence, kills victoire |
| `PowerUpConfig.hpp`   | intervalles spawn, durées, multiplicateurs par tier        |
| `WindowConfig.hpp`    | résolution, FPS cible                                      |
| `InputConfig.hpp`     | bindings clavier des deux joueurs                          |
| `ArenaConfig.hpp`     | dimensions et positions de l'arène                         |
| `DecorConfig.hpp`     | nombre et taille des rochers, marges                       |
| `ColorConfig.hpp`     | palette (fond, joueurs, murs)                              |
| `MenuConfig.hpp`      | mise en page menu / écran victoire                         |

## Assets

- **Police** : *Press Start 2P* (style pixel arcade)
- **Personnages** : 2 sprites animés (idle 6f, walk 8f, hit 3f, death 10f)
- **Décor** : 3 variantes de rochers, sol terre battue, murs en bois
- **Power-ups** : icône dédiée par type pour reconnaissance instantanée

Tous les chemins d'assets sont centralisés dans `src/config/AssetConfig.hpp`.
