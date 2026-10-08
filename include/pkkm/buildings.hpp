#pragma once

#include <cstdint>
#include <string>
#include <variant>
#include <vector>

namespace pkkm {

struct Settlement;

enum class CatalogRowKind { Improvement, Basic, Town, City, District, Unclassified };
using CatalogCell = std::variant<std::monostate, bool, std::int64_t, double, std::string>;

struct BuildingCatalogEntry {
    int source_row = 0;
    CatalogRowKind kind = CatalogRowKind::Unclassified;
    std::string name;
    std::vector<CatalogCell> source_values;
    int cost = 0;
    int lots = 0;
    int economy = 0;
    int loyalty = 0;
    int stability = 0;
    int defense = 0;
    int unrest = 0;
    int base_value = 0;
    std::string discounts;
    std::string magic_item;
    std::string upgrade_from;
    std::string upgrade_to;
    int corruption = 0;
    int crime = 0;
    int law = 0;
    int lore = 0;
    int society = 0;
    int productivity = 0;
    int fame = 0;
};

struct SettlementSize {
    int lots = 0;
    int districts = 0;
    int population = 0;
};

struct SettlementBuildingTotals {
    int lots = 0;
    int economy = 0;
    int loyalty = 0;
    int stability = 0;
    int defense = 0;
    int unrest = 0;
};

std::vector<BuildingCatalogEntry> load_building_catalog(const std::string& path);
std::vector<BuildingCatalogEntry> search_building_catalog(
    const std::vector<BuildingCatalogEntry>& catalog, const std::string& query);
SettlementSize calculate_settlement_size(
    const Settlement& settlement, const std::vector<BuildingCatalogEntry>& catalog);
SettlementBuildingTotals calculate_settlement_building_totals(
    const Settlement& settlement, const std::vector<BuildingCatalogEntry>& catalog);

} // namespace pkkm
