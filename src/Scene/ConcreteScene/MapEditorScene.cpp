#include "Scene/ConcreteScene/MapEditorScene.h"

#include <algorithm>
#include <cmath>
#include <exception>
#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <utility>

#include <nlohmann/json.hpp>

#include "Commands/FunctionalCommand.h"
#include "Game/World/LevelDataLoader.h"
#include "ResourceManager.h"
#include "Scene/ConcreteScene/InGameScene.h"
#include "Scene/SceneManager.h"

namespace {
using json = nlohmann::json;

const sf::Color categoryColor(MapEditorScene::Category category) {
    switch (category) {
        case MapEditorScene::Category::Blocks:
            return sf::Color(180, 105, 45);
        case MapEditorScene::Category::Items:
            return sf::Color(190, 145, 35);
        case MapEditorScene::Category::Enemies:
            return sf::Color(170, 70, 70);
        case MapEditorScene::Category::Players:
            return sf::Color(65, 105, 180);
    }
    return sf::Color(100, 100, 100);
}

}

MapEditorScene::MapEditorScene()
    : Scene("MapEditorScene"),
      _cells(
          static_cast<std::size_t>(MapWidth * MapHeight),
          '.'
      ),
      _mapView(
          {MapViewportWidth * 0.5f, MapViewportHeight * 0.5f},
          {MapViewportWidth, MapViewportHeight}
      ),
      _screenBackdrop({1920.0f, 1080.0f}),
      _gridBackdrop({MapWidth * CellSize, MapHeight * CellSize}),
      _paletteBackdrop({320.0f, 980.0f}),
      _titleText(
          ResourceManager::getInstance().getFont("SuperMario"),
          "MAP BUILDER",
          46
      ),
      _paletteTitleText(
          ResourceManager::getInstance().getFont("SuperMario"),
          "PALETTE",
          28
      ),
      _selectedText(
          ResourceManager::getInstance().getFont("moon_get"),
          "Selected: Brick (#)",
          19
      ),
      _statusText(
          ResourceManager::getInstance().getFont("moon_get"),
          "",
          16
      ),
      _instructionsBackdrop({LogicalScreenWidth, LogicalScreenHeight}),
      _instructionsPanel({1300.0f, 760.0f}),
      _instructionsTitle(
          ResourceManager::getInstance().getFont("SuperMario"),
          "MAP EDITOR INSTRUCTIONS",
          36
      ),
      _instructionsBody(
          ResourceManager::getInstance().getFont("moon_get"),
          "Mouse controls\n\n"
          "Wheel: zoom in/out\n"
          "Middle mouse + drag: move the camera\n"
          "Hold left mouse: place continuously\n"
          "Hold right mouse: erase continuously\n"
          "Shift + left mouse drag: fill a rectangle\n"
          "Shift + right mouse drag: erase a rectangle\n\n"
          "Keyboard controls\n\n"
          "Ctrl + Z: undo       Ctrl + Y: redo\n"
          "S: save map          P: save and play\n"
          "T: change theme       C: clear map\n"
          "1-4: choose a category\n"
          "Esc: close this screen / go back",
          22
      ) {
    _paletteEntries = {
        {Category::Blocks, "Brick", "brick", '#', sf::Color(183, 111, 46)},
        {Category::Blocks, "Ground", "terrain_grassland", 'A', sf::Color(70, 160, 86)},
        {Category::Blocks, "Coin Block", "block_coin", 'B', sf::Color(205, 157, 45)},
        {Category::Blocks, "Lucky Block", "block_lucky", '?', sf::Color(220, 175, 45)},
        {Category::Blocks, "Pipe", "pipe_basic", 'V', sf::Color(60, 160, 80)},
        {Category::Items, "Coin", "item_coin", 'c', sf::Color(244, 190, 35)},
        {Category::Items, "Fire Flower", "item_fire_flower", 'f', sf::Color(230, 75, 55)},
        {Category::Items, "Super Mushroom", "item_super_mushroom", 'u', sf::Color(210, 55, 55)},
        {Category::Items, "1-Up Mushroom", "item_one_up_mushroom", 'i', sf::Color(70, 185, 90)},
        {Category::Items, "Mega Mushroom", "item_mega_mushroom", 'G', sf::Color(215, 100, 50)},
        {Category::Items, "Super Star", "item_super_star", 's', sf::Color(250, 220, 75)},
        {Category::Items, "Mega Coin", "item_mega_coin", 'o', sf::Color(255, 185, 35)},
        {Category::Items, "Goal Flag", "item_flagpole", 'D', sf::Color(90, 190, 110)},
        {Category::Enemies, "Goomba", "enemy_goomba", 'e', sf::Color(160, 90, 55)},
        {Category::Enemies, "Koopa", "enemy_koopa", 'k', sf::Color(75, 165, 75)},
        {Category::Enemies, "Piranha Plant", "enemy_piranha_plant", 'p', sf::Color(200, 70, 70)},
        {Category::Players, "Mario", "player_mario", 'M', sf::Color(65, 105, 210)},
        {Category::Players, "Luigi", "player_luigi", 'L', sf::Color(55, 165, 85)}
    };

    _themeOptions = {
        {"sky", "Sky", "parallax_sky", "ground_theme"},
        {"underground", "Underground", "parallax_underground", "underground_theme"}
    };

    _mapView.setViewport({
        {0.0f, 0.0f},
        {MapViewportWidth / LogicalScreenWidth, 1.0f}
    });
    clampMapView();

    _screenBackdrop.setPosition({0.0f, 0.0f});
    _screenBackdrop.setFillColor(sf::Color(10, 20, 38));

    _gridBackdrop.setPosition({0.0f, 0.0f});
    _gridBackdrop.setFillColor(sf::Color(19, 43, 71));
    _gridBackdrop.setOutlineThickness(3.0f);
    _gridBackdrop.setOutlineColor(sf::Color(108, 163, 210));

    _paletteBackdrop.setPosition({1584.0f, 70.0f});
    _paletteBackdrop.setFillColor(sf::Color(24, 45, 72, 245));
    _paletteBackdrop.setOutlineThickness(2.0f);
    _paletteBackdrop.setOutlineColor(sf::Color(95, 142, 190));

    _titleText.setPosition({24.0f, 24.0f});
    _titleText.setFillColor(sf::Color(255, 226, 120));
    _titleText.setOutlineColor(sf::Color::Black);
    _titleText.setOutlineThickness(4.0f);

    _paletteTitleText.setPosition({PaletteLeft, 88.0f});
    _paletteTitleText.setFillColor(sf::Color(255, 226, 120));
    _paletteTitleText.setOutlineColor(sf::Color::Black);
    _paletteTitleText.setOutlineThickness(2.0f);

    _selectedText.setPosition({500.0f, 40.0f});
    _selectedText.setFillColor(sf::Color::White);

    _statusText.setPosition({500.0f, 78.0f});
    _statusText.setFillColor(sf::Color(180, 220, 255));
    _statusText.setOutlineColor(sf::Color::Black);
    _statusText.setOutlineThickness(1.0f);

    _instructionsBackdrop.setPosition({0.0f, 0.0f});
    _instructionsBackdrop.setFillColor(sf::Color(0, 0, 0, 205));

    _instructionsPanel.setPosition({310.0f, 110.0f});
    _instructionsPanel.setFillColor(sf::Color(24, 45, 72, 250));
    _instructionsPanel.setOutlineThickness(4.0f);
    _instructionsPanel.setOutlineColor(sf::Color(120, 180, 235));

    _instructionsTitle.setPosition({650.0f, 145.0f});
    _instructionsTitle.setFillColor(sf::Color(255, 226, 120));
    _instructionsTitle.setOutlineColor(sf::Color::Black);
    _instructionsTitle.setOutlineThickness(3.0f);

    _instructionsBody.setPosition({390.0f, 225.0f});
    _instructionsBody.setFillColor(sf::Color(235, 245, 255));
}

