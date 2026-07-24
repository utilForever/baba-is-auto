<p align="center">
  <picture>
    <img src="Medias/Logos/Logo.svg" width="400" alt="baba-is-auto logo" />
  </picture>
</p>
<p align="center">
  <b>A C++17 Baba Is You simulator with Python bindings, GUI, and reinforcement-learning examples</b>
</p>
<p align="center">
  <a href="LICENSE"><img src="https://img.shields.io/badge/License-MIT-yellow.svg" alt="License: MIT" /></a>
  <a href="https://github.com/utilForever/baba-is-auto/actions/workflows/windows.yml"><img src="https://github.com/utilForever/baba-is-auto/actions/workflows/windows.yml/badge.svg?branch=main" alt="Windows" /></a>
  <a href="https://github.com/utilForever/baba-is-auto/actions/workflows/ubuntu.yml"><img src="https://github.com/utilForever/baba-is-auto/actions/workflows/ubuntu.yml/badge.svg?branch=main" alt="Ubuntu" /></a>
  <a href="https://github.com/utilForever/baba-is-auto/actions/workflows/macos.yml"><img src="https://github.com/utilForever/baba-is-auto/actions/workflows/macos.yml/badge.svg?branch=main" alt="macOS" /></a>
  <br />
  <a href="https://github.com/utilForever/baba-is-auto/actions/workflows/ubuntu-codecov.yml"><img src="https://github.com/utilForever/baba-is-auto/actions/workflows/ubuntu-codecov.yml/badge.svg?branch=main" alt="Code Coverage" /></a>
  <a href="https://codecov.io/gh/utilForever/baba-is-auto"><img src="https://codecov.io/gh/utilForever/baba-is-auto/branch/main/graph/badge.svg" alt="Codecov" /></a>
  <a href="https://github.com/utilForever/baba-is-auto/actions/workflows/ubuntu-sonarcloud.yml"><img src="https://github.com/utilForever/baba-is-auto/actions/workflows/ubuntu-sonarcloud.yml/badge.svg?branch=main" alt="Static Analysis" /></a>
  <a href="https://sonarcloud.io/summary/new_code?id=utilForever_baba-is-auto"><img src="https://sonarcloud.io/api/project_badges/measure?project=utilForever_baba-is-auto&metric=alert_status" alt="Quality Gate Status" /></a>
  <a href="https://sonarcloud.io/summary/new_code?id=utilForever_baba-is-auto"><img src="https://sonarcloud.io/api/project_badges/measure?project=utilForever_baba-is-auto&metric=ncloc" alt="Lines of Code" /></a>
  <br />
  <a href="https://sonarcloud.io/summary/new_code?id=utilForever_baba-is-auto"><img src="https://sonarcloud.io/api/project_badges/measure?project=utilForever_baba-is-auto&metric=sqale_rating" alt="Maintainability Rating" /></a>
  <a href="https://sonarcloud.io/summary/new_code?id=utilForever_baba-is-auto"><img src="https://sonarcloud.io/api/project_badges/measure?project=utilForever_baba-is-auto&metric=reliability_rating" alt="Reliability Rating" /></a>
  <a href="https://sonarcloud.io/summary/new_code?id=utilForever_baba-is-auto"><img src="https://sonarcloud.io/api/project_badges/measure?project=utilForever_baba-is-auto&metric=security_rating" alt="Security Rating" /></a>
  <a href="https://sonarcloud.io/summary/new_code?id=utilForever_baba-is-auto"><img src="https://sonarcloud.io/api/project_badges/measure?project=utilForever_baba-is-auto&metric=bugs" alt="Bugs" /></a>
  <a href="https://sonarcloud.io/summary/new_code?id=utilForever_baba-is-auto"><img src="https://sonarcloud.io/api/project_badges/measure?project=utilForever_baba-is-auto&metric=vulnerabilities" alt="Vulnerabilities" /></a>
  <a href="https://sonarcloud.io/summary/new_code?id=utilForever_baba-is-auto"><img src="https://sonarcloud.io/api/project_badges/measure?project=utilForever_baba-is-auto&metric=sqale_index" alt="Technical Debt" /></a>
</p>

## What This Project Does

