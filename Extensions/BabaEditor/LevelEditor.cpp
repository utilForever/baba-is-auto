// Copyright (c) 2020-2026 Chris Ohk

// I am making my contributions/submissions to this project solely in our
// personal capacity and am not conveying any rights to any intellectual
// property of any third parties.

#include "LevelEditor.hpp"
#include "LevelFile.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <GLFW/glfw3.h>
#include <imgui_stdlib.h>

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <filesystem>
#include <queue>
#include <sstream>
#include <system_error>
#include <utility>

namespace baba_is_auto::editor
{
namespace
{
namespace fs = std::filesystem;

#ifndef BABA_EDITOR_MAPS_DIR
#define BABA_EDITOR_MAPS_DIR "Resources/Maps/"
#endif

#ifndef BABA_EDITOR_SPRITES_DIR
#define BABA_EDITOR_SPRITES_DIR "Extensions/BabaGUI/sprites/"
#endif

#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif

//! Convert RGB values in the range [0, 255] to an ImVec4 with alpha = 1.0.
ImVec4 ColorU8(int r, int g, int b)
{
    return { r / 255.0f, g / 255.0f, b / 255.0f, 1.0f };
}

//! Normalize text for case-insensitive comparisons.
std::string Normalize(std::string value)
{
    std::transform(
        value.begin(), value.end(), value.begin(),
        [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
    return value;
}

//! Return true if the given text contains the given pattern, ignoring case.
bool ContainsCaseInsensitive(std::string text, std::string pattern)
{
    if (pattern.empty())
    {
        return true;
    }

    text = Normalize(std::move(text));
    pattern = Normalize(std::move(pattern));
    return text.find(pattern) != std::string::npos;
}

//! Return true if the given text starts with the given pattern, ignoring case.
fs::path PathFromUtf8(const std::string& path)
{
    return fs::u8path(path);
}

//! Return a UTF-8 string representation of the given path, suitable for display
//! in the editor UI.
std::string PathToUtf8(const fs::path& path)
{
    return path.u8string();
}

//! Return the initial directory for a file dialog, given a path that may be
//! a file or directory. If the path is empty, return the default maps folder.
std::string InitialDirectory(const std::string& path)
{
    std::error_code ec;
    fs::path candidate =
        PathFromUtf8(path.empty() ? BABA_EDITOR_MAPS_DIR : path);

    if (fs::is_regular_file(candidate, ec))
    {
        candidate = candidate.parent_path();
    }

    if (!fs::is_directory(candidate, ec))
    {
        candidate = candidate.has_parent_path() ? candidate.parent_path()
                                                : fs::current_path(ec);
    }

    return PathToUtf8(candidate);
}

//! Return the filename portion of a path, or an empty string if the path is
//! empty or ends with a directory separator.
std::string FilenameOnly(const std::string& path)
{
    return PathToUtf8(PathFromUtf8(path).filename());
}

//! Return true if the given directory entry is a directory, handling errors
//! gracefully.
bool IsDirectory(const fs::directory_entry& entry)
{
    std::error_code ec;
    return entry.is_directory(ec);
}

//! Return true if the given directory entry is a regular file, handling errors
//! gracefully.
bool InputTextString(const char* label, std::string& value)
{
    return ImGui::InputText(label, &value);
}

//! Return a list of all objects that can be placed in the level editor, along
//! with their display names, groups, and colors.
const std::vector<ObjectEntry>& ObjectCatalog()
{
    // Expose only objects that have matching sprites in BabaGUI.
    static const std::vector<ObjectEntry> catalog = {
        { ObjectType::ICON_EMPTY, "EMPTY", "Special", ColorU8(18, 22, 32) },
        { ObjectType::BABA, "BABA", "Text: Nouns", ColorU8(238, 238, 238) },
        { ObjectType::FLAG, "FLAG", "Text: Nouns", ColorU8(238, 238, 238) },
        { ObjectType::GRASS, "GRASS", "Text: Nouns", ColorU8(238, 238, 238) },
        { ObjectType::LAVA, "LAVA", "Text: Nouns", ColorU8(238, 238, 238) },
        { ObjectType::ROCK, "ROCK", "Text: Nouns", ColorU8(238, 238, 238) },
        { ObjectType::SKULL, "SKULL", "Text: Nouns", ColorU8(238, 238, 238) },
        { ObjectType::WALL, "WALL", "Text: Nouns", ColorU8(238, 238, 238) },
        { ObjectType::WATER, "WATER", "Text: Nouns", ColorU8(238, 238, 238) },
        { ObjectType::IS, "IS", "Text: Operators", ColorU8(238, 238, 238) },
        { ObjectType::YOU, "YOU", "Text: Properties", ColorU8(238, 238, 238) },
        { ObjectType::STOP, "STOP", "Text: Properties",
          ColorU8(238, 238, 238) },
        { ObjectType::PUSH, "PUSH", "Text: Properties",
          ColorU8(238, 238, 238) },
        { ObjectType::WIN, "WIN", "Text: Properties", ColorU8(238, 238, 238) },
        { ObjectType::DEFEAT, "DEFEAT", "Text: Properties",
          ColorU8(238, 238, 238) },
        { ObjectType::SINK, "SINK", "Text: Properties",
          ColorU8(238, 238, 238) },
        { ObjectType::HOT, "HOT", "Text: Properties", ColorU8(238, 238, 238) },
        { ObjectType::MELT, "MELT", "Text: Properties",
          ColorU8(238, 238, 238) },
        { ObjectType::ICON_BABA, "BABA", "Objects", ColorU8(236, 236, 236) },
        { ObjectType::ICON_FLAG, "FLAG", "Objects", ColorU8(237, 217, 96) },
        { ObjectType::ICON_FLOWER, "FLOWER", "Objects", ColorU8(216, 52, 105) },
        { ObjectType::ICON_GRASS, "GRASS", "Objects", ColorU8(66, 128, 74) },
        { ObjectType::ICON_LAVA, "LAVA", "Objects", ColorU8(225, 74, 52) },
        { ObjectType::ICON_ROCK, "ROCK", "Objects", ColorU8(74, 67, 55) },
        { ObjectType::ICON_SKULL, "SKULL", "Objects", ColorU8(238, 238, 238) },
        { ObjectType::ICON_TILE, "TILE", "Objects", ColorU8(93, 92, 86) },
        { ObjectType::ICON_WALL, "WALL", "Objects", ColorU8(101, 96, 82) },
        { ObjectType::ICON_WATER, "WATER", "Objects", ColorU8(94, 158, 207) },
    };

    return catalog;
}

//! Return a short label for the given placement mode, suitable for a button.
const char* ModeLabel(PlacementMode mode)
{
    switch (mode)
    {
        case PlacementMode::Freeform:
            return "Free";
        case PlacementMode::Line:
            return "Line";
        case PlacementMode::Rectangle:
            return "Rect";
        case PlacementMode::FilledRectangle:
            return "Fill Rect";
        case PlacementMode::FloodFill:
            return "Flood";
        case PlacementMode::Eraser:
            return "Erase";
    }

    return "Free";
}

//! Initial positions and sizes for the editor's top-level windows.
struct EditorLayout
{
    ImVec2 objectsPos;
    ImVec2 objectsSize;
    ImVec2 levelPos;
    ImVec2 levelSize;
    ImVec2 settingsPos;
    ImVec2 settingsSize;
};

//! Return the positions and sizes of the editor's top-level windows, derived
//! from the current viewport size and aspect ratio.
EditorLayout CalculateLayout()
{
    // Derive all initial panel bounds from the usable viewport in one place.
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    const ImVec2 workPos = viewport->WorkPos;
    const ImVec2 workSize = viewport->WorkSize;

    // Use fixed margins and gaps, but scale the main level canvas to fill the
    // remaining space.
    constexpr float margin = 12.0f;
    constexpr float gap = 12.0f;
    constexpr float statusHeight = 28.0f;
    constexpr float objectsWidth = 280.0f;
    constexpr float settingsWidth = 320.0f;

    // The top bar is a main menu bar, which is always the height of a single
    // font line.
    const float menuHeight = ImGui::GetFrameHeight();
    const float top = workPos.y + menuHeight + margin;
    const float availableHeight = std::max(
        260.0f, workSize.y - statusHeight - menuHeight - margin * 2.0f);
    const float availableWidth = std::max(720.0f, workSize.x - margin * 2.0f);
    const float sideWidth =
        std::min(objectsWidth, std::max(220.0f, availableWidth * 0.24f));
    const float rightWidth =
        std::min(settingsWidth, std::max(240.0f, availableWidth * 0.25f));
    const float levelWidth =
        std::max(360.0f, availableWidth - sideWidth - rightWidth - gap * 2.0f);

    // Return the calculated layout for the editor's top-level windows.
    EditorLayout layout;
    layout.objectsPos = { workPos.x + margin, top };
    layout.objectsSize = { sideWidth, availableHeight };
    layout.levelPos = { layout.objectsPos.x + sideWidth + gap, top };
    layout.levelSize = { levelWidth, availableHeight };
    layout.settingsPos = { layout.levelPos.x + levelWidth + gap, top };
    layout.settingsSize = { rightWidth, availableHeight };
    return layout;
}

//! Return the lower of two size_t values.
std::size_t Lower(std::size_t a, std::size_t b)
{
    return std::min(a, b);
}

//! Return the higher of two size_t values.
std::size_t Upper(std::size_t a, std::size_t b)
{
    return std::max(a, b);
}
}  // namespace

//! Construct a new Level Editor object and load all sprite textures from disk.
LevelEditor::LevelEditor()
{
    const fs::path samplePath =
        PathFromUtf8(std::string(BABA_EDITOR_MAPS_DIR) + "editor_smoke.txt");

    LoadTextures();
    NewLevel(m_width, m_height);
    ImportLevel(samplePath);

    m_levelPath.clear();
    m_dirty = false;

    if (!m_textureStatus.empty())
    {
        m_status = m_textureStatus;
    }
}

//! Destruct the Level Editor object and release all sprite textures from
//! OpenGL.
LevelEditor::~LevelEditor()
{
    ReleaseTextures();
}

//! Draw the editor UI and handle user input. This should be called once per
//! frame while the editor's OpenGL context is active.
void LevelEditor::Draw()
{
    DrawTopBar();

    if (m_showObjects)
    {
        DrawObjectPalette();
    }

    if (m_showSettings)
    {
        DrawSettings();
    }

    DrawCanvas();
    DrawFileDialog();
    DrawActionConfirmation();
    DrawStatusBar();
}

//! Route native window close requests through the unsaved-change prompt.
void LevelEditor::RequestClose()
{
    RequestAction(PendingAction::Close);
}

//! Return true if the editor should close, after confirming any unsaved
//! changes.
bool LevelEditor::ShouldClose() const
{
    return m_shouldClose;
}

//! Create a new level with the given width and height, discarding any existing
//! level data.
void LevelEditor::NewLevel(std::size_t width, std::size_t height)
{
    m_width = std::max(MIN_LEVEL_WIDTH, width);
    m_height = std::max(MIN_LEVEL_HEIGHT, height);
    m_nextWidth = static_cast<int>(m_width);
    m_nextHeight = static_cast<int>(m_height);
    m_tiles.assign(m_width * m_height, { EMPTY, EMPTY, EMPTY });
    m_undoStack.clear();
    m_levelPath.clear();
    m_dirty = true;
    m_status = "Created a new level";
}

//! Resize the current level to the given width and height, preserving any
//! overlapping tiles in the top-left corner and discarding any out-of-range
//! tiles.
void LevelEditor::ResizeLevel(std::size_t width, std::size_t height)
{
    width = std::max(MIN_LEVEL_WIDTH, width);
    height = std::max(MIN_LEVEL_HEIGHT, height);

    if (width == m_width && height == m_height)
    {
        return;
    }

    LevelTiles resized(width * height, { EMPTY, EMPTY, EMPTY });
    const std::size_t copyWidth = std::min(width, m_width);
    const std::size_t copyHeight = std::min(height, m_height);

    // Preserve the top-left overlap; shrinking discards only out-of-range
    // cells.
    for (std::size_t y = 0; y < copyHeight; ++y)
    {
        std::copy_n(m_tiles.begin() + y * m_width, copyWidth,
                    resized.begin() + y * width);
    }

    m_width = width;
    m_height = height;
    m_nextWidth = static_cast<int>(m_width);
    m_nextHeight = static_cast<int>(m_height);
    m_tiles = std::move(resized);
    m_undoStack.clear();
    m_dirty = true;
    m_status = "Resized level";
}

//! Import a level from the given filename, replacing any existing level data.
bool LevelEditor::ImportLevel(const fs::path& filename)
{
    LevelFile level;
    const std::string displayName = PathToUtf8(filename);

    if (!LoadLevelFile(filename, level))
    {
        m_status = "Could not read " + displayName;
        return false;
    }

    m_width = level.width;
    m_height = level.height;
    m_nextWidth = static_cast<int>(m_width);
    m_nextHeight = static_cast<int>(m_height);
    m_tiles = std::move(level.tiles);
    m_undoStack.clear();
    m_dirty = false;
    m_status = "Opened " + displayName;
    return true;
}

//! Export the current level to the given filename, overwriting any existing
//! file.
bool LevelEditor::ExportLevel(const fs::path& filename) const
{
    LevelFile level;
    level.width = m_width;
    level.height = m_height;
    level.tiles = m_tiles;

    return SaveLevelFile(filename, level);
}

//! Open a file dialog for the user to select a level file to open or save. If
//! save is true, the dialog is for saving; otherwise, it is for opening.
void LevelEditor::OpenFileDialog(bool save)
{
    const std::string defaultPath =
        std::string(BABA_EDITOR_MAPS_DIR) +
        (save ? "editor_export.txt" : "editor_smoke.txt");
    const std::string path = m_levelPath.empty() ? defaultPath : m_levelPath;

    m_saveDialog = save;
    m_confirmOverwrite = false;
    m_dialogDirectory = InitialDirectory(path);
    m_dialogFilename = save ? FilenameOnly(path) : "";
    m_showFileDialog = true;
}

//! Open a file dialog for the user to select a level file to open, using the
//! current level path as the initial directory.
void LevelEditor::OpenLevelWithDialog()
{
    OpenFileDialog(false);
}

//! Save the current level to its existing path, or open a "Save As" dialog if
//! the path is empty.
void LevelEditor::SaveLevel()
{
    if (m_levelPath.empty())
    {
        SaveLevelAs();
        return;
    }

    if (ExportLevel(PathFromUtf8(m_levelPath)))
    {
        m_dirty = false;
        m_status = "Saved " + m_levelPath;
    }
    else
    {
        m_status = "Could not save " + m_levelPath;
    }
}

//! Open a "Save As" dialog for the user to select a level file to save, using
//! the current level path as the initial directory.
void LevelEditor::SaveLevelAs()
{
    OpenFileDialog(true);
}

//! Request a pending action that requires user confirmation before execution,
//! such as creating a new level, opening a level, resizing the level, or
//! closing the editor. If there are unsaved changes, the action will be
//! deferred until the user confirms.
void LevelEditor::RequestAction(PendingAction action)
{
    if (action == PendingAction::None || m_pendingAction != PendingAction::None)
    {
        return;
    }

    m_pendingAction = action;

    const bool shrinking = action == PendingAction::ResizeLevel &&
                           (m_nextWidth < static_cast<int>(m_width) ||
                            m_nextHeight < static_cast<int>(m_height));

    if ((!m_dirty && action != PendingAction::ResizeLevel) ||
        (action == PendingAction::ResizeLevel && !shrinking))
    {
        ExecutePendingAction();
    }
}

//! Execute the pending action that was previously requested, after the user has
//! confirmed any unsaved changes. This will create a new level, open a level,
//! resize the level, or close the editor, depending on the pending action.
void LevelEditor::ExecutePendingAction()
{
    const PendingAction action = m_pendingAction;
    m_pendingAction = PendingAction::None;

    switch (action)
    {
        case PendingAction::NewLevel:
            NewLevel(static_cast<std::size_t>(m_nextWidth),
                     static_cast<std::size_t>(m_nextHeight));
            break;
        case PendingAction::OpenLevel:
            OpenLevelWithDialog();
            break;
        case PendingAction::ResizeLevel:
            ResizeLevel(static_cast<std::size_t>(m_nextWidth),
                        static_cast<std::size_t>(m_nextHeight));
            break;
        case PendingAction::Close:
            m_shouldClose = true;
            break;
        case PendingAction::None:
            break;
    }
}

//! Draw the editor's top menu bar, including file and view menus, layer
//! buttons, and placement mode buttons.
void LevelEditor::DrawTopBar()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("New"))
            {
                RequestAction(PendingAction::NewLevel);
            }

