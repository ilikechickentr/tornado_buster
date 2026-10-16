#pragma once
#include "raylib.h"
#include <algorithm>

// ---------------------------------------------------------------------------
// Theme: "WindowsXP meets modern gamey" - glossy blue title bars, green
// accent buttons, medium-contrast panels. Two palettes: dark and light.
// ---------------------------------------------------------------------------
struct Theme {
    bool  dark;
    Color bg, sidebar, panel, panelAlt;
    Color titleA, titleB, titleText;
    Color text, textDim, border;
    Color accent, accentHover, selected;
    Color good, warn, bad;
    Color screen, screenText, grid;   // "monitor" surfaces (feeds, map, console)
};

inline Theme MakeTheme(bool dark) {
    Theme t{};
    t.dark = dark;
    if (dark) {
        t.bg        = {26, 31, 44, 255};
        t.sidebar   = {33, 40, 60, 255};
        t.panel     = {42, 50, 72, 255};
        t.panelAlt  = {33, 40, 58, 255};
        t.titleA    = {74, 128, 222, 255};
        t.titleB    = {42, 88, 184, 255};
        t.titleText = {240, 245, 255, 255};
        t.text      = {216, 224, 240, 255};
        t.textDim   = {142, 154, 180, 255};
        t.border    = {74, 90, 128, 255};
        t.accent    = {78, 180, 92, 255};
        t.accentHover = {102, 204, 116, 255};
        t.selected  = {56, 76, 120, 255};
        t.good      = {110, 208, 124, 255};
        t.warn      = {238, 190, 76, 255};
        t.bad       = {232, 102, 102, 255};
        t.screen    = {15, 19, 30, 255};
        t.grid      = {40, 52, 76, 255};
    } else {
        t.bg        = {206, 218, 238, 255};
        t.sidebar   = {190, 206, 232, 255};
        t.panel     = {238, 243, 251, 255};
        t.panelAlt  = {222, 231, 245, 255};
        t.titleA    = {92, 148, 236, 255};
        t.titleB    = {56, 108, 204, 255};
        t.titleText = {255, 255, 255, 255};
        t.text      = {34, 44, 66, 255};
        t.textDim   = {92, 106, 134, 255};
        t.border    = {132, 154, 192, 255};
        t.accent    = {64, 164, 80, 255};
        t.accentHover = {84, 186, 100, 255};
        t.selected  = {170, 192, 230, 255};
        t.good      = {46, 150, 66, 255};
        t.warn      = {200, 140, 24, 255};
        t.bad       = {200, 66, 66, 255};
        t.screen    = {34, 44, 66, 255};
        t.grid      = {52, 66, 94, 255};
    }
    t.screenText = {160, 176, 208, 255};
    return t;
}

// ---------------------------------------------------------------------------
// Small helpers
// ---------------------------------------------------------------------------
constexpr float kTitleH = 26.0f;

inline Color Shade(Color c, float f) {
    return { (unsigned char)std::clamp(c.r * f, 0.0f, 255.0f),
             (unsigned char)std::clamp(c.g * f, 0.0f, 255.0f),
             (unsigned char)std::clamp(c.b * f, 0.0f, 255.0f), c.a };
}

// raylib wants "roundness" (0..1); this converts a corner radius in pixels.
inline float RoundPx(Rectangle r, float px) {
    float m = std::min(r.width, r.height);
    return m <= 0.0f ? 0.0f : std::min(1.0f, 2.0f * px / m);
}

inline void BeginClip(Rectangle r) {
    BeginScissorMode((int)r.x, (int)r.y, std::max(0, (int)r.width), std::max(0, (int)r.height));
}

