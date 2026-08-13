#include <filesystem>
#include <fstream>
#include <iostream>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>

#include "Game/AI/AiGenomeCodec.h"
#include "Game/AI/AiPlayerController.h"
#include "Game/AI/HeuristicAiController.h"
#include "Game/Behaviours/Moveable.h"
#include "Game/GameSettings.h"
#include "Game/Minigame/MinigameContactRules.h"
#include "Game/Minigame/MinigameTypes.h"
#include "Game/Objects/GameObject.h"
#include "Game/Objects/Player/Player.h"
#include "Game/UserInput/IPlayerController.h"
#include "Game/World/GameWorld.h"
#include "Game/World/LevelDataLoader.h"
#include "Game/World/SpawnSpec.h"
#include "Game/World/WorldObjectStore.h"

#include <nlohmann/json.hpp>

#ifndef PROJECT_SOURCE_DIR
#define PROJECT_SOURCE_DIR "."
#endif

namespace {
int failures = 0;

void expect(bool condition, const std::string& message) {
    if (!condition) {
        std::cerr << "FAILED: " << message << '\n';
        ++failures;
    }
}

template <typename Action>
void expectThrows(Action&& action, const std::string& message) {
    try {
        action();
        expect(false, message);
    } catch (const std::exception&) {
    }
}

AiGenome zeroGenome() {
    AiGenome genome;
    genome.weights.assign(AiPolicy::weightCount, 0.0f);
    return genome;
}

AiGenome directionalGenome() {
    AiGenome genome = zeroGenome();
    // Hidden neuron 0 measures opponentX - selfX.
    genome.weights[1] = -2.0f;
    genome.weights[3] = 2.0f;
    // Hidden neuron 1 becomes positive only while grounded.
    genome.weights[11] = -1.0f;
    genome.weights[20] = 2.0f;
    // Output neurons: left, right, jump.
    genome.weights[133] = -3.0f;
    genome.weights[146] = 3.0f;
    genome.weights[160] = 3.0f;
    return genome;
}

void testPolicyInference() {
    const AiAction neutral = AiPolicy(zeroGenome()).decide({});
    expect(neutral.horizontal == 0, "zero genome produces neutral movement");
    expect(!neutral.jump, "zero genome does not jump");

    AiObservation observation;
    observation.selfX = -0.5f;
    observation.opponentX = 0.5f;
    observation.selfGrounded = 1.0f;
    const AiAction action = AiPolicy(directionalGenome()).decide(observation);
    expect(action.horizontal == 1, "policy moves toward an opponent on the right");
    expect(action.jump, "grounded policy requests jump");

    observation.selfX = 0.5f;
    observation.opponentX = -0.5f;
    observation.selfGrounded = 0.0f;
    const AiAction reversed = AiPolicy(directionalGenome()).decide(observation);
    expect(reversed.horizontal == -1, "policy moves toward an opponent on the left");
    expect(!reversed.jump, "airborne policy releases jump");
}

void testCodec() {
    const std::filesystem::path modelPath =
        std::filesystem::temp_directory_path() / "mario-ai-codec-test.json";
    AiGenome genome = directionalGenome();
    genome.trainingMap = "minigame";
    genome.trainingSeed = 1337;
    genome.generation = 4;
    genome.fitness = 123.5f;
    AiGenomeCodec::save(modelPath, genome);
    const AiGenome loaded = AiGenomeCodec::load(modelPath);
    expect(loaded.weights == genome.weights, "AI model weights round trip");
    expect(loaded.trainingMap == "minigame", "AI model metadata round trips");
    expect(loaded.generation == 4, "AI model generation round trips");

    AiGenome wrongSize;
    wrongSize.weights.push_back(0.0f);
    expectThrows(
        [&] { AiGenomeCodec::save(modelPath, wrongSize); },
        "codec rejects the wrong number of weights"
    );

    AiGenome nonFinite = zeroGenome();
    nonFinite.weights.front() = std::numeric_limits<float>::infinity();
    expectThrows(
        [&] { AiGenomeCodec::save(modelPath, nonFinite); },
        "codec rejects non-finite weights"
    );

    {
        std::ofstream malformed(modelPath, std::ios::trunc);
        malformed << "{\"version\":99,\"network\":{},\"weights\":[]}";
    }
    expectThrows(
        [&] { (void)AiGenomeCodec::load(modelPath); },
        "codec rejects an incompatible model version"
    );

    const std::filesystem::path runtimeModel =
        std::filesystem::path(PROJECT_SOURCE_DIR)
        / "assets/ai/minigame.json";
    const AiGenome bundled = AiGenomeCodec::load(runtimeModel);
    expect(
        bundled.weights.size() == AiPolicy::weightCount,
        "bundled runtime model passes codec validation"
    );
    std::error_code ignored;
    std::filesystem::remove(modelPath, ignored);
}

void testPlayerSlotValidation() {
    const nlohmann::json missingSlot = nlohmann::json::parse(R"json([
        {
          "id": 2,
          "kind": "Player",
          "typeKey": "Player",
          "texture": "mario_spritesheet",
          "size": [36, 80]
        }
    ])json");
    expectThrows(
        [&] { (void)parseSpawnSpecs(missingSlot); },
        "player spawn rejects a missing slot"
    );