            if (ImGui::MenuItem("Open"))
            {
                RequestAction(PendingAction::OpenLevel);
            }

            if (ImGui::MenuItem("Save", "Ctrl+S"))
            {
                SaveLevel();
            }

            if (ImGui::MenuItem("Save As"))
            {
                SaveLevelAs();
            }

            if (ImGui::MenuItem("Undo", "Ctrl+Z", false, !m_undoStack.empty()))
            {
                Undo();
            }

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View"))
        {
            ImGui::MenuItem("Objects", nullptr, &m_showObjects);
            ImGui::MenuItem("Settings", nullptr, &m_showSettings);
            ImGui::MenuItem("Grid", nullptr, &m_showGrid);
            ImGui::EndMenu();
        }

        ImGui::Separator();
        const std::string title =
            m_levelPath.empty() ? "Untitled" : FilenameOnly(m_levelPath);
        ImGui::Text("%s%s (%zux%zu)", title.c_str(), m_dirty ? "*" : "",
                    m_width, m_height);
        ImGui::Separator();

        for (std::size_t layer = 0; layer < LAYER_COUNT; ++layer)
        {
            ImGui::PushID(static_cast<int>(layer));

            const bool active = m_currentLayer == layer;

            if (active)
            {
                ImGui::PushStyleColor(ImGuiCol_Button, ColorU8(216, 52, 105));
            }

            std::string label = "L" + std::to_string(layer + 1);

            if (ImGui::Button(label.c_str(), { 42.0f, 0.0f }))
            {
                m_currentLayer = layer;
            }

            if (active)
            {
                ImGui::PopStyleColor();
            }

            ImGui::PopID();
        }