baba-is-auto simulates a subset of [Baba Is You](https://hempuli.com/baba/): it loads a text map, parses rules formed by word tiles, applies movement and object interactions, and exposes the resulting state to C++ and Python callers.

The repository provides:

- A reusable C++17 simulator library.
- A `pyBaba` Python extension built with pybind11.
- A pygame GUI that consumes the Python API.
- Gym-style reinforcement-learning environments with DQN and REINFORCE examples.
- An optional C++ ImGui [level editor](Extensions/BabaEditor/README.md).

For the simulator design and behavior contract, see [ARCHITECTURE.md](ARCHITECTURE.md). Contributors and coding agents should also read [AGENTS.md](AGENTS.md).

## What Is Baba Is You?

[Baba Is You](https://hempuli.com/baba/) is an award-winning puzzle game in which the rules are physical blocks. Moving those blocks can change the player, obstacles, and win condition while a level is in progress.

## Quick Start

### Prerequisites

- CMake 3.31.6 or later
- vcpkg
- A C++17 compiler
- Python 3 with development headers
- Git

Set `VCPKG_ROOT` to your vcpkg checkout, or pass its toolchain file through `CMAKE_TOOLCHAIN_FILE`.

### 1. Clone

```bash
git clone https://github.com/utilForever/baba-is-auto.git
cd baba-is-auto
```

### 2. Build

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

The root CMake project builds the `baba-is-auto` library, `pyBaba` extension, and `UnitTests` executable.

### 3. Run the C++ Tests

macOS and Linux:

```bash
./build/bin/UnitTests
```

Windows release builds:

```powershell
.\build\bin\Release\UnitTests.exe
```

### Python API

Build and install `pyBaba` with:

```bash
python -m pip install .
```

### Level Editor

Enable the optional editor feature and build its target:

```bash
cmake -S . -B build-editor -DBABA_BUILD_EDITOR=ON
cmake --build build-editor --target BabaEditor --config Release
```

## Reinforcement-Learning Examples

Each bundled environment uses the actions `UP`, `DOWN`, `LEFT`, and `RIGHT`.

| Environment          | Level                                                              | Algorithms                                                                                                         |
| -------------------- | ------------------------------------------------------------------ | ------------------------------------------------------------------------------------------------------------------ |
| `baba-babaisyou-v0`  | [Environment](Extensions/BabaRL/baba-babaisyou-v0/environment.py)  | [DQN](Extensions/BabaRL/baba-babaisyou-v0/DQN.py), [REINFORCE](Extensions/BabaRL/baba-babaisyou-v0/REINFORCE.py)   |
| `baba-outofreach-v0` | [Environment](Extensions/BabaRL/baba-outofreach-v0/environment.py) | [DQN](Extensions/BabaRL/baba-outofreach-v0/DQN.py), [REINFORCE](Extensions/BabaRL/baba-outofreach-v0/REINFORCE.py) |
| `baba-volcano-v0`    | [Environment](Extensions/BabaRL/baba-volcano-v0/environment.py)    | [DQN](Extensions/BabaRL/baba-volcano-v0/DQN.py), [REINFORCE](Extensions/BabaRL/baba-volcano-v0/REINFORCE.py)       |

## Architecture at a Glance

| Area                    | Paths                                    |
| ----------------------- | ---------------------------------------- |
| Core C++ API            | `Includes/baba-is-auto/`                 |
| Core C++ implementation | `Sources/baba-is-auto/`                  |
| Python binding          | `Extensions/BabaPython/`                 |
| GUI simulator           | `Extensions/BabaGUI/`                    |
| Level editor            | `Extensions/BabaEditor/`                 |
| RL examples             | `Extensions/BabaRL/`                     |
| C++ and Python tests    | `Tests/UnitTests/`, `Tests/PythonTests/` |
| Map fixtures            | `Resources/Maps/`                        |

The C++ core is the source of simulator behavior, and `Extensions/BabaPython/` mirrors its public surface. Changes to games, maps, rules, enums, objects, or agents should keep both layers in sync. The aggregate header `Includes/baba-is-auto.hpp` is generated during the CMake build; edit the individual headers or generator instead.

## Development

Run the Python checks used by CI after building the extension in place:

```bash
python -m pip install --require-hashes -r requirements.txt
python setup.py build_ext --inplace
python -m pytest Tests/PythonTests/
```

GitHub Actions builds and tests the project on Ubuntu, macOS, and Windows. See [AGENTS.md](AGENTS.md) for the complete validation matrix and repository rules.

## Contributing

Issues and pull requests are welcome. Keep changes focused, add the narrowest test that covers behavior changes, and follow [AGENTS.md](AGENTS.md).

## Acknowledgement

Thank you to [Arvi "Hempuli" Teikari](https://hempuli.com/) for allowing this project to be developed. Baba Is You and its original game concepts belong to Hempuli.

## License

This project is licensed under the [MIT License](LICENSE).

Copyright &copy; 2020-2026 [Chris Ohk](https://github.com/utilForever).
