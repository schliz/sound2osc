# [1.1.0](https://github.com/schliz/sound2osc/compare/v1.0.1...v1.1.0) (2026-01-19)


### Features

* **build:** x-plattform builds and releases ([1b8e7e0](https://github.com/schliz/sound2osc/commit/1b8e7e0152feba65fc710b5042e72823576aef84))

## [1.0.1](https://github.com/schliz/sound2osc/compare/v1.0.0...v1.0.1) (2026-01-19)


### Bug Fixes

* migrate dialog logic to qt6 ([53746b7](https://github.com/schliz/sound2osc/commit/53746b71f35fc0715c1b7c78bb0306e00d99766f))
* sign-conversion and old-style cast warnings ([4a623c9](https://github.com/schliz/sound2osc/commit/4a623c9f6aee5ef0eddb2cf9ddb71bb79ff9bd5b))

# 1.0.0 (2026-01-18)


* chore!: Modern CMake/Qt6 build system with restructured codebase ([2d63202](https://github.com/schliz/sound2osc/commit/2d63202052d2b1d4c341d41fe2343223414f1d21))


### Bug Fixes

* **build:** missing refactoring for qt6 ([555759d](https://github.com/schliz/sound2osc/commit/555759d20d17bcf4f2f3114beea51e0189a05bb9))


### BREAKING CHANGES

* new overall repository structure
- libs/sound2osc-core/ - UI-independent core library
- apps/gui/ - Qt6 QML GUI application
- third_party/ffft/ - Header-only FFT library
- cmake/ - CMake helper modules
- docs/ - Documentation, manuals, examples
* switched to modern build system
- CMake 3.16+ with Qt6 integration
- C++17 standard enabled
- Automatic MOC/UIC/RCC handling
- Platform-specific configurations (Windows GUI, macOS bundle)
- compile_commands.json for IDE support
