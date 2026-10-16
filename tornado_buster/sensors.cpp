//
// Created by toot on 10/16/2026.
//
#include "sensors.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

namespace {

constexpr int kHistory = 120;   // samples kept for the graphs (20 Hz -> 6 s)

struct Sensor {
    const char* name;
    const char* unit;
    float minV, maxV;           // display range
    float base, amp, speed, phase;   // placeholder signal shape
    int   dec;                  // decimals to show
    bool  online = true;
    float value  = 0.0f;
    std::array<float, kHistory> hist{};
};

std::vector<Sensor> g_sensors = {
    {"Wind Speed",     "m/s",  0.f,   60.f,  24.f,   14.f,  0.7f, 0.f, 1},
    {"Pressure",       "hPa",  960.f, 1040.f, 1002.f, 8.f,  0.3f, 1.f, 1},
    {"Temperature",    "C",    -10.f, 45.f,  21.f,   4.f,   0.2f, 2.f, 1},
    {"Humidity",       "%",    0.f,   100.f, 64.f,   15.f,  0.25f, 3.f, 1},
    {"Altitude",       "m",    0.f,   500.f, 120.f,  40.f,  0.15f, 4.f, 0},
    {"Vorticity",      "1/s",  0.f,   0.5f,  0.18f,  0.1f,  0.5f, 5.f, 2},
    {"GPS Satellites", "sats", 0.f,   16.f,  11.f,   2.f,   0.1f, 6.f, 0},
    {"Lidar Range",    "m",    0.f,   100.f, 42.f,   18.f,  0.4f, 7.f, 1},
};

float g_sampleTimer = 0.0f;

const char* FormatValue(const Sensor& s) {
    return TextFormat("%.*f %s", s.dec, s.value, s.unit);
}

float Frac(const Sensor& s) {
    return (s.value - s.minV) / (s.maxV - s.minV);
}

} // namespace

void SensorsUpdate(float dt) {
    // --- PLACEHOLDER: fake signals. Replace with real telemetry later. ---
    float time = (float)GetTime();
    for (auto& s : g_sensors) {
        float v = s.base + s.amp * std::sin(time * s.speed + s.phase)
                         + 0.2f * s.amp * std::sin(time * s.speed * 3.1f + s.phase * 2.0f);
        s.value = std::clamp(v, s.minV, s.maxV);
    }
    // history ring for graphs, sampled at 20 Hz
    g_sampleTimer += dt;
    while (g_sampleTimer >= 0.05f) {
        g_sampleTimer -= 0.05f;
        for (auto& s : g_sensors) {
            std::copy(s.hist.begin() + 1, s.hist.end(), s.hist.begin());
            s.hist.back() = s.value;
        }
    }
}

// Compact grid of readouts for the Overview tab.
void SensorsDrawSummary(Rectangle a, const Theme& t) {
    if (a.width < 40 || a.height < 20) return;
    const float gap = 6.0f;
    int n    = (int)g_sensors.size();
    int cols = std::max(1, (int)(a.width / 130));
    int rows = (n + cols - 1) / cols;
    float cw = (a.width - gap * (cols - 1)) / cols;
    float ch = std::min(64.0f, (a.height - gap * (rows - 1)) / rows);

    BeginClip(a);
    for (int i = 0; i < n; i++) {
        const Sensor& s = g_sensors[i];
        Rectangle r{a.x + (i % cols) * (cw + gap), a.y + (i / cols) * (ch + gap), cw, ch};
        DrawRectangleRounded(r, RoundPx(r, 4), 6, t.panelAlt);
        DrawRectangleLinesEx(r, 1, t.border);
        DrawText(s.name, (int)r.x + 6, (int)r.y + 4, 13, t.textDim);
        DrawText(FormatValue(s), (int)r.x + 6, (int)r.y + 19, 18, t.text);
        if (ch >= 52)
            DrawBar({r.x + 6, r.y + ch - 12, cw - 12, 6}, Frac(s), t.accent, t);
    }
    EndScissorMode();
}

// Full Sensors tab: one panel per sensor with a live graph. Scrolls with the mouse wheel.
void SensorsDrawPage(Rectangle a, const Theme& t) {
    if (a.width < 60 || a.height < 40) return;
    static float scroll = 0.0f;
    const float gap = 10.0f, cardH = 150.0f;

    int n    = (int)g_sensors.size();
    int cols = std::max(1, (int)(a.width / 300));
    int rows = (n + cols - 1) / cols;
    float cardW    = (a.width - gap * (cols - 1)) / cols;
    float total    = rows * (cardH + gap) - gap;
    float maxScroll = std::max(0.0f, total - a.height);

    if (CheckCollisionPointRec(GetMousePosition(), a)) scroll += GetMouseWheelMove() * 30.0f;
    scroll = std::clamp(scroll, -maxScroll, 0.0f);

    BeginClip(a);
    for (int i = 0; i < n; i++) {
        const Sensor& s = g_sensors[i];
        Rectangle r{a.x + (i % cols) * (cardW + gap), a.y + (i / cols) * (cardH + gap) + scroll, cardW, cardH};
        if (r.y + r.height < a.y || r.y > a.y + a.height) continue;

        Rectangle c = DrawPanel(r, s.name, t);
        if (c.width <= 0) continue;

        DrawText(FormatValue(s), (int)c.x + 4, (int)c.y + 2, 24, t.text);
        const char* status = s.online ? "ONLINE" : "NO DATA";
        int sw = MeasureText(status, 12);
        DrawText(status, (int)(c.x + c.width - sw - 4), (int)c.y + 8, 12, s.online ? t.good : t.bad);

        // graph
        Rectangle g{c.x, c.y + 34, c.width, c.height - 34};
        DrawRectangleRec(g, t.screen);
        DrawLine((int)g.x, (int)(g.y + g.height / 2), (int)(g.x + g.width), (int)(g.y + g.height / 2), t.grid);
        float span = s.maxV - s.minV;
        float step = g.width / (kHistory - 1);
        Vector2 prev{};
        for (int k = 0; k < kHistory; k++) {
            float f = std::clamp((s.hist[k] - s.minV) / span, 0.0f, 1.0f);
            Vector2 p{g.x + k * step, g.y + g.height - 2 - f * (g.height - 4)};
            if (k > 0) DrawLineEx(prev, p, 2.0f, t.accent);
            prev = p;
        }
        DrawText(TextFormat("%.*f", s.dec, s.maxV), (int)g.x + 3, (int)g.y + 2, 10, t.screenText);
        DrawText(TextFormat("%.*f", s.dec, s.minV), (int)g.x + 3, (int)(g.y + g.height - 12), 10, t.screenText);
        DrawRectangleLinesEx(g, 1, t.border);
    }
    EndScissorMode();
}