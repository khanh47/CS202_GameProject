#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <limits>
#include <random>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "Game/AI/AiGenomeCodec.h"
#include "Game/AI/AiPlayerController.h"
#include "Game/GameSettings.h"
#include "Game/Minigame/MinigameTypes.h"
#include "Game/Objects/Player/Player.h"
#include "Game/World/GameWorld.h"
#include "Game/World/LevelDataLoader.h"

namespace {
struct TrainerConfig {
    std::filesystem::path mapPath =
        "assets/datas/minigames/ai/minigame.json";
    std::filesystem::path outputPath = "assets/ai/minigame.json";
    int populationSize = 64;
    int generations = 150;
    int maximumSteps = 900;
    unsigned int seed = 1337;
};

constexpr float fixedDt = 1.0f / 60.0f;
constexpr std::size_t leagueOpponentCount = 4;
constexpr std::size_t hallOfFameCapacity = 8;

int parsePositiveInt(const char* value, const char* option) {
    try {
        const int parsed = std::stoi(value);
        if (parsed <= 0) {
            throw std::runtime_error("");
        }
        return parsed;
    } catch (...) {
        throw std::runtime_error(std::string(option) + " requires a positive integer");
    }
}

TrainerConfig parseArguments(int argc, char** argv) {
    TrainerConfig config;
    for (int index = 1; index < argc; ++index) {
        const std::string_view option(argv[index]);
        if (option == "--help") {
            std::cout
                << "MarioAiTrainer options:\n"
                << "  --map PATH\n"
                << "  --output PATH\n"
                << "  --population N\n"
                << "  --generations N\n"
                << "  --max-steps N\n"
                << "  --seed N\n";
            std::exit(0);
        }
        if (index + 1 >= argc) {
            throw std::runtime_error(std::string(option) + " requires a value");
        }
        const char* value = argv[++index];
        if (option == "--map") {
            config.mapPath = value;
        } else if (option == "--output") {
            config.outputPath = value;
        } else if (option == "--population") {
            config.populationSize = parsePositiveInt(value, "--population");
        } else if (option == "--generations") {
            config.generations = parsePositiveInt(value, "--generations");
        } else if (option == "--max-steps") {
            config.maximumSteps = parsePositiveInt(value, "--max-steps");
        } else if (option == "--seed") {
            config.seed = static_cast<unsigned int>(
                parsePositiveInt(value, "--seed")
            );
        } else {
            throw std::runtime_error("Unknown option: " + std::string(option));
        }
    }
    if (config.populationSize < 2) {
        throw std::runtime_error("Population must contain at least two genomes");
    }
    return config;
}

AiGenome randomGenome(std::mt19937& random) {
    std::uniform_real_distribution<float> weightDistribution(-1.0f, 1.0f);
    AiGenome genome;
    genome.weights.resize(AiPolicy::weightCount);
    for (float& weight : genome.weights) {
        weight = weightDistribution(random);
    }
    return genome;
}

float potential(const Player& self, const Player& opponent, const GameWorld& world) {
    const sf::FloatRect bounds = world.getBounds();
    const sf::Vector2f selfPosition = self.getPosition();
    const sf::Vector2f opponentPosition = opponent.getPosition();
    const float width = std::max(bounds.size.x, 1.0f);
    const float height = std::max(bounds.size.y, 1.0f);
    const float relativeX = (opponentPosition.x - selfPosition.x) / width;
    const float relativeY = (opponentPosition.y - selfPosition.y) / height;
    const float relativeDistance = std::clamp(
        std::sqrt(relativeX * relativeX + relativeY * relativeY)
            / std::sqrt(2.0f),
        0.0f,
        1.0f
    );
    // SFML's Y axis points down, so this is positive when self is above.
    const float verticalAdvantage = std::clamp(
        (opponentPosition.y - selfPosition.y) / height,
        -1.0f,
        1.0f
    );
    // Reward closing the full 2D gap. Because episode fitness receives only
    // the change in this potential, standing nearby cannot farm score.
    constexpr float relativeDistanceWeight = 20.0f;
    constexpr float verticalAdvantageWeight = 1.0f;
    return -relativeDistanceWeight * relativeDistance
        + verticalAdvantageWeight * verticalAdvantage;
}

bool candidateWon(MinigameResult result, PlayerSlot candidateSlot) {
    return (result == MinigameResult::PlayerOneWon
            && candidateSlot == PlayerSlot::One)
        || (result == MinigameResult::PlayerTwoWon
            && candidateSlot == PlayerSlot::Two);
}

bool candidateLost(MinigameResult result, PlayerSlot candidateSlot) {
    return (result == MinigameResult::PlayerOneWon
            && candidateSlot == PlayerSlot::Two)
        || (result == MinigameResult::PlayerTwoWon
            && candidateSlot == PlayerSlot::One);
}

float playEpisode(
    const AiGenome& candidate,
    const AiGenome& opponentGenome,
    PlayerSlot candidateSlot,
    const LevelData& levelData,
    int maximumSteps
) {
    GameWorld world;
    world.loadMap(levelData);

    const std::shared_ptr<Player> playerOne = world.getPlayer(PlayerSlot::One);
    const std::shared_ptr<Player> playerTwo = world.getPlayer(PlayerSlot::Two);
    if (!playerOne || !playerTwo) {
        throw std::runtime_error("Training level must contain player slots one and two");
    }

    const AiGenome& playerOneGenome = candidateSlot == PlayerSlot::One
        ? candidate : opponentGenome;
    const AiGenome& playerTwoGenome = candidateSlot == PlayerSlot::Two
        ? candidate : opponentGenome;
    world.addController(std::make_unique<AiPlayerController>(
        *playerOne, *playerTwo, world, AiPolicy(playerOneGenome)
    ));
    world.addController(std::make_unique<AiPlayerController>(
        *playerTwo, *playerOne, world, AiPolicy(playerTwoGenome)
    ));

    Player& candidatePlayer = candidateSlot == PlayerSlot::One
        ? *playerOne : *playerTwo;
    Player& opponentPlayer = candidateSlot == PlayerSlot::One
        ? *playerTwo : *playerOne;

    float fitness = 0.0f;
    float previousPotential = potential(candidatePlayer, opponentPlayer, world);
    int step = 0;
    for (; step < maximumSteps; ++step) {
        world.updateSimulation(fixedDt);
        fitness *= 0.7f;

        const float nextPotential = potential(candidatePlayer, opponentPlayer, world);
        fitness += nextPotential - previousPotential;
        previousPotential = nextPotential;

        if (world.getMinigameResult() != MinigameResult::Running) {
            break;
        }
    }

    if (world.getMinigameResult() == MinigameResult::Running) {
        world.finishMinigameAsTimeout();
    }
    const MinigameResult result = world.getMinigameResult();
    if (candidateWon(result, candidateSlot)) {
        fitness += 1000.0f;
    } else if (candidateLost(result, candidateSlot)) {
        fitness -= 1000.0f;
    } else {
        fitness -= 100.0f;
    }
    return fitness;
}

const AiGenome& randomFrom(
    const std::vector<AiGenome>& genomes,
    std::mt19937& random
) {
    std::uniform_int_distribution<std::size_t> distribution(
        0, genomes.size() - 1
    );
    return genomes[distribution(random)];
}

std::vector<AiGenome> selectLeague(
    const std::vector<AiGenome>& population,
    const std::vector<AiGenome>& hallOfFame,
    std::mt19937& random
) {
    std::vector<AiGenome> opponents;
    opponents.reserve(leagueOpponentCount);
    if (hallOfFame.empty()) {
        while (opponents.size() < leagueOpponentCount) {
            opponents.push_back(randomFrom(population, random));
        }
        return opponents;
    }

    opponents.push_back(hallOfFame.back());
    opponents.push_back(randomFrom(hallOfFame, random));
    opponents.push_back(randomFrom(hallOfFame, random));
    opponents.push_back(randomFrom(population, random));
    return opponents;
}

void evaluatePopulation(
    std::vector<AiGenome>& population,
    const std::vector<AiGenome>& hallOfFame,
    const LevelData& levelData,
    int maximumSteps,
    std::mt19937& random
) {
    for (AiGenome& candidate : population) {
        const std::vector<AiGenome> opponents = selectLeague(
            population, hallOfFame, random
        );
        float totalFitness = 0.0f;
        int episodeCount = 0;
        for (const AiGenome& opponent : opponents) {
            totalFitness += playEpisode(
                candidate, opponent, PlayerSlot::One, levelData, maximumSteps
            );
            totalFitness += playEpisode(
                candidate, opponent, PlayerSlot::Two, levelData, maximumSteps
            );
            episodeCount += 2;
        }
        candidate.fitness = totalFitness / static_cast<float>(episodeCount);
    }
}

const AiGenome& tournamentSelect(
    const std::vector<AiGenome>& population,
    std::mt19937& random
) {
    constexpr int tournamentSize = 4;
    std::uniform_int_distribution<std::size_t> distribution(
        0, population.size() - 1
    );
    const AiGenome* best = nullptr;
    for (int index = 0; index < tournamentSize; ++index) {
        const AiGenome& candidate = population[distribution(random)];
        if (!best || candidate.fitness > best->fitness) {
            best = &candidate;
        }
    }
    return *best;
}

AiGenome reproduce(
    const AiGenome& parentA,
    const AiGenome& parentB,
    std::mt19937& random
) {
    std::bernoulli_distribution inheritFromA(0.5);
    std::bernoulli_distribution mutateWeight(0.10);
    std::normal_distribution<float> mutation(0.0f, 0.20f);

    AiGenome child;
    child.weights.resize(AiPolicy::weightCount);
    for (std::size_t index = 0; index < child.weights.size(); ++index) {
        float weight = inheritFromA(random)
            ? parentA.weights[index]
            : parentB.weights[index];
        if (mutateWeight(random)) {
            weight += mutation(random);
        }
        child.weights[index] = std::clamp(weight, -5.0f, 5.0f);
    }
    return child;
}

void createNextGeneration(
    std::vector<AiGenome>& population,
    std::mt19937& random
) {
    const std::size_t eliteCount = std::min<std::size_t>(
        6, std::max<std::size_t>(1, population.size() / 10)
    );
    std::vector<AiGenome> next;
    next.reserve(population.size());
    for (std::size_t index = 0; index < eliteCount; ++index) {
        next.push_back(population[index]);
    }
    while (next.size() < population.size()) {
        next.push_back(reproduce(
            tournamentSelect(population, random),
            tournamentSelect(population, random),
            random
        ));
    }
    population = std::move(next);
}
}

