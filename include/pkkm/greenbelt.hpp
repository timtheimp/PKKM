#pragma once

#include "pkkm/kingdom.hpp"
#include <string>
#include <vector>

namespace pkkm {

struct GreenbeltPoint {
    double x = 0.0;
    double y = 0.0;
};

struct GreenbeltCoordinate {
    int q = 0;
    int r = 0;
};

struct GreenbeltBoundaryEdge {
    int q = 0;
    int r = 0;
    int side = 0;
};

struct GreenbeltImprovementDefinition {
    std::string id;
    std::string name;
    std::string terrain_requirement;
    std::string effect;
    std::string cost_description;
    int cost_bp = 0;
    bool cost_is_fixed = false;
    bool shareable = false;
    bool third_party = false;
    bool reference_only = false;
};

struct GreenbeltImprovementPreview {
    bool known = false;
    bool allowed = false;
    bool manual_effect = true;
    RuleProfile rule_profile = RuleProfile::Workbook;
    int bp_delta = 0;
    std::string summary;
    std::vector<std::string> warnings;
};

struct GreenbeltImprovementPreviewFacts {
    std::string terrain;
    RuleProfile rule_profile = RuleProfile::Workbook;
    bool claimed = false;
    bool water_adjacent = false;
    bool river_adjacent = false;
    bool canal_adjacent = false;
    bool settlement_endpoint = false;
    int neighboring_farms = 0;
    bool connected_road = false;
    bool river_crossing = false;
    int kingdom_size = 0;
    std::vector<std::string> existing_improvements;
    std::string requested_upgrade;
    int route_length = 0;
};

const std::vector<GreenbeltImprovementDefinition>& greenbelt_improvement_catalog();
GreenbeltHex make_next_greenbelt_hex(const std::vector<GreenbeltHex>& hexes);
std::vector<std::string> greenbelt_terrain_tokens(const std::string& terrain);
GreenbeltPoint greenbelt_hex_center(const GreenbeltCalibration& calibration, int q, int r);
GreenbeltCoordinate greenbelt_hex_at_point(
    const GreenbeltCalibration& calibration, double x, double y);
bool greenbelt_hex_inside_image(const GreenbeltCalibration& calibration, int q, int r,
    int image_width, int image_height);
std::vector<GreenbeltPoint> greenbelt_hex_vertices(
    const GreenbeltCalibration& calibration, int q, int r);
std::vector<GreenbeltBoundaryEdge> greenbelt_claimed_perimeter(
    const std::vector<GreenbeltHex>& hexes);
const GreenbeltImprovementDefinition* find_greenbelt_improvement(
    const std::vector<GreenbeltImprovementDefinition>& catalog, const std::string& id);
GreenbeltImprovementPreview preview_greenbelt_improvement(
    const std::vector<GreenbeltImprovementDefinition>& catalog,
    const std::string& id, const std::string& terrain, bool claimed,
    bool manual_override);
GreenbeltImprovementPreview preview_greenbelt_improvement(
    const std::vector<GreenbeltImprovementDefinition>& catalog,
    const std::string& id, const GreenbeltImprovementPreviewFacts& facts,
    bool manual_override);

} // namespace pkkm
