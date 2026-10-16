#pragma once
#include "raylib.h"
#include "theme.h"

// Sensors module (implemented in sensors.cpp).
// Right now every value is simulated - replace the body of SensorsUpdate()
// with real telemetry when the drone link exists.

void SensorsUpdate(float dt);                              // call once per frame
void SensorsDrawSummary(Rectangle area, const Theme& t);   // compact readout (Overview tab)
void SensorsDrawPage(Rectangle area, const Theme& t);      // full page with graphs (Sensors tab)