        ImGui::Separator();

        for (PlacementMode mode :
             { PlacementMode::Freeform, PlacementMode::Line,
               PlacementMode::Rectangle, PlacementMode::FilledRectangle,
               PlacementMode::FloodFill, PlacementMode::Eraser })
        {
            const bool active = m_mode == mode;

            if (active)
            {
                ImGui::PushStyleColor(ImGuiCol_Button, ColorU8(64, 82, 160));
            }

            if (ImGui::Button(ModeLabel(mode)))
            {
                m_mode = mode;
            }

            if (active)
            {
                ImGui::PopStyleColor();
            }
        }

        ImGui::EndMainMenuBar();
    }

    if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_Z))
    {
        Undo();
    }

    if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_S))
    {
        SaveLevel();
    }
}

//! Draw the object palette window, allowing the user to select an object to
//! place in the level editor.
void LevelEditor::DrawObjectPalette()
{
    const EditorLayout layout = CalculateLayout();
    ImGui::SetNextWindowPos(layout.objectsPos, ImGuiCond_Once);
    ImGui::SetNextWindowSize(layout.objectsSize, ImGuiCond_Once);
    ImGui::Begin("Objects", &m_showObjects);
    InputTextString("Search", m_search);

    const ImGuiStyle& style = ImGui::GetStyle();
    const ImVec2 buttonSize{ 56.0f, 56.0f };
    const float contentMaxX =
        ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
    const char* currentGroup = "";
    const auto& catalog = ObjectCatalog();

    for (std::size_t i = 0; i < catalog.size(); ++i)
    {
        const ObjectEntry& entry = catalog[i];

        if (!ContainsCaseInsensitive(entry.name, m_search) &&
            !ContainsCaseInsensitive(entry.group, m_search))
        {
            continue;
        }

        if (std::string(currentGroup) != entry.group)
        {
            currentGroup = entry.group;
            ImGui::Separator();
            ImGui::TextUnformatted(currentGroup);
        }

        ImGui::PushID(static_cast<int>(entry.type));

        if (DrawSpriteButton(entry.type, buttonSize))
        {
            m_selected = entry.type;

            if (m_mode == PlacementMode::Eraser)
            {
                m_mode = PlacementMode::Freeform;
            }
        }

        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("%s\n%s\nid %d", entry.name, entry.group,
                              static_cast<int>(entry.type));
        }

        const ObjectEntry* nextEntry = nullptr;

        for (std::size_t next = i + 1; next < catalog.size(); ++next)
        {
            if (ContainsCaseInsensitive(catalog[next].name, m_search) ||
                ContainsCaseInsensitive(catalog[next].group, m_search))
            {
                nextEntry = &catalog[next];
                break;
            }
        }

        if (nextEntry != nullptr &&
            std::string(nextEntry->group) == entry.group)
        {
            const float nextButtonMaxX =
                ImGui::GetItemRectMax().x + style.ItemSpacing.x + buttonSize.x;

            if (nextButtonMaxX <= contentMaxX)
            {
                ImGui::SameLine();
            }
        }

        ImGui::PopID();
    }

    ImGui::End();
}

