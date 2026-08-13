#pragma once

#include <cstddef>
#include <string>
#include <vector>

struct AiObservation {
    float selfX = 0.0f;
    float selfY = 0.0f;
    float opponentX = 0.0f;
    float opponentY = 0.0f;
    float selfVelocityX = 0.0f;
    float selfVelocityY = 0.0f;
    float opponentVelocityX = 0.0f;
    float opponentVelocityY = 0.0f;
    float selfGrounded = 0.0f;
    float opponentGrounded = 0.0f;
};

struct AiAction {
    int horizontal = 0;
    bool jump = false;
};

struct AiGenome {
    std::vector<float> weights;
    std::string trainingMap;
    unsigned int trainingSeed = 0;
    int generation = 0;
    float fitness = 0.0f;
};

class AiPolicy {
public:
    static constexpr int modelVersion = 1;
    static constexpr std::size_t inputCount = 10;
    static constexpr std::size_t hiddenCount = 12;
    static constexpr std::size_t outputCount = 3;
    static constexpr std::size_t weightCount =
        (inputCount + 1) * hiddenCount
        + (hiddenCount + 1) * outputCount;

    explicit AiPolicy(AiGenome genome);

    AiAction decide(const AiObservation& observation) const;
    const AiGenome& genome() const noexcept { return _genome; }

private:
    AiGenome _genome;
};
