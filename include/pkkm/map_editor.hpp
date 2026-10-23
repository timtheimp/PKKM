#pragma once
#ifdef _WIN32
#include "pkkm/kingdom.hpp"
#include "pkkm/buildings.hpp"
#include <windows.h>
#include <functional>

namespace pkkm::ui {
bool open_map_editor(HWND owner, HINSTANCE instance, const SettlementMap& initial,
    const std::vector<Building>& inventory, const std::vector<BuildingCatalogEntry>& catalog,
    std::function<void(SettlementMap)> on_apply);
void close_map_editor(HWND owner);
}
#endif
