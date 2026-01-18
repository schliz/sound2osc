# AI Agent Instructions

> **Project Status: Under Renovation**
> 
> This project is undergoing a major modernization effort. See [MODERNIZATION_PLAN.md](./MODERNIZATION_PLAN.md) for the full plan and current status.

## Interaction Protocol

When you need the user to perform a manual action (install packages, run commands requiring sudo, etc.):

1. Use the **question tool** to prompt the user
2. The user will select "Type your own answer" to provide the result
3. This allows for interactive feedback during tasks

Example: If you need `sudo apt install some-package`, ask via the question tool and the user will confirm completion or report errors.

## Current Phase

**Phase 1: Repository Cleanup & Build System**

We are migrating from Qt5/qmake to Qt6/CMake with a restructured codebase.

## Key Context

- **Build System:** Migrating from qmake to CMake
- **Qt Version:** Migrating from Qt5 to Qt6
- **C++ Standard:** Upgrading from C++11 to C++17
- **Architecture:** Separating core library from GUI frontend

## File Locations (After Restructure)

| Component | Location |
|-----------|----------|
| Core library | `libs/sound2osc-core/` |
| GUI application | `apps/gui/` |
| Third-party deps | `third_party/` |
| CMake modules | `cmake/` |
| Documentation | `docs/` |

## Important Notes

1. The `src/` directory contains the **legacy** code structure
2. New code should follow the structure in `MODERNIZATION_PLAN.md`
3. We are targeting Qt6 - do not use deprecated Qt5 APIs
4. Use C++17 features where appropriate
5. Core library should be UI-independent (no Qt GUI/QML dependencies)

## Building

```bash
# Configure
cmake -B build -G Ninja

# Build
cmake --build build

# Run
./build/apps/gui/sound2osc
```