// XP-style window panel: rounded frame + glossy gradient title strip.
// Returns the content rectangle (width == 0 if the panel is too small to draw).
inline Rectangle DrawPanel(Rectangle r, const char* title, const Theme& t) {
    if (r.width < 24 || r.height < kTitleH + 12) return {r.x, r.y, 0, 0};

    DrawRectangleRounded(r, RoundPx(r, 6), 8, t.border);
    Rectangle in{r.x + 1, r.y + 1, r.width - 2, r.height - 2};
    DrawRectangleRounded(in, RoundPx(in, 5), 8, t.panel);

    Rectangle tb{in.x + 3, in.y + 3, in.width - 6, kTitleH - 3};
    DrawRectangleGradientV((int)tb.x, (int)tb.y, (int)tb.width, (int)tb.height, t.titleA, t.titleB);
    DrawRectangle((int)tb.x, (int)tb.y, (int)tb.width, 1, Fade(WHITE, 0.35f));

    BeginClip(tb);
    DrawText(title, (int)tb.x + 8, (int)(tb.y + (tb.height - 16) / 2), 16, t.titleText);
    EndScissorMode();

    // Decorative min / max / close buttons
    if (tb.width > 180) {
        float bx = tb.x + tb.width - 18;
        float by = tb.y + (tb.height - 14) / 2;
        DrawRectangleRounded({bx, by, 14, 14}, 0.3f, 4, Color{214, 84, 70, 255});
        bx -= 18; DrawRectangleRounded({bx, by, 14, 14}, 0.3f, 4, Fade(WHITE, 0.30f));
        bx -= 18; DrawRectangleRounded({bx, by, 14, 14}, 0.3f, 4, Fade(WHITE, 0.30f));
    }
    return {r.x + 6, r.y + kTitleH + 5, r.width - 12, r.height - kTitleH - 11};
}

// Glossy green XP-"Start" style button. Returns true when clicked.
inline bool DrawButton(Rectangle r, const char* label, const Theme& t) {
    bool hover = CheckCollisionPointRec(GetMousePosition(), r);
    bool down  = hover && IsMouseButtonDown(MOUSE_BUTTON_LEFT);
    if (down) r.y += 1;
    Color base = hover ? t.accentHover : t.accent;

    DrawRectangleRounded(r, RoundPx(r, 8), 8, Shade(base, 0.6f));
    Rectangle in{r.x + 1.5f, r.y + 1.5f, r.width - 3, r.height - 3};
    DrawRectangleRounded(in, RoundPx(in, 7), 8, base);
    DrawRectangleGradientV((int)in.x + 5, (int)in.y + 2, (int)in.width - 10, (int)in.height / 2 - 2,
                           Fade(WHITE, 0.35f), Fade(WHITE, 0.0f));

    int tw = MeasureText(label, 16);
    float tx = r.x + (r.width - tw) / 2 + 8;
    float ty = r.y + (r.height - 16) / 2;
    DrawText(label, (int)tx + 1, (int)ty + 1, 16, Fade(BLACK, 0.35f));
    DrawText(label, (int)tx, (int)ty, 16, WHITE);

    // little sun / moon icon
    Vector2 ic{r.x + 18, r.y + r.height / 2};
    if (t.dark) {                       // button switches TO light -> sun
        DrawCircleV(ic, 5, WHITE);
        DrawCircleLines((int)ic.x, (int)ic.y, 8, Fade(WHITE, 0.8f));
    } else {                            // button switches TO dark -> moon
        DrawCircleV(ic, 7, WHITE);
        DrawCircleV({ic.x + 4, ic.y - 3}, 6, base);
    }
    return hover && IsMouseButtonReleased(MOUSE_BUTTON_LEFT);
}

// Rounded progress bar.
inline void DrawBar(Rectangle r, float frac, Color fill, const Theme& t) {
    if (r.width < 4 || r.height < 4) return;
    frac = std::clamp(frac, 0.0f, 1.0f);
    DrawRectangleRounded(r, RoundPx(r, r.height / 2), 6, t.border);
    Rectangle in{r.x + 1, r.y + 1, r.width - 2, r.height - 2};
    DrawRectangleRounded(in, RoundPx(in, in.height / 2), 6, t.screen);
    Rectangle f{in.x, in.y, in.width * frac, in.height};
    if (f.width > 2) DrawRectangleRounded(f, RoundPx(f, f.height / 2), 6, fill);
}