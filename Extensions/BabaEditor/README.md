# BabaEditor

Experimental C++17 + ImGui level editor for `baba-is-auto`.

## Build

Set `VCPKG_ROOT` to a vcpkg checkout, or provide the toolchain path directly.

```sh
cmake -S . -B build -DBABA_BUILD_EDITOR=ON \
  -DCMAKE_TOOLCHAIN_FILE="$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
cmake --build build --target BabaEditor --config Release
```

The editor target uses GLFW, OpenGL, and ImGui from vcpkg. The main project keeps `BABA_BUILD_EDITOR` off by default so normal simulator and Python builds do not open a desktop dependency path unless requested.

## Scope

- `L1` editing, with disabled `L2`/`L3` placeholders for future layered-map support.
- Object palette filtered to the text and object sprites currently supported by the repository's GUI assets and sample maps.
- Palette and canvas tiles render the matching GIF sprites instead of plain text labels when the asset exists.
- Missing or failed sprite loads are shown as a small red X instead of falling back to text, and the status bar reports the sprite load count.
- Freeform, line, rectangle, filled rectangle, flood fill, and eraser placement.
- Right-click tile picker, grid toggle, undo, and cross-platform in-app Open/Save/Save As dialogs or a manually typed path.
- Open/save compatibility with the existing `baba-is-auto` map format: first line is `width height`, followed by one numeric `ObjectType` per tile.

Because the current simulator map format stores one object per saved tile, the editor opens and saves maps on `L1` only.