//! Draw the settings window, allowing the user to view and modify level
//! properties, including the current file path, level dimensions, selected
//! object, current layer, and grid visibility.
void LevelEditor::DrawSettings()
{
    const EditorLayout layout = CalculateLayout();
    ImGui::SetNextWindowPos(layout.settingsPos, ImGuiCond_Once);
    ImGui::SetNextWindowSize(layout.settingsSize, ImGuiCond_Once);
    ImGui::Begin("Settings", &m_showSettings);

    InputTextString("File path", m_levelPath);

    if (ImGui::Button("Open##FilePath"))
    {
        RequestAction(PendingAction::OpenLevel);
    }

    ImGui::SameLine();

    if (ImGui::Button("Save##FilePath"))
    {
        SaveLevel();
    }

    ImGui::SameLine();

    if (ImGui::Button("Save As##FilePath"))
    {
        SaveLevelAs();
    }

    ImGui::SliderInt("Width", &m_nextWidth, static_cast<int>(MIN_LEVEL_WIDTH),
                     static_cast<int>(MAX_LEVEL_WIDTH));
    ImGui::SliderInt("Height", &m_nextHeight,
                     static_cast<int>(MIN_LEVEL_HEIGHT),
                     static_cast<int>(MAX_LEVEL_HEIGHT));

    if (ImGui::Button("Resize") && (m_nextWidth != static_cast<int>(m_width) ||
                                    m_nextHeight != static_cast<int>(m_height)))
    {
        RequestAction(PendingAction::ResizeLevel);
    }

    ImGui::Separator();
    ImGui::TextUnformatted("Selection");
    ImGui::Text("Held: %s",
                NameFor(m_mode == PlacementMode::Eraser ? EMPTY : m_selected));
    ImGui::Text("Layer: L%zu", m_currentLayer + 1);
    ImGui::Checkbox("Show grid", &m_showGrid);

    if (ImGui::Button("Pick empty"))
    {
        m_selected = EMPTY;
        m_mode = PlacementMode::Freeform;
    }

    ImGui::SameLine();

    if (ImGui::Button("Eraser"))
    {
        m_mode = PlacementMode::Eraser;
    }

    ImGui::Separator();
    ImGui::TextUnformatted("Compatibility");
    ImGui::TextWrapped(
        "Files use the baba-is-auto numeric map format. L1, L2, and L3 are "
        "saved as consecutive grids.");

    ImGui::End();
}

