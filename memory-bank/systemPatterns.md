# System Patterns

This file documents recurring patterns and standards used in the project.
It is optional, but recommended to be updated as the project evolves.

## Coding Patterns

* Object-Oriented Design - Character hierarchy with Player and Enemy classes
* Component-Based Architecture - Separate systems for rendering, input, AI, and game logic
* Header/Implementation Separation - Standard C++ .h/.cpp file organization
* CMake Build System - Modern C++ project structure with custom module finders

## Architectural Patterns

* Game Loop Pattern - Central game state management and update cycles
* Factory Pattern - Potential use for character and projectile creation
* Observer Pattern - Likely event handling between game components
* State Pattern - Game state management and transitions

## Testing Patterns

* To be determined - Analysis needed to identify current testing approach
* Unit testing framework integration opportunities
* Performance testing considerations for game loop optimization

2025-05-31 12:52:26 - Initial system patterns documented from file structure analysis.
## Analyzed Architecture Patterns

2025-05-31 12:53:18 - Completed comprehensive source code analysis

### Object-Oriented Hierarchy
* **Character Base Class**: Abstract base with virtual Move() method
* **Player Class**: Inherits from Character, implements player-specific movement
* **Enemy Class**: Inherits from Character, integrates with AICentral for AI behavior
* **Projectile Class**: Inherits from Character but Move() not implemented (placeholder)

### Core Game Architecture
* **Game Class**: Central orchestrator managing player, enemy, and game loop
* **GameMap Class**: 2D grid-based world representation with collision detection
* **AICentral Class**: Singleton-like AI system with map awareness (20x32 grid)
* **Renderer Class**: SDL3-based graphics engine with object type enumeration
* **Controller Class**: Input handling system

### Modern C++ Features Used
* Smart pointers (std::shared_ptr) for memory management
* Async/Future patterns for enemy movement threading
* Enum classes for type safety (Direction, MapObject, ObjectType)
* Move constructors/assignment operators properly deleted in AICentral
* Resource Acquisition Is Initialization (RAII) pattern

### Game Loop Pattern Implementation
* Fixed timestep at 60 FPS with frame duration targeting
* Input → Update → Render cycle
* Asynchronous enemy AI processing
* Frame rate compensation with SDL_DelayNS()