int main(int argc, char** argv) {
    try {
        const TrainerConfig config = parseArguments(argc, argv);
        GameSettings& settings = GameSettings::getInstance();
        settings.gameMode = GameMode::Minigame;
        settings.minigameMode = MinigameMode::VsAi;

        const LevelData levelData = LevelDataLoader::load(config.mapPath);
        std::mt19937 random(config.seed);
        std::vector<AiGenome> population;
        population.reserve(static_cast<std::size_t>(config.populationSize));
        for (int index = 0; index < config.populationSize; ++index) {
            population.push_back(randomGenome(random));
        }

        std::vector<AiGenome> hallOfFame;
        float bestFitness = -std::numeric_limits<float>::infinity();
        for (int generation = 0; generation < config.generations; ++generation) {
            evaluatePopulation(
                population,
                hallOfFame,
                levelData,
                config.maximumSteps,
                random
            );
            std::sort(
                population.begin(), population.end(),
                [](const AiGenome& left, const AiGenome& right) {
                    return left.fitness > right.fitness;
                }
            );

            AiGenome champion = population.front();
            champion.trainingMap = config.mapPath.stem().string();
            champion.trainingSeed = config.seed;
            champion.generation = generation;
            if (champion.fitness > bestFitness) {
                bestFitness = champion.fitness;
                AiGenomeCodec::save(config.outputPath, champion);
            }

            hallOfFame.push_back(champion);
            if (hallOfFame.size() > hallOfFameCapacity) {
                hallOfFame.erase(hallOfFame.begin());
            }

            std::cout << "Generation " << generation
                      << " best fitness: " << champion.fitness
                      << " overall best: " << bestFitness << '\n';
            if (generation + 1 < config.generations) {
                createNextGeneration(population, random);
            }
        }
        std::cout << "Training complete. Model saved to "
                  << config.outputPath.string() << '\n';
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "AI trainer error: " << error.what() << '\n';
        return 1;
    }
}