//! Draw the level canvas window, allowing the user to view and edit the current
//! level. This includes rendering the tiles, handling mouse input for painting,
//! and displaying the current placement mode, layer, and selected object.
void LevelEditor::DrawCanvas()
{
    const EditorLayout layout = CalculateLayout();
    ImGui::SetNextWindowPos(layout.levelPos, ImGuiCond_Once);
    ImGui::SetNextWindowSize(layout.levelSize, ImGuiCond_Once);
    ImGui::Begin("Level");

    const ImVec2 content = ImGui::GetContentRegionAvail();
    const float tileWidth = content.x / static_cast<float>(m_width);
    const float tileHeight = (content.y - 24.0f) / static_cast<float>(m_height);
    const float tileSize = std::max(16.0f, std::min(tileWidth, tileHeight));

    ImGui::Text("Mode: %s | Layer L%zu | Brush: %s", ModeLabel(m_mode),
                m_currentLayer + 1,
                NameFor(m_mode == PlacementMode::Eraser ? EMPTY : m_selected));

    for (std::size_t y = 0; y < m_height; ++y)
    {
        for (std::size_t x = 0; x < m_width; ++x)
        {
            ImGui::PushID(static_cast<int>(y * m_width + x));
            DrawTileButton(x, y, tileSize);
            ImGui::PopID();

            if (x + 1 < m_width)
            {
                ImGui::SameLine(0.0f, 0.0f);
            }
        }
    }

    if (ImGui::IsMouseReleased(ImGuiMouseButton_Left))
    {
        m_dragging = false;
    }

    ImGui::End();
}

