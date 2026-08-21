#pragma once

#include <SFML/Graphics.hpp>
#include <cstddef>
#include <optional>
#include <string>
#include <vector>

#include "Scene/Scene.h"
#include "Button/ButtonMenu.h"

class MapEditorScene : public Scene {
public:
    MapEditorScene();
    ~MapEditorScene() override = default;

    static constexpr const char* savedMapPath() noexcept {
        return "assets/datas/levels/custom-map.json";
    }

    void init() override;
    void onEnter() override;
    void onExit() override;
    void handleInput(const sf::Event& event) override;
    void updateSimulation(const float& fixedDt) override;
    void updateVisuals(float deltaTime) override;
    void render(sf::RenderTarget& target) override;

public:
    enum class Category {
        Blocks,
        Items,
        Enemies,
        Players
    };

private:
    struct PaletteEntry {
        Category category;
        std::string label;
        std::string prefabId;
        char symbol;
        sf::Color previewColor;
    };

    struct ThemeChoice {
        std::string key;
        std::string label;
        std::string background;
        std::string music;
    };

    static constexpr int MapWidth = 80;
    static constexpr int MapHeight = 40;
    static constexpr float CellSize = 64.0f;
    static constexpr float LogicalScreenWidth = 1920.0f;
    static constexpr float LogicalScreenHeight = 1080.0f;
    static constexpr float MapViewportWidth = 1584.0f;
    static constexpr float MapViewportHeight = LogicalScreenHeight;
    static constexpr float MinimumZoom = 0.20f;
    static constexpr float MaximumZoom = 3.0f;
    static constexpr float CameraPanMargin = CellSize * 8.0f;
    static constexpr float PaletteLeft = 1605.0f;

    void setupMenus();
    void setupCategoryMenu();
    void setupPaletteMenu();
    void setupThemeMenu();
    void setupActionMenu();
    void setupInstructionsMenu();
    void selectCategory(Category category);
    void selectSymbol(char symbol);
    void cycleTheme();
    void refreshThemeButton();
    void applyLoadedTheme(
        const std::string& theme,
        const std::string& background,
        const std::string& music
    );
    void handleMouseMoved(const sf::Event::MouseMoved& mouseEvent);
    void handleMousePressed(const sf::Event::MouseButtonPressed& mouseEvent);
    void handleMouseReleased(const sf::Event::MouseButtonReleased& mouseEvent);
    void updateHover(sf::Vector2i screenPosition);
    void beginPaint(sf::Mouse::Button button, sf::Vector2i screenPosition);
    void continuePaint(sf::Vector2i screenPosition);
    void endPaint(sf::Mouse::Button button);
    void panMap(sf::Vector2i screenPosition);
    void zoomMap(float wheelDelta, sf::Vector2i screenPosition);
    void clampMapView();
    sf::Vector2f mapScreenToWorld(sf::Vector2f screenPosition) const;
    std::optional<sf::Vector2i> mapCellAtScreen(sf::Vector2i screenPosition) const;
    bool isMapCanvasPosition(sf::Vector2i screenPosition) const;
    bool isShiftHeld() const;
    void drawMap(sf::RenderTarget& target);
    void drawMapGrid(sf::RenderTarget& target);
    void eraseCell(int column, int row);
    void placeCell(int column, int row);
    void applyPaintCell(int column, int row, char symbol);
    void applyPaintRectangle(int startColumn, int startRow, int endColumn, int endRow, char symbol);
    void clearMap();
    void undoLastEdit();
    void redoLastEdit();
    void rememberBeforeEdit();
    bool loadSavedMap();
    bool saveMap();
    void saveAndPlay();
    void setStatus(const std::string& status, const sf::Color& color = sf::Color::White);
    const PaletteEntry* findEntry(char symbol) const;
    static const char* categoryName(Category category);

    std::vector<PaletteEntry> _paletteEntries;
    std::vector<char> _cells;
    std::vector<std::vector<char>> _undoHistory;
    std::vector<std::vector<char>> _redoHistory;
    sf::View _mapView;
    std::vector<ThemeChoice> _themeOptions;
    std::size_t _themeIndex = 0;
    std::string _themeKey = "sky";
    Category _activeCategory = Category::Blocks;
    char _selectedSymbol = '#';
    float _zoom = 1.0f;
    int _hoverColumn = -1;
    int _hoverRow = -1;
    int _dragStartColumn = -1;
    int _dragStartRow = -1;
    sf::Vector2i _lastMousePosition{};
    sf::Mouse::Button _paintButton = sf::Mouse::Button::Left;
    bool _leftMouseHeld = false;
    bool _rightMouseHeld = false;
    bool _middleMouseHeld = false;
    bool _paintActive = false;
    bool _rectangleDrag = false;
    bool _strokeUndoCaptured = false;
    bool _dirty = false;
    bool _showInstructions = false;

    UI::ButtonMenu _categoryMenu;
    UI::ButtonMenu _paletteMenu;
    UI::ButtonMenu _themeMenu;
    UI::ButtonMenu _actionMenu;
    UI::ButtonMenu _instructionsMenu;

    sf::RectangleShape _screenBackdrop;
    sf::RectangleShape _gridBackdrop;
    sf::RectangleShape _paletteBackdrop;
    sf::Text _titleText;
    sf::Text _paletteTitleText;
    sf::Text _selectedText;
    sf::Text _statusText;
    sf::RectangleShape _instructionsBackdrop;
    sf::RectangleShape _instructionsPanel;
    sf::Text _instructionsTitle;
    sf::Text _instructionsBody;
};