void MapEditorScene::init() {
    setupMenus();
    selectSymbol('#');
    if (!loadSavedMap()) {
        setStatus("New map: 80 x 40 cells");
    }
    refreshThemeButton();
}

void MapEditorScene::onEnter() {
    Scene::onEnter();
}

void MapEditorScene::onExit() {
    Scene::onExit();
}

void MapEditorScene::updateSimulation(const float& fixedDt) {
    (void)fixedDt;
}

void MapEditorScene::updateVisuals(float deltaTime) {
    _categoryMenu.updateVisuals(deltaTime);
    _paletteMenu.updateVisuals(deltaTime);
    _themeMenu.updateVisuals(deltaTime);
    _actionMenu.updateVisuals(deltaTime);
    _instructionsMenu.updateVisuals(deltaTime);
}

void MapEditorScene::handleInput(const sf::Event& event) {
    if (_showInstructions) {
        if (const auto* keyEvent = event.getIf<sf::Event::KeyPressed>()) {
            if (keyEvent->code == sf::Keyboard::Key::Escape) {
                _showInstructions = false;
                return;
            }
        }
        _instructionsMenu.processEvent(event);
        return;
    }

    if (const auto* keyEvent = event.getIf<sf::Event::KeyPressed>()) {
        if (keyEvent->control && keyEvent->code == sf::Keyboard::Key::Z) {
            undoLastEdit();
            return;
        }
        if (keyEvent->control && keyEvent->code == sf::Keyboard::Key::Y) {
            redoLastEdit();
            return;
        }

        switch (keyEvent->code) {
            case sf::Keyboard::Key::Escape:
                if (auto* manager = getSceneManager()) {
                    manager->requestPopScene();
                }
                return;
            case sf::Keyboard::Key::S:
                saveMap();
                return;
            case sf::Keyboard::Key::P:
                saveAndPlay();
                return;
            case sf::Keyboard::Key::U:
                undoLastEdit();
                return;
            case sf::Keyboard::Key::T:
                cycleTheme();
                return;
            case sf::Keyboard::Key::C:
                clearMap();
                return;
            case sf::Keyboard::Key::Num1:
                selectCategory(Category::Blocks);
                return;
            case sf::Keyboard::Key::Num2:
                selectCategory(Category::Items);
                return;
            case sf::Keyboard::Key::Num3:
                selectCategory(Category::Enemies);
                return;
            case sf::Keyboard::Key::Num4:
                selectCategory(Category::Players);
                return;
            default:
                return;
        }
    }

    if (const auto* mouseWheel = event.getIf<sf::Event::MouseWheelScrolled>()) {
        zoomMap(
            mouseWheel->delta,
            mouseWheel->position
        );
        return;
    }

    if (const auto* mouseMove = event.getIf<sf::Event::MouseMoved>()) {
        handleMouseMoved(*mouseMove);
        _categoryMenu.processEvent(event);
        _paletteMenu.processEvent(event);
        _themeMenu.processEvent(event);
        _actionMenu.processEvent(event);
        return;
    }

    if (const auto* mousePress = event.getIf<sf::Event::MouseButtonPressed>()) {
        _categoryMenu.processEvent(event);
        _paletteMenu.processEvent(event);
        _themeMenu.processEvent(event);
        _actionMenu.processEvent(event);
        handleMousePressed(*mousePress);
        return;
    }

    if (const auto* mouseRelease = event.getIf<sf::Event::MouseButtonReleased>()) {
        handleMouseReleased(*mouseRelease);
        return;
    }

    if (event.is<sf::Event::FocusLost>() || event.is<sf::Event::MouseLeft>()) {
        _leftMouseHeld = false;
        _rightMouseHeld = false;
        _middleMouseHeld = false;
        _paintActive = false;
        _rectangleDrag = false;
        _strokeUndoCaptured = false;
        _hoverColumn = -1;
        _hoverRow = -1;
    }
}

