#include "raylib.h"
#include "theme.h"
#include "sensors.h"

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

// ===========================================================================
// App state (placeholders - wire these to the real drone later)
// ===========================================================================
enum class Page { Overview = 0, CameraFeed, Sensors, Console };

static bool  g_connected = true;
static float g_battery   = 78.0f;   // percent

struct LogLine { std::string text; int level; };   // 0 info, 1 warn, 2 error
static std::vector<LogLine> g_log;

static void LogAdd(const std::string& msg, int level = 0) {
    int s = (int)GetTime();
    g_log.push_back({TextFormat("[%02d:%02d] ", (s / 60) % 60, s % 60) + msg, level});
    if (g_log.size() > 200) g_log.erase(g_log.begin());
}

static void LogPlaceholderTick() {
    static const struct { const char* msg; int lvl; } kMsgs[] = {
        {"Telemetry packet received", 0}, {"GPS lock: 11 satellites", 0},
        {"Wind gusts increasing", 1},     {"Camera 2 frame dropped", 1},
        {"Vorticity threshold approaching", 1}, {"Link RSSI -61 dBm", 0},
        {"Lidar timeout (placeholder error)", 2}, {"Heartbeat OK", 0},
    };
    static float timer = 0.0f; static int idx = 0;
    timer += GetFrameTime();
    if (timer > 2.0f) {
        timer = 0.0f;
        LogAdd(kMsgs[idx].msg, kMsgs[idx].lvl);
        idx = (idx + 1) % (int)(sizeof(kMsgs) / sizeof(kMsgs[0]));
    }
}

// ===========================================================================
// Reusable widgets
// ===========================================================================
static void DrawConsole(Rectangle c, const Theme& t, bool showInput) {
    DrawRectangleRec(c, t.screen);
    DrawRectangleLinesEx(c, 1, t.border);
    BeginClip(c);
    const int lineH = 16;
    float inputH = showInput ? 28.0f : 0.0f;
    int visible = std::max(0, (int)((c.height - 10 - inputH) / lineH));
    int start   = std::max(0, (int)g_log.size() - visible);
    float y = c.y + 5;
    for (int i = start; i < (int)g_log.size(); i++, y += lineH) {
        Color col = g_log[i].level == 2 ? t.bad : g_log[i].level == 1 ? t.warn : t.screenText;
        DrawText(g_log[i].text.c_str(), (int)c.x + 6, (int)y, 14, col);
    }
    if (showInput) {
        Rectangle in{c.x + 4, c.y + c.height - inputH, c.width - 8, inputH - 4};
        DrawRectangleRec(in, Fade(WHITE, 0.06f));
        DrawText(">", (int)in.x + 6, (int)in.y + 5, 16, t.good);
        DrawText("type a command... (placeholder)", (int)in.x + 22, (int)in.y + 6, 14, Fade(t.screenText, 0.5f));
    }
    EndScissorMode();
}

static void DrawMap(Rectangle c, const Theme& t) {
    DrawRectangleRec(c, t.screen);
    BeginClip(c);
    for (float x = c.x; x < c.x + c.width; x += 32)  DrawLine((int)x, (int)c.y, (int)x, (int)(c.y + c.height), t.grid);
    for (float y = c.y; y < c.y + c.height; y += 32) DrawLine((int)c.x, (int)y, (int)(c.x + c.width), (int)y, t.grid);

    float m = std::min(c.width, c.height);
    DrawCircleV({c.x + c.width * 0.25f, c.y + c.height * 0.65f}, m * 0.22f, Fade(t.good, 0.10f));
    DrawCircleV({c.x + c.width * 0.60f, c.y + c.height * 0.80f}, m * 0.15f, Fade(t.good, 0.08f));
    DrawCircleV({c.x + c.width * 0.80f, c.y + c.height * 0.25f}, m * 0.18f, Fade(t.titleA, 0.10f));

    Vector2 home{c.x + c.width * 0.2f, c.y + c.height * 0.75f};
    Vector2 storm{c.x + c.width * 0.75f, c.y + c.height * 0.3f};
    float a = (float)GetTime() * 0.4f;
    Vector2 drone{c.x + c.width * 0.46f + std::cos(a) * c.width * 0.08f,
                  c.y + c.height * 0.52f + std::sin(a) * c.height * 0.08f};

    DrawLineEx(home, drone, 2.0f, Fade(t.accent, 0.8f));
    DrawCircleLines((int)storm.x, (int)storm.y, 22, Fade(t.bad, 0.5f));
    DrawCircleLines((int)storm.x, (int)storm.y, 14, Fade(t.bad, 0.8f));
    DrawCircleV(storm, 6, t.bad);
    DrawText("STORM", (int)storm.x + 10, (int)storm.y + 24, 12, t.screenText);

    DrawRectangle((int)home.x - 5, (int)home.y - 5, 10, 10, t.warn);
    DrawText("HOME", (int)home.x + 8, (int)home.y - 6, 12, t.screenText);

    DrawTriangle({drone.x, drone.y - 9}, {drone.x - 7, drone.y + 7}, {drone.x + 7, drone.y + 7}, t.accentHover);
    DrawText("DRONE", (int)drone.x + 10, (int)drone.y - 6, 12, t.screenText);

    DrawText("N", (int)(c.x + c.width - 18), (int)c.y + 6, 16, t.screenText);
    DrawText("MAP PLACEHOLDER", (int)c.x + 6, (int)(c.y + c.height - 16), 12, Fade(t.screenText, 0.6f));
    EndScissorMode();
    DrawRectangleLinesEx(c, 1, t.border);
}

