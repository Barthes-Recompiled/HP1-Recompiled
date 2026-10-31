// The options panel on F10, after SCDA-XBERecomp's (src/app/menu.cpp): one tab per page of the settings' row table
// (rows.cpp, shared with the game's own Settings pages), each row stepped left / right, applied at once and saved to
// hpps.toml. Alt+Enter flips windowed and borderless. The fps counter draws here too.
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <cstring>
#include <windows.h>

#include <algorithm>
#include <atomic>
#include <cfloat>
#include <cstdio>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

#include <imgui.h>
#include <backends/imgui_impl_win32.h>

#include "app/app.h"
#include "core/log.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND, UINT, WPARAM, LPARAM);
extern "C" void xbox_FramebufferWindowSetMsgHook(int (*fn)(void*, unsigned, uintptr_t, intptr_t));

namespace app {

namespace {

std::atomic<bool> g_open{false};
// xboxrecomp: the Archipelago travel window, F10 > Debug (the game's own is the pause menu's Travel page)
std::atomic<bool> g_travel{false};
std::atomic<bool> g_start{false};  // xboxrecomp: the start menu is up, the title waits for it
std::atomic<bool> g_ui{false};  // ImGui is set up

// a setting changed: applied by the renderer next frame, kept in hpps.toml
void Changed(bool display = false) {
    Save();
    if (display) ApplyDisplay();
}

void Section(const char* text) {
    ImGui::Spacing();
    ImGui::SeparatorText(text);
}

// xboxrecomp: a value row as SCDA's OptionRow: the label left, the value between arrows right. The row is the one nav
// stop (left / right on it step the value, held arrows keep stepping); -1 or 1 when it was stepped
int OptionRow(const char* label, const char* value) {
    const ImGuiStyle& style = ImGui::GetStyle();
    float scale = ImGui::GetIO().FontGlobalScale, arrow = ImGui::GetFrameHeight();
    float value_w = 220 * scale, block_w = value_w + 2 * (arrow + style.ItemSpacing.x);
    ImVec2 at = ImGui::GetCursorPos();
    float row_w = std::max(ImGui::GetContentRegionAvail().x, 440 * scale);
    ImGui::PushID(label);
    int step = 0;
    ImGui::Selectable("##row", false, ImGuiSelectableFlags_AllowOverlap, ImVec2(row_w, arrow));
    if (ImGui::IsItemFocused()) {
        if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow) || ImGui::IsKeyPressed(ImGuiKey_GamepadDpadLeft)) step = -1;
        if (ImGui::IsKeyPressed(ImGuiKey_RightArrow) || ImGui::IsKeyPressed(ImGuiKey_GamepadDpadRight)) step = 1;
    }
    ImGui::SetCursorPos(at);
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted(label);
    ImGui::PushItemFlag(ImGuiItemFlags_NoNav | ImGuiItemFlags_ButtonRepeat, true);
    float x = at.x + row_w - block_w;
    ImGui::SetCursorPos(ImVec2(x, at.y));
    if (ImGui::ArrowButton("##dec", ImGuiDir_Left)) step = -1;
    float pad = std::max((value_w - ImGui::CalcTextSize(value).x) * 0.5f, 0.0f);
    ImGui::SetCursorPos(ImVec2(x + arrow + style.ItemSpacing.x + pad, at.y));
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted(value);
    ImGui::SetCursorPos(ImVec2(x + arrow + value_w + 2 * style.ItemSpacing.x, at.y));
    if (ImGui::ArrowButton("##inc", ImGuiDir_Right)) step = 1;
    ImGui::PopItemFlag();
    ImGui::PopID();
    return step;
}

// xboxrecomp: the Controls tab's keyboard and mouse bindings: click, then press a key or a mouse button; Escape clears
void DrawKeyBindings(Settings& s, const char* label) {
    if (!ImGui::TreeNode(label)) return;
    if (ImGui::BeginTable("##keys", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp)) {
        for (uint32_t i = 0; i < kKeyBinds; i++) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(kKeyBindNames[i]);
            ImGui::TableNextColumn();
            ImGui::PushID(int(i));
            bool waiting = KeyCaptureBinding() == int(i);
            std::string text = waiting ? "Press a key or mouse button (Esc clears)" : KeyName(s.keys[i]);
            if (ImGui::Button(text.c_str(), ImVec2(-1, 0))) {
                if (waiting) CancelKeyCapture();
                else StartKeyCapture(int(i));
            }
            ImGui::PopID();
        }
        ImGui::EndTable();
    }
    if (ImGui::Button("Default keys")) {
        Settings d;
        std::copy(std::begin(d.keys), std::end(d.keys), std::begin(s.keys));
        Changed();
    }
    ImGui::TreePop();
}