void MapEditorScene::render(sf::RenderTarget& target) {
    target.setView(target.getDefaultView());
    target.draw(_screenBackdrop);

    drawMap(target);

    target.setView(target.getDefaultView());
    target.draw(_paletteBackdrop);
    target.draw(_titleText);
    target.draw(_paletteTitleText);
    target.draw(_selectedText);
    target.draw(_statusText);

    _categoryMenu.render(target);
    _paletteMenu.render(target);
    _themeMenu.render(target);
    _actionMenu.render(target);

    if (_showInstructions) {
        target.draw(_instructionsBackdrop);
        target.draw(_instructionsPanel);
        target.draw(_instructionsTitle);
        target.draw(_instructionsBody);
        _instructionsMenu.render(target);
    }
}

void MapEditorScene::setupMenus() {
    setupCategoryMenu();
    setupPaletteMenu();
    setupThemeMenu();
    setupActionMenu();
    setupInstructionsMenu();

    _categoryMenu.setMouseOnly(true);
    _paletteMenu.setMouseOnly(true);
    _themeMenu.setMouseOnly(true);
    _actionMenu.setMouseOnly(true);
    _instructionsMenu.setMouseOnly(true);
}

void MapEditorScene::setupCategoryMenu() {
    _categoryMenu.clear();
    _categoryMenu.setLayoutProperties(
        {1610.0f, 132.0f},
        {72.0f, 42.0f},
        76.0f,
        true,
        sf::Color(62, 105, 157),
        15
    );

    const std::vector<std::pair<Category, std::string>> categories = {
        {Category::Blocks, "Blocks"},
        {Category::Items, "Items"},
        {Category::Enemies, "Enemies"},
        {Category::Players, "Players"}
    };
    for (const auto& [category, label] : categories) {
        _categoryMenu.addButtonAuto(
            label,
            15,
            std::make_unique<FunctionalCommand>(
                label,
                [this, category]() { selectCategory(category); }
            ),
            categoryColor(category)
        );
    }
}

void MapEditorScene::setupPaletteMenu() {
    _paletteMenu.clear();
    _paletteMenu.setLayoutProperties(
        {1605.0f, 210.0f},
        {285.0f, 50.0f},
        53.0f,
        false,
        categoryColor(_activeCategory),
        18
    );

    for (const PaletteEntry& entry : _paletteEntries) {
        if (entry.category != _activeCategory) {
            continue;
        }

        _paletteMenu.addButtonAuto(
            entry.label,
            18,
            std::make_unique<FunctionalCommand>(
                entry.label,
                [this, symbol = entry.symbol]() { selectSymbol(symbol); }
            ),
            entry.previewColor
        );
    }
}