static void DrawFeed(Rectangle r, const char* label, const Theme& t, bool big = false) {
    if (r.width < 8 || r.height < 8) return;
    DrawRectangleRec(r, t.screen);
    BeginClip(r);
    DrawLine((int)r.x, (int)r.y, (int)(r.x + r.width), (int)(r.y + r.height), Fade(t.screenText, 0.15f));
    DrawLine((int)(r.x + r.width), (int)r.y, (int)r.x, (int)(r.y + r.height), Fade(t.screenText, 0.15f));

    int fs = (int)std::clamp(r.height * 0.12f, 10.0f, 22.0f);
    const char* msg = "NO SIGNAL";
    DrawText(msg, (int)(r.x + (r.width - MeasureText(msg, fs)) / 2), (int)(r.y + (r.height - fs) / 2), fs, t.screenText);
    DrawText(label, (int)r.x + 6, (int)r.y + 5, std::max(10, fs - 4), Fade(t.screenText, 0.8f));
    if (big && std::fmod(GetTime(), 1.2) < 0.7)
        DrawCircleV({r.x + r.width - 14, r.y + 14}, 5, t.bad);
    EndScissorMode();
    DrawRectangleLinesEx(r, 1, t.border);
}

// 4 small feeds on top, one large 16:9 feed below (shared by Overview + Camera tab)
static void DrawCameraGrid(Rectangle c, const Theme& t) {
    const float gap = 8.0f;
    float sw  = (c.width - gap * 3) / 4;
    float smH = std::min(sw * 9.0f / 16.0f, c.height * 0.25f);
    for (int i = 0; i < 4; i++)
        DrawFeed({c.x + i * (sw + gap), c.y, sw, smH}, TextFormat("CAM %d", i + 1), t);

    float bh = c.height - smH - gap;
    float bw = bh * 16.0f / 9.0f;
    if (bw > c.width) { bw = c.width; bh = bw * 9.0f / 16.0f; }
    DrawFeed({c.x + (c.width - bw) / 2, c.y + smH + gap, bw, bh}, "MAIN FEED 16:9", t, true);
}

static void DrawStatus(Rectangle c, const Theme& t) {
    BeginClip(c);
    float half = c.width / 2;
    DrawRectangle((int)(c.x + half), (int)c.y + 2, 1, (int)c.height - 4, t.border);

    // --- connection ---
    float x = c.x + 4, y = c.y + 2;
    DrawText("DRONE LINK", (int)x, (int)y, 13, t.textDim);
    Color col = g_connected ? t.good : t.bad;
    float pulse = 0.65f + 0.35f * std::sin((float)GetTime() * 4.0f);
    DrawCircleV({x + 8, y + 31}, 7, Fade(col, g_connected ? pulse : 1.0f));
    DrawText(g_connected ? "CONNECTED" : "DISCONNECTED", (int)x + 22, (int)y + 21, 20, col);
    for (int b = 0; b < 4; b++)
        DrawRectangle((int)x + b * 8, (int)(y + 62 - (b + 1) * 5), 6, (b + 1) * 5, b < 3 ? t.good : t.border);
    DrawText("42 ms  |  -61 dBm", (int)x + 40, (int)y + 48, 14, t.textDim);

    // --- battery ---
    float x2 = c.x + half + 10;
    DrawText("BATTERY", (int)x2, (int)y, 13, t.textDim);
    Color bc = g_battery > 50 ? t.good : g_battery > 20 ? t.warn : t.bad;
    DrawText(TextFormat("%d%%", (int)g_battery), (int)x2, (int)y + 16, 28, bc);
    DrawBar({x2, y + 48, half - 24, 14}, g_battery / 100.0f, bc, t);
    DrawText("~ 18 min remaining", (int)x2, (int)y + 68, 13, t.textDim);
    EndScissorMode();
}

