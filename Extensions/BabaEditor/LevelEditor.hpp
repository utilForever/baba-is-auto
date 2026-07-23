//! Copyright (c) 2020-2026 Chris Ohk

//! I am making my contributions/submissions to this project solely in our
//! personal capacity and am not conveying any rights to any intellectual
//! property of any third parties.

// Members are used by the out-of-line LevelEditor implementation.
// cppcheck-suppress-file unusedStructMember
#ifndef BABA_EDITOR_LEVEL_EDITOR_HPP
#define BABA_EDITOR_LEVEL_EDITOR_HPP

#include <baba-is-auto/Enums/GameEnums.hpp>

#include <imgui.h>

#include <array>
#include <cstddef>
#include <filesystem>
#include <map>
#include <string>
#include <vector>

namespace baba_is_auto::editor
{
//! Editor brush modes for painting tiles on the canvas.
enum class PlacementMode
{
    Freeform,
    Line,
    Rectangle,
    FilledRectangle,
    FloodFill,
    Eraser
};

//! A single entry in the object palette, including its type, name, group, and
//! color.
struct ObjectEntry
{
    ObjectType type;
    const char* name;
    const char* group;
    ImVec4 color;
};

//! Owns the editable map state, editor windows, and OpenGL sprite textures.
//! An active OpenGL context must outlive this object.
class LevelEditor
{
 public:
    // Construct a new Level Editor object and load all sprite textures from
    // disk.
    LevelEditor();
    ~LevelEditor();

    // Draw the editor UI and handle user input. This should be called once per
    // frame while the editor's OpenGL context is active.
    void Draw();

    // Route native window close requests through the unsaved-change prompt.
    void RequestClose();
    bool ShouldClose() const;

 private:
    //! Pending actions that require user confirmation before execution.
    enum class PendingAction
    {
        None,
        NewLevel,
        OpenLevel,
        ResizeLevel,
        Close
    };

    // L2/L3 remain reserved for layered-map support; current files use L1 only.
    static constexpr std::size_t LAYER_COUNT = 3;
    static constexpr ObjectType EMPTY = ObjectType::ICON_EMPTY;

    using LayerTile = std::array<ObjectType, LAYER_COUNT>;
    using LevelTiles = std::vector<LayerTile>;

    // Level lifecycle and current numeric map-format I/O.
    void NewLevel(std::size_t width, std::size_t height);
    void ResizeLevel(std::size_t width, std::size_t height);
    bool ImportLevel(const std::filesystem::path& filename);
    bool ExportLevel(const std::filesystem::path& filename) const;
    void OpenFileDialog(bool save);
    void OpenLevelWithDialog();
    void SaveLevel();
    void SaveLevelAs();
    void RequestAction(PendingAction action);
    void ExecutePendingAction();

    // Top-level ImGui windows rendered once per frame.
    void DrawTopBar();
    void DrawObjectPalette();
    void DrawSettings();
    void DrawCanvas();
    void DrawFileDialog();
    void DrawActionConfirmation();
    void DrawStatusBar();

    // Sprite-backed palette, tile rendering, and paint interactions.
    void DrawSpriteInRect(ObjectType type, const ImVec2& min,
                          const ImVec2& max) const;
    bool DrawSpriteButton(ObjectType type, const ImVec2& size);
    void DrawTileButton(std::size_t x, std::size_t y, float tileSize);
    void HandleTileInteraction(std::size_t x, std::size_t y);
    void ApplyBrush(std::size_t x, std::size_t y);
    void ApplyLine(std::size_t x0, std::size_t y0, std::size_t x1,
                   std::size_t y1);
    void ApplyRectangle(std::size_t x0, std::size_t y0, std::size_t x1,
                        std::size_t y1, bool filled);
    void ApplyFloodFill(std::size_t x, std::size_t y);
    void SetTile(std::size_t x, std::size_t y, ObjectType type);

    // Tile accessors for the current layer and visible tile (topmost
    // non-empty).
    ObjectType VisibleTile(std::size_t x, std::size_t y) const;
    ObjectType CurrentLayerTile(std::size_t x, std::size_t y) const;
    LayerTile& Tile(std::size_t x, std::size_t y);
    const LayerTile& Tile(std::size_t x, std::size_t y) const;

    // Undo stack management.
    void PushUndo();
    void Undo();

    // Palette and sprite texture management.
    const ObjectEntry& EntryFor(ObjectType type) const;
    const char* NameFor(ObjectType type) const;

    //! Texture IDs are valid only while the editor's OpenGL context is active.
    struct SpriteTexture
    {
        unsigned int id = 0;
        int width = 0;
        int height = 0;
    };

    // Load all sprite textures from disk into OpenGL and store their IDs.
    void LoadTextures();
    bool LoadTexture(ObjectType type, const std::string& filename);
    void ReleaseTextures();
    const SpriteTexture* TextureFor(ObjectType type) const;
    std::string SpritePath(const ObjectEntry& entry) const;

    // Level dimensions and tile data.
    std::size_t m_width = 22;
    std::size_t m_height = 14;
    int m_nextWidth = 22;
    int m_nextHeight = 14;
    LevelTiles m_tiles;
    std::vector<LevelTiles> m_undoStack;

    // File paths and status messages.
    std::string m_levelPath;
    std::string m_status = "Ready";
    std::string m_textureStatus;
    std::string m_search;
    std::string m_dialogDirectory;
    std::string m_dialogFilename;
    std::map<ObjectType, SpriteTexture> m_textures;

    // Editor state variables.
    ObjectType m_selected = ObjectType::ICON_BABA;
    PlacementMode m_mode = PlacementMode::Freeform;
    std::size_t m_currentLayer = 0;
    bool m_showGrid = true;
    bool m_showSettings = true;
    bool m_showObjects = true;
    bool m_showFileDialog = false;
    bool m_saveDialog = false;
    bool m_confirmOverwrite = false;
    bool m_dirty = false;
    bool m_shouldClose = false;
    bool m_dragging = false;
    PendingAction m_pendingAction = PendingAction::None;
    std::size_t m_dragStartX = 0;
    std::size_t m_dragStartY = 0;
};
}  // namespace baba_is_auto::editor

#endif