void MapEditorScene::setupThemeMenu() {
    _themeMenu.clear();
    _themeMenu.setLayoutProperties(
        {1605.0f, 650.0f},
        {285.0f, 40.0f},
        46.0f,
        false,
        sf::Color(70, 110, 150),
        16
    );

    _themeMenu.addButtonAuto(
        "Theme",
        16,
        std::make_unique<FunctionalCommand>(
            "Theme", [this]() { cycleTheme(); }
        )
    );
}

void MapEditorScene::setupInstructionsMenu() {
    _instructionsMenu.clear();
    _instructionsMenu.setLayoutProperties(
        {825.0f, 805.0f},
        {270.0f, 55.0f},
        60.0f,
        false,
        sf::Color(62, 105, 157),
        20
    );
    _instructionsMenu.addButtonAuto(
        "Close",
        20,
        std::make_unique<FunctionalCommand>(
            "Close", [this]() { _showInstructions = false; }
        )
    );
}

void MapEditorScene::refreshThemeButton() {
    if (const auto themeButton = _themeMenu.getButton(0);
        themeButton && !_themeOptions.empty()) {
        themeButton->setText(
            "Theme: " + _themeOptions[_themeIndex].label
        );
    }
}

void MapEditorScene::cycleTheme() {
    if (_themeOptions.empty()) {
        return;
    }

    _themeIndex = (_themeIndex + 1) % _themeOptions.size();
    _themeKey = _themeOptions[_themeIndex].key;
    _dirty = true;
    refreshThemeButton();
    setStatus("Theme: " + _themeOptions[_themeIndex].label);
}

void MapEditorScene::applyLoadedTheme(
    const std::string& theme,
    const std::string& background,
    const std::string& music
) {
    if (_themeOptions.empty()) {
        return;
    }

    const auto themeMatches = [&theme, &background, &music](
        const ThemeChoice& choice
    ) {
        return (!theme.empty() && choice.key == theme)
            || (theme.empty()
                && (choice.background == background || choice.music == music));
    };
    const auto themeIt = std::find_if(
        _themeOptions.begin(),
        _themeOptions.end(),
        themeMatches
    );
    _themeIndex = themeIt == _themeOptions.end()
        ? 0
        : static_cast<std::size_t>(
            std::distance(_themeOptions.begin(), themeIt)
        );
    _themeKey = _themeOptions[_themeIndex].key;
    refreshThemeButton();
}

void MapEditorScene::setupActionMenu() {
    _actionMenu.clear();
    _actionMenu.setLayoutProperties(
        {1605.0f, 730.0f},
        {285.0f, 40.0f},
        42.0f,
        false,
        sf::Color(53, 91, 130),
        16
    );

    _actionMenu.addButtonAuto(
        "Save Map",
        std::make_unique<FunctionalCommand>(
            "Save Map", [this]() { saveMap(); }
        )
    );
    _actionMenu.addButtonAuto(
        "Save & Play",
        std::make_unique<FunctionalCommand>(
            "Save & Play", [this]() { saveAndPlay(); }
        )
    );
    _actionMenu.addButtonAuto(
        "Undo",
        16,
        std::make_unique<FunctionalCommand>(
            "Undo", [this]() { undoLastEdit(); }
        )
    );
    _actionMenu.addButtonAuto(
        "Redo",
        16,
        std::make_unique<FunctionalCommand>(
            "Redo", [this]() { redoLastEdit(); }
        )
    );
    _actionMenu.addButtonAuto(
        "Clear Map",
        16,
        std::make_unique<FunctionalCommand>(
            "Clear Map", [this]() { clearMap(); }
        )
    );
    _actionMenu.addButtonAuto(
        "Back",
        16,
        std::make_unique<FunctionalCommand>(
            "Back", [this]() {
                if (auto* manager = getSceneManager()) {
                    manager->requestPopScene();
                }
            }
        )
    );
    _actionMenu.addButtonAuto(
        "Instructions",
        16,
        std::make_unique<FunctionalCommand>(
            "Instructions", [this]() { _showInstructions = true; }
        )
    );
}

void MapEditorScene::handleMouseMoved(
    const sf::Event::MouseMoved& mouseEvent
) {
    if (_middleMouseHeld) {
        panMap(mouseEvent.position);
    }

    updateHover(mouseEvent.position);
    if (_paintActive && !_middleMouseHeld) {
        continuePaint(mouseEvent.position);
    }
}

void MapEditorScene::handleMousePressed(
    const sf::Event::MouseButtonPressed& mouseEvent
) {
    switch (mouseEvent.button) {
        case sf::Mouse::Button::Left:
            _leftMouseHeld = true;
            beginPaint(mouseEvent.button, mouseEvent.position);
            break;
        case sf::Mouse::Button::Right:
            _rightMouseHeld = true;
            beginPaint(mouseEvent.button, mouseEvent.position);
            break;
        case sf::Mouse::Button::Middle:
            if (isMapCanvasPosition(mouseEvent.position)) {
                _middleMouseHeld = true;
                _lastMousePosition = mouseEvent.position;
            }
            break;
        default:
            break;
    }
}