// xboxrecomp: the controller's remapping: the pad button each game button reads
void DrawPadMap(Settings& s, const char* label) {
    if (!ImGui::TreeNode(label)) return;
    ImGui::TextDisabled("The pad button each game button reads (a 360/One pad's LB is White, RB Black).");
    ImGui::PushItemWidth(160 * ImGui::GetIO().FontGlobalScale);
    for (uint32_t i = 0; i < kPadInputs; i++) {
        int m = int(std::min(s.pad_map[i], kPadInputs - 1));
        std::string name = std::string("Game ") + kPadInputNames[i];
        if (ImGui::Combo(name.c_str(), &m, kPadInputNames, int(kPadInputs))) {
            s.pad_map[i] = uint32_t(m);
            Changed();
        }
    }
    ImGui::PopItemWidth();
    if (ImGui::Button("Default buttons")) {
        for (uint32_t i = 0; i < kPadInputs; i++) s.pad_map[i] = i;
        Changed();
    }
    ImGui::TreePop();
}

// xboxrecomp: one F10 tab: its rows from the shared table (rows.cpp), as the game's own Settings pages list them
void DrawRows(Settings& s, int page) {
    size_t count = 0;
    const rows::Row* list = rows::Page(page, &count);
    for (size_t i = 0; i < count; i++) {
        const rows::Row& r = list[i];
        if (r.kind == rows::Kind::Heading) {
            Section(r.label);
            continue;
        }
        if (!rows::Shown(r)) continue;
        bool enabled = rows::Enabled(r);
        ImGui::BeginDisabled(!enabled);
        if (r.kind == rows::Kind::Keys) {
            DrawKeyBindings(s, r.label);
        } else if (r.kind == rows::Kind::PadMap) {
            DrawPadMap(s, r.label);
        } else if (int step = OptionRow(r.label, rows::Value(r).c_str()); step && enabled) {
            rows::Step(r, step);
        }
        ImGui::EndDisabled();
        if (const char* note = rows::Note(r)) ImGui::TextDisabled("%s", note);
    }
}

