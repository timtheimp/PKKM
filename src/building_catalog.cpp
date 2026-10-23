#include "pkkm/buildings.hpp"
#include "pkkm/kingdom.hpp"
#include <nlohmann/json.hpp>
#include <algorithm>
#include <cctype>
#include <fstream>
#include <iterator>
#include <limits>
#include <random>
#include <regex>
#include <set>
#include <stdexcept>
#include <utility>

namespace pkkm {
namespace {
using Json = nlohmann::json;

int optional_int(const Json& object, const char* key) {
    if (!object.contains(key)) return 0;
    const auto& value = object.at(key);
    if (!value.is_number_integer() && !value.is_number_unsigned())
        throw std::runtime_error(std::string("Catalog field must be an integer: ") + key);
    const auto number = value.get<long long>();
    if (number < std::numeric_limits<int>::min() || number > std::numeric_limits<int>::max())
        throw std::runtime_error(std::string("Catalog field is out of range: ") + key);
    return static_cast<int>(number);
}

std::string optional_string(const Json& object, const char* key) {
    if (!object.contains(key)) return {};
    const auto& value = object.at(key);
    if (!value.is_string()) throw std::runtime_error(std::string("Catalog field must be text: ") + key);
    return value.get<std::string>();
}

CatalogRowKind parse_kind(const std::string& value) {
    if (value == "improvement") return CatalogRowKind::Improvement;
    if (value == "basic") return CatalogRowKind::Basic;
    if (value == "town") return CatalogRowKind::Town;
    if (value == "city") return CatalogRowKind::City;
    if (value == "district") return CatalogRowKind::District;
    if (value == "unclassified") return CatalogRowKind::Unclassified;
    throw std::runtime_error("Unsupported catalog row kind: " + value);
}

CatalogCell read_cell(const Json& value) {
    if (value.is_null()) return std::monostate{};
    if (value.is_boolean()) return value.get<bool>();
    if (value.is_number_unsigned()) return static_cast<std::int64_t>(value.get<unsigned long long>());
    if (value.is_number_integer()) return static_cast<std::int64_t>(value.get<long long>());
    if (value.is_number_float()) return value.get<double>();
    if (value.is_string()) return value.get<std::string>();
    throw std::runtime_error("Unsupported value in catalog source row");
}

std::string lowercase(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char character) {
        return static_cast<char>(std::tolower(character));
    });
    return value;
}
}