void MapEditorScene::handleMouseReleased(
    const sf::Event::MouseButtonReleased& mouseEvent
) {
    switch (mouseEvent.button) {
        case sf::Mouse::Button::Left:
            _leftMouseHeld = false;
            break;
        case sf::Mouse::Button::Right:
            _rightMouseHeld = false;
            break;
        case sf::Mouse::Button::Middle:
            _middleMouseHeld = false;
            break;
        default:
            break;
    }

    endPaint(mouseEvent.button);
}

void MapEditorScene::updateHover(sf::Vector2i screenPosition) {
    const std::optional<sf::Vector2i> cell = mapCellAtScreen(screenPosition);
    if (!cell.has_value()) {
        _hoverColumn = -1;
        _hoverRow = -1;
        return;
    }

    _hoverColumn = cell->x;
    _hoverRow = cell->y;
}

void MapEditorScene::beginPaint(
    sf::Mouse::Button button,
    sf::Vector2i screenPosition
) {
    if (_middleMouseHeld) {
        _paintActive = false;
        return;
    }

    const std::optional<sf::Vector2i> cell = mapCellAtScreen(screenPosition);
    if (!cell.has_value()) {
        _paintActive = false;
        _rectangleDrag = false;
        return;
    }

    _paintButton = button;
    _dragStartColumn = cell->x;
    _dragStartRow = cell->y;
    _rectangleDrag = isShiftHeld();
    _strokeUndoCaptured = false;
    _paintActive = true;
    updateHover(screenPosition);
    continuePaint(screenPosition);
}

void MapEditorScene::continuePaint(sf::Vector2i screenPosition) {
    if (!_paintActive) {
        return;
    }

    const std::optional<sf::Vector2i> cell = mapCellAtScreen(screenPosition);
    if (!cell.has_value()) {
        return;
    }

    const char symbol = _paintButton == sf::Mouse::Button::Left
        ? _selectedSymbol
        : '.';
    if (_rectangleDrag) {
        applyPaintRectangle(
            _dragStartColumn,
            _dragStartRow,
            cell->x,
            cell->y,
            symbol
        );
    } else {
        applyPaintCell(cell->x, cell->y, symbol);
    }
}

void MapEditorScene::endPaint(sf::Mouse::Button button) {
    if (button != _paintButton) {
        return;
    }

    _paintActive = false;
    _rectangleDrag = false;
    _strokeUndoCaptured = false;
    _dragStartColumn = -1;
    _dragStartRow = -1;
}

void MapEditorScene::panMap(sf::Vector2i screenPosition) {
    const sf::Vector2i delta = screenPosition - _lastMousePosition;
    const sf::Vector2f viewSize = _mapView.getSize();
    _mapView.move({
        -static_cast<float>(delta.x) * viewSize.x / MapViewportWidth,
        -static_cast<float>(delta.y) * viewSize.y / MapViewportHeight
    });
    clampMapView();
    _lastMousePosition = screenPosition;
}

void MapEditorScene::zoomMap(
    float wheelDelta,
    sf::Vector2i screenPosition
) {
    if (!isMapCanvasPosition(screenPosition) || wheelDelta == 0.0f) {
        return;
    }

    const sf::Vector2f worldBeforeZoom = mapScreenToWorld({
        static_cast<float>(screenPosition.x),
        static_cast<float>(screenPosition.y)
    });

    _zoom = std::clamp(
        _zoom * std::pow(1.15f, wheelDelta),
        MinimumZoom,
        MaximumZoom
    );
    _mapView.setSize({
        MapViewportWidth / _zoom,
        MapViewportHeight / _zoom
    });

    const sf::Vector2f worldAfterZoom = mapScreenToWorld({
        static_cast<float>(screenPosition.x),
        static_cast<float>(screenPosition.y)
    });
    _mapView.move(worldBeforeZoom - worldAfterZoom);
    clampMapView();

    setStatus(
        "Zoom: " + std::to_string(static_cast<int>(std::round(_zoom * 100.0f))) + "%"
    );
}

void MapEditorScene::clampMapView() {
    const sf::Vector2f mapSize = {
        MapWidth * CellSize,
        MapHeight * CellSize
    };
    sf::Vector2f center = _mapView.getCenter();

    // Keep a generous empty margin around the map so the editor can inspect
    // the map boundary from outside it, while still preventing the canvas
    // from being dragged indefinitely away from the level.
    center.x = std::clamp(
        center.x,
        -CameraPanMargin,
        mapSize.x + CameraPanMargin
    );
    center.y = std::clamp(
        center.y,
        -CameraPanMargin,
        mapSize.y + CameraPanMargin
    );

    _mapView.setCenter(center);
}

