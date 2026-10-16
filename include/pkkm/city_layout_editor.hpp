#pragma once
#ifdef _WIN32
#include "pkkm/kingdom.hpp"
#include <windows.h>
#include <functional>

namespace pkkm::ui {
bool open_city_layout_worksheet(HWND owner, HINSTANCE instance,
    const CityLayoutWorksheet& initial,
    std::function<void(CityLayoutWorksheet)> on_apply);
void close_city_layout_worksheet(HWND owner);
}
#endif