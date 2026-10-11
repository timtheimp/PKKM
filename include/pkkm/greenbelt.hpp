#pragma once

#include "pkkm/kingdom.hpp"
#include <string>
#include <vector>

namespace pkkm {

struct GreenbeltPoint {
    double x = 0.0;
    double y = 0.0;
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
    int bp_delta = 0;
    std::string summary;
    std::vector<std::string> warnings;
};

const std::vector<GreenbeltImprovementDefinition>& greenbelt_improvement_catalog();
GreenbeltPoint greenbelt_hex_center(const GreenbeltCalibration& calibration, int q, int r);
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

} // namespace pkkm