sf::Vector2f MapEditorScene::mapScreenToWorld(
    sf::Vector2f screenPosition
) const {
    const sf::Vector2f viewSize = _mapView.getSize();
    const sf::Vector2f viewTopLeft = _mapView.getCenter() - viewSize * 0.5f;
    return viewTopLeft + sf::Vector2f{
        screenPosition.x / MapViewportWidth * viewSize.x,
        screenPosition.y / MapViewportHeight * viewSize.y
    };
}

std::optional<sf::Vector2i> MapEditorScene::mapCellAtScreen(
    sf::Vector2i screenPosition
) const {
    if (!isMapCanvasPosition(screenPosition)) {
        return std::nullopt;
    }

    const sf::Vector2f worldPosition = mapScreenToWorld({
        static_cast<float>(screenPosition.x),
        static_cast<float>(screenPosition.y)
    });
    const int column = static_cast<int>(std::floor(worldPosition.x / CellSize));
    const int row = static_cast<int>(std::floor(worldPosition.y / CellSize));
    if (column < 0 || column >= MapWidth || row < 0 || row >= MapHeight) {
        return std::nullopt;
    }

    return sf::Vector2i{column, row};
}

bool MapEditorScene::isMapCanvasPosition(sf::Vector2i screenPosition) const {
    if (screenPosition.x < 0 || screenPosition.x >= MapViewportWidth
        || screenPosition.y < 0 || screenPosition.y >= MapViewportHeight) {
        return false;
    }
    return true;
}

bool MapEditorScene::isShiftHeld() const {
    return sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift)
        || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RShift);
}

void MapEditorScene::drawMap(sf::RenderTarget& target) {
    target.setView(_mapView);
    target.draw(_gridBackdrop);
    drawMapGrid(target);

    const sf::FloatRect viewBounds(
        _mapView.getCenter() - _mapView.getSize() * 0.5f,
        _mapView.getSize()
    );
    const int startColumn = std::max(
        0,
        static_cast<int>(std::floor(viewBounds.position.x / CellSize)) - 1
    );
    const int endColumn = std::min(
        MapWidth - 1,
        static_cast<int>(std::ceil(
            (viewBounds.position.x + viewBounds.size.x) / CellSize
        )) + 1
    );
    const int startRow = std::max(
        0,
        static_cast<int>(std::floor(viewBounds.position.y / CellSize)) - 1
    );
    const int endRow = std::min(
        MapHeight - 1,
        static_cast<int>(std::ceil(
            (viewBounds.position.y + viewBounds.size.y) / CellSize
        )) + 1
    );

    const sf::Font& font = ResourceManager::getInstance().getFont("SuperMario");
    for (int row = startRow; row <= endRow; ++row) {
        for (int column = startColumn; column <= endColumn; ++column) {
            const char symbol = _cells[
                static_cast<std::size_t>(row * MapWidth + column)
            ];
            if (symbol == '.') {
                continue;
            }

            const PaletteEntry* entry = findEntry(symbol);
            const sf::Color fill = entry != nullptr
                ? sf::Color(entry->previewColor.r, entry->previewColor.g, entry->previewColor.b, 170)
                : sf::Color(100, 100, 100, 170);

            sf::RectangleShape cell({CellSize - 4.0f, CellSize - 4.0f});
            cell.setPosition({
                column * CellSize + 2.0f,
                row * CellSize + 2.0f
            });
            cell.setFillColor(fill);
            cell.setOutlineThickness(1.0f);
            cell.setOutlineColor(sf::Color(235, 245, 255, 190));
            target.draw(cell);

            sf::Text symbolText(font, std::string(1, symbol), 30);
            const sf::FloatRect bounds = symbolText.getLocalBounds();
            symbolText.setOrigin({
                bounds.position.x + bounds.size.x * 0.5f,
                bounds.position.y + bounds.size.y * 0.5f
            });
            symbolText.setPosition({
                column * CellSize + CellSize * 0.5f,
                row * CellSize + CellSize * 0.5f
            });
            symbolText.setFillColor(sf::Color::White);
            symbolText.setOutlineColor(sf::Color::Black);
            symbolText.setOutlineThickness(2.0f);
            target.draw(symbolText);
        }
    }

    if (_rectangleDrag && _paintActive
        && _hoverColumn >= 0 && _hoverRow >= 0) {
        const int left = std::min(_dragStartColumn, _hoverColumn);
        const int top = std::min(_dragStartRow, _hoverRow);
        const int right = std::max(_dragStartColumn, _hoverColumn);
        const int bottom = std::max(_dragStartRow, _hoverRow);
        sf::RectangleShape selection({
            (right - left + 1) * CellSize,
            (bottom - top + 1) * CellSize
        });
        selection.setPosition({left * CellSize, top * CellSize});
        selection.setFillColor(
            _paintButton == sf::Mouse::Button::Left
                ? sf::Color(255, 235, 100, 45)
                : sf::Color(255, 100, 100, 45)
        );
        selection.setOutlineThickness(3.0f);
        selection.setOutlineColor(
            _paintButton == sf::Mouse::Button::Left
                ? sf::Color(255, 235, 100)
                : sf::Color(255, 130, 130)
        );
        target.draw(selection);
    }

    if (_hoverColumn >= 0 && _hoverRow >= 0) {
        sf::RectangleShape hover({CellSize - 4.0f, CellSize - 4.0f});
        hover.setPosition({
            _hoverColumn * CellSize + 2.0f,
            _hoverRow * CellSize + 2.0f
        });
        hover.setFillColor(sf::Color(255, 255, 255, 35));
        hover.setOutlineThickness(3.0f);
        hover.setOutlineColor(sf::Color(255, 235, 100));
        target.draw(hover);
    }
}

