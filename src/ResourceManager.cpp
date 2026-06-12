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

ResourceManager::~ResourceManager() {
	_textures.clear();
	_fonts.clear();
	_MappingAliasToFilename.clear();
}

ResourceManager::ResourceManager() {
	_preLoadFont("assets/fonts/Roboto-VariableFont_wdth,wght.ttf", "Roboto");
}