// start: the start menu (the aspect ratio applies at once, Start game instead of Close); otherwise F10's panel
void DrawPanel(bool start) {
    Settings& s = Get();
    ImGuiIO& io = ImGui::GetIO();
    uint32_t vk;
    if (int b = KeyCaptureBinding(); b >= 0 && TakeCapturedKey(&vk)) {
        s.keys[b] = vk;
        Changed();
    }
    // centred every frame: the rows' height settles a few frames after it opens
    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always,
                            ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(560 * io.FontGlobalScale, 0), ImGuiCond_Appearing);
    bool open = true;
    const char* title = start ? "Harry Potter and the Philosopher's Stone - Settings###start" : "Options (F10)";
    if (!ImGui::Begin(title, start ? nullptr : &open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::End();
        if (!open) g_open = false;
        return;
    }

    static int page = rows::kGraphics;  // the open tab, for Defaults
    if (ImGui::BeginTabBar("##tabs")) {
        for (int p = 0; p < rows::kPages; p++) {
            if (!ImGui::BeginTabItem(rows::PageName(p))) continue;
            page = p;
            // the rows scroll past two thirds of the screen's height (Graphics at 720 lines)
            ImGui::SetNextWindowSizeConstraints(ImVec2(0, 0), ImVec2(FLT_MAX, io.DisplaySize.y * 0.66f));
            ImGui::BeginChild("##rows", ImVec2(540 * io.FontGlobalScale, 0),
                              ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_NavFlattened);
            DrawRows(s, p);
            ImGui::EndChild();
            if (p == rows::kDebug) {
                // xboxrecomp: the Archipelago travel window (the game's own is the pause menu's Travel page)
                if (ImGui::Button("Archipelago travel window")) {
                    g_travel = true;
                    g_open = false;
                }
                ImGui::TextDisabled("The pause menu's Hogwarts Travel page as a PC window (both triggers + Back opens "
                                    "the page).");
            }
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }

    ImGui::Spacing();
    ImGui::Separator();
    if (ImGui::Button("Defaults (this page)")) rows::Defaults(page);
    ImGui::SameLine();
    if (start) {
        if (ImGui::Checkbox("Show this page at startup", &s.start_menu)) Changed();
        ImGui::Spacing();
        if (ImGui::Button("Start game", ImVec2(160 * io.FontGlobalScale, 0))) {
            SetBootAspect(s.aspect);
            Changed();
            g_start = false;
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        ImGui::TextDisabled("Saved to hpps.toml; F10 in game changes them too");
    } else {
        if (ImGui::Button("Close")) open = false;
        ImGui::SameLine();
        if (ImGui::Checkbox("Show the launcher at startup", &s.show_launcher)) Changed();
        ImGui::SameLine();
        ImGui::TextDisabled("Saved to hpps.toml");
    }
    ImGui::End();
    if (!open) g_open = false;
}

void DrawFps() {
    char text[32];
    std::snprintf(text, sizeof(text), "%.0f FPS", g_fps.load());
    ImDrawList* dl = ImGui::GetForegroundDrawList();
    float sz = ImGui::GetFontSize() * 1.25f;
    ImVec2 p(12, 8);
    dl->AddText(ImGui::GetFont(), sz, ImVec2(p.x + 1, p.y + 1), IM_COL32(0, 0, 0, 200), text);
    dl->AddText(ImGui::GetFont(), sz, p, IM_COL32(255, 255, 80, 255), text);
}

// the window thread's messages: the pointer hidden, F10 and Alt+Enter, then ImGui; while the panel is open it keeps the
// keyboard and mouse
int MsgHook(void* hwnd, unsigned msg, uintptr_t wp, intptr_t lp) {
    InputMessage(hwnd, msg, wp, lp);  // xboxrecomp: keyboard and mouse as controller 1 (input.cpp)
    // xboxrecomp: no mouse pointer over the game; the panel shows it again
    if (msg == WM_SETCURSOR && LOWORD(lp) == HTCLIENT && !g_open && !g_start) {
        SetCursor(nullptr);
        return 1;
    }
    bool down = msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN;
    if (down && wp == VK_F10) {
        if (!(lp & (1 << 30)) && !g_start) ToggleMenu();  // not on auto-repeat; the start menu is already up
        return 1;
    }
    if (msg == WM_SYSKEYDOWN && wp == VK_RETURN && (lp & (1 << 29))) {
        if (!(lp & (1 << 30))) {
            Settings& s = Get();
            s.display_mode = s.display_mode == DisplayMode::Windowed ? DisplayMode::Borderless : DisplayMode::Windowed;
            Changed(true);
        }
        return 1;
    }
    if (!g_ui) return 0;
    std::lock_guard lock(UiLock());
    ImGui_ImplWin32_WndProcHandler(static_cast<HWND>(hwnd), msg, WPARAM(wp), LPARAM(lp));
    if (!g_open && !g_start) return 0;
    switch (msg) {
        case WM_KEYDOWN: case WM_CHAR: case WM_MOUSEWHEEL: case WM_LBUTTONDOWN: case WM_RBUTTONDOWN:
            return 1;
        case WM_SYSKEYDOWN:
            return wp != VK_F4;  // Alt+F4 still closes (fb_present.c ends the process)
        default:
            return 0;
    }
}

}  // namespace

bool MenuOpen() { return g_open || g_start || g_travel; }

void ToggleMenu() { g_open = !g_open; }

void OpenStartMenu() { g_start = true; }

bool StartMenuOpen() { return g_start; }

}  // namespace app

extern "C" {
int hpps_ap_active(void);
int hpps_ap_travel_entry(int i, const char** name, int* level, const char** script, int* dungeon, int* unlocked);
void hpps_ApTravelRequest(int entry);
void hpps_MenuPadInjected(uint16_t* buttons, uint8_t* a, uint8_t* b);
void app_TravelToggle(void) { app::g_travel = !app::g_travel; }
}