    nlohmann::json invalidSlot = missingSlot;
    invalidSlot[0]["playerSlot"] = 3;
    expectThrows(
        [&] { (void)parseSpawnSpecs(invalidSlot); },
        "player spawn rejects an invalid slot"
    );

    const std::filesystem::path temporaryDirectory =
        std::filesystem::temp_directory_path() / "mario-ai-slot-tests";
    std::filesystem::create_directories(temporaryDirectory);
    const std::filesystem::path levelPath = temporaryDirectory / "level.json";
    const std::filesystem::path spawnsPath = temporaryDirectory / "spawns.json";
    {
        std::ofstream level(levelPath, std::ios::trunc);
        level << R"json({"rows":["2 3"]})json";
        std::ofstream spawns(spawnsPath, std::ios::trunc);
        spawns << R"json({"spawns":[
          {"id":2,"kind":"Player","typeKey":"Player","texture":"mario_spritesheet","size":[36,80],"playerSlot":1},
          {"id":3,"kind":"Player","typeKey":"Player","texture":"luigi_spritesheet","size":[36,80],"playerSlot":1}
        ]})json";
    }
    expectThrows(
        [&] { (void)LevelDataLoader::load(levelPath, 10, 10, spawnsPath); },
        "minigame level rejects duplicate player slots"
    );
    std::error_code ignored;
    std::filesystem::remove_all(temporaryDirectory, ignored);
}

void testStompContactRule() {
    expect(
        isPlayerStompContact(1, 0.75f),
        "top player contact is classified as a stomp"
    );
    expect(
        !isPlayerStompContact(1, 0.1f),
        "lateral player contact is not classified as a stomp"
    );
    expect(
        !isPlayerStompContact(0, 1.0f),
        "an empty manifold cannot award a stomp"
    );
}

class ProbeObject : public GameObject {
public:
    explicit ProbeObject(bool& controllerUpdated)
        : _controllerUpdated(controllerUpdated) {}

    void updateSimulation(const float& fixedDt) override {
        (void)fixedDt;
        sawControllerUpdate = _controllerUpdated;
    }

    bool sawControllerUpdate = false;

private:
    bool& _controllerUpdated;
};

class ProbeController : public IPlayerController {
public:
    explicit ProbeController(bool& updated) : _updated(updated) {}
    void fixedUpdate(float fixedDt) override {
        (void)fixedDt;
        _updated = true;
    }
    bool isPlayerPendingDestroy() const override { return false; }

private:
    bool& _updated;
};

void testControllerOrdering() {
    bool controllerUpdated = false;
    auto object = std::make_shared<ProbeObject>(controllerUpdated);
    WorldObjectStore store;
    store.addObject(object);
    store.addController(std::make_unique<ProbeController>(controllerUpdated));
    store.updateSimulation(1.0f / 60.0f);
    expect(object->sawControllerUpdate, "controllers update before game objects");
}

std::string trainingMapPath() {
    return std::string(PROJECT_SOURCE_DIR)
        + "/assets/datas/minigames/ai/minigame.json";
}

void testHeuristicControllerDecisions() {
    AiObservation observation;
    observation.selfGrounded = 1.0f;

    observation.opponentX = 0.5f;
    const AiAction towardRight = HeuristicAiController::decide(observation);
    expect(
        towardRight.horizontal == 1,
        "heuristic chases an opponent on the right"
    );

    observation.opponentX = -0.5f;
    const AiAction towardLeft = HeuristicAiController::decide(observation);
    expect(
        towardLeft.horizontal == -1,
        "heuristic chases an opponent on the left"
    );

    observation.selfX = 0.0f;
    observation.opponentX = 0.1f;
    const AiAction groundedAligned = HeuristicAiController::decide(observation);
    expect(
        groundedAligned.jump,
        "grounded aligned heuristic jumps toward the opponent"
    );

    observation.selfGrounded = 0.0f;
    const AiAction airborne = HeuristicAiController::decide(observation);
    expect(
        !airborne.jump,
        "airborne heuristic releases jump"
    );

    observation.selfGrounded = 1.0f;
    observation.opponentX = 0.3f;
    observation.opponentY = -0.4f;
    observation.selfY = 0.1f;
    const AiAction evasive = HeuristicAiController::decide(observation);
    expect(
        evasive.horizontal == -1,
        "heuristic retreats when the opponent is above and close"
    );

    observation.opponentX = 0.6f;
    observation.opponentY = 0.1f;
    observation.selfY = 0.1f;
    observation.selfX = 0.95f;
    const AiAction edgeEscape = HeuristicAiController::decide(observation);
    expect(
        edgeEscape.horizontal == -1,
        "heuristic steers away from the arena edge"
    );
}

