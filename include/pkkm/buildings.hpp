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
    int corruption = 0;
    int crime = 0;
    int law = 0;
    int lore = 0;
    int society = 0;
    int productivity = 0;
    int fame = 0;
};

struct SettlementMarketReference {
    int base_value = 0;
    int buy_limit = 0;
    int sell_limit = 0;
    int spellcasting_level = 0;
};
struct SettlementMagicItemPreview {
    int minor_slots = 0;
    int medium_slots = 0;
    int major_slots = 0;
    int qualifying_buildings = 0;
    std::vector<std::string> sources;
};
enum class RandomMagicItemTier { Minor, Medium, Major };
struct SettlementRandomMagicItemCandidate {
    RandomMagicItemTier tier = RandomMagicItemTier::Minor;
    int slot_index = 0;
    int category_roll = 0;
    std::string category;
    std::string variant;
    std::string resolution_status;
};
struct SettlementRandomMagicItemPreview {
    std::uint64_t seed = 0;
    std::string source_profile = "d20pfsrd-all-captured";
    std::vector<SettlementRandomMagicItemCandidate> candidates;
};
struct RandomMagicItemCatalogReference {
    std::string source_profile;
    int record_count = 0;
    struct Record {
        std::string category;
        std::string tier;
        std::string variant;
        std::string table;
        int low = 0;
        int high = 0;
        std::string name;
        int price_gp = 0;
        std::string source_url;
    };
    std::vector<Record> records;
};
struct RandomMagicItemAvailability {
    std::string tier;
    int slot_index = 0;
    int roll = 0;
    bool available = false;
    std::string status;
};
struct SettlementBuildingDiscountPreview {
    bool stacking_enabled = true;
    std::vector<std::string> source_buildings;
};

std::vector<BuildingCatalogEntry> load_building_catalog(const std::string& path);
std::vector<BuildingCatalogEntry> search_building_catalog(
    const std::vector<BuildingCatalogEntry>& catalog, const std::string& query);
SettlementSize calculate_settlement_size(
    const Settlement& settlement, const std::vector<BuildingCatalogEntry>& catalog);
SettlementBuildingTotals calculate_settlement_building_totals(
    const Settlement& settlement, const std::vector<BuildingCatalogEntry>& catalog);
int calculate_settlement_catalog_cost(
    const Settlement& settlement, const std::vector<BuildingCatalogEntry>& catalog);
SettlementMarketReference calculate_settlement_market_reference(
    const Settlement& settlement, const std::vector<BuildingCatalogEntry>& catalog);
SettlementMagicItemPreview preview_settlement_magic_items(
    const Settlement& settlement, const std::vector<BuildingCatalogEntry>& catalog);
SettlementRandomMagicItemPreview preview_settlement_random_magic_items(
    const SettlementMagicItemPreview& slots, std::uint64_t seed);
RandomMagicItemCatalogReference load_random_magic_item_catalog(const std::string& path);
int apply_random_magic_item_upkeep(Settlement& settlement,
    const SettlementMagicItemPreview& slots,
    const RandomMagicItemCatalogReference& catalog,
    std::uint64_t seed, int turn);
void remove_random_magic_item_stock(Settlement& settlement,
    const std::string& tier, int slot_index, int turn);
std::vector<RandomMagicItemAvailability> preview_random_magic_item_availability(
    const Settlement& settlement, int base_value, std::uint64_t seed);
SettlementBuildingDiscountPreview preview_settlement_building_discounts(
    const Settlement& settlement, const std::string& target_name,
    const std::vector<BuildingCatalogEntry>& catalog, bool stacking_enabled);

} // namespace pkkm
