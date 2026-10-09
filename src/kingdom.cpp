#include "pkkm/kingdom.hpp"
#include "pkkm/buildings.hpp"
#include <nlohmann/json.hpp>
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <limits>
#include <stdexcept>
#ifdef _WIN32
#define NOMINMAX
#include <windows.h>
#endif

namespace pkkm {
namespace {
using Json = nlohmann::json;

int required_int(const Json& object, const char* key) {
    const auto& field = object.at(key);
    if (!field.is_number_integer()) throw std::runtime_error(std::string("Invalid integer field: ") + key);
    if (field.is_number_unsigned()) {
        const auto value = field.get<Json::number_unsigned_t>();
        if (value > static_cast<Json::number_unsigned_t>(std::numeric_limits<int>::max()))
            throw std::runtime_error(std::string("Integer field out of range: ") + key);
        return static_cast<int>(value);
    }
    const auto value = field.get<Json::number_integer_t>();
    if (value < std::numeric_limits<int>::min() || value > std::numeric_limits<int>::max())
        throw std::runtime_error(std::string("Integer field out of range: ") + key);
    return static_cast<int>(value);
}

int optional_int(const Json& object, const char* key, int default_value) {
    if (!object.contains(key)) return default_value;
    return required_int(object, key);
}

std::string required_string(const Json& object, const char* key) {
    const auto& field = object.at(key);
    if (!field.is_string()) throw std::runtime_error(std::string("Invalid string field: ") + key);
    return field.get<std::string>();
}

bool required_bool(const Json& object, const char* key) {
    const auto& field = object.at(key);
    if (!field.is_boolean()) throw std::runtime_error(std::string("Invalid boolean field: ") + key);
    return field.get<bool>();
}

Json stat_base_json(const KingdomStatBaseInput& value) {
    return {{"events", value.events}, {"improvements", value.improvements}, {"other", value.other}};
}

KingdomStatBaseInput read_stat_base(const Json& value) {
    if (!value.is_object()) throw std::runtime_error("Stat inputs must be an object");
    return {required_int(value, "events"), required_int(value, "improvements"), required_int(value, "other")};
}

Json slot_json(const LeadershipSlot& value) { return {{"filled", value.filled}, {"bonus", value.bonus}}; }

LeadershipSlot read_slot(const Json& value) {
    if (!value.is_object()) throw std::runtime_error("Leadership slot must be an object");
    return {required_bool(value, "filled"), required_int(value, "bonus")};
}

Json rule_inputs_json(const KingdomRuleInputs& r) {
    const auto& l = r.leadership;
    return {
        {"alignment", r.alignment},
        {"territory", {{"kingdom_size", r.kingdom_size}, {"total_city_districts", r.total_city_districts},
            {"control_dc_other", r.control_dc_other}, {"city_count", r.city_count}, {"total_city_population", r.total_city_population}}},
        {"laws", {{"promotion", static_cast<int>(r.laws.promotion)}, {"has_cathedral", r.laws.has_cathedral},
            {"taxation", static_cast<int>(r.laws.taxation)}, {"has_waterfront", r.laws.has_waterfront},
            {"holiday_interval_months", r.laws.holiday_interval_months}}},
        {"leadership", {{"baron_bonus", l.baron_bonus},
            {"baron_benefits", {{"economy", l.baron_benefits.economy}, {"loyalty", l.baron_benefits.loyalty}, {"stability", l.baron_benefits.stability}}},
            {"spymaster_benefit", static_cast<int>(l.spymaster_benefit)}, {"councilor", slot_json(l.councilor)},
            {"general", slot_json(l.general)}, {"grand_diplomat", slot_json(l.grand_diplomat)},
            {"high_priest", slot_json(l.high_priest)}, {"magister", slot_json(l.magister)},
            {"marshal", slot_json(l.marshal)}, {"royal_enforcer", slot_json(l.royal_enforcer)},
            {"spymaster", slot_json(l.spymaster)}, {"treasurer", slot_json(l.treasurer)}, {"warden", slot_json(l.warden)}}},
        {"stats", {{"economy", stat_base_json(r.economy)}, {"loyalty", stat_base_json(r.loyalty)}, {"stability", stat_base_json(r.stability)}}}
    };
}

int required_enum(const Json& object, const char* key, int maximum) {
    const int value = required_int(object, key);
    if (value < 0 || value > maximum) throw std::runtime_error(std::string("Unsupported enum field: ") + key);
    return value;
}

KingdomRuleInputs read_rule_inputs(const Json& root) {
    if (!root.is_object()) throw std::runtime_error("rules must be an object");
    KingdomRuleInputs r;
    r.alignment = required_string(root, "alignment");
    const auto& territory = root.at("territory"); const auto& laws = root.at("laws");
    const auto& leadership = root.at("leadership"); const auto& benefits = leadership.at("baron_benefits");
    const auto& stats = root.at("stats");
    if (!territory.is_object() || !laws.is_object() || !leadership.is_object() || !benefits.is_object() || !stats.is_object())
        throw std::runtime_error("Invalid rules object structure");
    r.kingdom_size = required_int(territory, "kingdom_size"); r.total_city_districts = required_int(territory, "total_city_districts");
    r.control_dc_other = required_int(territory, "control_dc_other"); r.city_count = required_int(territory, "city_count");
    r.total_city_population = required_int(territory, "total_city_population");
    r.laws.promotion = static_cast<PromotionLaw>(required_enum(laws, "promotion", 4));
    r.laws.has_cathedral = required_bool(laws, "has_cathedral"); r.laws.taxation = static_cast<TaxationLaw>(required_enum(laws, "taxation", 4));
    r.laws.has_waterfront = required_bool(laws, "has_waterfront"); r.laws.holiday_interval_months = required_int(laws, "holiday_interval_months");
    auto& l = r.leadership;
    l.baron_bonus = required_int(leadership, "baron_bonus");
    l.baron_benefits = {required_bool(benefits, "economy"), required_bool(benefits, "loyalty"), required_bool(benefits, "stability")};
    l.spymaster_benefit = static_cast<KingdomStatFocus>(required_enum(leadership, "spymaster_benefit", 2));
    l.councilor = read_slot(leadership.at("councilor")); l.general = read_slot(leadership.at("general"));
    l.grand_diplomat = read_slot(leadership.at("grand_diplomat")); l.high_priest = read_slot(leadership.at("high_priest"));
    l.magister = read_slot(leadership.at("magister")); l.marshal = read_slot(leadership.at("marshal"));
    l.royal_enforcer = read_slot(leadership.at("royal_enforcer")); l.spymaster = read_slot(leadership.at("spymaster"));
    l.treasurer = read_slot(leadership.at("treasurer")); l.warden = read_slot(leadership.at("warden"));
    r.economy = read_stat_base(stats.at("economy")); r.loyalty = read_stat_base(stats.at("loyalty"));
    r.stability = read_stat_base(stats.at("stability"));
    return r;
}

void require_fields(const Json& root) {
    if (!root.is_object()) throw std::runtime_error("Kingdom document must be a JSON object");
    for (const char* key : {"schema_version", "name", "turn", "treasury_bp", "unrest", "settlements", "buildings"}) {
        if (!root.contains(key)) throw std::runtime_error(std::string("Missing JSON field: ") + key);
    }
    if (!root.at("settlements").is_array()) throw std::runtime_error("settlements must be an array");
    if (!root.at("buildings").is_array()) throw std::runtime_error("buildings must be an array");
}
}

void validate(const Kingdom& k) {
    if (k.schema_version != 9) throw std::invalid_argument("Unsupported schema version");
    if (k.name.empty()) throw std::invalid_argument("Kingdom name must not be empty");
    if (k.turn < 1) throw std::invalid_argument("Turn must be at least 1");
    if (k.treasury_bp < 0) throw std::invalid_argument("Treasury cannot be negative");
    for (const auto& s : k.settlements) {
        if (s.name.empty() || s.population < 0 || s.districts < 0)
            throw std::invalid_argument("Settlement must have a name and nonnegative population/districts");
        for (const auto& building : s.building_inventory) {
            if (building.name.empty() || building.count < 0)
                throw std::invalid_argument("Settlement building must have a name and nonnegative count");
        }
        const auto& map = s.map;
        if (map.rows == 0 && map.columns == 0 && map.placements.empty() && map.labels.empty()) continue;
        if (map.rows <= 0 || map.columns <= 0)
            throw std::invalid_argument("Settlement map must have positive row and column counts");
        for (size_t i = 0; i < map.placements.size(); ++i) {
            const auto& placement = map.placements[i];
            if (placement.building_name.empty() || placement.row < 0 || placement.column < 0
                || placement.width <= 0 || placement.height <= 0)
                throw std::invalid_argument("Map placement needs a name, nonnegative coordinates, and positive footprint");
            const long long row_end = static_cast<long long>(placement.row) + placement.height;
            const long long column_end = static_cast<long long>(placement.column) + placement.width;
            if (row_end > map.rows || column_end > map.columns)
                throw std::invalid_argument("Map placement extends beyond the grid");
            for (size_t j = 0; j < i; ++j) {
                const auto& other = map.placements[j];
                const long long other_row_end = static_cast<long long>(other.row) + other.height;
                const long long other_column_end = static_cast<long long>(other.column) + other.width;
                if (placement.row < other_row_end && other.row < row_end
                    && placement.column < other_column_end && other.column < column_end)
                    throw std::invalid_argument("Map placements cannot overlap");
            }
        }
        for (size_t i = 0; i < map.labels.size(); ++i) {
            const auto& label = map.labels[i];
            if (label.text.empty() || label.row < 0 || label.column < 0
                || label.width <= 0 || label.height <= 0)
                throw std::invalid_argument("Map label needs text, nonnegative coordinates, and positive footprint");
            const long long row_end = static_cast<long long>(label.row) + label.height;
            const long long column_end = static_cast<long long>(label.column) + label.width;
            if (row_end > map.rows || column_end > map.columns)
                throw std::invalid_argument("Map label extends beyond the grid");
            for (size_t j = 0; j < i; ++j) {
                const auto& other = map.labels[j];
                const long long other_row_end = static_cast<long long>(other.row) + other.height;
                const long long other_column_end = static_cast<long long>(other.column) + other.width;
                if (label.row < other_row_end && other.row < row_end
                    && label.column < other_column_end && other.column < column_end)
                    throw std::invalid_argument("Map labels cannot overlap");
            }
            for (const auto& placement : map.placements) {
                const long long placement_row_end = static_cast<long long>(placement.row) + placement.height;
                const long long placement_column_end = static_cast<long long>(placement.column) + placement.width;
                if (label.row < placement_row_end && placement.row < row_end
                    && label.column < placement_column_end && placement.column < column_end)
                    throw std::invalid_argument("Map labels cannot overlap building placements");
            }
        }
    }
    for (const auto& b : k.buildings) if (b.name.empty() || b.count < 0) throw std::invalid_argument("Building must have a name and nonnegative count");
    std::vector<int> seen_turns;
    for (const auto& progress : k.turn_progress) {
        if (progress.turn < 1) throw std::invalid_argument("Checklist turn must be at least 1");
        if (std::find(seen_turns.begin(), seen_turns.end(), progress.turn) != seen_turns.end())
            throw std::invalid_argument("Turn checklist records must have unique turn numbers");
        seen_turns.push_back(progress.turn);
        std::vector<std::string> seen_steps;
        for (const auto& id : progress.completed_step_ids) {
            const auto& steps = turn_checklist_steps();
            if (std::none_of(steps.begin(), steps.end(), [&id](const auto& step) { return step.id == id; }))
                throw std::invalid_argument("Turn checklist contains an unknown step id");
            if (std::find(seen_steps.begin(), seen_steps.end(), id) != seen_steps.end())
                throw std::invalid_argument("Turn checklist step ids must be unique");
            seen_steps.push_back(id);
        }
    }
    std::vector<std::string> seen_calendar_entries;
    for (const auto& note : k.calendar_notes) {
        const auto& entries = calendar_template_entries();
        if (std::none_of(entries.begin(), entries.end(), [&note](const auto& entry) {
                return entry.id == note.entry_id;
            }))
            throw std::invalid_argument("Calendar note references an unknown source entry");
        if (std::find(seen_calendar_entries.begin(), seen_calendar_entries.end(), note.entry_id)
            != seen_calendar_entries.end())
            throw std::invalid_argument("Calendar note entry IDs must be unique");
        seen_calendar_entries.push_back(note.entry_id);
    }
    (void)alignment_bonuses(k.rule_inputs.alignment);
    (void)calculate_laws(k.rule_inputs.laws);
    (void)calculate_leadership(k.rule_inputs.leadership);
}

const std::vector<TurnChecklistStep>& turn_checklist_steps() {
    static const std::vector<TurnChecklistStep> steps = {
        {"upkeep.stability", "Phase 1 Upkeep", "Step 1", "Stability Check", false},
        {"upkeep.consumption", "Phase 1 Upkeep", "Step 2", "Pay Consumption", false},
        {"upkeep.magic_items", "Phase 1 Upkeep", "Step 3", "Make Magic Items", false},
        {"upkeep.modify_unrest", "Phase 1 Upkeep", "Step 4", "Modify Unrest", false},
        {"upkeep.loyalty_check", "Phase 1 Upkeep", "Optional", "Loyalty Check", true},
        {"edicts.assign_leadership", "Phase 2 Edicts", "Step 1", "Assign Leadership", false},
        {"edicts.claim_hexes", "Phase 2 Edicts", "Step 2", "Claim Hexes", false},
        {"edicts.abandon_hexes", "Phase 2 Edicts", "Step 3", "Abandon Hexes", false},
        {"edicts.build_improvements", "Phase 2 Edicts", "Step 4", "Build Improvements", false},
        {"edicts.city", "Phase 2 Edicts", "Step 5", "Create/Improve City", false},
        {"edicts.armies", "Phase 2 Edicts", "Step 6", "Create Armies", false},
        {"edicts.adjust_laws", "Phase 2 Edicts", "Step 7", "Adjust Edict Laws", false},
        {"income.withdraw_gold", "Phase 3 Income", "Step 1", "Withdrawal Gold", false},
        {"income.deposit_gold", "Phase 3 Income", "Step 2", "Deposit Gold", false},
        {"income.deposit_items", "Phase 3 Income", "Step 3", "Deposit Items", false},
        {"income.buy_magic_item", "Phase 3 Income", "Optional 1", "Buy Magic Item", true},
        {"income.economy_check", "Phase 3 Income", "Optional 2", "Economy Check", true},
        {"income.buy_for_kingdom", "Phase 3 Income", "Optional 3", "Buy for kingdom", true},
        {"income.taxes", "Phase 3 Income", "Step 4", "Taxes: Econ Check", false},
        {"events.roll_d100", "Phase 4 Events", "Step 1", "Roll d% for event", false}
    };
    return steps;
}

StabilityFailureOutcome classify_stability_failure(int failure_by) {
    if (failure_by < 4) return StabilityFailureOutcome::FailByLessThanFour;
    if (failure_by == 4) return StabilityFailureOutcome::UnresolvedExactlyFour;
    return StabilityFailureOutcome::FailByAtLeastFive;
}

TurnActionOutcome resolve_turn_action(TurnAction action) {
    switch (action) {
        case TurnAction::ClaimHex: return {-1, 1, 0};
        case TurnAction::AbandonHex: return {0, -1, 1};
        case TurnAction::AbandonCity: return {0, -1, 4};
        default: return {};
    }
}

KingdomSizeLimits kingdom_size_limits(int kingdom_size) {
    if (kingdom_size < 1) throw std::invalid_argument("Kingdom size must be positive");
    if (kingdom_size <= 10) return {1, 1, false, 2, 1};
    if (kingdom_size <= 25) return {1, 2, false, 3, 2};
    if (kingdom_size <= 50) return {1, 5, false, 5, 3};
    if (kingdom_size <= 100) return {2, 10, false, 7, 4};
    if (kingdom_size <= 200) return {3, 20, false, 9, 8};
    return {4, 0, true, 12, 12};
}

TurnEconomyRates turn_economy_rates() {
    return {2000, 4000, 8000, 1, 2000};
}

int resolve_tax_bp_delta(bool economy_check_success, int result_divided_by_three, int other_bp_gained) {
    return other_bp_gained + (economy_check_success ? result_divided_by_three : 0);
}

const std::vector<CalendarTemplateEntry>& calendar_template_entries() {
    static const std::vector<CalendarTemplateEntry> entries = [] {
        const std::vector<std::string> months = {
            "Pharast (March)", "Gozran (April)", "Desnus (may)", "Sarenith (June)",
            "Erastus (July)", "Arodus (Aug)", "Rova (Sept)", "Lamashan (Oct)",
            "Neth (Nov)", "Kuthona (Dec)", "Abadus (Jan)", "Kalestril (Feb)"
        };
        std::vector<CalendarTemplateEntry> result;
        result.reserve(85);
        for (int index = 0; index < 85; ++index) {
            std::string marker;
            if (index == 12) marker = "1 yr!";
            else if (index == 24) marker = "2 years!";
            else if (index == 36) marker = "year 3";
            const int source_row = index + 2;
            result.push_back({"calendar.row." + std::to_string(source_row), source_row,
                months[static_cast<size_t>(index % static_cast<int>(months.size()))], std::move(marker)});
        }
        return result;
    }();
    return entries;
}

const char* workbook_map_template_name(WorkbookMapTemplate preset) {
    switch (preset) {
        case WorkbookMapTemplate::Capital: return "Capital Map";
        case WorkbookMapTemplate::City: return "City # Map";
        case WorkbookMapTemplate::Example: return "Example Map";
        case WorkbookMapTemplate::MultiDistrict: return "Multi-District Map";
        default: return "Unknown map template";
    }
}

SettlementMap workbook_map_template(WorkbookMapTemplate preset) {
    SettlementMap map;
    map.rows = 8;
    map.columns = 8;
    map.labels = {
        {"Hill", 0, 1, 6, 1},
        {"Forest", 1, 0, 1, 6},
        {"Water", 1, 7, 1, 6},
        {"Water", 7, 1, 6, 1}
    };

    switch (preset) {
        case WorkbookMapTemplate::Capital:
            map.placements.push_back({"Castle", 5, 5, 2, 2});
            break;
        case WorkbookMapTemplate::City:
            break;
        case WorkbookMapTemplate::Example:
            map.labels.push_back({"2x lots", 5, 1, 1, 2});
            map.labels.push_back({"To be built", 5, 2, 1, 1});
            map.labels.push_back({"4x lots eg. castle", 5, 5, 2, 2});
            break;
        case WorkbookMapTemplate::MultiDistrict:
            map.rows = 15;
            map.columns = 15;
            map.labels = {
                {"Hill", 0, 1, 6, 1},
                {"Hill", 0, 8, 2, 1},
                {"Gate", 0, 10, 2, 1},
                {"Hill", 0, 12, 2, 1},
                {"Forest", 1, 0, 1, 6},
                {"Forest", 1, 7, 1, 6},
                {"Water", 1, 14, 1, 6},
                {"Forest", 7, 1, 6, 1},
                {"Hill", 7, 8, 6, 1},
                {"Forest", 8, 0, 1, 6},
                {"Forest", 8, 7, 1, 6},
                {"Water", 8, 14, 1, 6},
                {"Water", 14, 1, 6, 1},
                {"Water", 14, 8, 6, 1}
            };
            break;
        default:
            throw std::invalid_argument("Unsupported workbook map template");
    }

    Kingdom candidate;
    Settlement settlement;
    settlement.name = "Workbook map template";
    settlement.map = map;
    candidate.settlements.push_back(std::move(settlement));
    validate(candidate);
    return map;
}

KingdomStatResult calculate_stat(const KingdomStatInput& input, int control_dc) {
    const long long total = static_cast<long long>(input.events) + input.alignment + input.improvements + input.leadership + input.laws
        - input.unrest - input.vacancies + input.other;
    const long long deficit = static_cast<long long>(control_dc) - total;
    const long long threshold = deficit > 1 ? (deficit - 1) * 5 : 5;
    if (total < std::numeric_limits<int>::min() || total > std::numeric_limits<int>::max()
        || threshold > std::numeric_limits<int>::max()) {
        throw std::overflow_error("Kingdom stat calculation exceeds supported range");
    }
    return {static_cast<int>(total), static_cast<int>(threshold)};
}

KingdomAlignmentBonuses alignment_bonuses(const std::string& alignment) {
    static const std::vector<std::string> valid = {"LG", "NG", "CG", "LN", "N", "CN", "LE", "NE", "CE"};
    if (std::find(valid.begin(), valid.end(), alignment) == valid.end()) {
        throw std::invalid_argument("Unsupported kingdom alignment: " + alignment);
    }
    KingdomAlignmentBonuses result;
    const char first = alignment.front();
    const char last = alignment.back();
    if (first == 'L') result.economy += 2;
    if (last == 'E') result.economy += 2;
    if (first == 'C') result.loyalty += 2;
    if (last == 'G') result.loyalty += 2;
    if (first == 'N') result.stability += 2;
    if (last == 'N') result.stability += 2;
    return result;
}

namespace {
int round_away_from_zero(int value, int divisor) {
    if (value == 0) return 0;
    const long long magnitude = value > 0 ? value : -static_cast<long long>(value);
    const long long rounded = (magnitude + divisor - 1) / divisor;
    return static_cast<int>(value > 0 ? rounded : -rounded);
}
}

KingdomLawEffects calculate_laws(const KingdomLawInput& input) {
    KingdomLawEffects result;
    switch (input.promotion) {
        case PromotionLaw::None: result.stability = -1; break;
        case PromotionLaw::Token: result.stability = 1; result.consumption = 1; break;
        case PromotionLaw::Standard: result.stability = 2; result.consumption = 2; break;
        case PromotionLaw::Aggressive: result.stability = 3; result.consumption = 4; break;
        case PromotionLaw::Expansionist: result.stability = 4; result.consumption = 8; break;
        default: throw std::invalid_argument("Unsupported promotion law");
    }
    if (input.has_cathedral) result.consumption = round_away_from_zero(result.consumption, 2);

    int taxation_loyalty = 0;
    switch (input.taxation) {
        case TaxationLaw::None: result.loyalty = 1; break;
        case TaxationLaw::Light: result.economy = 1; taxation_loyalty = -1; break;
        case TaxationLaw::Normal: result.economy = 2; taxation_loyalty = -2; break;
        case TaxationLaw::Heavy: result.economy = 3; taxation_loyalty = -4; break;
        case TaxationLaw::Overwhelming: result.economy = 4; taxation_loyalty = -8; break;
        default: throw std::invalid_argument("Unsupported taxation law");
    }
    result.loyalty += input.has_waterfront ? round_away_from_zero(taxation_loyalty, 2) : taxation_loyalty;

    switch (input.holiday_interval_months) {
        case 0: result.loyalty -= 1; break;
        case 1: result.loyalty += 1; result.consumption += 1; break;
        case 6: result.loyalty += 2; result.consumption += 2; break;
        case 12: result.loyalty += 3; result.consumption += 4; break;
        case 24: result.loyalty += 4; result.consumption += 8; break;
        default: throw std::invalid_argument("Unsupported holiday interval");
    }
    return result;
}

namespace {
int checked_int(long long value, const char* description) {
    if (value < std::numeric_limits<int>::min() || value > std::numeric_limits<int>::max()) {
        throw std::overflow_error(std::string(description) + " exceeds supported range");
    }
    return static_cast<int>(value);
}
}

SettlementTotals aggregate_settlements(const std::vector<Settlement>& settlements) {
    long long population = 0, districts = 0, economy = 0, loyalty = 0, stability = 0, defense = 0;
    for (const auto& settlement : settlements) {
        if (settlement.name.empty() || settlement.population < 0 || settlement.districts < 0)
            throw std::invalid_argument("Settlement must have a name and nonnegative population/districts");
        if (settlement.apply_catalog_stat_effects)
            throw std::invalid_argument("Building catalog is required when catalog stat effects are enabled");
        population += settlement.population;
        districts += settlement.districts;
        economy += settlement.economy;
        loyalty += settlement.loyalty;
        stability += settlement.stability;
        defense += settlement.defense;
    }
    return {checked_int(static_cast<long long>(settlements.size()), "Settlement count"),
        checked_int(population, "Settlement population"), checked_int(districts, "Settlement districts"),
        checked_int(economy, "Settlement Economy"), checked_int(loyalty, "Settlement Loyalty"),
        checked_int(stability, "Settlement Stability"), checked_int(defense, "Settlement defense")};
}

SettlementTotals aggregate_settlements(const std::vector<Settlement>& settlements,
                                       const std::vector<BuildingCatalogEntry>& catalog) {
    long long population = 0, districts = 0, economy = 0, loyalty = 0, stability = 0, defense = 0, lots = 0;
    for (const auto& settlement : settlements) {
        const auto size = calculate_settlement_size(settlement, catalog);
        population += size.population;
        districts += size.districts;
        const auto effects = settlement.apply_catalog_stat_effects
            ? calculate_settlement_building_totals(settlement, catalog) : SettlementBuildingTotals{};
        economy += static_cast<long long>(settlement.economy) + effects.economy;
        loyalty += static_cast<long long>(settlement.loyalty) + effects.loyalty;
        stability += static_cast<long long>(settlement.stability) + effects.stability;
        defense += settlement.defense;
        lots += size.lots;
    }
    return {checked_int(static_cast<long long>(settlements.size()), "Settlement count"),
        checked_int(population, "Settlement population"), checked_int(districts, "Settlement districts"),
        checked_int(economy, "Settlement Economy"), checked_int(loyalty, "Settlement Loyalty"),
        checked_int(stability, "Settlement Stability"), checked_int(defense, "Settlement defense"),
        checked_int(lots, "Settlement lots")};
}

KingdomStatBreakdown calculate_stat_breakdown(const KingdomStatInput& input, int control_dc) {
    const auto result = calculate_stat(input, control_dc);
    return {input.events, input.alignment, input.improvements, input.leadership, input.laws,
        checked_int(-static_cast<long long>(input.unrest), "Unrest adjustment"),
        checked_int(-static_cast<long long>(input.vacancies), "Vacancy adjustment"),
        input.other, result.total, result.check_threshold_percent};
}

KingdomLeadershipResult calculate_leadership(const KingdomLeadershipInput& input) {
    long long economy = static_cast<long long>(input.magister.bonus) + input.marshal.bonus + input.treasurer.bonus;
    long long loyalty = static_cast<long long>(input.councilor.bonus) + input.royal_enforcer.bonus + input.warden.bonus;
    long long stability = static_cast<long long>(input.general.bonus) + input.grand_diplomat.bonus + input.high_priest.bonus;
    if (input.baron_benefits.economy) economy += input.baron_bonus;
    if (input.baron_benefits.loyalty) loyalty += input.baron_bonus;
    if (input.baron_benefits.stability) stability += input.baron_bonus;
    switch (input.spymaster_benefit) {
        case KingdomStatFocus::Economy: economy += input.spymaster.bonus; break;
        case KingdomStatFocus::Loyalty: loyalty += input.spymaster.bonus; break;
        case KingdomStatFocus::Stability: stability += input.spymaster.bonus; break;
        default: throw std::invalid_argument("Unsupported Spymaster benefit");
    }
    const int economy_vacancy = (input.magister.filled ? 0 : 4) + (input.marshal.filled ? 0 : 4) + (input.treasurer.filled ? 0 : 4);
    const int loyalty_vacancy = (input.general.filled ? 0 : 4) + (input.high_priest.filled ? 0 : 2) + (input.warden.filled ? 0 : 2);
    const int stability_vacancy = (input.grand_diplomat.filled ? 0 : 2) + (input.warden.filled ? 0 : 2);
    return {checked_int(economy, "Economy leadership bonus"), checked_int(loyalty, "Loyalty leadership bonus"),
        checked_int(stability, "Stability leadership bonus"), economy_vacancy, loyalty_vacancy, stability_vacancy};
}

int control_dc(int kingdom_size, int total_districts, int other) {
    return checked_int(20LL + kingdom_size + total_districts + other, "Control DC");
}

int population_total(int kingdom_size, int city_count, int total_city_population) {
    return checked_int(static_cast<long long>(total_city_population) + (static_cast<long long>(kingdom_size) - city_count) * 250,
        "Kingdom population");
}

KingdomSummary calculate_kingdom_summary(int unrest, const KingdomRuleInputs& input,
                                         const std::vector<Settlement>& settlements,
                                         const std::vector<BuildingCatalogEntry>* catalog) {
    const auto alignment = alignment_bonuses(input.alignment);
    const auto laws = calculate_laws(input.laws);
    const auto leadership = calculate_leadership(input.leadership);
    const auto totals = catalog ? aggregate_settlements(settlements, *catalog)
                                : aggregate_settlements(settlements);
    const bool has_settlements = !settlements.empty();
    const int district_count = has_settlements ? totals.districts : input.total_city_districts;
    const int city_count = has_settlements ? totals.count : input.city_count;
    const int city_population = has_settlements ? totals.population : input.total_city_population;
    const int dc = control_dc(input.kingdom_size, district_count, input.control_dc_other);
    const auto make_stat = [unrest, dc](const KingdomStatBaseInput& base, int settlement_bonus, int alignment_bonus,
                                        int leadership_bonus, int law_bonus, int vacancy) {
        const int improvements = checked_int(static_cast<long long>(base.improvements) + settlement_bonus,
                                             "Kingdom improvement modifier");
        return calculate_stat_breakdown({base.events, alignment_bonus, improvements, leadership_bonus,
                                         law_bonus, unrest, vacancy, base.other}, dc);
    };
    return {dc,
        make_stat(input.economy, totals.economy, alignment.economy, leadership.economy_bonus, laws.economy, leadership.economy_vacancy),
        make_stat(input.loyalty, totals.loyalty, alignment.loyalty, leadership.loyalty_bonus, laws.loyalty, leadership.loyalty_vacancy),
        make_stat(input.stability, totals.stability, alignment.stability, leadership.stability_bonus, laws.stability, leadership.stability_vacancy),
        laws, leadership, totals,
        population_total(input.kingdom_size, city_count, city_population)};
}

void save(const Kingdom& k, const std::string& path) {
    validate(k);
    const auto dest = std::filesystem::u8path(path);
    auto temp = dest;
    temp += ".tmp";
    const Json document = {
        {"schema_version", k.schema_version}, {"name", k.name}, {"turn", k.turn},
        {"treasury_bp", k.treasury_bp}, {"unrest", k.unrest},
        {"settlements", Json::array()}, {"buildings", Json::array()},
        {"turn_progress", Json::array()}, {"calendar_notes", Json::array()},
        {"rules", rule_inputs_json(k.rule_inputs)}
    };
    Json output = document;
    for (const auto& s : k.settlements) {
        Json item = {{"name", s.name}, {"population", s.population}, {"districts", s.districts},
            {"economy", s.economy}, {"loyalty", s.loyalty}, {"stability", s.stability},
            {"defense", s.defense}, {"building_inventory", Json::array()},
            {"apply_catalog_stat_effects", s.apply_catalog_stat_effects},
            {"map", {{"rows", s.map.rows}, {"columns", s.map.columns},
                {"placements", Json::array()}, {"labels", Json::array()}}}};
        for (const auto& building : s.building_inventory)
            item["building_inventory"].push_back({{"name", building.name}, {"count", building.count}});
        for (const auto& placement : s.map.placements)
            item["map"]["placements"].push_back({{"building_name", placement.building_name},
                {"row", placement.row}, {"column", placement.column},
                {"width", placement.width}, {"height", placement.height}});
        for (const auto& label : s.map.labels)
            item["map"]["labels"].push_back({{"text", label.text}, {"row", label.row},
                {"column", label.column}, {"width", label.width}, {"height", label.height}});
        output["settlements"].push_back(std::move(item));
    }
    for (const auto& b : k.buildings) output["buildings"].push_back({{"name", b.name}, {"count", b.count}});
    for (const auto& progress : k.turn_progress)
        output["turn_progress"].push_back({{"turn", progress.turn}, {"completed_step_ids", progress.completed_step_ids}});
    for (const auto& note : k.calendar_notes)
        output["calendar_notes"].push_back({{"entry_id", note.entry_id}, {"holiday", note.holiday},
            {"kingdom_upgrades", note.kingdom_upgrades}, {"events", note.events}, {"other", note.other}});
    {
        std::ofstream file(temp, std::ios::binary | std::ios::trunc);
        if (!file) throw std::runtime_error("Cannot create temporary save file");
        const auto serialized = output.dump(2, ' ', false, Json::error_handler_t::strict);
        file.write(serialized.data(), static_cast<std::streamsize>(serialized.size()));
        file.put('\n');
        file.flush();
        if (!file) throw std::runtime_error("Failed writing temporary save file");
        file.close();
        if (file.fail()) throw std::runtime_error("Failed closing temporary save file");
    }
#ifdef _WIN32
    if (!MoveFileExW(temp.c_str(), dest.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH)) {
        const auto error = GetLastError();
        DeleteFileW(temp.c_str());
        throw std::runtime_error("Cannot finalize save (Windows error " + std::to_string(error) + ")");
    }
#else
    std::error_code error;
    std::filesystem::rename(temp, dest, error);
    if (error) {
        std::filesystem::remove(temp);
        throw std::runtime_error("Cannot finalize save: " + error.message());
    }
#endif
}

Kingdom load(const std::string& path) {
    std::ifstream file(std::filesystem::u8path(path), std::ios::binary);
    if (!file) throw std::runtime_error("Cannot open kingdom file");
    const std::string text((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    const Json root = Json::parse(text);
    require_fields(root);
    Kingdom k;
    const int document_version = required_int(root, "schema_version");
    if (document_version < 1 || document_version > 9) throw std::runtime_error("Unsupported schema version: " + std::to_string(document_version));
    if (document_version >= 2 && !root.contains("rules")) throw std::runtime_error("Missing JSON field: rules");
    if (document_version >= 8 && (!root.contains("turn_progress") || !root.at("turn_progress").is_array()))
        throw std::runtime_error("turn_progress must be an array");
    if (document_version >= 9 && (!root.contains("calendar_notes") || !root.at("calendar_notes").is_array()))
        throw std::runtime_error("calendar_notes must be an array");
    k.schema_version = 9;
    k.name = required_string(root, "name");
    k.turn = required_int(root, "turn");
    k.treasury_bp = required_int(root, "treasury_bp");
    k.unrest = required_int(root, "unrest");
    if (root.contains("rules")) k.rule_inputs = read_rule_inputs(root.at("rules"));
    for (const auto& item : root.at("settlements")) {
        if (!item.is_object() || !item.contains("name") || !item.contains("population")) throw std::runtime_error("Invalid settlement entry");
        const auto read_settlement_int = [document_version, &item](const char* key) {
            if (document_version >= 3 && !item.contains(key))
                throw std::runtime_error(std::string("Missing settlement field: ") + key);
            return optional_int(item, key, 0);
        };
        k.settlements.push_back({required_string(item, "name"), required_int(item, "population"),
            read_settlement_int("districts"), read_settlement_int("economy"), read_settlement_int("loyalty"),
            read_settlement_int("stability"), read_settlement_int("defense")});
        if (document_version >= 5)
            k.settlements.back().apply_catalog_stat_effects = required_bool(item, "apply_catalog_stat_effects");
        if (document_version >= 4) {
            if (!item.contains("building_inventory") || !item.at("building_inventory").is_array())
                throw std::runtime_error("Settlement building_inventory must be an array");
            for (const auto& building : item.at("building_inventory")) {
                if (!building.is_object() || !building.contains("name") || !building.contains("count"))
                    throw std::runtime_error("Invalid settlement building entry");
                k.settlements.back().building_inventory.push_back(
                    {required_string(building, "name"), required_int(building, "count")});
            }
        }
        if (document_version >= 6) {
            if (!item.contains("map") || !item.at("map").is_object())
                throw std::runtime_error("Settlement map must be an object");
            const auto& map_json = item.at("map");
            if (!map_json.contains("placements") || !map_json.at("placements").is_array())
                throw std::runtime_error("Settlement map placements must be an array");
            auto& map = k.settlements.back().map;
            map.rows = required_int(map_json, "rows");
            map.columns = required_int(map_json, "columns");
            for (const auto& placement : map_json.at("placements")) {
                if (!placement.is_object()) throw std::runtime_error("Invalid map placement");
                map.placements.push_back({required_string(placement, "building_name"),
                    required_int(placement, "row"), required_int(placement, "column"),
                    required_int(placement, "width"), required_int(placement, "height")});
            }
            if (document_version >= 7) {
                if (!map_json.contains("labels") || !map_json.at("labels").is_array())
                    throw std::runtime_error("Settlement map labels must be an array");
                for (const auto& label : map_json.at("labels")) {
                    if (!label.is_object()) throw std::runtime_error("Invalid map label");
                    map.labels.push_back({required_string(label, "text"), required_int(label, "row"),
                        required_int(label, "column"), required_int(label, "width"), required_int(label, "height")});
                }
            }
        }
    }
    for (const auto& item : root.at("buildings")) {
        if (!item.is_object() || !item.contains("name") || !item.contains("count")) throw std::runtime_error("Invalid building entry");
        k.buildings.push_back({required_string(item, "name"), required_int(item, "count")});
    }
    if (document_version >= 8) {
        for (const auto& item : root.at("turn_progress")) {
            if (!item.is_object() || !item.contains("completed_step_ids")
                || !item.at("completed_step_ids").is_array())
                throw std::runtime_error("Invalid turn checklist record");
            TurnProgress progress;
            progress.turn = required_int(item, "turn");
            for (const auto& step_id : item.at("completed_step_ids")) {
                if (!step_id.is_string()) throw std::runtime_error("Turn checklist step id must be a string");
                progress.completed_step_ids.push_back(step_id.get<std::string>());
            }
            k.turn_progress.push_back(std::move(progress));
        }
    }
    if (document_version >= 9) {
        for (const auto& item : root.at("calendar_notes")) {
            if (!item.is_object()) throw std::runtime_error("Invalid calendar note");
            k.calendar_notes.push_back({required_string(item, "entry_id"), required_string(item, "holiday"),
                required_string(item, "kingdom_upgrades"), required_string(item, "events"),
                required_string(item, "other")});
        }
    }
    validate(k);
    return k;
}
}

