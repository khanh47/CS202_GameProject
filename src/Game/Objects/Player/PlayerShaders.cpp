#include "Game/Objects/Player/PlayerShaders.h"

#include <iostream>
#include <string_view>

// ─── Blink shader ────────────────────────────────────────────────────────────
// Rapidly alternates every pixel between pure white and the raw texture color.
// Uses a square-wave derived from sin() so the transition is an abrupt toggle
// rather than a smooth fade, giving the classic NES-style power-up blink.
static constexpr std::string_view kBlinkFragmentSrc = R"glsl(
    uniform sampler2D texture;
    uniform float u_time;

    void main() {
        vec4 pixel = texture2D(texture, gl_TexCoord[0].xy);

        // Square-wave at ~8 Hz: positive half → white, negative half → raw
        float wave = sin(u_time * 50.2655);   // 8 * 2π ≈ 50.2655
        float blend = step(0.0, wave);

        vec4 white = vec4(1.0, 1.0, 1.0, pixel.a);
        gl_FragColor = mix(pixel, white, blend);
    }
)glsl";

// ─── Rainbow shader ─────────────────────────────────────────────────────────
// Shifts the hue of every pixel continuously. Converts RGB → HSV, adds a
// time-dependent offset to the hue channel, then converts back to RGB.
static constexpr std::string_view kRainbowFragmentSrc = R"glsl(
    uniform sampler2D texture;
    uniform float u_time;

    vec3 rgb2hsv(vec3 c) {
        vec4 K = vec4(0.0, -1.0/3.0, 2.0/3.0, -1.0);
        vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
        vec4 q = mix(vec4(p.xyw, c.r),  vec4(c.r, p.yzx), step(p.x, c.r));
        float d = q.x - min(q.w, q.y);
        float e = 1.0e-10;
        return vec3(abs(q.z + (q.w - q.y) / (6.0*d + e)), d / (q.x + e), q.x);
    }

    vec3 hsv2rgb(vec3 c) {
        vec3 p = abs(fract(vec3(c.x) + vec3(1.0, 2.0/3.0, 1.0/3.0)) * 6.0 - 3.0);
        return c.z * mix(vec3(1.0), clamp(p - 1.0, 0.0, 1.0), c.y);
    }

    void main() {
        vec4 pixel = texture2D(texture, gl_TexCoord[0].xy);
        if (pixel.a < 0.01) {
            gl_FragColor = pixel;
            return;
        }

        vec3 hsv = rgb2hsv(pixel.rgb);
        // Rotate hue over time — full cycle every ~2 seconds
        hsv.x = fract(hsv.x + u_time * 0.5);
        // Boost saturation slightly so pastel sprites still look vivid
        hsv.y = min(hsv.y + 0.3, 1.0);
        gl_FragColor = vec4(hsv2rgb(hsv), pixel.a);
    }
)glsl";

// ─── Singleton ──────────────────────────────────────────────────────────────

PlayerShaders& PlayerShaders::getInstance() {
    static PlayerShaders instance;
    return instance;
}

PlayerShaders::PlayerShaders() {
    if (!sf::Shader::isAvailable()) {
        std::cerr << "[PlayerShaders] Shaders not available on this system.\n";
        _shadersAvailable = false;
        return;
    }

    bool blinkOk = _blinkShader.loadFromMemory(
        std::string(kBlinkFragmentSrc), sf::Shader::Type::Fragment
    );
    bool rainbowOk = _rainbowShader.loadFromMemory(
        std::string(kRainbowFragmentSrc), sf::Shader::Type::Fragment
    );

    if (!blinkOk) {
        std::cerr << "[PlayerShaders] Failed to compile blink shader.\n";
    }
    if (!rainbowOk) {
        std::cerr << "[PlayerShaders] Failed to compile rainbow shader.\n";
    }

    _shadersAvailable = blinkOk && rainbowOk;

    if (_shadersAvailable) {
        _blinkShader.setUniform("texture", sf::Shader::CurrentTexture);
        _rainbowShader.setUniform("texture", sf::Shader::CurrentTexture);
    }
}

void PlayerShaders::update(float deltaTime) {
    _time += deltaTime;
}

sf::Shader* PlayerShaders::getBlinkShader() {
    if (!_shadersAvailable) return nullptr;
    _blinkShader.setUniform("u_time", _time);
    return &_blinkShader;
}

sf::Shader* PlayerShaders::getRainbowShader() {
    if (!_shadersAvailable) return nullptr;
    _rainbowShader.setUniform("u_time", _time);
    return &_rainbowShader;
}