namespace app {

// the days and nights in order, each with its set pieces under it; a locked one is greyed out. A (or a click) goes
// there, B (or Close) shuts the window
void DrawTravel() {
    ImGuiIO& io = ImGui::GetIO();
    // injected test input (hle_input.c: the pad the game last read, only the trainer's input while a panel is up)
    // drives a cursor of its own: the d-pad moves it over the open entries, A goes, B closes. A PC controller works
    // through ImGui's own gamepad navigation as in the F10 panel.
    static int cursor = -1;
    static bool was[4];
    bool edge[4] = {};
    {
        uint16_t buttons;
        uint8_t a, b;
        hpps_MenuPadInjected(&buttons, &a, &b);
        const bool now[4] = {(buttons & 0x1) != 0, (buttons & 0x2) != 0, a > 128, b > 128};
        for (int k = 0; k < 4; k++) edge[k] = now[k] && !was[k], was[k] = now[k];
    }
    struct Row { std::string label; bool dungeon, unlocked; };
    std::vector<Row> rows;
    {
        const char *name, *script;
        int level, dungeon, unlocked;
        for (int i = 0; hpps_ap_travel_entry(i, &name, &level, &script, &dungeon, &unlocked); i++)
            rows.push_back({unlocked ? std::string(name) : std::string(name) + "  (locked)", dungeon != 0,
                            unlocked != 0});
    }
    auto step = [&](int dir) {
        for (int k = 0, c = cursor; k < int(rows.size()); k++) {
            c = (c + dir + int(rows.size())) % int(rows.size());
            if (rows[size_t(c)].unlocked) {
                cursor = c;
                return;
            }
        }
    };
    if (edge[0]) step(-1);
    if (edge[1]) step(+1);

    ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always,
                            ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(io.DisplaySize.x * 0.42f, io.DisplaySize.y * 0.75f), ImGuiCond_Always);
    bool open = true;
    static bool shown;
    if (!shown) ImGui::SetNextWindowFocus();  // a PC controller's d-pad and A go to the window as it opens
    shown = true;
    if (ImGui::Begin("Hogwarts Travel", &open, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize |
                                                    ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings)) {
        if (!hpps_ap_active()) {
            ImGui::TextWrapped("Not connected to an Archipelago slot: days and set pieces open as their unlocks "
                               "arrive.");
        }
        for (int i = 0; i < int(rows.size()); i++) {
            ImGui::PushID(i);
            if (rows[size_t(i)].dungeon) ImGui::Indent(ImGui::GetFontSize() * 1.5f);
            ImGui::BeginDisabled(!rows[size_t(i)].unlocked);
            if (ImGui::Selectable(rows[size_t(i)].label.c_str(), i == cursor) ||
                (edge[2] && i == cursor && rows[size_t(i)].unlocked)) {
                hpps_ApTravelRequest(i);
                open = false;
            }
            ImGui::EndDisabled();
            if (rows[size_t(i)].dungeon) ImGui::Unindent(ImGui::GetFontSize() * 1.5f);
            ImGui::PopID();
        }
        ImGui::Separator();
        if (ImGui::Button("Close") || ImGui::IsKeyPressed(ImGuiKey_GamepadFaceRight) || edge[3]) open = false;
        // xboxrecomp: the port everything here stands on
        ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_TextDisabled));
        ImGui::TextWrapped("Huge thanks to MiddleDucky, whose wonderful Philosopher's Stone port makes all of this "
                           "possible.");
        ImGui::PopStyleColor();
    }
    ImGui::End();
    if (!open) {
        g_travel = false;
        shown = false;
        cursor = -1;
    }
}

void InitUi() {
    std::lock_guard lock(UiLock());
    if (g_ui) return;
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard | ImGuiConfigFlags_NavEnableGamepad |
                      ImGuiConfigFlags_NoMouseCursorChange;
    ImGui::StyleColorsDark();
    ImGui::GetStyle().WindowRounding = 4;
    ImGui_ImplWin32_Init(Window());
    g_ui = true;
    xbox_FramebufferWindowSetMsgHook(MsgHook);
    LOGI("options: F10 opens the panel, Alt+Enter flips windowed / borderless");
}

void DrawUi() {
    InputFrame();
    ImGuiIO& io = ImGui::GetIO();
    // ImGui's 13 px font at 720 lines, bigger with the window
    io.FontGlobalScale = std::max(1.0f, io.DisplaySize.y / 720.0f);
    io.MouseDrawCursor = false;
    if (g_start) DrawPanel(true);
    else if (g_open) DrawPanel(false);
    else if (g_travel) DrawTravel();
    if (Get().fps_counter) DrawFps();
    // xboxrecomp: the keys' pace a moment after the mouse wheel steps it, so a turn of the wheel is seen to do something
    if (GetTickCount64() - g_pace_note.load() < 2000 && !SneakOn()) {
        ImDrawList* dl = ImGui::GetForegroundDrawList();
        float sz = ImGui::GetFontSize() * 1.25f;
        ImVec2 p(12, Get().fps_counter ? 8 + sz * 1.3f : 8);
        char text[32];
        std::snprintf(text, sizeof text, "Keys: %s", kMoveModeNames[std::min<uint32_t>(Get().move_mode, kMoveModes - 1)]);
        dl->AddText(ImGui::GetFont(), sz, ImVec2(p.x + 1, p.y + 1), IM_COL32(0, 0, 0, 200), text);
        dl->AddText(ImGui::GetFont(), sz, p, IM_COL32(255, 255, 255, 255), text);
    }
    if (SneakOn()) {  // xboxrecomp: the sneak toggle is on: a small mark, so it is not left on unknowingly
        ImDrawList* dl = ImGui::GetForegroundDrawList();
        float sz = ImGui::GetFontSize() * 1.25f;
        ImVec2 p(12, Get().fps_counter ? 8 + sz * 1.3f : 8);
        dl->AddText(ImGui::GetFont(), sz, ImVec2(p.x + 1, p.y + 1), IM_COL32(0, 0, 0, 200), "Sneaking");
        dl->AddText(ImGui::GetFont(), sz, p, IM_COL32(170, 200, 255, 255), "Sneaking");
    }
}

}  // namespace app

// hle_input.c: the panel has the controller while it is open
extern "C" int hpps_MenuOpen(void) { return app::MenuOpen(); }