void MapEditorScene::drawMapGrid(sf::RenderTarget& target) {
    sf::VertexArray gridLines(sf::PrimitiveType::Lines);
    for (int column = 0; column <= MapWidth; ++column) {
        const float x = column * CellSize;
        gridLines.append(sf::Vertex({x, 0.0f}, sf::Color(80, 123, 160)));
        gridLines.append(sf::Vertex({x, MapHeight * CellSize}, sf::Color(80, 123, 160)));
    }
    for (int row = 0; row <= MapHeight; ++row) {
        const float y = row * CellSize;
        gridLines.append(sf::Vertex({0.0f, y}, sf::Color(80, 123, 160)));
        gridLines.append(sf::Vertex({MapWidth * CellSize, y}, sf::Color(80, 123, 160)));
    }
    target.draw(gridLines);
}

void MapEditorScene::selectCategory(Category category) {
    _activeCategory = category;
    setupPaletteMenu();
    _paletteTitleText.setString(
        std::string("PALETTE: ") + categoryName(category)
    );
    setStatus(
        std::string("Choose a ") + categoryName(category) + " item"
    );
}

void MapEditorScene::selectSymbol(char symbol) {
    _selectedSymbol = symbol;
    const PaletteEntry* entry = findEntry(symbol);
    if (entry == nullptr) {
        _selectedText.setString("Selected: Eraser (.)");
        return;
    }

    _selectedText.setString(
        "Selected: " + entry->label + " (" + std::string(1, entry->symbol) + ")"
    );
    setStatus("Selected " + entry->label);
}

void MapEditorScene::eraseCell(int column, int row) {
    applyPaintCell(column, row, '.');
}

void MapEditorScene::placeCell(int column, int row) {
    applyPaintCell(column, row, _selectedSymbol);
}

void MapEditorScene::applyPaintCell(int column, int row, char symbol) {
    if (column < 0 || column >= MapWidth || row < 0 || row >= MapHeight) {
        return;
    }

    const std::size_t index = static_cast<std::size_t>(row * MapWidth + column);
    if (_cells[index] == symbol) {
        return;
    }

    if (!_strokeUndoCaptured) {
        rememberBeforeEdit();
        _strokeUndoCaptured = true;
    }
    _cells[index] = symbol;
    _dirty = true;
}

void MapEditorScene::applyPaintRectangle(
    int startColumn,
    int startRow,
    int endColumn,
    int endRow,
    char symbol
) {
    const int left = std::min(startColumn, endColumn);
    const int top = std::min(startRow, endRow);
    const int right = std::max(startColumn, endColumn);
    const int bottom = std::max(startRow, endRow);

    for (int row = top; row <= bottom; ++row) {
        for (int column = left; column <= right; ++column) {
            applyPaintCell(column, row, symbol);
        }
    }
}

void MapEditorScene::clearMap() {
    if (std::all_of(_cells.begin(), _cells.end(), [](char symbol) {
            return symbol == '.';
        })) {
        setStatus("Map is already empty");
        return;
    }

    rememberBeforeEdit();
    std::fill(_cells.begin(), _cells.end(), '.');
    _dirty = true;
    setStatus("Map cleared");
}

void MapEditorScene::undoLastEdit() {
    if (_undoHistory.empty()) {
        setStatus("Nothing to undo", sf::Color(255, 205, 120));
        return;
    }

    _redoHistory.push_back(_cells);
    _cells = std::move(_undoHistory.back());
    _undoHistory.pop_back();
    _strokeUndoCaptured = false;
    _dirty = true;
    setStatus("Last edit undone");
}

void MapEditorScene::redoLastEdit() {
    if (_redoHistory.empty()) {
        setStatus("Nothing to redo", sf::Color(255, 205, 120));
        return;
    }

    _undoHistory.push_back(_cells);
    _cells = std::move(_redoHistory.back());
    _redoHistory.pop_back();
    _strokeUndoCaptured = false;
    _dirty = true;
    setStatus("Last edit redone");
}

