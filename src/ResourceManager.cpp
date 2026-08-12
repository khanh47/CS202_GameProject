#include "ResourceManager.h"
#include <iostream>
#include <stdexcept>

ResourceManager &ResourceManager::getInstance() {
	static ResourceManager instance; // Created only once
	return instance; // Always returns same instance
}

void ResourceManager::_preLoadTexture(const std::string &filename, const std::string &alias) {
	if (_textures.find(filename) != _textures.end()) {
		std::cerr << "Texture already loaded: " << filename << std::endl;
		return;
	}

	sf::Texture texture;
	if (!texture.loadFromFile(filename)) {
		throw std::runtime_error("Failed to load texture: " + filename);
	}

	_textures[filename] = texture;
	_MappingAliasToFilename[alias] = filename;
}

void ResourceManager::_preLoadFont(const std::string &filename, const std::string &alias) {
	if (_fonts.find(filename) != _fonts.end())
		return;

	sf::Font font;
	if (!font.openFromFile(filename))
		throw std::runtime_error("Failed to load font: " + filename);

	_fonts[filename] = font;
	_MappingAliasToFilename[alias] = filename;
}

void ResourceManager::_preLoadMusic(const std::string &filename, const std::string &alias) {
	if (_musics.find(filename) != _musics.end())
		return;

	auto music = std::make_unique<sf::Music>();
	if (!music->openFromFile(filename))
		throw std::runtime_error("Failed to load music: " + filename);

	_musics.emplace(filename, std::move(music));
	_MappingAliasToFilename[alias] = filename;
}

void ResourceManager::_preLoadSound(const std::string &filename, const std::string &alias) {
	if (_soundBuffers.find(filename) != _soundBuffers.end())
		return;

	auto buffer = std::make_unique<sf::SoundBuffer>();
	if (!buffer->loadFromFile(filename))
		throw std::runtime_error("Failed to load sound: " + filename);

	_soundBuffers.emplace(filename, std::move(buffer));
	_MappingAliasToFilename[alias] = filename;
}

void ResourceManager::preLoadSound(const std::string &filePath, const std::string &alias) {
	_preLoadSound(filePath, alias);
}

void ResourceManager::_unloadTexture(const std::string &alias) {
	auto it = _MappingAliasToFilename.find(alias);
	if (it != _MappingAliasToFilename.end()) {
		const std::string &filename = it->second;
		auto texIt = _textures.find(filename);
		if (texIt != _textures.end()) {
			_textures.erase(texIt);
		}
		_MappingAliasToFilename.erase(it);
	}
}

void ResourceManager::_unloadFont(const std::string &alias) {
	auto it = _MappingAliasToFilename.find(alias);
	if (it != _MappingAliasToFilename.end()) {
		const std::string &filename = it->second;
		auto fontIt = _fonts.find(filename);
		if (fontIt != _fonts.end()) {
			_fonts.erase(fontIt);
		}
		_MappingAliasToFilename.erase(it);
	}
}

void ResourceManager::_unloadMusic(const std::string &alias) {
	auto it = _MappingAliasToFilename.find(alias);
	if (it != _MappingAliasToFilename.end()) {
		const std::string &filename = it->second;
		auto musicIt = _musics.find(filename);
		if (musicIt != _musics.end()) {
			_musics.erase(musicIt);
		}
		_MappingAliasToFilename.erase(it);
	}
}

// flyweight pattern implementation
sf::Texture &ResourceManager::getTexture(const std::string &alias) {
	auto it = _MappingAliasToFilename.find(alias);
	if (it != _MappingAliasToFilename.end()) {
		const std::string &filename = it->second;
		auto texIt = _textures.find(filename);
		if (texIt != _textures.end()) {
			return texIt->second;
		} else {
			throw std::runtime_error("Texture not loaded: " + filename);
		}
	} else {
		throw std::runtime_error("Texture alias not found: " + alias);
	}
}

sf::Font &ResourceManager::getFont(const std::string &alias) {
	auto it = _MappingAliasToFilename.find(alias);
	if (it != _MappingAliasToFilename.end()) {
		const std::string &filename = it->second;
		auto fontIt = _fonts.find(filename);
		if (fontIt != _fonts.end()) {
			return fontIt->second;
		} else {
			throw std::runtime_error("Font not found: " + filename);
		}
	} else {
		throw std::runtime_error("Font alias not found: " + alias);
	}
}