// ===========================================================================
// Pages
// ===========================================================================
static void DrawOverview(Rectangle a, const Theme& t) {
    const float pad = 10, gap = 10;
    Rectangle area{a.x + pad, a.y + pad, a.width - 2 * pad, a.height - 2 * pad};

    // ---- left column: console (top) + mini map (below) ----
    float leftW = area.width * 0.38f;
    float consoleH = (area.height - gap) * 0.55f;
    Rectangle consoleR{area.x, area.y, leftW, consoleH};
    Rectangle mapR{area.x, area.y + consoleH + gap, leftW, area.height - consoleH - gap};

    Rectangle c = DrawPanel(consoleR, "Console", t);
    if (c.width > 0) DrawConsole(c, t, false);
    c = DrawPanel(mapR, "Mini Map", t);
    if (c.width > 0) DrawMap(c, t);

    // ---- right column: cameras, sensors, then status in whatever is left ----
    Rectangle col{area.x + leftW + gap, area.y, area.width - leftW - gap, area.height};

    float cw    = col.width - 12;
    float sw    = (cw - 24) / 4;
    float ideal = kTitleH + 11 + sw * 9 / 16 + 8 + cw * 9 / 16;       // full-width 16:9 feed
    float camH  = std::min(ideal, col.height * 0.60f);
    float sensH = std::clamp(col.height * 0.22f, 84.0f, 190.0f);
    float statH = col.height - camH - sensH - 2 * gap;
    if (statH < 96) {                                                  // keep status readable
        statH = 96;
        camH  = std::max(110.0f, col.height - sensH - statH - 2 * gap);
    }

    Rectangle camR{col.x, col.y, col.width, camH};
    Rectangle senR{col.x, col.y + camH + gap, col.width, sensH};
    Rectangle staR{col.x, senR.y + sensH + gap, col.width, std::max(0.0f, col.y + col.height - (senR.y + sensH + gap))};

    c = DrawPanel(camR, "Camera Feeds", t);
    if (c.width > 0) DrawCameraGrid(c, t);
    c = DrawPanel(senR, "Sensors", t);
    if (c.width > 0) SensorsDrawSummary(c, t);
    c = DrawPanel(staR, "System Status", t);
    if (c.width > 0) DrawStatus(c, t);
}

static void DrawCameraPage(Rectangle a, const Theme& t) {
    Rectangle r{a.x + 10, a.y + 10, a.width - 20, a.height - 20};
    Rectangle c = DrawPanel(r, "Camera Feeds", t);
    if (c.width > 0) DrawCameraGrid(c, t);
}

static void DrawConsolePage(Rectangle a, const Theme& t) {
    Rectangle r{a.x + 10, a.y + 10, a.width - 20, a.height - 20};
    Rectangle c = DrawPanel(r, "Console", t);
    if (c.width > 0) DrawConsole(c, t, true);
}

// ===========================================================================
// Sidebar
// ===========================================================================
struct Sidebar {
    float width = 210.0f;
    bool  dragging = false;
};

static void DrawNavIcon(int idx, float x, float y, Color c) {
    switch (idx) {
        case 0: for (int i = 0; i < 4; i++) DrawRectangle((int)x + (i % 2) * 10, (int)y + (i / 2) * 10, 8, 8, c); break;
        case 1: DrawRectangleLinesEx({x, y + 3, 18, 12}, 2, c); DrawCircleV({x + 9, y + 9}, 3, c); break;
        case 2: DrawRectangle((int)x + 1, (int)y + 9, 4, 9, c); DrawRectangle((int)x + 7, (int)y + 4, 4, 14, c);
                DrawRectangle((int)x + 13, (int)y, 4, 18, c); break;
        case 3: DrawText(">_", (int)x, (int)y + 1, 16, c); break;
    }
}

