#include "pkkm/greenbelt.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <sstream>
#include <stdexcept>
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

bool has_terrain_token(const std::vector<std::string>& tokens, const std::string& token) {
    return std::find(tokens.begin(), tokens.end(), token) != tokens.end();
}

bool terrain_matches(const std::string& requirement, const std::string& terrain) {
    const auto tokens = greenbelt_terrain_tokens(terrain);
    if (requirement == "any claimed land")
        return !tokens.empty() && !has_terrain_token(tokens, "water");
    if (requirement == "plains or grassland")
        return has_terrain_token(tokens, "plains") || has_terrain_token(tokens, "grassland");
    if (requirement == "hills") return has_terrain_token(tokens, "hills");
    if (requirement == "mountains") return has_terrain_token(tokens, "mountains");
    if (requirement == "water or coast")
        return has_terrain_token(tokens, "water") || has_terrain_token(tokens, "coast");
    if (requirement == "river") return has_terrain_token(tokens, "river");
    return requirement.empty() || has_terrain_token(tokens, requirement);
}

} // namespace

GreenbeltPoint greenbelt_hex_center(const GreenbeltCalibration& calibration, int q, int r) {
    const double scale = calibration.scale;
    return {calibration.origin_x + (static_cast<double>(q) + static_cast<double>(r) * 0.5)
            * calibration.hex_width * scale,
        calibration.origin_y + static_cast<double>(r) * calibration.hex_height * scale * 0.75};
}

GreenbeltCoordinate greenbelt_hex_at_point(
    const GreenbeltCalibration& calibration, double x, double y) {
    const double scaled_width = calibration.hex_width * calibration.scale;
    const double scaled_height = calibration.hex_height * calibration.scale;
    if (scaled_width <= 0.0 || scaled_height <= 0.0)
        return {};
    const double r = (y - calibration.origin_y) / (scaled_height * 0.75);
    const double q = (x - calibration.origin_x) / scaled_width - r * 0.5;
    return {static_cast<int>(std::lround(q)), static_cast<int>(std::lround(r))};
}

bool greenbelt_hex_inside_image(const GreenbeltCalibration& calibration, int q, int r,
    int image_width, int image_height) {
    if (image_width <= 0 || image_height <= 0 || calibration.scale <= 0.0
        || calibration.hex_width <= 0.0 || calibration.hex_height <= 0.0)
        return false;
    for (const auto& vertex : greenbelt_hex_vertices(calibration, q, r)) {
        if (vertex.x < 0.0 || vertex.y < 0.0
            || vertex.x > static_cast<double>(image_width)
            || vertex.y > static_cast<double>(image_height))
            return false;
    }
    return true;
}