void MapEditorScene::rememberBeforeEdit() {
    _undoHistory.push_back(_cells);
    _redoHistory.clear();
}

bool MapEditorScene::loadSavedMap() {
    const std::filesystem::path path(savedMapPath());
    if (!std::filesystem::exists(path)) {
        return false;
    }

    try {
        const LevelData levelData = LevelDataLoader::load(
            path,
            MapWidth,
            MapHeight
        );
        std::fill(_cells.begin(), _cells.end(), '.');
        const std::size_t rowCount = std::min(
            static_cast<std::size_t>(MapHeight),
            levelData.layer.size()
        );
        for (std::size_t row = 0; row < rowCount; ++row) {
            const std::size_t copyWidth = std::min(
                static_cast<std::size_t>(MapWidth),
                levelData.layer[row].size()
            );
            std::copy_n(
                levelData.layer[row].begin(),
                copyWidth,
                _cells.begin() + row * MapWidth
            );
        }
        _undoHistory.clear();
        _redoHistory.clear();
        _strokeUndoCaptured = false;
        applyLoadedTheme(
            levelData.theme,
            levelData.background,
            levelData.music
        );
        _dirty = false;
        setStatus("Loaded custom-map.json", sf::Color(160, 255, 175));
    } catch (const std::exception& error) {
        setStatus(
            std::string("Could not load saved map: ") + error.what(),
            sf::Color(255, 180, 120)
        );
    }
    return true;
}

bool MapEditorScene::saveMap() {
    json document;
    json tileMapping = {
        {".", "empty"}
    };
    for (const PaletteEntry& entry : _paletteEntries) {
        tileMapping[std::string(1, entry.symbol)] = entry.prefabId;
    }

    std::vector<std::string> layer;
    layer.reserve(MapHeight);
    for (int row = 0; row < MapHeight; ++row) {
        layer.emplace_back(
            _cells.begin() + row * MapWidth,
            _cells.begin() + (row + 1) * MapWidth
        );
    }

    document["tileMapping"] = std::move(tileMapping);
    document["layer"] = std::move(layer);
    document["theme"] = _themeKey;
    document["editor"] = {
        {"width", MapWidth},
        {"height", MapHeight},
        {"description", "Map created in the in-game map builder"}
    };
    document["prefabs"] = {
        {
            "terrain_grassland",
            {
                {"texture", "at_grassland"},
                {"solid", true},
                {"autotile", "grassland_terrain"},
                {"addSeamFilter", true}
            }
        },
        {
            "pipe_basic",
            {
                {"kind", "pipe"},
                {"typeKey", "Pipe"},
                {"texture", "pipes_spritesheet"},
                {"size", {128, 192}},
                {"pipeOrientation", "vertical"},
                {"pipeEndSide", "top"},
                {"pipeBodyLength", 2},
                {"pipeIsWarp", false},
                {"addSeamFilter", true}
            }
        }
    };

    try {
        const std::filesystem::path path(savedMapPath());
        if (path.has_parent_path()) {
            std::filesystem::create_directories(path.parent_path());
        }

        std::ofstream output(path);
        if (!output) {
            setStatus("Could not open custom-map.json", sf::Color(255, 130, 130));
            return false;
        }
        output << document.dump(4) << '\n';
        if (!output) {
            setStatus("Could not write custom-map.json", sf::Color(255, 130, 130));
            return false;
        }
    } catch (const std::exception& error) {
        setStatus(
            std::string("Save failed: ") + error.what(),
            sf::Color(255, 130, 130)
        );
        return false;
    }

    _dirty = false;
    setStatus("Saved custom-map.json", sf::Color(160, 255, 175));
    return true;
}

void MapEditorScene::saveAndPlay() {
    if (!saveMap()) {
        return;
    }

    if (auto* manager = getSceneManager()) {
        manager->pushScene(
            std::make_unique<InGameScene>(savedMapPath())
        );
    }
}

void MapEditorScene::setStatus(
    const std::string& status,
    const sf::Color& color
) {
    _statusText.setString(status);
    _statusText.setFillColor(color);
}

const MapEditorScene::PaletteEntry* MapEditorScene::findEntry(char symbol) const {
    const auto it = std::find_if(
        _paletteEntries.begin(),
        _paletteEntries.end(),
        [symbol](const PaletteEntry& entry) {
            return entry.symbol == symbol;
        }
    );
    return it == _paletteEntries.end() ? nullptr : &*it;
}

const char* MapEditorScene::categoryName(Category category) {
    switch (category) {
        case Category::Blocks:
            return "Blocks";
        case Category::Items:
            return "Items";
        case Category::Enemies:
            return "Enemies";
        case Category::Players:
            return "Players";
    }
    return "Palette";
}