// Returns the page chosen; sets toggleTheme when the theme button is clicked.
static Page DrawSidebar(Sidebar& sb, Page current, const Theme& t, int H, bool& toggleTheme) {
    static const char* kNames[] = {"Overview", "Camera Feed", "Sensors", "Console"};
    Vector2 m = GetMousePosition();

    DrawRectangle(0, 0, (int)sb.width, H, t.sidebar);
    BeginClip({0, 0, sb.width, (float)H});

    // header
    DrawRectangleGradientV(0, 0, (int)sb.width, 64, t.titleA, t.titleB);
    DrawRectangle(0, 0, (int)sb.width, 1, Fade(WHITE, 0.4f));
    DrawText("TORNADO", 14, 10, 24, t.titleText);
    DrawText("BUSTER", 14, 36, 18, Fade(t.titleText, 0.85f));

    // nav items
    Page result = current;
    for (int i = 0; i < 4; i++) {
        Rectangle r{8, 76.0f + i * 46.0f, sb.width - 16, 42};
        bool hover = CheckCollisionPointRec(m, r) && !sb.dragging;
        bool sel   = (int)current == i;
        if (sel)        DrawRectangleRounded(r, RoundPx(r, 8), 8, t.selected);
        else if (hover) DrawRectangleRounded(r, RoundPx(r, 8), 8, Fade(t.selected, 0.5f));
        if (sel) DrawRectangleRounded({r.x, r.y + 6, 4, r.height - 12}, 1.0f, 4, t.accent);
        Color ic = sel ? t.text : t.textDim;
        DrawNavIcon(i, r.x + 14, r.y + 12, sel ? t.accentHover : ic);
        DrawText(kNames[i], (int)r.x + 44, (int)r.y + 12, 18, ic);
        if (hover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) result = (Page)i;
    }

    // theme toggle
    Rectangle tb{10, (float)H - 50, sb.width - 20, 36};
    if (DrawButton(tb, t.dark ? "Light Mode" : "Dark Mode", t)) toggleTheme = true;
    EndScissorMode();

    // edge + resize handle
    DrawRectangle((int)sb.width - 1, 0, 1, H, t.border);
    Rectangle handle{sb.width - 4, 0, 8, (float)H};
    bool hot = CheckCollisionPointRec(m, handle);
    if (hot && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) sb.dragging = true;
    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) sb.dragging = false;
    if (sb.dragging) sb.width = m.x;
    if (hot || sb.dragging) DrawRectangle((int)sb.width - 2, 0, 3, H, t.accent);
    SetMouseCursor((hot || sb.dragging) ? MOUSE_CURSOR_RESIZE_EW : MOUSE_CURSOR_DEFAULT);
    return result;
}

// ===========================================================================
int main() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);
    InitWindow(1280, 720, "tornado_buster.exe");
    SetWindowMinSize(480, 320);
    SetTargetFPS(60);


    bool dark = true;
    Theme theme = MakeTheme(dark);
    Sidebar sb;
    Page page = Page::Overview;

    LogAdd("Tornado Buster ground station started");
    LogAdd("Waiting for drone link...");
    LogAdd("Drone connected (placeholder)");
    LogAdd("Sensors online: 8/8");

    while (!WindowShouldClose()) {
        const int W = GetScreenWidth();
        const int H = GetScreenHeight();

        SensorsUpdate(GetFrameTime());
        LogPlaceholderTick();

        BeginDrawing();
        ClearBackground(theme.bg);

        // keep the sidebar within sane limits for the current window size
        sb.width = std::clamp(sb.width, 150.0f, std::max(150.0f, std::min(420.0f, W * 0.5f)));

        bool toggle = false;
        page = DrawSidebar(sb, page, theme, H, toggle);
        sb.width = std::clamp(sb.width, 150.0f, std::max(150.0f, std::min(420.0f, W * 0.5f)));

        Rectangle area{sb.width, 0, W - sb.width, (float)H};
        switch (page) {
            case Page::Overview:   DrawOverview(area, theme);   break;
            case Page::CameraFeed: DrawCameraPage(area, theme); break;
            case Page::Sensors:    SensorsDrawPage({area.x + 10, area.y + 10, area.width - 20, area.height - 20}, theme); break;
            case Page::Console:    DrawConsolePage(area, theme); break;
        }
        EndDrawing();

        if (toggle) { dark = !dark; theme = MakeTheme(dark); }
    }
    CloseWindow();
    return 0;
}