std::vector<GreenbeltPoint> greenbelt_hex_vertices(
    const GreenbeltCalibration& calibration, int q, int r) {
    const GreenbeltPoint center = greenbelt_hex_center(calibration, q, r);
    constexpr double sqrt_three = 1.73205080756887729353;
    const double radius_x = calibration.hex_width * calibration.scale / sqrt_three;
    const double radius_y = calibration.hex_height * calibration.scale * 0.5;
    std::vector<GreenbeltPoint> vertices;
    vertices.reserve(6);
    constexpr double pi = 3.14159265358979323846;
    for (int side = 0; side < 6; ++side) {
        const double angle = -pi * 0.5 + static_cast<double>(side) * pi / 3.0;
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
        std::pair<int, int>{1, -1}, std::pair<int, int>{1, 0}, std::pair<int, int>{0, 1},
        std::pair<int, int>{-1, 1}, std::pair<int, int>{-1, 0}, std::pair<int, int>{0, -1}};
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

GreenbeltHex make_next_greenbelt_hex(const std::vector<GreenbeltHex>& hexes) {
    GreenbeltHex result;
    while (std::find_if(hexes.begin(), hexes.end(), [&result](const GreenbeltHex& hex) {
        return hex.q == result.q && hex.r == result.r;
    }) != hexes.end()) {
        ++result.q;
    }
    return result;
}

std::vector<std::string> greenbelt_terrain_tokens(const std::string& terrain) {
    std::vector<std::string> tokens;
    std::stringstream input(terrain);
    std::string token;
    while (std::getline(input, token, ',')) {
        const auto first = token.find_first_not_of(" \t");
        if (first == std::string::npos) continue;
        const auto last = token.find_last_not_of(" \t");
        token = token.substr(first, last - first + 1);
        std::transform(token.begin(), token.end(), token.begin(), [](unsigned char value) {
            return static_cast<char>(std::tolower(value));
        });
        if (!token.empty()) tokens.push_back(token);
    }
    return tokens;
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
    if (item->reference_only)
        preview.summary += " Derived effects remain manual-only.";
    if (!claimed && !manual_override)
        preview.warnings.push_back("The hex is not claimed; manual override is required to preview placement.");
    if (!terrain_matches(item->terrain_requirement, terrain) && !manual_override)
        preview.warnings.push_back("Terrain does not meet the catalog prerequisite.");
    if (item->id == "highway" && !manual_override)
        preview.warnings.push_back("Highway requires a connected road network; confirm the route manually.");

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
    preview.rule_profile = facts.rule_profile;
    preview.summary = item->name + ": " + item->effect
        + " Preview only; BP and kingdom totals are unchanged. Rule profile: "
        + rule_profile_name(facts.rule_profile) + ".";
    if (item->reference_only)
        preview.summary += " Derived effects remain manual-only.";
    if (facts.route_length > 0)
        preview.summary += " Route length " + std::to_string(facts.route_length)
            + " supplied; route effects remain manual.";
    if (id == "road" || id == "highway")
        preview.summary += " Road spacing uses 12 miles center-to-center (approximately 7 miles per side); adjacent-road connectivity is caller-supplied.";
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
    const auto terrain_tokens = greenbelt_terrain_tokens(facts.terrain);
    const auto has_terrain = [&terrain_tokens](const char* token) {
        return has_terrain_token(terrain_tokens, token);
    };
    if (id == "farm") {
        terrain_allowed = has_terrain("plains") || has_terrain("hills")
            || has_terrain("grassland") || has_terrain("desert");
    } else if (id == "fishery") {
        terrain_allowed = has_terrain("water") || has_terrain("coast")
            || has_terrain("river") || has_terrain("marsh");
    } else if (id == "mine") {
        terrain_allowed = has_terrain("cavern") || has_terrain("desert")
            || has_terrain("hills") || has_terrain("mountains");
    } else if (id == "quarry") {
        terrain_allowed = has_terrain("cavern") || has_terrain("hills")
            || has_terrain("mountains");
    } else if (id == "sawmill") {
        terrain_allowed = has_terrain("forest") || has_terrain("jungle");
    } else if (id == "canal") {
        terrain_allowed = has_terrain("desert") || has_terrain("hills")
            || has_terrain("plains") || has_terrain("grassland");
    }
    if (!terrain_allowed && !manual_override)
        preview.warnings.push_back("Terrain does not meet the reconciled prerequisite.");

    if (id == "highway") {
        if (!facts.connected_road && !manual_override)
            preview.warnings.push_back("Highway requires a connected Road network.");
        if (facts.kingdom_size < 26 && !manual_override)
            preview.warnings.push_back("Highway requires kingdom Size 26 or greater.");
        if (std::find(facts.existing_improvements.begin(), facts.existing_improvements.end(), "road")
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
        preview.summary += " River crossing doubles the reference Road cost; confirm the manual cost manually.";

    preview.allowed = preview.warnings.empty() || manual_override;
    if (manual_override && !preview.warnings.empty())
        preview.warnings.push_back("Manual override accepted; unresolved prerequisites remain recorded.");
    return preview;
}

GreenbeltImprovementPreview apply_greenbelt_improvement(
    GreenbeltMap& map, const std::vector<GreenbeltImprovementDefinition>& catalog,
    int q, int r, const std::string& id,
    const GreenbeltImprovementPreviewFacts& facts, bool manual_override) {
    const auto found = std::find_if(map.hexes.begin(), map.hexes.end(), [q, r](const auto& hex) {
        return hex.q == q && hex.r == r;
    });
    if (found == map.hexes.end())
        throw std::invalid_argument("Cannot apply an improvement to an unknown Greenbelt hex");
    GreenbeltImprovementPreviewFacts effective_facts = facts;
    effective_facts.terrain = found->terrain;
    effective_facts.claimed = found->claimed;
    effective_facts.existing_improvements = found->improvements;
    if (!effective_facts.requested_upgrade.empty() && effective_facts.requested_upgrade != id)
        throw std::invalid_argument("Requested Greenbelt upgrade must match the applied improvement");
    const auto* item = find_greenbelt_improvement(catalog, id);
    if (!item)
        throw std::invalid_argument("Improvement is not present in the Greenbelt catalog");
    const std::string source = id == "highway" ? "road" : id == "fort" ? "watchtower" : "";
    if (effective_facts.requested_upgrade.empty() && !source.empty()
        && std::find(found->improvements.begin(), found->improvements.end(), source)
            != found->improvements.end())
        effective_facts.requested_upgrade = id;
    const auto preview = preview_greenbelt_improvement(catalog, id, effective_facts, manual_override);
    if (!preview.allowed) {
        std::string message = "Greenbelt improvement cannot be applied";
        if (!preview.warnings.empty()) message += ": " + preview.warnings.front();
        throw std::invalid_argument(message);
    }
    if (id == "highway" && std::find(found->improvements.begin(), found->improvements.end(), "road")
            == found->improvements.end() && !manual_override)
        throw std::invalid_argument("Highway requires an existing Road improvement");
    if (effective_facts.requested_upgrade == id) {
        if (!source.empty()) {
            const auto source_it = std::find(found->improvements.begin(), found->improvements.end(), source);
            if (source_it != found->improvements.end()) found->improvements.erase(source_it);
        }
    }
    found->improvements.push_back(id);
    return preview;
}

} // namespace pkkm
