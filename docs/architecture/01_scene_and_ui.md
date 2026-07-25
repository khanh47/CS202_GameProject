# 01: Scene Management & UI Subsystem

This document covers the **Scene Management Architecture** and the **UI Component System**.

---

## Subsystem Overview

The scene management system handles screen states (e.g., Main Menu, Gameplay, Settings) and transitions between them. It employs the **State Pattern** combined with a **Scene Stack** (`std::stack`) to support overlaid scenes (such as pause menus).

The UI subsystem provides accessible, keyboard/mouse navigable menu components (`UI::Button` and `UI::ButtonMenu`) bound to execution actions using the **Command Pattern**.

---

## Class Diagram

```mermaid
classDiagram
    class Scene {
        <<abstract>>
        #bool _isActive
        #SceneManager* _sceneManager
        -string _name
        +init()*
        +onEnter()*
        +onExit()*
        +cleanup()*
        +handleInput(event)*
        +updateSimulation(fixedDt)
        +updateVisuals(deltaTime)*
        +render(target)*
        +getName() string*
        +isActive() bool*
        +setSceneManager(manager)
        +getSceneManager() SceneManager*
    }

    class SceneManager {
        -stack~unique_ptr~Scene~~ _sceneStack
        -queue~function~ _deferredActions
        -SceneFactory* _factory
        -sf::RenderWindow* _window
        +pushScene(scene)
        +popScene()
        +replaceScene(scene)
        +requestPopScene()
        +pushSceneByName(sceneName)
        +processEvents(event)
        +updateSimulation(fixedDt)
        +updateVisuals(deltaTime)
        +render(target)
        +getCurrentScene() Scene*
        +getSceneName() string
        +isEmpty() bool
        -requestDeferredAction(action)
        -processDeferredActions()
    }

    class SceneFactory {
        -unordered_map~string, function~ _factories
        +registerScene(stateName, factory)
        +createScene(stateName) unique_ptr~Scene~
    }

    class MainMenuScene {
        -UI::ButtonMenu _menu
        +init()
        +onEnter()
        +onExit()
        +cleanup()
        +handleInput(event)
        +updateVisuals(deltaTime)
        +render(target)
    }

    class SettingsScene {
        -UI::ButtonMenu _menu
        +init()
        +onEnter()
        +onExit()
        +cleanup()
        +handleInput(event)
        +updateVisuals(deltaTime)
        +render(target)
    }

    class InGameScene {
        -GameWorld _gameWorld
        +init()
        +onEnter()
        +onExit()
        +cleanup()
        +handleInput(event)
        +updateSimulation(fixedDt)
        +updateVisuals(deltaTime)
        +render(target)
    }

    class Button {
        -sf::ConvexShape shape
        -sf::Text label
        -optional~sf::Sprite~ icon
        -unique_ptr~ICommand~ buttonCommand
        -bool _isHovered
        -bool _isFocused
        +setCommand(command)
        +execute()
        +processEvent(event)
        +render(target)
        +setFocused(focused)
        +isHovered() bool
    }

    class ButtonMenu {
        -vector~shared_ptr~Button~~ _buttonMenu
        -LayoutProperties _layout
        -int _focusedIndex
        +setLayoutProperties(...)
        +addButton(button)
        +addButtonAuto(text, command, iconAlias)
        +processEvent(event)
        +render(target)
        +setFocusedIndex(index)
        +getFocusedIndex() int
        -syncFocus()
    }

    class ICommand {
        <<interface>>
        +execute()*
        +getName() string*
        +getType() CommandType*
    }

    SceneManager "1" *-- "many" Scene : owns stack
    SceneManager o-- SceneFactory : uses
    Scene <|-- MainMenuScene
    Scene <|-- SettingsScene
    Scene <|-- InGameScene
    MainMenuScene *-- ButtonMenu : contains
    SettingsScene *-- ButtonMenu : contains
    ButtonMenu "1" *-- "many" Button : manages layout
    Button *-- ICommand : executes
```

---

## Key Classes & Responsibilities

| Class | Type | Responsibility |
| :--- | :--- | :--- |
| `Scene` | Abstract Base | Defines lifecycle (`init`, `onEnter`, `onExit`, `cleanup`) and frame loop methods (`handleInput`, `updateSimulation`, `updateVisuals`, `render`). |
| `SceneManager` | Stack Owner & Orchestrator | Manages the active scene stack, forwards event/update/render calls to the top scene, and executes deferred scene switching safely. |
| `SceneFactory` | Registry / Factory | Decouples scene switching logic from concrete scene classes using factory lambdas indexed by string keys. |
| `MainMenuScene` | Concrete Scene | Controls the main menu UI, title display, and navigation commands. |
| `InGameScene` | Concrete Scene | Holds the `GameWorld` instance, runs gameplay simulation, and handles in-game pause/HUD overlay logic. |
| `SettingsScene` | Concrete Scene | Provides audio, debug toggle, and game setting controls via `ButtonMenu`. |
| `UI::Button` | UI Component | Individual rounded button with hover/focus states, text label, optional icon sprite, and attached `ICommand`. |
| `UI::ButtonMenu` | UI Layout Manager | Manages a list of `UI::Button` instances, handling vertical/horizontal spatial navigation and keyboard focus sync (`Up`/`Down`/`Enter`). |