//! Draw the file dialog window, allowing the user to select a level file to
//! open or save. This includes displaying the current directory, file list, and
//! handling user input for selecting files and confirming overwrites.
void LevelEditor::DrawFileDialog()
{
    if (!m_showFileDialog)
    {
        return;
    }

    const char* title = m_saveDialog ? "Save As" : "Open";
    ImGui::OpenPopup(title);
    ImGui::SetNextWindowSize({ 640.0f, 500.0f }, ImGuiCond_Appearing);
    if (ImGui::BeginPopupModal(title, &m_showFileDialog,
                               ImGuiWindowFlags_NoSavedSettings))
    {
        std::error_code ec;
        fs::path directory = PathFromUtf8(m_dialogDirectory);

        // Resolve the typed name relative to the visible directory.
        const auto selectedPath = [&]() {
            fs::path selected = directory / PathFromUtf8(m_dialogFilename);

            if (m_saveDialog && selected.extension().empty())
            {
                selected += ".txt";
            }

            return selected;
        };

        // Share validation and open/save behavior between buttons and
        // double-click.
        const auto acceptSelected = [&]() {
            if (m_dialogFilename.empty())
            {
                m_status = "Choose a file";
                return;
            }

            const fs::path selected = selectedPath();

            if (m_saveDialog && fs::exists(selected, ec))
            {
                m_confirmOverwrite = true;
                return;
            }

            if (m_saveDialog)
            {
                const std::string selectedName = PathToUtf8(selected);

                if (ExportLevel(selected))
                {
                    m_levelPath = selectedName;
                    m_dirty = false;
                    m_status = "Saved " + m_levelPath;
                    m_showFileDialog = false;
                }
                else
                {
                    m_status = "Could not save " + selectedName;
                }

                return;
            }

            if (ImportLevel(selected))
            {
                m_levelPath = PathToUtf8(selected);
                m_showFileDialog = false;
            }
        };

        ImGui::TextUnformatted("Folder");
        ImGui::PushItemWidth(-1.0f);
        InputTextString("##DialogFolder", m_dialogDirectory);
        ImGui::PopItemWidth();

        if (ImGui::Button("Up") && directory.has_parent_path())
        {
            m_dialogDirectory = PathToUtf8(directory.parent_path());
        }

        ImGui::SameLine();

        if (ImGui::Button("Refresh"))
        {
            m_dialogDirectory = PathToUtf8(directory);
        }

        std::vector<fs::directory_entry> entries;

        for (const auto& entry : fs::directory_iterator(directory, ec))
        {
            entries.push_back(entry);
        }

        // Present directories first, then use case-insensitive name ordering.
        std::sort(
            entries.begin(), entries.end(),
            [](const fs::directory_entry& lhs, const fs::directory_entry& rhs) {
                const bool lhsDir = IsDirectory(lhs);
                const bool rhsDir = IsDirectory(rhs);

                if (lhsDir != rhsDir)
                {
                    return lhsDir;
                }

                return Normalize(PathToUtf8(lhs.path().filename())) <
                       Normalize(PathToUtf8(rhs.path().filename()));
            });

        ImGui::Separator();
        ImGui::BeginChild("Files", { 0.0f, 310.0f }, true);

        if (ec)
        {
            ImGui::Text("Could not read directory: %s", ec.message().c_str());
        }
        else if (entries.empty())
        {
            ImGui::TextUnformatted("No files");
        }

        for (const auto& entry : entries)
        {
            const bool isDir = entry.is_directory(ec);
            const std::string name = PathToUtf8(entry.path().filename());
            const std::string label = isDir ? name + "/" : name;

            if (ImGui::Selectable(label.c_str(), m_dialogFilename == name))
            {
                if (isDir)
                {
                    m_dialogDirectory = PathToUtf8(entry.path());
                    m_dialogFilename.clear();
                }
                else
                {
                    m_dialogFilename = name;
                }
            }

            if (!m_saveDialog && !isDir && ImGui::IsItemHovered() &&
                ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
            {
                acceptSelected();
            }
        }

        ImGui::EndChild();

        ImGui::Separator();
        ImGui::TextUnformatted("File name");
        ImGui::PushItemWidth(-1.0f);
        InputTextString("##DialogFileName", m_dialogFilename);
        ImGui::PopItemWidth();

        const char* acceptLabel = m_saveDialog ? "Save" : "Open";
        constexpr float buttonWidth = 90.0f;
        const float footerWidth =
            buttonWidth * 2.0f + ImGui::GetStyle().ItemSpacing.x;
        ImGui::SetCursorPosX(std::max(ImGui::GetCursorPosX(),
                                      ImGui::GetCursorPosX() +
                                          ImGui::GetContentRegionAvail().x -
                                          footerWidth));

        if (ImGui::Button(acceptLabel, { buttonWidth, 0.0f }))
        {
            acceptSelected();
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel", { buttonWidth, 0.0f }))
        {
            m_showFileDialog = false;
            m_confirmOverwrite = false;
        }

        if (m_confirmOverwrite)
        {
            ImGui::OpenPopup("Overwrite?");
        }

        if (ImGui::BeginPopupModal("Overwrite?", nullptr,
                                   ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::TextUnformatted("File exists. Overwrite it?");

            if (ImGui::Button("Overwrite", { buttonWidth, 0.0f }))
            {
                const fs::path selected = selectedPath();

                const std::string selectedName = PathToUtf8(selected);

                if (ExportLevel(selected))
                {
                    m_levelPath = selectedName;
                    m_dirty = false;
                    m_status = "Saved " + m_levelPath;
                    m_showFileDialog = false;
                }
                else
                {
                    m_status = "Could not save " + selectedName;
                }

                m_confirmOverwrite = false;
                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine();

            if (ImGui::Button("Cancel", { buttonWidth, 0.0f }))
            {
                m_confirmOverwrite = false;
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }

        ImGui::EndPopup();
    }
}

//! Draw a modal confirmation dialog for any pending action that requires user
//! confirmation, such as creating a new level, opening a level, resizing the
//! level, or closing the editor. If there are unsaved changes, the user can
//! choose to discard them or cancel the action.
void LevelEditor::DrawActionConfirmation()
{
    if (m_pendingAction == PendingAction::None)
    {
        return;
    }

    const bool resizing = m_pendingAction == PendingAction::ResizeLevel;

    if (!resizing && !m_dirty)
    {
        ExecutePendingAction();
        return;
    }

    ImGui::OpenPopup("Confirm action");

    if (ImGui::BeginPopupModal("Confirm action", nullptr,
                               ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::TextUnformatted(resizing
                                   ? "Resize and discard out-of-range tiles?"
                                   : "Discard unsaved changes?");

        if (ImGui::Button(resizing ? "Resize" : "Discard", { 90.0f, 0.0f }))
        {
            ImGui::CloseCurrentPopup();
            ExecutePendingAction();
        }

        ImGui::SameLine();

        if (ImGui::Button("Cancel", { 90.0f, 0.0f }))
        {
            m_pendingAction = PendingAction::None;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

//! Draw the status bar at the bottom of the editor window, displaying the
//! current status message.
void LevelEditor::DrawStatusBar()
{
    ImGuiViewport* viewport = ImGui::GetMainViewport();
    constexpr float height = 28.0f;

    ImGui::SetNextWindowPos(
        { viewport->Pos.x, viewport->Pos.y + viewport->Size.y - height });
    ImGui::SetNextWindowSize({ viewport->Size.x, height });
    ImGui::Begin("StatusBar", nullptr,
                 ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoSavedSettings |
                     ImGuiWindowFlags_NoBringToFrontOnFocus);
    ImGui::TextUnformatted(m_status.c_str());
    ImGui::End();
}

//! Draw the sprite for the given object type within the specified rectangle. If
//! the object type is EMPTY or has no valid texture, a visible marker is drawn
//! instead.
void LevelEditor::DrawSpriteInRect(ObjectType type, const ImVec2& min,
                                   const ImVec2& max) const
{
    if (type == EMPTY)
    {
        return;
    }

    const SpriteTexture* texture = TextureFor(type);

    if (texture == nullptr || texture->id == 0)
    {
        // A visible marker is safer than silently falling back to a text label.
        const ImVec2 center{ (min.x + max.x) * 0.5f, (min.y + max.y) * 0.5f };
        constexpr float halfSize = 4.0f;
        ImDrawList* drawList = ImGui::GetWindowDrawList();

        drawList->AddLine({ center.x - halfSize, center.y - halfSize },
                          { center.x + halfSize, center.y + halfSize },
                          ImGui::ColorConvertFloat4ToU32(ColorU8(216, 52, 105)),
                          2.0f);
        drawList->AddLine({ center.x + halfSize, center.y - halfSize },
                          { center.x - halfSize, center.y + halfSize },
                          ImGui::ColorConvertFloat4ToU32(ColorU8(216, 52, 105)),
                          2.0f);
        return;
    }

    const float width = max.x - min.x;
    const float height = max.y - min.y;
    const float size = std::min(width, height) * 0.86f;
    const float x = min.x + (width - size) * 0.5f;
    const float y = min.y + (height - size) * 0.5f;
    const ImVec2 imageMin{ x, y };
    const ImVec2 imageMax{ x + size, y + size };

    ImGui::GetWindowDrawList()->AddImage((ImTextureID)(intptr_t)texture->id,
                                         imageMin, imageMax);
}

//! Draw a button for the given object type, using its sprite as the button
//! image. The button is highlighted if the object type is currently selected,
//! and a hover effect is applied when the mouse is over the button. Returns
//! true if the button was clicked.
bool LevelEditor::DrawSpriteButton(ObjectType type, const ImVec2& size)
{
    const ObjectEntry& entry = EntryFor(type);
    const bool clicked = ImGui::InvisibleButton(entry.name, size);
    const ImVec2 min = ImGui::GetItemRectMin();
    const ImVec2 max = ImGui::GetItemRectMax();
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    drawList->AddRectFilled(min, max,
                            ImGui::ColorConvertFloat4ToU32(entry.color), 3.0f);
    drawList->AddRect(
        min, max, ImGui::ColorConvertFloat4ToU32(ColorU8(54, 70, 145)), 3.0f);

    if (m_selected == type)
    {
        drawList->AddRect(min, max,
                          ImGui::ColorConvertFloat4ToU32(ColorU8(216, 52, 105)),
                          3.0f, 0, 3.0f);
    }
    else if (ImGui::IsItemHovered())
    {
        drawList->AddRect(min, max,
                          ImGui::ColorConvertFloat4ToU32(ColorU8(237, 217, 96)),
                          3.0f, 0, 2.0f);
    }

    DrawSpriteInRect(type, min, max);
    return clicked;
}

//! Draw a button for the tile at the given coordinates, using its visible
//! sprite as the button image. The button is highlighted if the tile is
//! hovered, and a tooltip is displayed showing the tile's coordinates, visible
//! object type, and current layer object type. User interactions with the
//! button are handled to support picking, painting, and drag operations for
//! line and rectangle placement modes.
void LevelEditor::DrawTileButton(std::size_t x, std::size_t y, float tileSize)
{
    const ObjectType visible = VisibleTile(x, y);
    const ObjectType layerValue = CurrentLayerTile(x, y);

    ImGui::InvisibleButton("tile", { tileSize, tileSize });

    const ImVec2 min = ImGui::GetItemRectMin();
    const ImVec2 max = ImGui::GetItemRectMax();
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    drawList->AddRectFilled(min, max,
                            ImGui::ColorConvertFloat4ToU32(ColorU8(0, 0, 0)));

    if (m_showGrid)
    {
        drawList->AddRect(min, max,
                          ImGui::ColorConvertFloat4ToU32(ColorU8(54, 70, 145)));
    }

    if (ImGui::IsItemHovered())
    {
        drawList->AddRect(min, max,
                          ImGui::ColorConvertFloat4ToU32(ColorU8(216, 52, 105)),
                          0.0f, 0, 2.0f);
    }

    DrawSpriteInRect(visible, min, max);

    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip("(%zu, %zu)\nVisible: %s\nL%zu: %s", x, y,
                          NameFor(visible), m_currentLayer + 1,
                          NameFor(layerValue));
    }

    HandleTileInteraction(x, y);
}

//! Handle user interactions with the tile at the given coordinates, including
//! picking, painting, and drag operations for line and rectangle placement
//! modes.
void LevelEditor::HandleTileInteraction(std::size_t x, std::size_t y)
{
    if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
    {
        const ObjectType picked = CurrentLayerTile(x, y);

        if (picked != EMPTY)
        {
            m_selected = picked;
            m_mode = PlacementMode::Freeform;
            m_status = "Picked " + std::string(NameFor(picked));
        }

        return;
    }

    if (m_mode == PlacementMode::Line || m_mode == PlacementMode::Rectangle ||
        m_mode == PlacementMode::FilledRectangle)
    {
        // Drag modes commit once on release so a whole shape is one undo step.
        if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
        {
            m_dragging = true;
            m_dragStartX = x;
            m_dragStartY = y;
        }

        if (m_dragging && ImGui::IsItemHovered() &&
            ImGui::IsMouseReleased(ImGuiMouseButton_Left))
        {
            PushUndo();

            if (m_mode == PlacementMode::Line)
            {
                ApplyLine(m_dragStartX, m_dragStartY, x, y);
            }
            else
            {
                ApplyRectangle(m_dragStartX, m_dragStartY, x, y,
                               m_mode == PlacementMode::FilledRectangle);
            }

            m_dragging = false;
        }

        return;
    }

    if (ImGui::IsItemClicked(ImGuiMouseButton_Left))
    {
        PushUndo();

        if (m_mode == PlacementMode::FloodFill)
        {
            ApplyFloodFill(x, y);
        }
        else
        {
            ApplyBrush(x, y);
        }
    }
    else if ((m_mode == PlacementMode::Freeform ||
              m_mode == PlacementMode::Eraser) &&
             ImGui::IsMouseDown(ImGuiMouseButton_Left) &&
             ImGui::IsItemHovered())
    {
        ApplyBrush(x, y);
    }
}

//! Apply the current brush to the tile at the given coordinates, setting it to
//! the selected object type or erasing it if the eraser mode is active.
void LevelEditor::ApplyBrush(std::size_t x, std::size_t y)
{
    SetTile(x, y, m_mode == PlacementMode::Eraser ? EMPTY : m_selected);
}

//! Apply a line from (x0, y0) to (x1, y1) using Bresenham's algorithm, setting
//! the tiles along the line to the selected object type.
void LevelEditor::ApplyLine(std::size_t x0, std::size_t y0, std::size_t x1,
                            std::size_t y1)
{
    // Integer Bresenham rasterization covers all slopes without floating point.
    int x = static_cast<int>(x0);
    int y = static_cast<int>(y0);
    const int endX = static_cast<int>(x1);
    const int endY = static_cast<int>(y1);
    const int dx = std::abs(endX - x);
    const int sx = x < endX ? 1 : -1;
    const int dy = -std::abs(endY - y);
    const int sy = y < endY ? 1 : -1;
    int error = dx + dy;

    while (true)
    {
        SetTile(static_cast<std::size_t>(x), static_cast<std::size_t>(y),
                m_selected);

        if (x == endX && y == endY)
        {
            break;
        }

        const int error2 = 2 * error;

        if (error2 >= dy)
        {
            error += dy;
            x += sx;
        }

        if (error2 <= dx)
        {
            error += dx;
            y += sy;
        }
    }
}

//! Apply a rectangle from (x0, y0) to (x1, y1), optionally filled, setting the
//! tiles along the rectangle's border (or all tiles if filled) to the selected
//! object type.
void LevelEditor::ApplyRectangle(std::size_t x0, std::size_t y0, std::size_t x1,
                                 std::size_t y1, bool filled)
{
    for (std::size_t y = Lower(y0, y1); y <= Upper(y0, y1); ++y)
    {
        for (std::size_t x = Lower(x0, x1); x <= Upper(x0, x1); ++x)
        {
            if (filled || x == Lower(x0, x1) || x == Upper(x0, x1) ||
                y == Lower(y0, y1) || y == Upper(y0, y1))
            {
                SetTile(x, y, m_selected);
            }
        }
    }
}

//! Apply a flood fill starting from (x, y), replacing all orthogonally
//! connected tiles of the same type with the selected object type or erasing
//! them if the eraser mode is active. The flood fill only affects the current
//! layer.
void LevelEditor::ApplyFloodFill(std::size_t x, std::size_t y)
{
    const ObjectType target = CurrentLayerTile(x, y);
    const ObjectType replacement =
        m_mode == PlacementMode::Eraser ? EMPTY : m_selected;

    if (target == replacement)
    {
        return;
    }

    // Flood only orthogonally connected cells on the active layer.
    std::queue<std::pair<std::size_t, std::size_t>> pending;
    pending.emplace(x, y);

    while (!pending.empty())
    {
        const auto [cx, cy] = pending.front();
        pending.pop();

        if (CurrentLayerTile(cx, cy) != target)
        {
            continue;
        }

        SetTile(cx, cy, replacement);

        if (cx > 0)
        {
            pending.emplace(cx - 1, cy);
        }

        if (cy > 0)
        {
            pending.emplace(cx, cy - 1);
        }

        if (cx + 1 < m_width)
        {
            pending.emplace(cx + 1, cy);
        }

        if (cy + 1 < m_height)
        {
            pending.emplace(cx, cy + 1);
        }
    }
}

//! Set the tile at the given coordinates to the specified object type on the
//! current layer.
void LevelEditor::SetTile(std::size_t x, std::size_t y, ObjectType type)
{
    ObjectType& tile = Tile(x, y)[m_currentLayer];

    if (tile != type)
    {
        tile = type;
        m_dirty = true;
    }
}

//! Get the visible object type at the given coordinates, considering all layers
//! and returning the topmost non-empty type. If all layers are empty, returns
//! EMPTY.
ObjectType LevelEditor::VisibleTile(std::size_t x, std::size_t y) const
{
    const LayerTile& tile = Tile(x, y);

    // Higher layers visually cover lower ones once layered maps are enabled.
    for (std::size_t layer = LAYER_COUNT; layer > 0; --layer)
    {
        const ObjectType type = tile[layer - 1];

        if (type != EMPTY)
        {
            return type;
        }
    }

    return EMPTY;
}

//! Get the object type at the given coordinates on the current layer.
ObjectType LevelEditor::CurrentLayerTile(std::size_t x, std::size_t y) const
{
    return Tile(x, y)[m_currentLayer];
}

//! Get a reference to the tile at the given coordinates, allowing modification
//! of its object types on all layers.
LevelEditor::LayerTile& LevelEditor::Tile(std::size_t x, std::size_t y)
{
    return m_tiles.at(y * m_width + x);
}

//! Get a const reference to the tile at the given coordinates, allowing
//! read-only access to its object types on all layers.
const LevelEditor::LayerTile& LevelEditor::Tile(std::size_t x,
                                                std::size_t y) const
{
    return m_tiles.at(y * m_width + x);
}

//! Push the current level state onto the undo stack, allowing the user to undo
//! recent changes. The undo stack is bounded to a maximum of 32 states to limit
//! memory usage while retaining enough history for normal editing.
void LevelEditor::PushUndo()
{
    m_undoStack.emplace_back(m_tiles);

    // Bound memory use while retaining enough history for normal editing.
    if (m_undoStack.size() > 32)
    {
        m_undoStack.erase(m_undoStack.begin());
    }
}

//! Undo the most recent change to the level, restoring the previous state from
//! the undo stack. If the undo stack is empty, no action is taken.
void LevelEditor::Undo()
{
    if (m_undoStack.empty())
    {
        return;
    }

    m_tiles = m_undoStack.back();
    m_undoStack.pop_back();
    m_dirty = true;
    m_status = "Undo";
}

//! Get a reference to the object entry for the given object type from the
//! object catalog. If the type is not found, returns the first entry in the
//! catalog, which is typically the EMPTY type.
const ObjectEntry& LevelEditor::EntryFor(ObjectType type) const
{
    const auto& catalog = ObjectCatalog();
    const auto iter = std::find_if(
        catalog.begin(), catalog.end(),
        [type](const ObjectEntry& entry) { return entry.type == type; });

    if (iter != catalog.end())
    {
        return *iter;
    }

    return catalog.front();
}

//! Get the name of the object type from the object catalog. If the type is not
//! found, returns the name of the first entry in the catalog, which is
//! typically the EMPTY type.
const char* LevelEditor::NameFor(ObjectType type) const
{
    return EntryFor(type).name;
}

//! Load all object textures from the sprite directory, storing them in the
//! texture map for later use. The function counts the number of successfully
//! loaded textures and the number of failures, reporting the last failed path
//! in the status message. Textures are loaded using stb_image and OpenGL, with
//! nearest-neighbor filtering to preserve pixel art quality.
void LevelEditor::LoadTextures()
{
    std::size_t loaded = 0;
    std::size_t failed = 0;
    std::string lastFailure;

    for (const ObjectEntry& entry : ObjectCatalog())
    {
        if (entry.type == EMPTY)
        {
            continue;
        }

        const std::string path = SpritePath(entry);

        if (LoadTexture(entry.type, path))
        {
            ++loaded;
        }
        else
        {
            ++failed;
            lastFailure = path;
        }
    }

    std::ostringstream stream;
    stream << "Sprites loaded: " << loaded;

    if (failed != 0)
    {
        stream << ", failed: " << failed << " (last: " << lastFailure << ")";
    }

    m_textureStatus = stream.str();
}

//! Load a single texture from the given filename for the specified object type,
//! storing it in the texture map. The function uses stb_image to load the image
//! data and OpenGL to create a texture object. If the loading or texture
//! creation fails, the function returns false; otherwise, it returns true.
bool LevelEditor::LoadTexture(ObjectType type, const std::string& filename)
{
    int width = 0;
    int height = 0;
    int channels = 0;
    stbi_uc* pixels =
        stbi_load(filename.c_str(), &width, &height, &channels, 4);

    if (pixels == nullptr)
    {
        return false;
    }

    unsigned int texture = 0;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // Preserve the source pixel art when tiles are scaled.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, pixels);
    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(pixels);
    m_textures[type] = { texture, width, height };
    return true;
}

//! Release all loaded textures, deleting the OpenGL texture objects and
//! clearing the texture map. This function should be called when the level
//! editor is closed or when textures need to be reloaded to free GPU resources.
void LevelEditor::ReleaseTextures()
{
    for (const auto& [type, texture] : m_textures)
    {
        (void)type;

        if (texture.id != 0)
        {
            const unsigned int id = texture.id;
            glDeleteTextures(1, &id);
        }
    }

    m_textures.clear();
}

//! Get a pointer to the sprite texture for the given object type. If the type
//! is not found in the texture map, returns nullptr. This function allows
//! read-only access to the texture data for rendering purposes.
const LevelEditor::SpriteTexture* LevelEditor::TextureFor(ObjectType type) const
{
    const auto iter = m_textures.find(type);
    return iter == m_textures.end() ? nullptr : &iter->second;
}

//! Get the file path for the sprite image corresponding to the given object
//! entry. The function constructs the path based on the asset directory, object
//! type (text or icon), and the entry's name, appending the ".gif" extension.
//! This path is used to load the sprite image for rendering in the level
//! editor.
std::string LevelEditor::SpritePath(const ObjectEntry& entry) const
{
    // BabaGUI keeps text and object GIFs in parallel asset directories.
    std::string path = BABA_EDITOR_SPRITES_DIR;
    path += IsTextType(entry.type) ? "text/" : "icon/";
    path += entry.name;
    path += ".gif";
    return path;
}
}  // namespace baba_is_auto::editor