void testHeuristicControllerEpisode() {
    GameSettings& settings = GameSettings::getInstance();
    settings.gameMode = GameMode::Minigame;
    settings.minigameMode = MinigameMode::VsAi;

    GameWorld world;
    world.loadLevel(trainingMapPath());
    const std::shared_ptr<Player> one = world.getPlayer(PlayerSlot::One);
    const std::shared_ptr<Player> two = world.getPlayer(PlayerSlot::Two);
    expect(one != nullptr, "heuristic episode exposes player slot one");
    expect(two != nullptr, "heuristic episode exposes player slot two");
    if (!one || !two) {
        return;
    }

    world.addController(std::make_unique<HeuristicAiController>(*one, *two, world));
    world.addController(std::make_unique<HeuristicAiController>(*two, *one, world));

    bool crashed = false;
    try {
        for (int step = 0; step < 600; ++step) {
            world.updateSimulation(1.0f / 60.0f);
        }
    } catch (const std::exception&) {
        crashed = true;
    }
    expect(
        !crashed,
        "heuristic versus heuristic episode runs without exceptions"
    );
}

void testSlotsAndResults() {
    GameSettings& settings = GameSettings::getInstance();
    settings.gameMode = GameMode::Minigame;
    settings.minigameMode = MinigameMode::VsAi;

    GameWorld world;
    world.loadLevel(trainingMapPath());
    const std::shared_ptr<Player> one = world.getPlayer(PlayerSlot::One);
    const std::shared_ptr<Player> two = world.getPlayer(PlayerSlot::Two);
    expect(one != nullptr, "level exposes player slot one");
    expect(two != nullptr, "level exposes player slot two");
    expect(one != two, "player slots resolve independently");

    one->setPosition({100.0f, 100.0f});
    two->setPosition({300.0f, 300.0f});
    const AiObservation observation = AiPlayerController::observe(
        *one, *two, world
    );
    expect(
        observation.selfY < observation.opponentY,
        "observation preserves SFML's smaller-Y-is-higher coordinate rule"
    );

    auto* movement = one->getBehaviour<Moveable>();
    expect(movement != nullptr, "AI-controlled player has movement behaviour");
    if (movement) {
        movement->beginGroundContact();
        AiPlayerController controller(
            *one, *two, world, AiPolicy(directionalGenome())
        );
        controller.fixedUpdate(1.0f / 60.0f);
        expect(movement->isMovingRight(), "AI controller applies horizontal action");
        expect(movement->isJumping(), "AI controller applies grounded jump action");
    }

    world.reportPlayerStomp(*one, *two);
    expect(
        world.getMinigameResult() == MinigameResult::PlayerOneWon,
        "slot one stomp records slot one victory"
    );
    world.reportPlayerStomp(*two, *one);
    expect(
        world.getMinigameResult() == MinigameResult::PlayerOneWon,
        "terminal result cannot be overwritten by duplicate dispatch"
    );

    GameWorld fallingWorld;
    fallingWorld.loadLevel(trainingMapPath());
    const std::shared_ptr<Player> fallingOne =
        fallingWorld.getPlayer(PlayerSlot::One);
    const std::shared_ptr<Player> fallingTwo =
        fallingWorld.getPlayer(PlayerSlot::Two);
    fallingOne->setPosition({100.0f, fallingWorld.getBounds().size.y + 128.0f});
    fallingWorld.updateSimulation(1.0f / 60.0f);
    expect(
        fallingWorld.getMinigameResult() == MinigameResult::PlayerTwoWon,
        "falling below the arena awards victory to the opponent"
    );

    GameWorld drawWorld;
    drawWorld.loadLevel(trainingMapPath());
    const float belowArena = drawWorld.getBounds().position.y
        + drawWorld.getBounds().size.y + 128.0f;
    drawWorld.getPlayer(PlayerSlot::One)->setPosition({100.0f, belowArena});
    drawWorld.getPlayer(PlayerSlot::Two)->setPosition({200.0f, belowArena});
    drawWorld.updateSimulation(1.0f / 60.0f);
    expect(
        drawWorld.getMinigameResult() == MinigameResult::Draw,
        "simultaneous falls produce a draw"
    );
}
}

int main(int argc, char** argv) {
    if (argc == 3 && std::string(argv[1]) == "--model") {
        try {
            const AiGenome genome = AiGenomeCodec::load(argv[2]);
            expect(
                genome.weights.size() == AiPolicy::weightCount,
                "trainer output loads through the runtime codec"
            );
        } catch (const std::exception& error) {
            std::cerr << "FAILED: trainer output could not be loaded: "
                      << error.what() << '\n';
            return 1;
        }
        return failures == 0 ? 0 : 1;
    }

    testPolicyInference();
    testCodec();
    testPlayerSlotValidation();
    testStompContactRule();
    testControllerOrdering();
    testSlotsAndResults();
    testHeuristicControllerDecisions();
    testHeuristicControllerEpisode();
    if (failures == 0) {
        std::cout << "All AI tests passed\n";
    }
    return failures == 0 ? 0 : 1;
}
