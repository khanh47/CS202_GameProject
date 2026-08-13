#include "Game/AI/AiPolicy.h"

#include <array>
#include <cmath>
#include <stdexcept>

AiPolicy::AiPolicy(AiGenome genome) : _genome(std::move(genome)) {
    if (_genome.weights.size() != weightCount) {
        throw std::invalid_argument(
            "AI genome must contain exactly "
            + std::to_string(weightCount) + " weights"
        );
    }
    for (const float weight : _genome.weights) {
        if (!std::isfinite(weight)) {
            throw std::invalid_argument("AI genome contains a non-finite weight");
        }
    }
}

AiAction AiPolicy::decide(const AiObservation& observation) const {
    const std::array<float, inputCount> inputs = {
        observation.selfX,
        observation.selfY,
        observation.opponentX,
        observation.opponentY,
        observation.selfVelocityX,
        observation.selfVelocityY,
        observation.opponentVelocityX,
        observation.opponentVelocityY,
        observation.selfGrounded,
        observation.opponentGrounded
    };

    std::array<float, hiddenCount> hidden{};
    std::size_t weightIndex = 0;
    for (float& hiddenValue : hidden) {
        float sum = _genome.weights[weightIndex++];
        for (const float input : inputs) {
            sum += input * _genome.weights[weightIndex++];
        }
        hiddenValue = std::tanh(sum);
    }

    std::array<float, outputCount> outputs{};
    for (float& output : outputs) {
        float sum = _genome.weights[weightIndex++];
        for (const float hiddenValue : hidden) {
            sum += hiddenValue * _genome.weights[weightIndex++];
        }
        output = std::tanh(sum);
    }

    AiAction action;
    constexpr float horizontalThreshold = 0.15f;
    const float horizontalDifference = outputs[1] - outputs[0];
    if (horizontalDifference > horizontalThreshold) {
        action.horizontal = 1;
    } else if (horizontalDifference < -horizontalThreshold) {
        action.horizontal = -1;
    }
    action.jump = outputs[2] > 0.0f;
    return action;
}