std::vector<BuildingCatalogEntry> load_building_catalog(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) throw std::runtime_error("Cannot open building catalog: " + path);
    const std::string text((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    const Json root = Json::parse(text);
    if (!root.is_object() || !root.contains("records") || !root.at("records").is_array())
        throw std::runtime_error("Building catalog must contain a records array");

    std::vector<BuildingCatalogEntry> result;
    for (const auto& item : root.at("records")) {
        if (!item.is_object() || !item.contains("source_row") || !item.contains("kind")
            || !item.contains("name") || !item.contains("source_values"))
            throw std::runtime_error("Catalog record is missing source metadata");
        if (!item.at("source_row").is_number_integer() || !item.at("kind").is_string()
            || !item.at("name").is_string() || !item.at("source_values").is_array())
            throw std::runtime_error("Catalog record has invalid source metadata");

        BuildingCatalogEntry entry;
        entry.source_row = item.at("source_row").get<int>();
        if (entry.source_row < 1) throw std::runtime_error("Catalog source row must be positive");
        entry.kind = parse_kind(item.at("kind").get<std::string>());
        entry.name = item.at("name").get<std::string>();
        for (const auto& cell : item.at("source_values")) entry.source_values.push_back(read_cell(cell));
        if (entry.kind == CatalogRowKind::Improvement) {
            if (entry.name.empty() || !item.contains("building") || !item.at("building").is_object())
                throw std::runtime_error("Improvement catalog record is missing building data");
            const auto& building = item.at("building");
            entry.cost = optional_int(building, "cost");
            entry.lots = optional_int(building, "lots");
            entry.economy = optional_int(building, "economy");
            entry.loyalty = optional_int(building, "loyalty");
            entry.stability = optional_int(building, "stability");
            entry.defense = optional_int(building, "defense");
            entry.unrest = optional_int(building, "unrest");
            entry.base_value = optional_int(building, "base_value");
            entry.discounts = optional_string(building, "discounts");
            entry.magic_item = optional_string(building, "magic_item");
            entry.upgrade_from = optional_string(building, "upgrade_from");
            entry.upgrade_to = optional_string(building, "upgrade_to");
            entry.corruption = optional_int(building, "corruption");
            entry.crime = optional_int(building, "crime");
            entry.law = optional_int(building, "law");
            entry.lore = optional_int(building, "lore");
            entry.society = optional_int(building, "society");
            entry.productivity = optional_int(building, "productivity");
            entry.fame = optional_int(building, "fame");
        }
        result.push_back(std::move(entry));
    }
    return result;
}

std::vector<BuildingCatalogEntry> search_building_catalog(
    const std::vector<BuildingCatalogEntry>& catalog, const std::string& query) {
    const auto needle = lowercase(query);
    std::vector<BuildingCatalogEntry> matches;
    for (const auto& entry : catalog) {
        if (entry.kind == CatalogRowKind::Improvement && lowercase(entry.name).find(needle) != std::string::npos)
            matches.push_back(entry);
    }
    return matches;
}

SettlementBuildingTotals calculate_settlement_building_totals(
    const Settlement& settlement, const std::vector<BuildingCatalogEntry>& catalog) {
    if (settlement.name.empty() || settlement.population < 0 || settlement.districts < 0)
        throw std::invalid_argument("Settlement must have a name and nonnegative population/district overrides");

    SettlementBuildingTotals totals;
    const auto add_scaled = [](int& total, int per_building, int count, const char* field) {
        const long long next = static_cast<long long>(total)
            + static_cast<long long>(per_building) * count;
        if (next < std::numeric_limits<int>::min() || next > std::numeric_limits<int>::max())
            throw std::overflow_error(std::string("Derived settlement building ") + field + " exceeds supported range");
        total = static_cast<int>(next);
    };
    for (const auto& item : settlement.building_inventory) {
        if (item.name.empty() || item.count < 0)
            throw std::invalid_argument("Building inventory requires a name and nonnegative count");
        const auto found = std::find_if(catalog.begin(), catalog.end(), [&item](const BuildingCatalogEntry& entry) {
            return entry.kind == CatalogRowKind::Improvement && entry.name == item.name;
        });
        if (found == catalog.end())
            throw std::invalid_argument("Building is not present in the improvement catalog: " + item.name);
        if (found->lots < 0)
            throw std::invalid_argument("Building lot value cannot be negative: " + item.name);
        add_scaled(totals.lots, found->lots, item.count, "lots");
        add_scaled(totals.economy, found->economy, item.count, "Economy");
        add_scaled(totals.loyalty, found->loyalty, item.count, "Loyalty");
        add_scaled(totals.stability, found->stability, item.count, "Stability");
        add_scaled(totals.defense, found->defense, item.count, "Defense");
        add_scaled(totals.unrest, found->unrest, item.count, "unrest");
        add_scaled(totals.corruption, found->corruption, item.count, "corruption");
        add_scaled(totals.crime, found->crime, item.count, "crime");
        add_scaled(totals.law, found->law, item.count, "law");
        add_scaled(totals.lore, found->lore, item.count, "lore");
        add_scaled(totals.society, found->society, item.count, "society");
        add_scaled(totals.productivity, found->productivity, item.count, "productivity");
        add_scaled(totals.fame, found->fame, item.count, "fame");
    }
    return totals;
}

SettlementSize calculate_settlement_size(
    const Settlement& settlement, const std::vector<BuildingCatalogEntry>& catalog) {
    const auto building_totals = calculate_settlement_building_totals(settlement, catalog);
    const long long lots = building_totals.lots;

    const auto to_int = [](long long value, const char* field) {
        if (value < std::numeric_limits<int>::min() || value > std::numeric_limits<int>::max())
            throw std::overflow_error(std::string("Derived settlement ") + field + " exceeds supported range");
        return static_cast<int>(value);
    };
    const int total_lots = to_int(lots, "lots");
    const long long minimum_districts = lots == 0 ? 1 : (lots + 35) / 36;
    const long long occupied_map_districts = map_occupied_district_count(settlement.map);
    const long long districts = std::max({minimum_districts,
        static_cast<long long>(settlement.districts), occupied_map_districts});
    const long long minimum_population = lots * 250;
    const long long population = settlement.population >= minimum_population
        ? settlement.population : minimum_population;
    return {total_lots, to_int(districts, "districts"), to_int(population, "population")};
}

int calculate_settlement_catalog_cost(
    const Settlement& settlement, const std::vector<BuildingCatalogEntry>& catalog) {
    long long total = 0;
    for (const auto& item : settlement.building_inventory) {
        if (item.name.empty() || item.count < 0)
            throw std::invalid_argument("Building inventory requires a name and nonnegative count");
        const auto found = std::find_if(catalog.begin(), catalog.end(), [&item](const BuildingCatalogEntry& entry) {
            return entry.kind == CatalogRowKind::Improvement && entry.name == item.name;
        });
        if (found == catalog.end())
            throw std::invalid_argument("Building is not present in the improvement catalog: " + item.name);
        if (found->cost < 0)
            throw std::invalid_argument("Building cost cannot be negative: " + item.name);
        total += static_cast<long long>(found->cost) * item.count;
        if (total > std::numeric_limits<int>::max())
            throw std::overflow_error("Settlement catalog cost exceeds supported range");
    }
    return static_cast<int>(total);
}

SettlementMarketReference calculate_settlement_market_reference(
    const Settlement& settlement, const std::vector<BuildingCatalogEntry>& catalog) {
    const auto size = calculate_settlement_size(settlement, catalog);
    long long base_value = 0;
    for (const auto& item : settlement.building_inventory) {
        const auto found = std::find_if(catalog.begin(), catalog.end(), [&item](const BuildingCatalogEntry& entry) {
            return entry.kind == CatalogRowKind::Improvement && entry.name == item.name;
        });
        if (found == catalog.end())
            throw std::invalid_argument("Building is not present in the improvement catalog: " + item.name);
        if (found->base_value < 0)
            throw std::invalid_argument("Building base value cannot be negative: " + item.name);
        base_value += static_cast<long long>(found->base_value) * item.count;
        if (base_value > std::numeric_limits<int>::max())
            throw std::overflow_error("Settlement base value exceeds supported range");
    }
    const auto tier = [](int population, int first, int second, int third, int fourth, int fifth, int sixth) {
        if (population < 250) return first;
        if (population < 2000) return second;
        if (population < 5000) return third;
        if (population < 10000) return fourth;
        if (population < 25000) return fifth;
        return sixth;
    };
    const int buy_tier = tier(size.population, 500, 1000, 2000, 4000, 8000, 16000);
    const int sell_tier = size.population <= 250 ? 2500 : tier(size.population, 0, 5000, 10000, 25000, 50000, 100000);
    const auto add_checked = [](int base, int increment, const char* field) {
        const long long value = static_cast<long long>(base) + increment;
        if (value > std::numeric_limits<int>::max())
            throw std::overflow_error(std::string("Settlement ") + field + " exceeds supported range");
        return static_cast<int>(value);
    };
    return {static_cast<int>(base_value), add_checked(static_cast<int>(base_value), buy_tier, "buy limit"),
        add_checked(static_cast<int>(base_value), sell_tier, "sell limit"), tier(size.population, 3, 4, 5, 6, 7, 8)};
}

SettlementMagicItemPreview preview_settlement_magic_items(
    const Settlement& settlement, const std::vector<BuildingCatalogEntry>& catalog) {
    SettlementMagicItemPreview preview;
    const std::regex slot_pattern(R"((\d+)\s+[^,;]*\b(minor|medium|major)\b)",
        std::regex_constants::icase);
    for (const auto& item : settlement.building_inventory) {
        if (item.name.empty() || item.count < 0)
            throw std::invalid_argument("Building inventory requires a name and nonnegative count");
        if (item.count == 0) continue;
        const auto found = std::find_if(catalog.begin(), catalog.end(), [&item](const BuildingCatalogEntry& entry) {
            return entry.kind == CatalogRowKind::Improvement && entry.name == item.name;
        });
        if (found == catalog.end())
            throw std::invalid_argument("Building is not present in the improvement catalog: " + item.name);
        if (found->magic_item.empty()) continue;
        ++preview.qualifying_buildings;
        preview.sources.push_back(item.name + ": " + found->magic_item);
        for (std::sregex_iterator it(found->magic_item.begin(), found->magic_item.end(), slot_pattern), end;
             it != end; ++it) {
            const long long source_slots = std::stoll((*it)[1].str());
            if (source_slots < 0 || (item.count != 0
                && source_slots > std::numeric_limits<long long>::max() / item.count))
                throw std::overflow_error("Settlement magic-item slots exceed supported range");
            const long long slots = source_slots * static_cast<long long>(item.count);
            if (slots > std::numeric_limits<int>::max())
                throw std::overflow_error("Settlement magic-item slots exceed supported range");
            const std::string tier = (*it)[2].str();
            int* target = nullptr;
            if (tier.size() > 1 && (tier[1] == 'e' || tier[1] == 'E')) target = &preview.medium_slots;
            else if (tier.size() > 1 && (tier[1] == 'a' || tier[1] == 'A')) target = &preview.major_slots;
            else target = &preview.minor_slots;
            if (!target) continue;
            const long long next = static_cast<long long>(*target) + slots;
            if (next > std::numeric_limits<int>::max())
                throw std::overflow_error("Settlement magic-item slots exceed supported range");
            *target = static_cast<int>(next);
        }
    }
    return preview;
}

SettlementRandomMagicItemPreview preview_settlement_random_magic_items(
    const SettlementMagicItemPreview& slots, std::uint64_t seed) {
    if (slots.minor_slots < 0 || slots.medium_slots < 0 || slots.major_slots < 0)
        throw std::invalid_argument("Random magic-item slots cannot be negative");
    SettlementRandomMagicItemPreview preview;
    preview.seed = seed;
    std::mt19937_64 generator(seed);
    std::uniform_int_distribution<int> roll_distribution(1, 100);
    std::uniform_int_distribution<int> variant_distribution(0, 1);
    const auto category = [](RandomMagicItemTier tier, int roll) -> std::string {
        if (tier == RandomMagicItemTier::Minor) {
            if (roll <= 4) return "Armor and shields";
            if (roll <= 9) return "Weapons";
            if (roll <= 44) return "Potions";
            if (roll <= 46) return "Rings";
            if (roll <= 81) return "Scrolls";
            if (roll <= 91) return "Wands";
            return "Wondrous items";
        }
        if (tier == RandomMagicItemTier::Medium) {
            if (roll <= 10) return "Armor and shields";
            if (roll <= 20) return "Weapons";
            if (roll <= 30) return "Potions";
            if (roll <= 40) return "Rings";
            if (roll <= 50) return "Rods";
            if (roll <= 65) return "Scrolls";
            if (roll <= 68) return "Staves";
            if (roll <= 83) return "Wands";
            return "Wondrous items";
        }
        if (roll <= 10) return "Armor and shields";
        if (roll <= 20) return "Weapons";
        if (roll <= 25) return "Potions";
        if (roll <= 35) return "Rings";
        if (roll <= 45) return "Rods";
        if (roll <= 55) return "Scrolls";
        if (roll <= 75) return "Staves";
        if (roll <= 80) return "Wands";
        return "Wondrous items";
    };
    const auto add_candidates = [&](RandomMagicItemTier tier, int count) -> void {
        for (int index = 1; index <= count; ++index) {
            const int roll = roll_distribution(generator);
            const std::string variant = variant_distribution(generator) == 0 ? "lesser" : "greater";
            preview.candidates.push_back({tier, index, roll, category(tier, roll), variant,
                "Category resolved; lesser/greater variant selected 50/50; final item roll pending"});
        }
    };
    add_candidates(RandomMagicItemTier::Minor, slots.minor_slots);
    add_candidates(RandomMagicItemTier::Medium, slots.medium_slots);
    add_candidates(RandomMagicItemTier::Major, slots.major_slots);
    return preview;
}

RandomMagicItemCatalogReference load_random_magic_item_catalog(const std::string& path) {
    std::ifstream input(path);
    if (!input) throw std::runtime_error("Cannot open random magic-item catalog: " + path);
    Json document;
    input >> document;
    if (!document.is_object() || !document.contains("source_profile")
        || !document.contains("record_count") || !document.contains("records"))
        throw std::runtime_error("Invalid random magic-item catalog document");
    const auto& source_profile = document.at("source_profile");
    const auto& record_count = document.at("record_count");
    const auto& records = document.at("records");
    if (!source_profile.is_string() || !record_count.is_number_integer() || !records.is_array())
        throw std::runtime_error("Invalid random magic-item catalog fields");
    const auto count = record_count.get<int>();
    if (count < 0 || count != static_cast<int>(records.size()))
        throw std::runtime_error("Random magic-item catalog record count is inconsistent");
    RandomMagicItemCatalogReference result{source_profile.get<std::string>(), count, {}};
    result.records.reserve(records.size());
    for (const auto& record : records) {
        if (!record.is_object()) throw std::runtime_error("Invalid random magic-item catalog record");
        RandomMagicItemCatalogReference::Record entry;
        entry.category = record.at("category").get<std::string>();
        entry.tier = record.at("tier").get<std::string>();
        entry.variant = record.at("variant").get<std::string>();
        entry.table = record.at("table").get<std::string>();
        entry.low = record.at("low").get<int>();
        entry.high = record.at("high").get<int>();
        entry.name = record.at("name").get<std::string>();
        if (record.contains("price_gp") && !record.at("price_gp").is_null())
            entry.price_gp = record.at("price_gp").get<int>();
        entry.source_url = record.at("source_url").get<std::string>();
        result.records.push_back(std::move(entry));
    }
    return result;
}

int apply_random_magic_item_upkeep(Settlement& settlement,
    const SettlementMagicItemPreview& slots,
    const RandomMagicItemCatalogReference& catalog,
    std::uint64_t seed, int turn) {
    if (turn < 1) throw std::invalid_argument("Random magic-item upkeep turn must be positive");
    if (catalog.record_count != static_cast<int>(catalog.records.size()))
        throw std::invalid_argument("Random magic-item catalog record count is inconsistent");
    const auto category_slug = [](const std::string& category) -> std::string {
        if (category == "Armor and shields") return "magic-armor";
        if (category == "Weapons") return "magic-weapons";
        if (category == "Potions") return "potions";
        if (category == "Rings") return "rings";
        if (category == "Rods") return "rods";
        if (category == "Scrolls") return "scrolls";
        if (category == "Staves") return "staves";
        if (category == "Wands") return "wands";
        if (category == "Wondrous items") return "wondrous-items";
        return {};
    };
    std::mt19937_64 generator(seed);
    std::uniform_int_distribution<int> item_roll_distribution(1, 100);
    int populated = 0;
    const auto preview = preview_settlement_random_magic_items(slots, seed);
    for (const auto& candidate : preview.candidates) {
        const std::string tier = candidate.tier == RandomMagicItemTier::Minor ? "minor"
            : candidate.tier == RandomMagicItemTier::Medium ? "medium" : "major";
        const auto existing = std::find_if(settlement.random_magic_item_stock.begin(),
            settlement.random_magic_item_stock.end(), [&](const SettlementMagicItemStock& stock) {
                return stock.tier == tier && stock.slot_index == candidate.slot_index;
            });
        if (existing != settlement.random_magic_item_stock.end()
            && existing->state != "empty" && existing->state != "sold") continue;
        SettlementMagicItemStock stock;
        stock.tier = tier;
        stock.slot_index = candidate.slot_index;
        stock.generated_turn = turn;
        stock.last_changed_turn = turn;
        const auto slug = category_slug(candidate.category);
        const int item_roll = item_roll_distribution(generator);
        std::vector<const RandomMagicItemCatalogReference::Record*> matches;
        std::set<std::string> tables;
        for (const auto& record : catalog.records) {
            if (record.category == slug && record.tier == tier && record.variant == candidate.variant) {
                tables.insert(record.table);
                if (record.low <= item_roll && item_roll <= record.high) matches.push_back(&record);
            }
        }
        if ((candidate.category == "Potions" || candidate.category == "Scrolls"
             || candidate.category == "Wands") || tables.size() != 1 || matches.size() != 1) {
            stock.state = "manual_resolution";
            stock.source_table = candidate.category;
        } else {
            const auto* record = matches.front();
            stock.state = "generated";
            stock.item_name = record->name;
            stock.price_gp = record->price_gp;
            stock.source_table = record->table;
            stock.source_url = record->source_url;
        }
        if (existing == settlement.random_magic_item_stock.end())
            settlement.random_magic_item_stock.push_back(std::move(stock));
        else
            *existing = std::move(stock);
        ++populated;
    }
    return populated;
}

void remove_random_magic_item_stock(Settlement& settlement,
    const std::string& tier, int slot_index, int turn) {
    if (tier != "minor" && tier != "medium" && tier != "major")
        throw std::invalid_argument("Random magic-item stock has an unsupported tier");
    if (slot_index < 1 || turn < 1)
        throw std::invalid_argument("Random magic-item stock removal requires positive slot and turn");
    const auto found = std::find_if(settlement.random_magic_item_stock.begin(),
        settlement.random_magic_item_stock.end(), [&](const SettlementMagicItemStock& stock) {
            return stock.tier == tier && stock.slot_index == slot_index;
        });
    if (found == settlement.random_magic_item_stock.end())
        throw std::out_of_range("Random magic-item stock slot is not recorded");
    if (found->state != "generated")
        throw std::invalid_argument("Only generated random magic-item stock can be removed");
    found->state = "empty";
    found->item_name.clear();
    found->price_gp = 0;
    found->source_table.clear();
    found->source_url.clear();
    found->last_changed_turn = turn;
}

std::vector<RandomMagicItemAvailability> preview_random_magic_item_availability(
    const Settlement& settlement, int base_value, std::uint64_t seed) {
    if (base_value < 0) throw std::invalid_argument("Settlement base value cannot be negative");
    std::mt19937_64 generator(seed);
    std::uniform_int_distribution<int> availability_roll(1, 100);
    std::vector<RandomMagicItemAvailability> result;
    for (const auto& stock : settlement.random_magic_item_stock) {
        if (stock.state != "generated") continue;
        RandomMagicItemAvailability entry;
        entry.tier = stock.tier;
        entry.slot_index = stock.slot_index;
        if (stock.price_gp > base_value) {
            entry.available = true;
            entry.status = "above_base_value_always_available";
        } else {
            entry.roll = availability_roll(generator);
            entry.available = entry.roll <= 75;
            entry.status = entry.available ? "at_or_below_base_value_available"
                : "at_or_below_base_value_unavailable";
        }
        result.push_back(std::move(entry));
    }
    return result;
}

SettlementBuildingDiscountPreview preview_settlement_building_discounts(
    const Settlement& settlement, const std::string& target_name,
    const std::vector<BuildingCatalogEntry>& catalog, bool stacking_enabled) {
    if (target_name.empty()) throw std::invalid_argument("Discount preview requires a target building");
    SettlementBuildingDiscountPreview preview;
    preview.stacking_enabled = stacking_enabled;
    for (const auto& item : settlement.building_inventory) {
        if (item.name.empty() || item.count < 0)
            throw std::invalid_argument("Building inventory requires a name and nonnegative count");
        if (item.count == 0) continue;
        const auto source = std::find_if(catalog.begin(), catalog.end(), [&item](const BuildingCatalogEntry& entry) {
            return entry.kind == CatalogRowKind::Improvement && entry.name == item.name;
        });
        if (source == catalog.end())
            throw std::invalid_argument("Building is not present in the improvement catalog: " + item.name);
        std::size_t start = 0;
        while (start <= source->discounts.size()) {
            const std::size_t comma = source->discounts.find(',', start);
            std::string token = source->discounts.substr(start,
                comma == std::string::npos ? std::string::npos : comma - start);
            const auto first = token.find_first_not_of(" \t\r\n");
            const auto last = token.find_last_not_of(" \t\r\n");
            token = first == std::string::npos ? std::string{} : token.substr(first, last - first + 1);
            if (token == target_name) {
                preview.source_buildings.push_back(source->name);
                break;
            }
            if (comma == std::string::npos) break;
            start = comma + 1;
        }
        if (!stacking_enabled && !preview.source_buildings.empty()) break;
    }
    return preview;
}

} // namespace pkkm