sf::Music &ResourceManager::getMusic(const std::string &alias) {
	auto it = _MappingAliasToFilename.find(alias);
	if (it != _MappingAliasToFilename.end()) {
		const std::string &filename = it->second;
		auto musicIt = _musics.find(filename);
		if (musicIt != _musics.end()) {
			return *musicIt->second;
		}
		throw std::runtime_error("Music not found: " + filename);
	}

	throw std::runtime_error("Music alias not found: " + alias);
}

sf::SoundBuffer &ResourceManager::getSoundBuffer(const std::string &alias) {
	auto it = _MappingAliasToFilename.find(alias);
	if (it != _MappingAliasToFilename.end()) {
		const std::string &filename = it->second;
		auto bufIt = _soundBuffers.find(filename);
		if (bufIt != _soundBuffers.end()) {
			return *bufIt->second;
		}
		throw std::runtime_error("Sound buffer not loaded: " + filename);
	}

	throw std::runtime_error("Sound alias not found: " + alias);
}

ResourceManager::~ResourceManager() {
	_textures.clear();
	_fonts.clear();
	_musics.clear();
	_MappingAliasToFilename.clear();
}

ResourceManager::ResourceManager() {
	_preLoadFont("assets/fonts/SuperMario256.ttf", "SuperMario");
	_preLoadFont("assets/fonts/moon_get-Heavy.ttf", "moon_get");

	_preLoadMusic("assets/soundtrack/title_screen.mp3", "title_screen");
	_preLoadMusic("assets/soundtrack/ground_theme.mp3", "ground_theme");
	_preLoadMusic("assets/soundtrack/underground_theme.mp3", "underground_theme");
	_preLoadMusic("assets/soundtrack/course_clear.mp3", "course_clear");
	
	_preLoadTexture("assets/sprites/Brick.png", "brick");
	_preLoadTexture("assets/sprites/Tiles/mario_and_items.png", "mario_and_items");
	_preLoadTexture("assets/sprites/Tilesets/mutiple_tilesets.png", "mutiple_tilesets");
	_preLoadTexture("assets/sprites/Tilesets/general_tiles.png", "general_tiles");

	_preLoadTexture("assets/spritesheets/mario_spritesheet.png", "mario_spritesheet");
	_preLoadTexture("assets/spritesheets/fire_mario_spritesheet.png", "fire_mario_spritesheet");
	_preLoadTexture("assets/spritesheets/luigi_spritesheet.png", "luigi_spritesheet");
	_preLoadTexture("assets/spritesheets/fire_luigi_spritesheet.png", "fire_luigi_spritesheet");

	_preLoadTexture("assets/backgrounds/far_sky.png", "far_sky");
	_preLoadTexture("assets/backgrounds/close_bush.png", "close_bush");
	_preLoadTexture("assets/backgrounds/far_underground.png", "far_underground");
	_preLoadTexture("assets/backgrounds/close_underground.png", "close_underground");
	_preLoadTexture("assets/guis/game_over.png", "game_over");
	_preLoadTexture("assets/spritesheets/goomba_spritesheet.png", "goomba_spritesheet");
	_preLoadTexture("assets/spritesheets/koopa_spritesheet.png", "koopa_spritesheet");
	_preLoadTexture("assets/spritesheets/piranha_plant_spritesheet.png", "piranha_plant_spritesheet");
	_preLoadTexture("assets/spritesheets/transparent_coin_strip.png", "coin_spritesheet");
	_preLoadTexture("assets/spritesheets/transparent_coin_block_spritesheet.png", "coin_block_spritesheet");
	_preLoadTexture("assets/spritesheets/transparent_lucky_block_spritesheet.png", "lucky_block_spritesheet");
	_preLoadTexture("assets/spritesheets/transparent_mega_coin_strip.png", "mega_coin_spritesheet");

	_preLoadTexture("assets/spritesheets/goal_flag_spritesheet.png", "goal_flag_spritesheet");
	_preLoadTexture("assets/spritesheets/checkpoint_flag_spritesheet.png", "checkpoint_flag_spritesheet");
	_preLoadTexture("assets/spritesheets/pipes_spritesheet.png", "pipes_spritesheet");
}
