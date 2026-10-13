#include "pkkm/greenbelt.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <unordered_set>
#include <utility>

namespace pkkm {
namespace {

GreenbeltImprovementDefinition definition(
    const char* id, const char* name, const char* terrain, const char* effect,
    const char* cost, int cost_bp = 0, bool fixed = false, bool shareable = false,
    bool third_party = false, bool reference_only = false) {
    return {id, name, terrain, effect, cost, cost_bp, fixed, shareable, third_party, reference_only};
}

bool terrain_matches(const std::string& requirement, const std::string& terrain) {
    if (requirement == "any claimed land") return !terrain.empty() && terrain != "water";
    if (requirement == "plains or grassland") return terrain == "plains" || terrain == "grassland";
    if (requirement == "hills") return terrain == "hills";
    if (requirement == "mountains") return terrain == "mountains";
    if (requirement == "water or coast") return terrain == "water" || terrain == "coast";
    if (requirement == "river") return terrain == "river";
    return requirement.empty() || requirement == terrain;
}

} // namespace

GreenbeltPoint greenbelt_hex_center(const GreenbeltCalibration& calibration, int q, int r) {
    const double scale = calibration.scale;
    return {calibration.origin_x + (static_cast<double>(q) + static_cast<double>(r) * 0.5)
            * calibration.hex_width * scale,
        calibration.origin_y + static_cast<double>(r) * calibration.hex_height * scale};
}

std::vector<GreenbeltPoint> greenbelt_hex_vertices(
    const GreenbeltCalibration& calibration, int q, int r) {
    const GreenbeltPoint center = greenbelt_hex_center(calibration, q, r);
    const double radius_x = calibration.hex_width * calibration.scale * 0.5;
    const double radius_y = calibration.hex_height * calibration.scale * 0.5;
    std::vector<GreenbeltPoint> vertices;
    vertices.reserve(6);
    constexpr double pi = 3.14159265358979323846;
    for (int side = 0; side < 6; ++side) {
        const double angle = static_cast<double>(side) * pi / 3.0;
        vertices.push_back({center.x + radius_x * std::cos(angle),
            center.y + radius_y * std::sin(angle)});
    }
    return vertices;
}

std::vector<GreenbeltBoundaryEdge> greenbelt_claimed_perimeter(
    const std::vector<GreenbeltHex>& hexes) {
    const auto key = [](int q, int r) {
        return std::to_string(q) + ":" + std::to_string(r);
    };
    const std::array<std::pair<int, int>, 6> directions = {
        std::pair<int, int>{1, 0}, std::pair<int, int>{0, 1}, std::pair<int, int>{-1, 1},
        std::pair<int, int>{-1, 0}, std::pair<int, int>{0, -1}, std::pair<int, int>{1, -1}};
    std::unordered_set<std::string> claimed;
    for (const auto& hex : hexes) if (hex.claimed) claimed.insert(key(hex.q, hex.r));
    std::vector<GreenbeltBoundaryEdge> perimeter;
    for (const auto& hex : hexes) {
        if (!hex.claimed) continue;
        for (int side = 0; side < static_cast<int>(directions.size()); ++side) {
            const auto [dq, dr] = directions[static_cast<size_t>(side)];
            if (claimed.find(key(hex.q + dq, hex.r + dr)) == claimed.end())
                perimeter.push_back({hex.q, hex.r, side});
        }
    }
    return perimeter;
}

const std::vector<GreenbeltImprovementDefinition>& greenbelt_improvement_catalog() {
    static const std::vector<GreenbeltImprovementDefinition> catalog = {
        definition("aqueduct", "Aqueduct", "any claimed land",
            "Provides water infrastructure; placement and settlement connection require manual confirmation.",
            "Source-dependent", 0, false, false, false, true),
        definition("bridge", "Bridge", "river",
            "Provides a crossing; route connection and placement remain manual.",
            "Source-dependent", 0, false, true, false, true),
        definition("canal", "Canal", "water or coast",
            "Provides a navigable water connection; route and settlement effects remain manual.",
            "Source-dependent", 0, false, true, false, true),
        definition("farm", "Farm", "plains or grassland",
            "Reduces Consumption when the source conditions are satisfied.",
            "Source-dependent", 0, false, true, false, false),
        definition("fishery", "Fishery", "water or coast",
            "Reduces Consumption when the source conditions are satisfied.",
            "Source-dependent", 0, false, true, false, false),
        definition("fort", "Fort", "any claimed land",
            "Provides a defensive improvement; defense and settlement effects remain manual.",
            "Source-dependent", 0, false, true, false, true),
        definition("highway", "Highway", "any claimed land",
            "Improves overland route travel; requires a connected road network.",
            "Source-dependent", 0, false, true, false, true),
        definition("mine", "Mine", "mountains",
            "Provides a resource improvement; resource yield remains manual.",
            "Source-dependent", 0, false, true, false, true),
        definition("quarry", "Quarry", "hills",
            "Provides a stone resource; resource yield remains manual.",
            "Source-dependent", 0, false, true, false, true),
        definition("road", "Road", "any claimed land",
            "Provides an overland route connection; route geometry remains manual.",
            "Source-dependent", 0, false, true, false, true),
        definition("sawmill", "Sawmill", "any claimed land",
            "Provides a lumber resource; resource yield remains manual.",
            "Source-dependent", 0, false, true, false, true),
        definition("watchtower", "Watchtower", "any claimed land",
            "Provides a defensive observation point; derived defense remains manual.",
            "Source-dependent", 0, false, true, false, true),
        definition("lumber_camp", "Lumber Camp", "any claimed land",
            "Provides a lumber resource; placement and yield remain manual.",
            "Source-dependent", 0, false, true, false, true),
        definition("vineyard", "Vineyard", "hills",
            "Provides a source-specific agricultural improvement; its effects remain manual.",
            "3 BP", 3, true, true, true, true),
    };
    return catalog;
}

const GreenbeltImprovementDefinition* find_greenbelt_improvement(
    const std::vector<GreenbeltImprovementDefinition>& catalog, const std::string& id) {
    const auto found = std::find_if(catalog.begin(), catalog.end(), [&id](const auto& item) {
        return item.id == id;
    });
    return found == catalog.end() ? nullptr : &*found;
}

GreenbeltImprovementPreview preview_greenbelt_improvement(
    const std::vector<GreenbeltImprovementDefinition>& catalog,
    const std::string& id, const std::string& terrain, bool claimed,
    bool manual_override) {
    GreenbeltImprovementPreview preview;
    const auto* item = find_greenbelt_improvement(catalog, id);
    if (!item) {
        preview.warnings.push_back("Improvement is not present in the Greenbelt catalog.");
        return preview;
    }
    preview.known = true;
    preview.summary = item->name + ": " + item->effect +
        " Preview only; BP and kingdom totals are unchanged.";
    if (!claimed && !manual_override)
        preview.warnings.push_back("The hex is not claimed; manual override is required to preview placement.");
    if (!terrain_matches(item->terrain_requirement, terrain) && !manual_override)
        preview.warnings.push_back("Terrain does not meet the catalog prerequisite.");
    if (item->id == "highway" && !manual_override)
        preview.warnings.push_back("Highway requires a connected road network; confirm the route manually.");
    if (item->reference_only)
        preview.warnings.push_back("Placement or derived rule effects remain manual-only.");
    preview.allowed = preview.warnings.empty() || manual_override;
    if (manual_override && !preview.warnings.empty())
        preview.warnings.push_back("Manual override accepted; unresolved prerequisites remain recorded.");
    return preview;
}

GreenbeltImprovementPreview preview_greenbelt_improvement(
    const std::vector<GreenbeltImprovementDefinition>& catalog,
    const std::string& id, const GreenbeltImprovementPreviewFacts& facts,
    bool manual_override) {
    GreenbeltImprovementPreview preview;
    const auto* item = find_greenbelt_improvement(catalog, id);
    if (!item) {
        preview.warnings.push_back("Improvement is not present in the Greenbelt catalog.");
        return preview;
    }
    preview.known = true;
    preview.summary = item->name + ": " + item->effect
        + " Preview only; BP and kingdom totals are unchanged.";
    if (facts.route_length > 0)
        preview.summary += " Route length " + std::to_string(facts.route_length)
            + " supplied; route effects remain manual.";
    if (facts.river_crossing)
        preview.summary += " River crossing supplied; crossing costs and effects remain manual.";
    if (!facts.claimed && !manual_override)
        preview.warnings.push_back("The hex is not claimed; manual override is required to preview placement.");
    if (id == "watchtower" && !manual_override
        && (std::find(facts.existing_improvements.begin(), facts.existing_improvements.end(), "fort")
                != facts.existing_improvements.end()
            || std::find(facts.existing_improvements.begin(), facts.existing_improvements.end(), "watchtower")
                != facts.existing_improvements.end()))
        preview.warnings.push_back("A Watchtower cannot share a hex with a Fort or another Watchtower.");
    if (id == "aqueduct" && !manual_override) {
        if (!(facts.water_adjacent || facts.river_adjacent || facts.canal_adjacent))
            preview.warnings.push_back("An Aqueduct requires a connected water source.");
        if (!facts.settlement_endpoint)
            preview.warnings.push_back("An Aqueduct requires a settlement endpoint.");
    }
    if (id == "bridge" && !facts.river_crossing && !manual_override)
        preview.warnings.push_back("A Bridge requires an explicit river crossing fact.");
    if (std::find(facts.existing_improvements.begin(), facts.existing_improvements.end(), id)
            != facts.existing_improvements.end()
        && !item->shareable && !manual_override)
        preview.warnings.push_back("This improvement is not shareable with an existing copy on the hex.");

    bool terrain_allowed = terrain_matches(item->terrain_requirement, facts.terrain);
    if (id == "farm") {
        terrain_allowed = facts.terrain == "plains" || facts.terrain == "hills"
            || facts.terrain == "grassland" || facts.terrain == "desert";
        if (facts.terrain == "desert"
            && !(facts.water_adjacent || facts.river_adjacent || facts.canal_adjacent))
            preview.warnings.push_back("A desert Farm requires water access or a Canal.");
        if (facts.terrain != "desert"
            && !(facts.water_adjacent || facts.river_adjacent || facts.canal_adjacent)
            && facts.neighboring_farms < 2)
            preview.warnings.push_back("A Farm requires water access or adjacency to two existing Farms.");
    } else if (id == "fishery") {
        terrain_allowed = facts.terrain == "water" || facts.terrain == "coast"
            || facts.terrain == "river" || facts.terrain == "marsh";
    } else if (id == "mine") {
        terrain_allowed = facts.terrain == "cavern" || facts.terrain == "desert"
            || facts.terrain == "hills" || facts.terrain == "mountains";
    } else if (id == "quarry") {
        terrain_allowed = facts.terrain == "cavern" || facts.terrain == "hills"
            || facts.terrain == "mountains";
    } else if (id == "sawmill") {
        terrain_allowed = facts.terrain == "forest" || facts.terrain == "jungle";
    }
    if (!terrain_allowed && !manual_override)
        preview.warnings.push_back("Terrain does not meet the reconciled prerequisite.");

    if (id == "highway") {
        if (!facts.connected_road && !manual_override)
            preview.warnings.push_back("Highway requires a connected Road network.");
        if (facts.kingdom_size < 26 && !manual_override)
            preview.warnings.push_back("Highway requires kingdom Size 26 or greater.");
        if (facts.requested_upgrade == "highway"
            && std::find(facts.existing_improvements.begin(), facts.existing_improvements.end(), "road")
                == facts.existing_improvements.end()
            && !manual_override)
            preview.warnings.push_back("Highway upgrade requires an existing Road.");
    }
    if (id == "fort" && facts.requested_upgrade == "fort"
        && std::find(facts.existing_improvements.begin(), facts.existing_improvements.end(), "watchtower")
            == facts.existing_improvements.end()
        && !manual_override)
        preview.warnings.push_back("A Fort upgrade requires an existing Watchtower improvement.");
    if ((id == "road" || id == "highway") && facts.river_crossing)
        preview.warnings.push_back("River crossing doubles the reference Road cost; confirm the manual cost.");
    if (item->reference_only)
        preview.warnings.push_back("Placement or derived rule effects remain manual-only.");
    preview.allowed = preview.warnings.empty() || manual_override;
    if (manual_override && !preview.warnings.empty())
        preview.warnings.push_back("Manual override accepted; unresolved prerequisites remain recorded.");
    return preview;
}

} // namespace pkkm
