#include "pkkm/kingdom.hpp"
#include "pkkm/buildings.hpp"
#include <nlohmann/json.hpp>
#include <algorithm>
#include <cctype>
#include <cmath>
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

void reject_spreadsheet_destination(const std::filesystem::path& destination) {
    auto extension = destination.extension().string();
    std::transform(extension.begin(), extension.end(), extension.begin(),
                   [](unsigned char character) { return static_cast<char>(std::tolower(character)); });
    if (extension == ".xlsx" || extension == ".xlsm" || extension == ".xls")
        throw std::invalid_argument("Refusing to overwrite a spreadsheet source file");
}

bool required_bool(const Json& object, const char* key) {
    const auto& field = object.at(key);
    if (!field.is_boolean()) throw std::runtime_error(std::string("Invalid boolean field: ") + key);
    return field.get<bool>();
}

double required_number(const Json& object, const char* key) {
    const auto& field = object.at(key);
    if (!field.is_number()) throw std::runtime_error(std::string("Invalid number field: ") + key);
    return field.get<double>();
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

const char* turn_action_name(TurnAction action) {
    switch (action) {
        case TurnAction::ClaimHex: return "claim_hex";
        case TurnAction::AbandonHex: return "abandon_hex";
        case TurnAction::AbandonCity: return "abandon_city";
    }
    throw std::invalid_argument("Unsupported Turn action");
}

TurnAction read_turn_action(const Json& object, const char* key) {
    const auto value = required_string(object, key);
    if (value == "claim_hex") return TurnAction::ClaimHex;
    if (value == "abandon_hex") return TurnAction::AbandonHex;
    if (value == "abandon_city") return TurnAction::AbandonCity;
    throw std::runtime_error("Unsupported Turn action: " + value);
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
    if (k.schema_version != 10) throw std::invalid_argument("Unsupported schema version");
    if (k.name.empty()) throw std::invalid_argument("Kingdom name must not be empty");
    if (k.turn < 1) throw std::invalid_argument("Turn must be at least 1");

    for (const auto& s : k.settlements) {
        if (s.name.empty() || s.population < 0 || s.districts < 0)
            throw std::invalid_argument("Settlement must have a name and nonnegative population/districts");
        if (s.city_layout.base_value < 0)
            throw std::invalid_argument("City worksheet base value cannot be negative");
        for (const auto& column : s.city_layout.item_columns)
            if (column.size() > 7) throw std::invalid_argument("City worksheet item columns cannot exceed seven entries");
        for (const auto& building : s.building_inventory) {
            if (building.name.empty() || building.count < 0)
                throw std::invalid_argument("Settlement building must have a name and nonnegative count");
        }
        for (const auto& stock : s.random_magic_item_stock) {
            if (stock.tier != "minor" && stock.tier != "medium" && stock.tier != "major")
                throw std::invalid_argument("Settlement random magic-item stock has an unsupported tier");
            if (stock.slot_index < 1 || stock.state.empty())
                throw std::invalid_argument("Settlement random magic-item stock requires a positive slot and state");
            if (stock.state != "empty" && stock.state != "generated"
                && stock.state != "sold" && stock.state != "manual_resolution")
                throw std::invalid_argument("Settlement random magic-item stock has an unsupported state");
            if (stock.price_gp < 0 || stock.generated_turn < 0 || stock.last_changed_turn < 0)
                throw std::invalid_argument("Settlement random magic-item stock values cannot be negative");
            if (stock.state == "generated" && stock.item_name.empty())
                throw std::invalid_argument("Generated random magic-item stock requires an item name");
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
    std::vector<std::string> settlement_ids;
    for (const auto& settlement : k.settlements) {
        if (settlement.id.empty()) continue;
        if (std::find(settlement_ids.begin(), settlement_ids.end(), settlement.id) != settlement_ids.end())
            throw std::invalid_argument("Settlement IDs must be unique");
        settlement_ids.push_back(settlement.id);
    }
    std::vector<std::pair<int, int>> greenbelt_coordinates;
    for (const auto& hex : k.greenbelt_map.hexes) {
        if (hex.terrain.empty()) throw std::invalid_argument("Greenbelt hex terrain must not be empty");
        if (hex.claimed && hex.claim_turn < 1) throw std::invalid_argument("Claimed Greenbelt hex needs a positive claim turn");
        if (!hex.claimed && hex.claim_turn != 0) throw std::invalid_argument("Unclaimed Greenbelt hex cannot have a claim turn");
        if (!hex.settlement_id.empty()
            && std::find(settlement_ids.begin(), settlement_ids.end(), hex.settlement_id) == settlement_ids.end())
            throw std::invalid_argument("Greenbelt hex references an unknown settlement ID");
        if (std::find(greenbelt_coordinates.begin(), greenbelt_coordinates.end(), std::make_pair(hex.q, hex.r))
            != greenbelt_coordinates.end())
            throw std::invalid_argument("Greenbelt hex coordinates must be unique");
        greenbelt_coordinates.emplace_back(hex.q, hex.r);
    }
    const auto& calibration = k.greenbelt_map.calibration;
    for (const double value : {calibration.origin_x, calibration.origin_y, calibration.hex_width,
                               calibration.hex_height, calibration.scale}) {
        if (!std::isfinite(value)) throw std::invalid_argument("Greenbelt calibration values must be finite");
    }
    if (k.greenbelt_map.image_width < 0 || k.greenbelt_map.image_height < 0
        || calibration.hex_width < 0 || calibration.hex_height < 0 || calibration.scale <= 0)
        throw std::invalid_argument("Greenbelt image and calibration dimensions are invalid");
    for (const auto& terrain : k.greenbelt_map.special_terrain) {
        if (terrain.type.empty()) throw std::invalid_argument("Greenbelt special terrain type must not be empty");
        if (terrain.name.empty()) throw std::invalid_argument("Greenbelt special terrain name must not be empty");
    }
    std::vector<std::string> custom_improvement_ids;
    for (const auto& improvement : k.greenbelt_map.custom_improvements) {
        if (improvement.id.empty() || improvement.name.empty())
            throw std::invalid_argument("Greenbelt custom improvements need an ID and name");
        if (std::find(custom_improvement_ids.begin(), custom_improvement_ids.end(), improvement.id)
            != custom_improvement_ids.end())
            throw std::invalid_argument("Greenbelt custom improvement IDs must be unique");
        if (std::any_of(improvement.effects.begin(), improvement.effects.end(),
                [](const auto& effect) { return effect.empty(); })
            || std::any_of(improvement.costs.begin(), improvement.costs.end(),
                [](const auto& cost) { return cost.empty(); }))
            throw std::invalid_argument("Greenbelt custom improvement effect and cost entries must not be empty");
        custom_improvement_ids.push_back(improvement.id);
    }
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
        for (const auto& id : progress.skipped_step_ids) {
            const auto& steps = turn_checklist_steps();
            const auto step = std::find_if(steps.begin(), steps.end(),
                [&id](const auto& candidate) { return candidate.id == id; });
            if (step == steps.end()) throw std::invalid_argument("Turn checklist contains an unknown skipped step id");
            if (!step->optional) throw std::invalid_argument("Only optional checklist steps can be skipped");
            if (std::find(seen_steps.begin(), seen_steps.end(), id) != seen_steps.end())
                throw std::invalid_argument("Turn checklist step ids must be unique");
            seen_steps.push_back(id);
        }
    }
    for (std::size_t index = 0; index < k.turn_actions.size(); ++index) {
        const auto& record = k.turn_actions[index];
        if (record.turn < 1 || record.count < 1)
            throw std::invalid_argument("Turn action records need a positive turn and count");
        if (record.reversal) {
            if (record.reverses_index < 0
                || static_cast<std::size_t>(record.reverses_index) >= index)
                throw std::invalid_argument("Turn action reversal must reference an earlier action");
            for (std::size_t prior = index + 1; prior < k.turn_actions.size(); ++prior) {
                if (k.turn_actions[prior].reverses_index == record.reverses_index)
                    throw std::invalid_argument("A Turn Action may only be reversed once");
            }
        } else if (record.reverses_index != -1) {
            throw std::invalid_argument("Non-reversal Turn Actions cannot reference another action");
        }
        const auto expected = resolve_turn_action(record.action);
        const long long sign = record.reversal ? -1LL : 1LL;
        const long long expected_treasury = static_cast<long long>(expected.treasury_delta)
            * record.count * sign;
        const long long expected_kingdom_size = static_cast<long long>(expected.kingdom_size_delta)
            * record.count * sign;
        const long long expected_unrest = static_cast<long long>(expected.unrest_delta)
            * record.count * sign;
        if (expected_treasury < std::numeric_limits<int>::min()
            || expected_treasury > std::numeric_limits<int>::max()
            || expected_kingdom_size < std::numeric_limits<int>::min()
            || expected_kingdom_size > std::numeric_limits<int>::max()
            || expected_unrest < std::numeric_limits<int>::min()
            || expected_unrest > std::numeric_limits<int>::max()
            || record.applied.treasury_delta != expected_treasury
            || record.applied.kingdom_size_delta != expected_kingdom_size
            || record.applied.unrest_delta != expected_unrest)
            throw std::invalid_argument("Turn action record has inconsistent deltas");
    }
    for (const auto& record : k.tax_actions) {
        if (record.turn < 1 || record.result_input < 0 || record.other_bp_gained < 0)
            throw std::invalid_argument("Tax records require a positive turn and non-negative inputs");
        if (record.treasury_delta != resolve_tax_bp_delta(record.rule_profile, record.economy_check_success,
                record.result_input, record.other_bp_gained))
            throw std::invalid_argument("Tax record has an inconsistent treasury delta");
    }
    for (const auto& record : k.gold_withdrawals) {
        const long long expected_gold = static_cast<long long>(record.bp_withdrawn) * 2000;
        if (record.turn < 1 || record.bp_withdrawn < 1
            || expected_gold != record.gold_received
            || record.treasury_delta != -record.bp_withdrawn
            || static_cast<long long>(record.unrest_delta)
                != static_cast<long long>(record.bp_withdrawn) + record.loyalty_failure_unrest_delta
            || record.loyalty_failure_unrest_delta != (record.loyalty_check_success ? 0 : record.bp_withdrawn))
            throw std::invalid_argument("Gold withdrawal record has inconsistent values");
    }
    for (const auto& record : k.stability_checks) {
        if (record.turn < 1 || record.failure_by < 0 || record.rolled_unrest < 0 || record.rolled_unrest > 4
            || (record.success && (record.failure_by != 0 || record.rolled_unrest != 0))
            || (!record.success && record.failure_by == 4)
            || record.treasury_delta < 0 || record.unrest_delta < -1
            || (record.success && !((record.treasury_delta == 1 && record.unrest_delta == 0)
                || (record.treasury_delta == 0 && record.unrest_delta == -1)))
            || (!record.success && record.treasury_delta != 0)
            || (!record.success && record.failure_by < 4 && record.unrest_delta != 1)
            || (!record.success && record.failure_by >= 5 && record.unrest_delta != 2))
            throw std::invalid_argument("Stability check record has inconsistent values");
    }
    for (const auto& record : k.consumption_actions) {
        if (record.turn < 1 || record.consumption_bp < 1
            || record.treasury_delta != -record.consumption_bp
            || record.unrest_delta != (record.paid ? 0 : 2))
            throw std::invalid_argument("Consumption record has inconsistent values");
    }
    for (const auto& record : k.unrest_modifications) {
        if (record.turn < 1 || record.negative_stat_count < 0
            || record.unrest_delta != record.negative_stat_count)
            throw std::invalid_argument("Unrest modification record has inconsistent values");
    }
    for (const auto& record : k.gold_deposits) {
        const long long expected_gold = static_cast<long long>(record.bp_received) * 4000;
        if (record.turn < 1 || record.gold_deposited < 1 || record.bp_received < 1
            || expected_gold != record.gold_deposited
            || record.treasury_delta != record.bp_received)
            throw std::invalid_argument("Gold deposit record has inconsistent values");
    }
    for (const auto& record : k.item_deposits) {
        const long long expected_value = static_cast<long long>(record.bp_received)
            * (record.rule_profile == RuleProfile::Workbook ? 8000 : 4000);
        if (record.turn < 1 || record.item_value < 1 || record.bp_received < 1
            || expected_value != record.item_value
            || record.treasury_delta != record.bp_received
            || record.item_value != static_cast<long long>(record.bp_received)
                * (record.rule_profile == RuleProfile::Workbook ? 8000 : 4000))
            throw std::invalid_argument("Item deposit record has inconsistent values");
    }
    for (const auto& record : k.loyalty_checks) {
        if (record.turn < 1
            || (record.success && (record.loyalty_delta != 0 || record.unrest_delta > 0))
            || (!record.success && (record.loyalty_delta != -1 || record.unrest_delta != 0)))
            throw std::invalid_argument("Loyalty check record has inconsistent values");
    }
    std::map<int, int> commissions_by_turn;
    for (const auto& record : k.commissions) {
        if (record.turn < 1 || record.item_name.empty() || record.recipient.empty()
            || record.cost_bp < 1 || record.exotic_crafters_available < 1)
            throw std::invalid_argument("Commission record has invalid values");
        const int count = ++commissions_by_turn[record.turn];
        if (count > record.exotic_crafters_available)
            throw std::invalid_argument("Commission records exceed available exotic crafters");
    }
    for (const auto& record : k.magic_item_outcomes) {
        const int expected_bp = record.commissioned || !record.sold ? 0 : magic_item_sale_bp(record.category);
        if (record.turn < 1 || (record.sold && record.commissioned)
            || record.bp_received != expected_bp
            || record.lost != (!record.commissioned && !record.sold))
            throw std::invalid_argument("Magic-item outcome record has inconsistent values");
    }
    for (std::size_t index = 0; index < k.valuable_item_sales.size(); ++index) {
        const auto& record = k.valuable_item_sales[index];
        const int expected_dc = valuable_item_sale_dc(record.category);
        const bool expected_sold = record.economy_check_result >= expected_dc;
        const int expected_bp = expected_sold ? magic_item_sale_bp(record.category) : 0;
        const auto city = std::find_if(k.settlements.begin(), k.settlements.end(),
            [&record](const Settlement& settlement) { return settlement.id == record.city_id; });
        if (record.turn < 1 || record.item_name.empty() || record.city_id.empty() || record.district < 1
            || city == k.settlements.end() || record.district > city->districts
            || record.item_value_gp <= 4000 || record.economy_check_dc != expected_dc
            || record.sold != expected_sold || record.bp_received != expected_bp)
            throw std::invalid_argument("Valuable-item sale record has inconsistent values");
        for (std::size_t prior = 0; prior < index; ++prior) {
            const auto& earlier = k.valuable_item_sales[prior];
            if (earlier.turn == record.turn && earlier.city_id == record.city_id
                && earlier.district == record.district)
                throw std::invalid_argument("Valuable-item sale records exceed one check per city district per turn");
        }
    }
    for (const auto& event : k.kingdom_events) {
        if (event.turn < 1 || event.d100_roll < 1 || event.d100_roll > 100
            || kingdom_event_from_d100(event.d100_roll).kind != event.kind
            || (event.continuous_active && !kingdom_event_from_d100(event.d100_roll).continuous)
            || (event.resolved && event.continuous_active))
            throw std::invalid_argument("Kingdom event record has inconsistent values");
    }
    for (std::size_t index = 0; index < k.construction_transactions.size(); ++index) {
        const auto& record = k.construction_transactions[index];
        if (record.turn < 1 || record.settlement_id.empty() || record.lines.empty()
            || (!record.reversal && record.bp_cost < 0)
            || (!record.successful && !record.reversal && record.failures.empty())
            || (!record.reversal && record.reverses_index != -1)
            || (record.reversal && (!record.successful || record.bp_cost > 0
                || record.reverses_index < 0 || static_cast<std::size_t>(record.reverses_index) >= index)))
            throw std::invalid_argument("Construction record has inconsistent values");
        if (record.reversal) {
            const auto& original = k.construction_transactions[static_cast<std::size_t>(record.reverses_index)];
            if (!original.successful || original.reversal
                || record.settlement_id != original.settlement_id
                || record.bp_cost != -original.bp_cost
                || record.lines.size() != original.lines.size())
                throw std::invalid_argument("Construction reversal does not reference a forward success");
            for (std::size_t line_index = 0; line_index < original.lines.size(); ++line_index) {
                const auto& reversed_line = record.lines[line_index];
                const auto& original_line = original.lines[line_index];
                if (reversed_line.building_name != original_line.building_name
                    || reversed_line.count != original_line.count
                    || reversed_line.upgrade_from != original_line.upgrade_from
                    || reversed_line.row != original_line.row
                    || reversed_line.column != original_line.column
                    || reversed_line.width != original_line.width
                    || reversed_line.height != original_line.height
                    || reversed_line.source_width != original_line.source_width
                    || reversed_line.source_height != original_line.source_height)
                    throw std::invalid_argument("Construction reversal lines do not match the forward success");
            }
            for (std::size_t prior = 0; prior < index; ++prior)
                if (prior != static_cast<std::size_t>(record.reverses_index)
                    && k.construction_transactions[prior].reversal
                    && k.construction_transactions[prior].reverses_index == record.reverses_index)
                    throw std::invalid_argument("Construction has duplicate reversal records");
        }
        for (const auto& line : record.lines) {
            if (line.building_name.empty() || line.count < 1 || line.row < 0 || line.column < 0
                || line.width < 1 || line.height < 1)
                throw std::invalid_argument("Construction record contains an invalid line");
        }
    }
    for (const auto& army : k.armies) {
        if (army.turn < 1 || army.request.name.empty() || army.request.unit_description.empty()
            || army.soldiers < 1 || army.army_cr < 1 || army.hit_points < 1
            || army.current_hit_points < 0 || army.current_hit_points > army.hit_points
            || army.defense_value < 1 || army.weekly_consumption < 1 || army.bp_cost < 0)
            throw std::invalid_argument("Army record has inconsistent values");
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

TurnChecklistStatus turn_checklist_status(const Kingdom& kingdom, int turn) {
    if (turn < 1) throw std::invalid_argument("Checklist turn must be at least 1");
    TurnChecklistStatus status;
    status.turn = turn;
    status.steps = turn_checklist_steps();
    const auto record = std::find_if(kingdom.turn_progress.begin(), kingdom.turn_progress.end(),
        [turn](const auto& progress) { return progress.turn == turn; });
    if (record == kingdom.turn_progress.end()) {
        status.next_step_id = status.steps.empty() ? std::string{} : status.steps.front().id;
        return status;
    }
    for (const auto& step : status.steps) {
        if (std::find(record->completed_step_ids.begin(), record->completed_step_ids.end(), step.id)
            != record->completed_step_ids.end())
            status.completed_step_ids.push_back(step.id);
        else if (std::find(record->skipped_step_ids.begin(), record->skipped_step_ids.end(), step.id)
                != record->skipped_step_ids.end())
            status.skipped_step_ids.push_back(step.id);
        else if (status.next_step_id.empty())
            status.next_step_id = step.id;
    }
    return status;
}

void set_turn_checklist_step(Kingdom& kingdom, int turn, const std::string& step_id, bool completed) {
    if (turn < 1) throw std::invalid_argument("Checklist turn must be at least 1");
    const auto& steps = turn_checklist_steps();
    if (std::none_of(steps.begin(), steps.end(),
            [&step_id](const auto& step) { return step.id == step_id; }))
        throw std::invalid_argument("Turn checklist contains an unknown step id");
    auto record = std::find_if(kingdom.turn_progress.begin(), kingdom.turn_progress.end(),
        [turn](const auto& progress) { return progress.turn == turn; });
    if (completed) {
        if (record == kingdom.turn_progress.end()) {
            kingdom.turn_progress.push_back({turn, {step_id}, {}});
        } else {
            record->skipped_step_ids.erase(
                std::remove(record->skipped_step_ids.begin(), record->skipped_step_ids.end(), step_id),
                record->skipped_step_ids.end());
            if (std::find(record->completed_step_ids.begin(), record->completed_step_ids.end(), step_id)
                    == record->completed_step_ids.end())
                record->completed_step_ids.push_back(step_id);
        }
    } else if (record != kingdom.turn_progress.end()) {
        record->completed_step_ids.erase(
            std::remove(record->completed_step_ids.begin(), record->completed_step_ids.end(), step_id),
            record->completed_step_ids.end());
        if (record->completed_step_ids.empty() && record->skipped_step_ids.empty()) kingdom.turn_progress.erase(record);
    }
    validate(kingdom);
}

void skip_turn_checklist_step(Kingdom& kingdom, int turn, const std::string& step_id) {
    if (turn < 1) throw std::invalid_argument("Checklist turn must be at least 1");
    const auto& steps = turn_checklist_steps();
    const auto step = std::find_if(steps.begin(), steps.end(),
        [&step_id](const auto& candidate) { return candidate.id == step_id; });
    if (step == steps.end()) throw std::invalid_argument("Turn checklist contains an unknown step id");
    if (!step->optional) throw std::invalid_argument("Only optional checklist steps can be skipped");
    auto record = std::find_if(kingdom.turn_progress.begin(), kingdom.turn_progress.end(),
        [turn](const auto& progress) { return progress.turn == turn; });
    if (record == kingdom.turn_progress.end()) {
        kingdom.turn_progress.push_back({turn, {}, {step_id}});
    } else {
        record->completed_step_ids.erase(
            std::remove(record->completed_step_ids.begin(), record->completed_step_ids.end(), step_id),
            record->completed_step_ids.end());
        if (std::find(record->skipped_step_ids.begin(), record->skipped_step_ids.end(), step_id)
                == record->skipped_step_ids.end())
            record->skipped_step_ids.push_back(step_id);
    }
    validate(kingdom);
}

StabilityFailureOutcome classify_stability_failure(int failure_by) {
    if (failure_by < 4) return StabilityFailureOutcome::FailByLessThanFour;
    if (failure_by == 4) throw std::invalid_argument("Stability failure margin of 4 is not used by this rule set");
    return StabilityFailureOutcome::FailByAtLeastFive;
}

StabilityCheckPreview preview_stability_check(RuleProfile profile, bool success,
    int current_unrest, int failure_by, int rolled_unrest) {
    if (current_unrest < 0) throw std::invalid_argument("Current unrest cannot be negative");
    if (success)
        return {true, StabilityFailureOutcome::FailByLessThanFour,
            current_unrest == 0 ? StabilityCheckOutcome{1, 0} : StabilityCheckOutcome{0, -1}};
    if (failure_by < 0) throw std::invalid_argument("Stability failure margin cannot be negative");
    const auto failure_outcome = classify_stability_failure(failure_by);
    return {false, failure_outcome,
        resolve_stability_check(profile, false, current_unrest, failure_by, rolled_unrest)};
}

StabilityCheckOutcome resolve_stability_check(bool success, int current_unrest,
    int failure_by, int rolled_unrest) {
    return resolve_stability_check(RuleProfile::Workbook, success, current_unrest,
        failure_by, rolled_unrest);
}

StabilityCheckOutcome resolve_stability_check(RuleProfile profile, bool success,
    int current_unrest, int failure_by, int rolled_unrest) {
    if (current_unrest < 0) throw std::invalid_argument("Current unrest cannot be negative");
    (void)profile;
    (void)rolled_unrest;
    if (success) return current_unrest == 0 ? StabilityCheckOutcome{1, 0} : StabilityCheckOutcome{0, -1};
    if (failure_by < 0) throw std::invalid_argument("Stability failure margin cannot be negative");
    switch (classify_stability_failure(failure_by)) {
        case StabilityFailureOutcome::FailByLessThanFour: return {0, 1};
        case StabilityFailureOutcome::FailByAtLeastFive:
            return {0, 2};
    }
    throw std::logic_error("Unknown Stability failure outcome");
}

FoodResourcePreview preview_food_resource(int starting_consumption, int farm_count, int extra_reduction) {
    if (starting_consumption < 0 || farm_count < 0 || extra_reduction < 0)
        throw std::invalid_argument("Food resource inputs cannot be negative");
    const long long farm_reduction = static_cast<long long>(farm_count) * 2;
    const long long effective = static_cast<long long>(starting_consumption) - farm_reduction - extra_reduction;
    if (farm_reduction > std::numeric_limits<int>::max()
        || effective < std::numeric_limits<int>::min() || effective > std::numeric_limits<int>::max())
        throw std::overflow_error("Food resource preview would overflow");
    const long long surplus = effective < 0 ? -effective : 0;
    if (surplus > std::numeric_limits<int>::max())
        throw std::overflow_error("Food resource surplus would overflow");
    return {starting_consumption, static_cast<int>(farm_reduction), extra_reduction,
        static_cast<int>(effective), static_cast<int>(surplus)};
}

int preview_quarry_income(int quarry_count) {
    if (quarry_count < 0) throw std::invalid_argument("Quarry count cannot be negative");
    return quarry_count;
}

OptionalMinePreview preview_optional_mine_rule(int starting_bp, int months_elapsed) {
    if (starting_bp < 0) throw std::invalid_argument("Mine starting value cannot be negative");
    if (months_elapsed < 0) throw std::invalid_argument("Mine elapsed months cannot be negative");
    const long long remaining = std::max(0LL,
        static_cast<long long>(starting_bp) - months_elapsed);
    const long long player_share = remaining * 75 / 100;
    const long long unresolved_remainder = remaining - player_share;
    if (remaining > std::numeric_limits<int>::max()
        || player_share > std::numeric_limits<int>::max()
        || unresolved_remainder > std::numeric_limits<int>::max())
        throw std::overflow_error("Optional mine preview would overflow");
    return {starting_bp, months_elapsed, static_cast<int>(remaining),
        static_cast<int>(player_share), static_cast<int>(unresolved_remainder),
        remaining == 0, true, true};
}

UpkeepPreview preview_upkeep(const Kingdom& kingdom, bool stability_success,
    int consumption_bp, bool royal_assassin_employed, int failure_by, int rolled_unrest) {
    validate(kingdom);
    UpkeepPreview preview;
    preview.starting_treasury = kingdom.treasury_bp;
    preview.starting_unrest = kingdom.unrest;
    preview.projected_treasury = kingdom.treasury_bp;
    preview.projected_unrest = kingdom.unrest;
    const bool controls_hex = std::any_of(kingdom.greenbelt_map.hexes.begin(),
        kingdom.greenbelt_map.hexes.end(), [](const auto& hex) { return hex.claimed; });
    if (!controls_hex) {
        preview.skipped_for_no_hexes = true;
        return preview;
    }
    if (consumption_bp < 1) throw std::invalid_argument("Consumption must be at least 1 BP");
    const auto stability = preview_stability_check(kingdom.rule_profile, stability_success,
        kingdom.unrest, failure_by, rolled_unrest);
    preview.stability_treasury_delta = stability.outcome.treasury_delta;
    preview.stability_unrest_delta = stability.outcome.unrest_delta;
    const long long treasury_after_stability = static_cast<long long>(kingdom.treasury_bp)
        + preview.stability_treasury_delta;
    if (treasury_after_stability < std::numeric_limits<int>::min()
        || treasury_after_stability > std::numeric_limits<int>::max())
        throw std::overflow_error("Upkeep stability result would overflow treasury");
    preview.consumption_paid = treasury_after_stability >= consumption_bp;
    preview.consumption_treasury_delta = -consumption_bp;
    preview.consumption_unrest_delta = preview.consumption_paid ? 0 : 2;
    const long long projected_treasury = treasury_after_stability + preview.consumption_treasury_delta;
    if (projected_treasury < std::numeric_limits<int>::min()
        || projected_treasury > std::numeric_limits<int>::max())
        throw std::overflow_error("Upkeep consumption result would overflow treasury");
    preview.projected_treasury = static_cast<int>(projected_treasury);
    preview.negative_treasury_threat = preview.projected_treasury < 0;
    preview.negative_treasury_unrest_delta = preview.negative_treasury_threat ? 2 : 0;
    const long long unrest_before_assassin = static_cast<long long>(kingdom.unrest)
        + preview.stability_unrest_delta + preview.consumption_unrest_delta
        + preview.negative_treasury_unrest_delta;
    if (unrest_before_assassin < 0 || unrest_before_assassin > std::numeric_limits<int>::max())
        throw std::overflow_error("Upkeep result would overflow unrest");
    preview.threshold_threat_before_royal_assassin = unrest_before_assassin >= 11;
    preview.anarchy_threat_before_royal_assassin = unrest_before_assassin >= 20;
    if (royal_assassin_employed && unrest_before_assassin > 0) {
        preview.royal_assassin_unrest_delta = -1;
    }
    preview.projected_unrest = static_cast<int>(unrest_before_assassin
        + preview.royal_assassin_unrest_delta);
    // The PDF resolves the 11/20 consequences before the Royal Assassin's
    // end-of-Upkeep reduction. Keep final flags tied to that threat state.
    preview.loses_hex = preview.threshold_threat_before_royal_assassin;
    preview.anarchy = preview.anarchy_threat_before_royal_assassin;
    return preview;
}

const char* turn_action_checklist_step(TurnAction action) {
    switch (action) {
        case TurnAction::ClaimHex: return "edicts.claim_hexes";
        case TurnAction::AbandonHex: return "edicts.abandon_hexes";
        case TurnAction::AbandonCity: return "edicts.city";
    }
    return "";
}

TurnActionOutcome resolve_turn_action(TurnAction action) {
    switch (action) {
        case TurnAction::ClaimHex: return {-1, 1, 0};
        case TurnAction::AbandonHex: return {0, -1, 1};
        case TurnAction::AbandonCity: return {0, -1, 4};
        default: return {};
    }
}

void advance_turn(Kingdom& kingdom) {
    validate(kingdom);
    if (kingdom.turn == std::numeric_limits<int>::max())
        throw std::overflow_error("Cannot advance beyond the maximum supported turn");
    ++kingdom.turn;
    validate(kingdom);
}

TurnResolutionPreview preview_turn_resolution(const Kingdom& kingdom,
    const TurnResolutionRequest& request) {
    validate(kingdom);
    if (kingdom.turn == std::numeric_limits<int>::max())
        throw std::overflow_error("Cannot preview beyond the maximum supported turn");
    const auto upkeep = preview_upkeep(kingdom, request.stability_success,
        request.consumption_bp, request.royal_assassin_employed,
        request.failure_by, request.rolled_unrest);
    return {kingdom.turn, kingdom.turn + 1, upkeep, true};
}

void apply_turn_resolution(Kingdom& kingdom, const TurnResolutionRequest& request) {
    validate(kingdom);
    const auto preview = preview_turn_resolution(kingdom, request);
    if (preview.upkeep.loses_hex || preview.upkeep.anarchy)
        throw std::logic_error("Turn resolution requires manual threshold consequence resolution");
    Kingdom candidate = kingdom;
    if (!preview.upkeep.skipped_for_no_hexes) {
        candidate.treasury_bp = preview.upkeep.projected_treasury;
        candidate.unrest = preview.upkeep.projected_unrest;
        candidate.stability_checks.push_back({candidate.turn, request.stability_success,
            request.failure_by, request.rolled_unrest,
            preview.upkeep.stability_treasury_delta, preview.upkeep.stability_unrest_delta,
            candidate.rule_profile});
        candidate.consumption_actions.push_back({candidate.turn, request.consumption_bp,
            preview.upkeep.consumption_paid, preview.upkeep.consumption_treasury_delta,
            preview.upkeep.consumption_unrest_delta});
    }
    if (candidate.turn == std::numeric_limits<int>::max())
        throw std::overflow_error("Cannot advance beyond the maximum supported turn");
    ++candidate.turn;
    validate(candidate);
    kingdom = std::move(candidate);
}

std::size_t record_kingdom_event(Kingdom& kingdom, int d100_roll) {
    validate(kingdom);
    const auto& definition = kingdom_event_from_d100(d100_roll);
    Kingdom candidate = kingdom;
    candidate.kingdom_events.push_back({candidate.turn, d100_roll, definition.kind, false,
        definition.continuous, {}, {}, {}, {}, {}, 0, 0, 0, false, false, false});
    validate(candidate);
    kingdom = std::move(candidate);
    return kingdom.kingdom_events.size() - 1;
}

void apply_manual_event_consequence(Kingdom& kingdom, std::size_t event_index) {
    validate(kingdom);
    if (event_index >= kingdom.kingdom_events.size())
        throw std::out_of_range("Kingdom event index is out of range");
    Kingdom candidate = kingdom;
    auto& event = candidate.kingdom_events[event_index];
    if (event.consequences_applied)
        throw std::invalid_argument("Manual event consequences were already applied");
    const long long treasury = static_cast<long long>(candidate.treasury_bp) + event.treasury_delta;
    const long long unrest = static_cast<long long>(candidate.unrest) + event.unrest_delta;
    if (treasury < std::numeric_limits<int>::min() || treasury > std::numeric_limits<int>::max()
        || unrest < 0 || unrest > std::numeric_limits<int>::max())
        throw std::overflow_error("Manual event consequence exceeds supported kingdom state");
    candidate.treasury_bp = static_cast<int>(treasury);
    candidate.unrest = static_cast<int>(unrest);
    event.consequences_applied = true;
    validate(candidate);
    kingdom = std::move(candidate);
}

void apply_turn_action(Kingdom& kingdom, TurnAction action, int count) {
    validate(kingdom);
    if (count < 1) throw std::invalid_argument("Turn action count must be positive");
    const auto single = resolve_turn_action(action);
    const long long treasury_delta = static_cast<long long>(single.treasury_delta) * count;
    const long long kingdom_size_delta = static_cast<long long>(single.kingdom_size_delta) * count;
    const long long unrest_delta = static_cast<long long>(single.unrest_delta) * count;
    const long long projected_treasury = static_cast<long long>(kingdom.treasury_bp) + treasury_delta;
    const long long projected_kingdom_size = static_cast<long long>(kingdom.rule_inputs.kingdom_size)
        + kingdom_size_delta;
    const long long projected_unrest = static_cast<long long>(kingdom.unrest) + unrest_delta;
    if (treasury_delta < std::numeric_limits<int>::min()
        || treasury_delta > std::numeric_limits<int>::max()
        || kingdom_size_delta < std::numeric_limits<int>::min()
        || kingdom_size_delta > std::numeric_limits<int>::max()
        || unrest_delta < std::numeric_limits<int>::min()
        || unrest_delta > std::numeric_limits<int>::max()
        || projected_treasury < 0 || projected_treasury > std::numeric_limits<int>::max())
        throw std::overflow_error("Turn action exceeds supported treasury state");
    if (projected_treasury < 0)
        throw std::invalid_argument("Turn action would make treasury negative");
    if (projected_kingdom_size < 0 || projected_kingdom_size > std::numeric_limits<int>::max())
        throw std::invalid_argument("Turn action would make kingdom size negative");
    if (projected_unrest < 0 || projected_unrest > std::numeric_limits<int>::max())
        throw std::overflow_error("Turn action would exceed supported unrest state");
    const int treasury_delta_int = static_cast<int>(treasury_delta);
    const int kingdom_size_delta_int = static_cast<int>(kingdom_size_delta);
    const int unrest_delta_int = static_cast<int>(unrest_delta);
    kingdom.treasury_bp = static_cast<int>(projected_treasury);
    kingdom.rule_inputs.kingdom_size = static_cast<int>(projected_kingdom_size);
    kingdom.unrest = static_cast<int>(projected_unrest);
    kingdom.turn_actions.push_back({kingdom.turn, action, count,
        {treasury_delta_int, kingdom_size_delta_int, unrest_delta_int}, false});
    validate(kingdom);
}

void reverse_last_turn_action(Kingdom& kingdom) {
    validate(kingdom);
    if (kingdom.turn_actions.empty())
        throw std::invalid_argument("There is no Turn Action to reverse");
    reverse_turn_action(kingdom, kingdom.turn_actions.size() - 1);
}

void reverse_turn_action(Kingdom& kingdom, std::size_t index) {
    validate(kingdom);
    if (index >= kingdom.turn_actions.size())
        throw std::invalid_argument("Selected Turn Action does not exist");
    const auto& original = kingdom.turn_actions[index];
    if (original.reversal)
        throw std::invalid_argument("A reversal record cannot be reversed");
    for (const auto& record : kingdom.turn_actions) {
        if (record.reverses_index == static_cast<std::ptrdiff_t>(index))
            throw std::invalid_argument("The selected Turn Action has already been reversed");
    }
    const auto inverse = TurnActionOutcome{
        -original.applied.treasury_delta,
        -original.applied.kingdom_size_delta,
        -original.applied.unrest_delta};
    if (kingdom.treasury_bp + inverse.treasury_delta < 0)
        throw std::invalid_argument("Reversal would make treasury negative");
    if (kingdom.rule_inputs.kingdom_size + inverse.kingdom_size_delta < 0)
        throw std::invalid_argument("Reversal would make kingdom size negative");
    kingdom.treasury_bp += inverse.treasury_delta;
    kingdom.rule_inputs.kingdom_size += inverse.kingdom_size_delta;
    kingdom.unrest += inverse.unrest_delta;
    kingdom.turn_actions.push_back({kingdom.turn, original.action, original.count, inverse, true,
        static_cast<std::ptrdiff_t>(index)});
    validate(kingdom);
}

namespace {
const BuildingCatalogEntry& construction_catalog_entry(
    const std::vector<BuildingCatalogEntry>& catalog, const std::string& name) {
    const auto found = std::find_if(catalog.begin(), catalog.end(), [&name](const auto& entry) {
        return entry.kind == CatalogRowKind::Improvement && entry.name == name;
    });
    if (found == catalog.end())
        throw std::invalid_argument("Building is not present in the improvement catalog: " + name);
    return *found;
}

int construction_inventory_count(const Settlement& settlement, const std::string& name) {
    const auto found = std::find_if(settlement.building_inventory.begin(), settlement.building_inventory.end(),
        [&name](const auto& building) { return building.name == name; });
    return found == settlement.building_inventory.end() ? 0 : found->count;
}

void adjust_construction_inventory(Settlement& settlement, const std::string& name, int delta) {
    auto found = std::find_if(settlement.building_inventory.begin(), settlement.building_inventory.end(),
        [&name](const auto& building) { return building.name == name; });
    if (found == settlement.building_inventory.end()) {
        if (delta < 0) throw std::invalid_argument("Construction inventory cannot remove an absent building");
        settlement.building_inventory.push_back({name, delta});
        return;
    }
    const long long next = static_cast<long long>(found->count) + delta;
    if (next < 0 || next > std::numeric_limits<int>::max())
        throw std::invalid_argument("Construction inventory count is invalid");
    found->count = static_cast<int>(next);
}

bool construction_discount_matches(const std::string& discounts, const std::string& target) {
    std::size_t start = 0;
    while (start <= discounts.size()) {
        const std::size_t comma = discounts.find(',', start);
        std::string token = discounts.substr(start,
            comma == std::string::npos ? std::string::npos : comma - start);
        const auto first = token.find_first_not_of(" \t");
        const auto last = token.find_last_not_of(" \t");
        token = first == std::string::npos ? std::string{} : token.substr(first, last - first + 1);
        if (token == target) return true;
        if (comma == std::string::npos) break;
        start = comma + 1;
    }
    return false;
}

std::vector<std::pair<std::string, int>> construction_discount_sources(
    const Kingdom& kingdom, const Settlement& settlement,
    const std::vector<BuildingCatalogEntry>& catalog) {
    std::vector<std::pair<std::string, int>> sources;
    for (const auto& building : settlement.building_inventory) {
        if (building.count <= 0) continue;
        construction_catalog_entry(catalog, building.name);
        sources.push_back({building.name, building.count});
    }
    for (std::size_t index = 0; index < kingdom.construction_transactions.size(); ++index) {
        const auto& transaction = kingdom.construction_transactions[index];
        if (!transaction.successful || transaction.reversal || transaction.settlement_id != settlement.id)
            continue;
        bool reversed = false;
        for (const auto& later : kingdom.construction_transactions)
            if (later.reversal && later.reverses_index == static_cast<std::ptrdiff_t>(index)) {
                reversed = true;
                break;
            }
        if (reversed) continue;
        for (const auto& source_name : transaction.discount_sources) {
            const auto source = std::find_if(sources.begin(), sources.end(),
                [&source_name](const auto& value) { return value.first == source_name; });
            if (source != sources.end() && source->second > 0) --source->second;
        }
    }
    return sources;
}

bool construction_placement_overlaps(const MapPlacement& left, const MapPlacement& right) {
    const long long left_right = static_cast<long long>(left.column) + left.width;
    const long long right_right = static_cast<long long>(right.column) + right.width;
    const long long left_bottom = static_cast<long long>(left.row) + left.height;
    const long long right_bottom = static_cast<long long>(right.row) + right.height;
    return left.column < right_right && right.column < left_right
        && left.row < right_bottom && right.row < left_bottom;
}
}

ConstructionPreview apply_construction(Kingdom& kingdom,
    const ConstructionRequest& request, const std::vector<BuildingCatalogEntry>& catalog) {
    validate(kingdom);
    if (request.settlement_id.empty() || request.lines.empty())
        throw std::invalid_argument("Construction requires a settlement and at least one building line");
    auto settlement_it = std::find_if(kingdom.settlements.begin(), kingdom.settlements.end(),
        [&request](const auto& settlement) { return settlement.id == request.settlement_id; });
    if (settlement_it == kingdom.settlements.end())
        throw std::invalid_argument("Construction settlement does not exist: " + request.settlement_id);

    ConstructionPreview preview;
    int discount_slots = 0;
    long long full_cost = 0;
    long long discounted_cost = 0;
    long long construction_count = 0;
    bool non_overridable_failure = false;
    std::vector<MapPlacement> proposed_placements;
    for (const auto& line : request.lines) {
        if (line.building_name.empty() || line.count < 1 || line.row < 0 || line.column < 0
            || line.width < 1 || line.height < 1)
            throw std::invalid_argument("Construction lines require positive counts and footprints");
        if (line.count > 1)
            throw std::invalid_argument("Each constructed building requires its own district-grid placement line");
        const auto& target = construction_catalog_entry(catalog, line.building_name);
        if (target.cost < 0 || target.lots < 0)
            throw std::invalid_argument("Building cost and lots cannot be negative: " + line.building_name);
        const long long footprint_area = static_cast<long long>(line.width) * line.height;
        if (footprint_area < target.lots) {
            preview.failures.push_back("Construction footprint is smaller than the building lot requirement");
            non_overridable_failure = true;
        }
        const bool is_upgrade = !line.upgrade_from.empty();
        const BuildingCatalogEntry* source = nullptr;
        if (is_upgrade) {
            source = &construction_catalog_entry(catalog, line.upgrade_from);
            if (target.upgrade_from != line.upgrade_from) {
                preview.failures.push_back("Upgrade source does not match the catalog Upgrade From value");
                non_overridable_failure = true;
            }
            if (source->upgrade_to != line.building_name) {
                preview.failures.push_back("Upgrade target does not match the catalog Upgrade To value");
                non_overridable_failure = true;
            }
            if (construction_inventory_count(*settlement_it, line.upgrade_from) < line.count) {
                preview.failures.push_back("Upgrade source inventory is insufficient");
                non_overridable_failure = true;
            }
            const auto source_placements = std::count_if(settlement_it->map.placements.begin(),
                settlement_it->map.placements.end(), [&line](const auto& placement) {
                    return placement.building_name == line.upgrade_from
                        && placement.row == line.row && placement.column == line.column;
                });
            if (source_placements != 1) {
                preview.failures.push_back("Upgrade source placement is missing or ambiguous");
                non_overridable_failure = true;
            }
            if (target.cost < source->cost) {
                preview.failures.push_back("Upgrade target cost is lower than its source cost");
                non_overridable_failure = true;
            }
        }
        const long long unit_cost = is_upgrade
            ? static_cast<long long>(target.cost) - source->cost : target.cost;
        full_cost += unit_cost * line.count;
        construction_count += line.count;
        if (full_cost > std::numeric_limits<int>::max() || construction_count > std::numeric_limits<int>::max())
            throw std::overflow_error("Construction total exceeds supported range");
        for (int count = 0; count < line.count; ++count)
            proposed_placements.push_back({line.building_name, line.row, line.column, line.width, line.height});
    }
    auto recorded_lines = request.lines;
    for (std::size_t index = 0; index < request.lines.size(); ++index) {
        const auto& line = request.lines[index];
        if (line.upgrade_from.empty()) continue;
        const auto source_it = std::find_if(settlement_it->map.placements.begin(), settlement_it->map.placements.end(),
            [&line](const auto& placement) {
                return placement.building_name == line.upgrade_from
                    && placement.row == line.row && placement.column == line.column;
            });
        if (source_it != settlement_it->map.placements.end()) {
            recorded_lines[index].source_width = source_it->width;
            recorded_lines[index].source_height = source_it->height;
        }
    }
    auto available_discount_sources = construction_discount_sources(kingdom, *settlement_it, catalog);
    std::vector<std::vector<std::string>> discount_sources_by_line(request.lines.size());
    for (const auto& line : request.lines) {
        const std::size_t line_index = static_cast<std::size_t>(&line - request.lines.data());
        const auto& target = construction_catalog_entry(catalog, line.building_name);
        const bool is_upgrade = !line.upgrade_from.empty();
        const auto& source = is_upgrade ? construction_catalog_entry(catalog, line.upgrade_from) : target;
        const long long unit_cost = is_upgrade
            ? static_cast<long long>(target.cost) - source.cost : target.cost;
        const long long discounted_unit = unit_cost / 2;
        long long discounted_count = 0;
        for (int count = 0; count < line.count; ++count) {
            const auto source_it = std::find_if(available_discount_sources.begin(), available_discount_sources.end(),
                [&line, &catalog](const auto& value) {
                    const auto& source_entry = construction_catalog_entry(catalog, value.first);
                    return value.second > 0 && construction_discount_matches(source_entry.discounts, line.building_name);
                });
            if (source_it == available_discount_sources.end()) break;
            --source_it->second;
            ++discounted_count;
            discount_sources_by_line[line_index].push_back(source_it->first);
        }
        discounted_cost += discounted_unit * discounted_count
            + unit_cost * (line.count - discounted_count);
        discount_slots += static_cast<int>(discounted_count);
    }
    if (discounted_cost > std::numeric_limits<int>::max())
        throw std::overflow_error("Construction cost exceeds supported range");
    preview.full_cost = static_cast<int>(full_cost);
    preview.discounted_cost = static_cast<int>(discounted_cost);
    preview.discounted_buildings = discount_slots;
    preview.construction_count = static_cast<int>(construction_count);
    std::vector<std::string> applied_discount_sources;
    for (const auto& line_sources : discount_sources_by_line)
        applied_discount_sources.insert(applied_discount_sources.end(), line_sources.begin(), line_sources.end());

    const auto limits = kingdom_size_limits(kingdom.rule_inputs.kingdom_size);
    long long prior_construction_count = 0;
    std::vector<bool> reversed_transactions(kingdom.construction_transactions.size(), false);
    for (const auto& transaction : kingdom.construction_transactions) {
        if (!transaction.reversal || transaction.reverses_index < 0
            || static_cast<std::size_t>(transaction.reverses_index) >= reversed_transactions.size())
            continue;
        reversed_transactions[static_cast<std::size_t>(transaction.reverses_index)] = true;
    }
    for (std::size_t index = 0; index < kingdom.construction_transactions.size(); ++index) {
        const auto& transaction = kingdom.construction_transactions[index];
        if (transaction.turn == kingdom.turn && transaction.successful && !transaction.reversal
            && !reversed_transactions[index])
            for (const auto& line : transaction.lines) prior_construction_count += line.count;
    }
    if (!limits.unlimited_new_buildings && prior_construction_count + construction_count > limits.new_buildings
        && !request.override_building_limit)
        preview.failures.push_back("Construction exceeds the kingdom-size building limit for this turn");
    if (kingdom.treasury_bp < preview.discounted_cost && !request.override_failed_checks)
        preview.failures.push_back("Treasury cannot pay the finalized construction cost");
    for (const auto& placement : proposed_placements) {
        if (settlement_it->map.rows > 0 && settlement_it->map.columns > 0
            && (static_cast<long long>(placement.row) + placement.height > settlement_it->map.rows
                || static_cast<long long>(placement.column) + placement.width > settlement_it->map.columns)) {
            preview.failures.push_back("Construction footprint does not fit in the district grid");
            non_overridable_failure = true;
        }
        for (const auto& existing : settlement_it->map.placements) {
            bool replacing_source = false;
            for (const auto& line : request.lines) {
                if (line.building_name == placement.building_name && !line.upgrade_from.empty()
                    && line.upgrade_from == existing.building_name
                    && line.row == existing.row && line.column == existing.column) {
                    replacing_source = true;
                    break;
                }
            }
            if (!replacing_source && construction_placement_overlaps(existing, placement)) {
                preview.failures.push_back("Construction footprint overlaps an existing building");
                non_overridable_failure = true;
            }
        }
        for (const auto& proposed : proposed_placements) {
            if (&proposed != &placement && construction_placement_overlaps(proposed, placement)) {
                preview.failures.push_back("Construction footprints overlap each other");
                non_overridable_failure = true;
            }
        }
    }
    if (non_overridable_failure)
        throw std::invalid_argument("Construction request contains a structurally invalid line or placement");
    const bool has_building_limit_failure = std::find(preview.failures.begin(), preview.failures.end(),
    "Construction exceeds the kingdom-size building limit for this turn") != preview.failures.end();
    if ((!has_building_limit_failure || request.override_building_limit)
    && (preview.failures.empty() || request.override_failed_checks)) {
        Kingdom candidate = kingdom;
        auto candidate_settlement = std::find_if(candidate.settlements.begin(), candidate.settlements.end(),
            [&request](const auto& settlement) { return settlement.id == request.settlement_id; });
        for (const auto& line : request.lines) {
            if (!line.upgrade_from.empty()) {
                adjust_construction_inventory(*candidate_settlement, line.upgrade_from, -line.count);
                candidate_settlement->map.placements.erase(
                    std::remove_if(candidate_settlement->map.placements.begin(), candidate_settlement->map.placements.end(),
                        [&line](const auto& placement) {
                            return placement.building_name == line.upgrade_from
                                && placement.row == line.row && placement.column == line.column;
                        }), candidate_settlement->map.placements.end());
            }
            adjust_construction_inventory(*candidate_settlement, line.building_name, line.count);
            candidate_settlement->map.placements.push_back(
                {line.building_name, line.row, line.column, line.width, line.height});
        }
        if (candidate.treasury_bp < std::numeric_limits<int>::min() + preview.discounted_cost)
            throw std::overflow_error("Construction would overflow negative treasury");
        candidate.treasury_bp -= preview.discounted_cost;
        candidate.construction_transactions.push_back({candidate.turn, request.settlement_id, recorded_lines,
            preview.discounted_cost, true, request.override_building_limit, request.override_failed_checks, false, -1,
            preview.failures, request.terrain_facts, applied_discount_sources});
        validate(candidate);
        kingdom = std::move(candidate);
        preview.finalized = true;
        preview.successful = true;
        return preview;
    }
    kingdom.construction_transactions.push_back({kingdom.turn, request.settlement_id, recorded_lines,
        preview.discounted_cost, false, request.override_building_limit, request.override_failed_checks, false, -1,
        preview.failures, request.terrain_facts});
    validate(kingdom);
    return preview;
}

void reverse_construction(Kingdom& kingdom, std::size_t index,
    const std::vector<BuildingCatalogEntry>& catalog, bool override_reversal) {
    (void)catalog;
    validate(kingdom);
    if (!override_reversal)
        throw std::invalid_argument("Construction reversal requires the explicit override");
    if (index >= kingdom.construction_transactions.size())
        throw std::out_of_range("Construction record index is out of range");
    const auto& original = kingdom.construction_transactions[index];
    if (!original.successful || original.reversal)
        throw std::invalid_argument("Only a successful non-reversal construction may be reversed");
    for (const auto& record : kingdom.construction_transactions)
        if (record.reversal && record.reverses_index == static_cast<std::ptrdiff_t>(index))
            throw std::invalid_argument("Construction has already been reversed");
    for (std::size_t later_index = index + 1; later_index < kingdom.construction_transactions.size(); ++later_index) {
        const auto& later = kingdom.construction_transactions[later_index];
        if (later.settlement_id == original.settlement_id && (later.successful || later.reversal))
            throw std::invalid_argument("Construction cannot be reversed while a later transaction depends on its settlement state");
    }
    auto candidate = kingdom;
    auto settlement = std::find_if(candidate.settlements.begin(), candidate.settlements.end(),
        [&original](const auto& value) { return value.id == original.settlement_id; });
    if (settlement == candidate.settlements.end())
        throw std::invalid_argument("Construction reversal settlement does not exist");
    for (auto line_it = original.lines.rbegin(); line_it != original.lines.rend(); ++line_it) {
        const auto& line = *line_it;
        adjust_construction_inventory(*settlement, line.building_name, -line.count);
        settlement->map.placements.erase(
            std::remove_if(settlement->map.placements.begin(), settlement->map.placements.end(),
                [&line](const auto& placement) {
                    return placement.building_name == line.building_name
                        && placement.row == line.row && placement.column == line.column;
                }), settlement->map.placements.end());
        if (!line.upgrade_from.empty()) {
            adjust_construction_inventory(*settlement, line.upgrade_from, line.count);
            settlement->map.placements.push_back(
                {line.upgrade_from, line.row, line.column,
                    line.source_width > 0 ? line.source_width : line.width,
                    line.source_height > 0 ? line.source_height : line.height});
        }
    }
    if (candidate.treasury_bp > std::numeric_limits<int>::max() - original.bp_cost)
        throw std::overflow_error("Construction reversal would overflow treasury");
    candidate.treasury_bp += original.bp_cost;
    candidate.construction_transactions.push_back({candidate.turn, original.settlement_id, original.lines,
        -original.bp_cost, true, false, true, true, static_cast<std::ptrdiff_t>(index), {}, original.terrain_facts});
    validate(candidate);
    kingdom = std::move(candidate);
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
    return {2000, 4000, 4000, 1, 2000};
}

int resolve_tax_bp_delta(bool economy_check_success, int result_input, int other_bp_gained) {
    return resolve_tax_bp_delta(RuleProfile::Pdf, economy_check_success,
        result_input, other_bp_gained);
}

int resolve_tax_bp_delta(RuleProfile profile, bool economy_check_success,
                         int result_input, int other_bp_gained) {
    if (result_input < 0 || other_bp_gained < 0)
        throw std::invalid_argument("Tax inputs must be non-negative");
    const long long result_contribution = !economy_check_success ? 0
        : (profile == RuleProfile::Pdf || profile == RuleProfile::CityInfo
            ? result_input / 5 : result_input);
    const long long total = result_contribution + other_bp_gained;
    if (total > std::numeric_limits<int>::max())
        throw std::overflow_error("Tax result would overflow BP");
    return static_cast<int>(total);
}

void apply_tax_action(Kingdom& kingdom, bool economy_check_success,
    int result_input, int other_bp_gained) {
    validate(kingdom);
    if (result_input < 0 || other_bp_gained < 0)
        throw std::invalid_argument("Tax inputs must be non-negative");
    const int treasury_delta = resolve_tax_bp_delta(
        kingdom.rule_profile, economy_check_success, result_input, other_bp_gained);
    if (treasury_delta > 0 && kingdom.treasury_bp > std::numeric_limits<int>::max() - treasury_delta)
        throw std::overflow_error("Tax action would overflow treasury");
    kingdom.treasury_bp += treasury_delta;
    kingdom.tax_actions.push_back({kingdom.turn, economy_check_success, result_input,
        other_bp_gained, treasury_delta, kingdom.rule_profile});
    validate(kingdom);
}

void apply_stability_check(Kingdom& kingdom, bool success, int failure_by, int rolled_unrest) {
    validate(kingdom);
    const auto outcome = resolve_stability_check(
        kingdom.rule_profile, success, kingdom.unrest, failure_by, rolled_unrest);
    if (outcome.treasury_delta > 0 && kingdom.treasury_bp > std::numeric_limits<int>::max() - outcome.treasury_delta)
        throw std::overflow_error("Stability check would overflow treasury");
    if (outcome.unrest_delta < 0 && kingdom.unrest < -outcome.unrest_delta)
        throw std::logic_error("Stability check would make unrest negative");
    kingdom.treasury_bp += outcome.treasury_delta;
    kingdom.unrest += outcome.unrest_delta;
    kingdom.stability_checks.push_back({kingdom.turn, success, failure_by, rolled_unrest,
        outcome.treasury_delta, outcome.unrest_delta, kingdom.rule_profile});
    validate(kingdom);
}

void apply_consumption_action(Kingdom& kingdom, int consumption_bp) {
    validate(kingdom);
    if (consumption_bp < 1) throw std::invalid_argument("Consumption must be at least 1 BP");
    const bool paid = kingdom.treasury_bp >= consumption_bp;
    const int treasury_delta = -consumption_bp;
    const int unrest_delta = paid ? 0 : 2;
    const long long projected_treasury = static_cast<long long>(kingdom.treasury_bp) + treasury_delta;
    if (projected_treasury < std::numeric_limits<int>::min()
        || projected_treasury > std::numeric_limits<int>::max())
        throw std::overflow_error("Consumption would overflow negative treasury");
    const long long projected_unrest = static_cast<long long>(kingdom.unrest) + unrest_delta;
    if (projected_unrest > std::numeric_limits<int>::max())
        throw std::overflow_error("Consumption would overflow unrest");
    kingdom.treasury_bp = static_cast<int>(projected_treasury);
    kingdom.unrest = static_cast<int>(projected_unrest);
    kingdom.consumption_actions.push_back({kingdom.turn, consumption_bp, paid,
        treasury_delta, unrest_delta});
    validate(kingdom);
}

UnrestModificationPreview preview_unrest_modification(const Kingdom& kingdom, int negative_stat_count) {
    validate(kingdom);
    if (negative_stat_count < 0) throw std::invalid_argument("Negative stat count cannot be negative");
    if (negative_stat_count > std::numeric_limits<int>::max() - kingdom.unrest)
        throw std::overflow_error("Unrest modification would overflow unrest");
    const int projected_unrest = kingdom.unrest + negative_stat_count;
    return {kingdom.unrest, negative_stat_count, projected_unrest,
        projected_unrest >= 11, projected_unrest >= 20};
}

void apply_unrest_modification(Kingdom& kingdom, int negative_stat_count) {
    validate(kingdom);
    if (negative_stat_count < 0) throw std::invalid_argument("Negative stat count cannot be negative");
    if (negative_stat_count > std::numeric_limits<int>::max() - kingdom.unrest)
        throw std::overflow_error("Unrest modification would overflow unrest");
    kingdom.unrest += negative_stat_count;
    kingdom.unrest_modifications.push_back({kingdom.turn, negative_stat_count, negative_stat_count});
    validate(kingdom);
}

void apply_gold_withdrawal(Kingdom& kingdom, int bp_withdrawn, bool loyalty_check_success) {
    validate(kingdom);
    if (bp_withdrawn < 1) throw std::invalid_argument("Gold withdrawal must withdraw at least 1 BP");
    if (kingdom.treasury_bp < bp_withdrawn)
        throw std::invalid_argument("Gold withdrawal would make treasury negative");
    if (bp_withdrawn > std::numeric_limits<int>::max() / 2000)
        throw std::overflow_error("Gold withdrawal value overflow");
    const int loyalty_failure_unrest_delta = loyalty_check_success ? 0 : bp_withdrawn;
    const long long unrest_delta = static_cast<long long>(bp_withdrawn)
        + loyalty_failure_unrest_delta;
    const long long projected_unrest = static_cast<long long>(kingdom.unrest)
        + unrest_delta;
    if (projected_unrest > std::numeric_limits<int>::max())
        throw std::overflow_error("Gold withdrawal would overflow unrest");
    kingdom.treasury_bp -= bp_withdrawn;
    kingdom.unrest = static_cast<int>(projected_unrest);
    kingdom.gold_withdrawals.push_back({kingdom.turn, bp_withdrawn, bp_withdrawn * 2000,
        -bp_withdrawn, static_cast<int>(unrest_delta),
        loyalty_check_success, loyalty_failure_unrest_delta});
    validate(kingdom);
}

int resolve_gold_deposit_bp(int gold_deposited) {
    if (gold_deposited < 1) throw std::invalid_argument("Gold deposit must deposit at least 1 gp");
    if (gold_deposited % 4000 != 0)
        throw std::invalid_argument("Gold deposit must use exact 4,000 gp units; rounding is not defined");
    return gold_deposited / 4000;
}

void apply_gold_deposit(Kingdom& kingdom, int gold_deposited) {
    validate(kingdom);
    const int bp_received = resolve_gold_deposit_bp(gold_deposited);
    if (kingdom.treasury_bp > std::numeric_limits<int>::max() - bp_received)
        throw std::overflow_error("Gold deposit would overflow treasury");
    kingdom.treasury_bp += bp_received;
    kingdom.gold_deposits.push_back({kingdom.turn, gold_deposited, bp_received, bp_received});
    validate(kingdom);
}

int resolve_item_deposit_bp(int item_value) {
    return resolve_item_deposit_bp(RuleProfile::Pdf, item_value);
}

int resolve_item_deposit_bp(RuleProfile profile, int item_value) {
    if (item_value < 1) throw std::invalid_argument("Item deposit must have a base value of at least 1 gp");
    const int value_per_bp = profile == RuleProfile::Workbook ? 8000 : 4000;
    if (item_value % value_per_bp != 0)
        throw std::invalid_argument("Item deposit must use exact profile value units; rounding is not defined");
    return item_value / value_per_bp;
}

void apply_item_deposit(Kingdom& kingdom, int item_value) {
    validate(kingdom);
    const int bp_received = resolve_item_deposit_bp(kingdom.rule_profile, item_value);
    if (kingdom.treasury_bp > std::numeric_limits<int>::max() - bp_received)
        throw std::overflow_error("Item deposit would overflow treasury");
    kingdom.treasury_bp += bp_received;
    kingdom.item_deposits.push_back({kingdom.turn, item_value, bp_received, bp_received, kingdom.rule_profile});
    validate(kingdom);
}

int valuable_item_sale_dc(MagicItemCategory category) {
    switch (category) {
        case MagicItemCategory::Minor: return 20;
        case MagicItemCategory::Medium: return 35;
        case MagicItemCategory::Major: return 50;
    }
    throw std::invalid_argument("Unknown valuable-item category");
}

void apply_valuable_item_sale(Kingdom& kingdom, const ValuableItemSaleRequest& request) {
    validate(kingdom);
    if (request.item_name.empty() || request.city_id.empty())
        throw std::invalid_argument("Valuable-item sale requires an item name and market city");
    if (request.district < 1)
        throw std::invalid_argument("Valuable-item sale requires a positive city district");
    const auto city = std::find_if(kingdom.settlements.begin(), kingdom.settlements.end(),
        [&request](const Settlement& settlement) { return settlement.id == request.city_id; });
    if (city == kingdom.settlements.end())
        throw std::invalid_argument("Valuable-item sale requires an existing market city");
    if (request.district > city->districts)
        throw std::invalid_argument("Valuable-item sale district exceeds the market city's districts");
    if (request.item_value_gp <= 4000)
        throw std::invalid_argument("Valuable-item sale requires an item worth more than 4,000 gp");
    for (const auto& prior : kingdom.valuable_item_sales) {
        if (prior.turn == kingdom.turn && prior.city_id == request.city_id
            && prior.district == request.district)
            throw std::invalid_argument("Each city district permits only one valuable-item sale check per Income phase");
    }
    const int economy_check_dc = valuable_item_sale_dc(request.category);
    const bool sold = request.economy_check_result >= economy_check_dc;
    const int bp_received = sold ? magic_item_sale_bp(request.category) : 0;
    if (bp_received > 0 && kingdom.treasury_bp > std::numeric_limits<int>::max() - bp_received)
        throw std::overflow_error("Valuable-item sale would overflow treasury");
    kingdom.treasury_bp += bp_received;
    kingdom.valuable_item_sales.push_back({kingdom.turn, request.item_name, request.city_id,
        request.district, request.item_value_gp, request.category, economy_check_dc,
        request.economy_check_result, sold, bp_received});
    validate(kingdom);
}

void apply_loyalty_check(Kingdom& kingdom, bool success) {
    validate(kingdom);
    int unrest_delta = 0;
    int loyalty_delta = 0;
    if (success) {
        if (kingdom.unrest > 0) {
            --kingdom.unrest;
            unrest_delta = -1;
        }
    } else {
        if (kingdom.rule_inputs.loyalty.other == std::numeric_limits<int>::min())
            throw std::overflow_error("Loyalty check would underflow permanent Loyalty");
        --kingdom.rule_inputs.loyalty.other;
        loyalty_delta = -1;
    }
    kingdom.loyalty_checks.push_back({kingdom.turn, success, unrest_delta, loyalty_delta});
    validate(kingdom);
}

void apply_commission(Kingdom& kingdom, const std::string& item_name,
                      const std::string& recipient, int cost_bp,
                      int exotic_crafters_available) {
    validate(kingdom);
    if (item_name.empty() || recipient.empty())
        throw std::invalid_argument("Commission requires an item name and recipient");
    if (cost_bp < 1) throw std::invalid_argument("Commission cost must be at least 1 BP");
    if (exotic_crafters_available < 1)
        throw std::invalid_argument("Commission requires at least one available exotic crafter");
    if (kingdom.treasury_bp < cost_bp)
        throw std::invalid_argument("Commission would make treasury negative");
    const int current_turn_commissions = static_cast<int>(std::count_if(
        kingdom.commissions.begin(), kingdom.commissions.end(),
        [&kingdom](const CommissionRecord& record) { return record.turn == kingdom.turn; }));
    if (current_turn_commissions >= exotic_crafters_available)
        throw std::invalid_argument("Available exotic crafters cannot support another commission this turn");
    kingdom.treasury_bp -= cost_bp;
    kingdom.commissions.push_back({kingdom.turn, item_name, recipient, cost_bp,
        exotic_crafters_available});
    validate(kingdom);
}

int magic_item_sale_bp(MagicItemCategory category) {
    switch (category) {
        case MagicItemCategory::Minor: return 2;
        case MagicItemCategory::Medium: return 8;
        case MagicItemCategory::Major: return 15;
    }
    throw std::invalid_argument("Unknown magic-item category");
}

const char* magic_item_category_name(MagicItemCategory category) {
    switch (category) {
        case MagicItemCategory::Minor: return "minor";
        case MagicItemCategory::Medium: return "medium";
        case MagicItemCategory::Major: return "major";
    }
    throw std::invalid_argument("Unknown magic-item category");
}

MagicItemCategory magic_item_category_from_name(const std::string& name) {
    if (name == "minor") return MagicItemCategory::Minor;
    if (name == "medium") return MagicItemCategory::Medium;
    if (name == "major") return MagicItemCategory::Major;
    throw std::invalid_argument("Unknown magic-item category: " + name);
}

void apply_magic_item_outcome(Kingdom& kingdom, MagicItemCategory category,
                              bool sold, bool commissioned) {
    validate(kingdom);
    if (sold && commissioned)
        throw std::invalid_argument("A magic item outcome cannot be both sold and commissioned");
    const int sale_bp = magic_item_sale_bp(category);
    const bool lost = !commissioned && !sold;
    const int bp_received = commissioned || !sold ? 0 : sale_bp;
    if (bp_received > 0 && kingdom.treasury_bp > std::numeric_limits<int>::max() - bp_received)
        throw std::overflow_error("Magic-item sale would overflow treasury");
    kingdom.treasury_bp += bp_received;
    kingdom.magic_item_outcomes.push_back({kingdom.turn, category, sold, commissioned, lost, bp_received});
    validate(kingdom);
}

MagicItemWorkflow magic_item_workflow() {
    return {true, true, false, -1, 2000, true};
}

EventCadence event_cadence() {
    return {75, 25};
}

const std::vector<KingdomEventDefinition>& kingdom_event_definitions() {
    static const std::vector<KingdomEventDefinition> definitions = {
        {1, 3, KingdomEventKind::AssassinationAttempt, false, "Assassination Attempt"},
        {4, 12, KingdomEventKind::BanditActivity, true, "Bandit Activity"},
        {13, 19, KingdomEventKind::Disaster, false, "Disaster"},
        {20, 24, KingdomEventKind::EconomicBoom, false, "Economic Boom"},
        {25, 29, KingdomEventKind::Feud, false, "Feud"},
        {30, 32, KingdomEventKind::FoodShortage, false, "Food Shortage"},
        {33, 39, KingdomEventKind::FoodSurplus, false, "Food Surplus"},
        {40, 44, KingdomEventKind::GoodWeather, false, "Good Weather"},
        {45, 49, KingdomEventKind::MonsterAttack, true, "Monster Attack"},
        {50, 54, KingdomEventKind::NaturalBlessing, false, "Natural Blessing"},
        {55, 61, KingdomEventKind::OutstandingSuccess, false, "Outstanding Success"},
        {62, 64, KingdomEventKind::Plague, true, "Plague"},
        {65, 67, KingdomEventKind::PoliticalCalm, false, "Political Calm"},
        {68, 77, KingdomEventKind::PublicScandal, false, "Public Scandal"},
        {78, 85, KingdomEventKind::SensationalCrime, true, "Sensational Crime"},
        {86, 92, KingdomEventKind::NewVassals, false, "New Vassals"},
        {93, 100, KingdomEventKind::VisitingCelebrity, false, "Visiting Celebrity"}
    };
    return definitions;
}

const KingdomEventDefinition& kingdom_event_from_d100(int roll) {
    if (roll < 1 || roll > 100) throw std::invalid_argument("Event d100 roll must be between 1 and 100");
    for (const auto& definition : kingdom_event_definitions()) {
        if (roll >= definition.d100_min && roll <= definition.d100_max) return definition;
    }
    throw std::logic_error("No Kingdom event matches the d100 roll");
}

const char* kingdom_event_kind_name(KingdomEventKind kind) {
    for (const auto& definition : kingdom_event_definitions()) {
        if (definition.kind == kind) return definition.name;
    }
    throw std::invalid_argument("Unknown Kingdom event kind");
}

TurnTransitionPreview preview_turn_transition(const Kingdom& kingdom, bool event_occurred_last_turn) {
    validate(kingdom);
    if (kingdom.turn == std::numeric_limits<int>::max())
        throw std::overflow_error("Cannot preview beyond the maximum supported turn");
    const auto cadence = event_cadence();
    return {kingdom.turn, kingdom.turn + 1,
        event_occurred_last_turn ? cadence.event_last_month_percent : cadence.no_event_last_month_percent,
        true};
}

EventResolution classify_event_resolution(bool event_triggered) {
    return event_triggered ? EventResolution::ManualResolutionRequired : EventResolution::NoEvent;
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
        case WorkbookMapTemplate::CityLayout: return "PDF City Layout (6x6 / 3x3 districts)";
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
        case WorkbookMapTemplate::CityLayout:
            // The PDF worksheet shows three districts across and three down,
            // with each district divided into a 2x2 block: six cells per side.
            map.rows = 6;
            map.columns = 6;
            map.labels.clear();
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

int city_layout_district_index(const SettlementMap& map, int row, int column) {
    if (map.rows != 6 || map.columns != 6 || row < 0 || column < 0 || row >= map.rows || column >= map.columns)
        return 0;
    return (row / 2) * 3 + (column / 2) + 1;
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

const char* rule_profile_name(RuleProfile profile) {
    if (profile == RuleProfile::Pdf) return "pdf";
    return profile == RuleProfile::CityInfo ? "city_info" : "workbook";
}

const char* city_boundary_surface_name(CityBoundarySurface surface) {
    return surface == CityBoundarySurface::Water ? "water" : "land";
}

CityBoundarySurface city_boundary_surface_from_name(const std::string& name) {
    if (name == "land") return CityBoundarySurface::Land;
    if (name == "water") return CityBoundarySurface::Water;
    throw std::invalid_argument("Unknown city worksheet boundary surface: " + name);
}

RuleProfile rule_profile_from_name(const std::string& name) {
    if (name == "pdf") return RuleProfile::Pdf;
    if (name == "workbook") return RuleProfile::Workbook;
    if (name == "city_info") return RuleProfile::CityInfo;
    throw std::invalid_argument("Unknown rule profile: " + name);
}

void apply_army_creation(Kingdom& kingdom, const ArmyCreationRequest& request) {
    validate(kingdom);
    if (request.name.empty() || request.unit_description.empty())
        throw std::invalid_argument("Army requires a name and unit description");
    if (request.individual_cr < 1 || request.hd_average_tenths <= 0 || request.speed < 0
        || request.leader_charisma < -20 || request.leader_charisma > 20
        || request.mount_cr < 0 || request.improved_weapons < 0 || request.improved_weapons > 2
        || request.improved_armor < 0 || request.improved_armor > 2
        || request.healing_potions < 0 || request.ranged_weapons < 0 || request.siege_engines < 0)
        throw std::invalid_argument("Army worksheet contains an invalid numeric input");
    const std::map<std::string, std::pair<int, int>> sizes = {
        {"fine", {1, -8}}, {"diminutive", {10, -6}}, {"tiny", {25, -4}},
        {"small", {50, -2}}, {"medium", {100, 0}}, {"large", {200, 2}},
        {"huge", {500, 4}}, {"gargantuan", {1000, 6}}, {"colossal", {2000, 8}}};
    const auto size = sizes.find(request.size_category);
    if (size == sizes.end()) throw std::invalid_argument("Army worksheet has an unsupported size category");
    const int army_cr = request.individual_cr + size->second.second;
    if (army_cr < 1) throw std::invalid_argument("Army size produces an invalid army CR");
    const int soldiers = size->second.first;
    const int hit_points = (army_cr * request.hd_average_tenths) / 10;
    int bp_cost = request.mount_cr;
    bp_cost += request.improved_weapons == 1 ? 5 : request.improved_weapons == 2 ? 50 : 0;
    bp_cost += request.improved_armor == 1 ? 3 : request.improved_armor == 2 ? 15 : 0;
    bp_cost += request.healing_potions * 10;
    bp_cost += request.ranged_weapons * 2;
    bp_cost += request.siege_engines * 15;
    if (bp_cost < 0 || kingdom.treasury_bp < bp_cost)
        throw std::invalid_argument("Army creation requires sufficient Treasury BP");
    ArmyRecord record;
    record.turn = kingdom.turn;
    record.request = request;
    record.soldiers = soldiers;
    record.army_cr = army_cr;
    record.hit_points = hit_points;
    record.current_hit_points = hit_points;
    record.defense_value = army_cr + 10 + (request.improved_armor == 1 ? 1 : request.improved_armor == 2 ? 2 : 0)
        + (request.mount_cr > 0 ? 2 : 0);
    record.offense_modifier = army_cr + (request.improved_weapons == 1 ? 1 : request.improved_weapons == 2 ? 2 : 0)
        + (request.mount_cr > 0 ? 2 : 0) + (request.siege_engines > 0 ? 2 : 0);
    record.morale = 0;
    record.weekly_consumption = std::max(1, army_cr / 2);
    record.bp_cost = bp_cost;
    Kingdom candidate = kingdom;
    candidate.treasury_bp -= bp_cost;
    candidate.armies.push_back(std::move(record));
    validate(candidate);
    kingdom = std::move(candidate);
}

void save(const Kingdom& k, const std::string& path) {
    validate(k);
    const auto dest = std::filesystem::u8path(path);
    reject_spreadsheet_destination(dest);
    auto temp = dest;
    temp += ".tmp";
    const Json document = {
        {"schema_version", k.schema_version}, {"name", k.name}, {"turn", k.turn},
        {"treasury_bp", k.treasury_bp}, {"unrest", k.unrest}, {"rule_profile", rule_profile_name(k.rule_profile)},
        {"use_optional_mine_rule", k.use_optional_mine_rule},
        {"settlements", Json::array()}, {"buildings", Json::array()},
        {"turn_progress", Json::array()}, {"turn_actions", Json::array()}, {"tax_actions", Json::array()},
                {"stability_checks", Json::array()}, {"consumption_actions", Json::array()}, {"unrest_modifications", Json::array()}, {"gold_withdrawals", Json::array()}, {"gold_deposits", Json::array()}, {"item_deposits", Json::array()}, {"loyalty_checks", Json::array()}, {"commissions", Json::array()}, {"magic_item_outcomes", Json::array()}, {"valuable_item_sales", Json::array()}, {"calendar_notes", Json::array()},
        {"kingdom_events", Json::array()}, {"construction_transactions", Json::array()}, {"armies", Json::array()},
        {"greenbelt_map", {{"asset_path", k.greenbelt_map.asset_path},
            {"image_width", k.greenbelt_map.image_width}, {"image_height", k.greenbelt_map.image_height},
            {"calibration", {{"origin_x", k.greenbelt_map.calibration.origin_x},
                {"origin_y", k.greenbelt_map.calibration.origin_y},
                {"hex_width", k.greenbelt_map.calibration.hex_width},
                {"hex_height", k.greenbelt_map.calibration.hex_height},
                {"scale", k.greenbelt_map.calibration.scale}}},
            {"hexes", Json::array()}, {"special_terrain", Json::array()},
            {"custom_improvements", Json::array()}}},
        {"rules", rule_inputs_json(k.rule_inputs)}
    };
    Json output = document;
    for (const auto& s : k.settlements) {
        Json item = {{"name", s.name}, {"id", s.id}, {"population", s.population}, {"districts", s.districts},
            {"economy", s.economy}, {"loyalty", s.loyalty}, {"stability", s.stability},
            {"defense", s.defense}, {"building_inventory", Json::array()}, {"random_magic_item_stock", Json::array()},
            {"apply_catalog_stat_effects", s.apply_catalog_stat_effects},
            {"city_layout", {{"north", city_boundary_surface_name(s.city_layout.north)},
                {"south", city_boundary_surface_name(s.city_layout.south)},
                {"west", city_boundary_surface_name(s.city_layout.west)},
                {"east", city_boundary_surface_name(s.city_layout.east)},
                {"base_value", s.city_layout.base_value}, {"item_columns", Json::array()}}},
            {"map", {{"rows", s.map.rows}, {"columns", s.map.columns},
                {"placements", Json::array()}, {"labels", Json::array()}}}};
        for (const auto& building : s.building_inventory)
            item["building_inventory"].push_back({{"name", building.name}, {"count", building.count}});
        for (const auto& stock : s.random_magic_item_stock)
            item["random_magic_item_stock"].push_back({
                {"tier", stock.tier}, {"slot_index", stock.slot_index}, {"state", stock.state},
                {"item_name", stock.item_name}, {"price_gp", stock.price_gp},
                {"source_table", stock.source_table}, {"source_url", stock.source_url},
                {"generated_turn", stock.generated_turn}, {"last_changed_turn", stock.last_changed_turn}});
        for (const auto& placement : s.map.placements)
            item["map"]["placements"].push_back({{"building_name", placement.building_name},
                {"row", placement.row}, {"column", placement.column},
                {"width", placement.width}, {"height", placement.height}});
        for (const auto& label : s.map.labels)
            item["map"]["labels"].push_back({{"text", label.text}, {"row", label.row},
                {"column", label.column}, {"width", label.width}, {"height", label.height}});
        for (const auto& column : s.city_layout.item_columns)
            item["city_layout"]["item_columns"].push_back(column);
        output["settlements"].push_back(std::move(item));
    }
    for (const auto& b : k.buildings) output["buildings"].push_back({{"name", b.name}, {"count", b.count}});
    for (const auto& progress : k.turn_progress)
        output["turn_progress"].push_back({{"turn", progress.turn},
            {"completed_step_ids", progress.completed_step_ids},
            {"skipped_step_ids", progress.skipped_step_ids}});
    for (const auto& action : k.turn_actions)
        output["turn_actions"].push_back({{"turn", action.turn}, {"action", turn_action_name(action.action)},
            {"count", action.count}, {"treasury_delta", action.applied.treasury_delta},
            {"kingdom_size_delta", action.applied.kingdom_size_delta}, {"unrest_delta", action.applied.unrest_delta},
            {"reversal", action.reversal}, {"reverses_index", action.reverses_index}});
    for (const auto& tax : k.tax_actions)
        output["tax_actions"].push_back({{"turn", tax.turn},
            {"economy_check_success", tax.economy_check_success},
            {"result_input", tax.result_input},
            {"other_bp_gained", tax.other_bp_gained}, {"treasury_delta", tax.treasury_delta},
            {"rule_profile", rule_profile_name(tax.rule_profile)}});
    for (const auto& withdrawal : k.gold_withdrawals)
        output["gold_withdrawals"].push_back({{"turn", withdrawal.turn},
            {"bp_withdrawn", withdrawal.bp_withdrawn}, {"gold_received", withdrawal.gold_received},
            {"treasury_delta", withdrawal.treasury_delta}, {"unrest_delta", withdrawal.unrest_delta},
            {"loyalty_check_success", withdrawal.loyalty_check_success},
            {"loyalty_failure_unrest_delta", withdrawal.loyalty_failure_unrest_delta}});
    for (const auto& stability : k.stability_checks)
        output["stability_checks"].push_back({{"turn", stability.turn}, {"success", stability.success},
            {"failure_by", stability.failure_by}, {"rolled_unrest", stability.rolled_unrest},
            {"treasury_delta", stability.treasury_delta}, {"unrest_delta", stability.unrest_delta},
            {"rule_profile", rule_profile_name(stability.rule_profile)}});
    for (const auto& consumption : k.consumption_actions)
        output["consumption_actions"].push_back({{"turn", consumption.turn},
            {"consumption_bp", consumption.consumption_bp}, {"paid", consumption.paid},
            {"treasury_delta", consumption.treasury_delta}, {"unrest_delta", consumption.unrest_delta}});
    for (const auto& unrest : k.unrest_modifications)
        output["unrest_modifications"].push_back({{"turn", unrest.turn},
            {"negative_stat_count", unrest.negative_stat_count}, {"unrest_delta", unrest.unrest_delta}});
    for (const auto& deposit : k.gold_deposits)
        output["gold_deposits"].push_back({{"turn", deposit.turn},
            {"gold_deposited", deposit.gold_deposited}, {"bp_received", deposit.bp_received},
            {"treasury_delta", deposit.treasury_delta}});
    for (const auto& deposit : k.item_deposits)
        output["item_deposits"].push_back({{"turn", deposit.turn},
            {"item_value", deposit.item_value}, {"bp_received", deposit.bp_received},
            {"treasury_delta", deposit.treasury_delta},
            {"rule_profile", rule_profile_name(deposit.rule_profile)}});
    for (const auto& check : k.loyalty_checks)
        output["loyalty_checks"].push_back({{"turn", check.turn}, {"success", check.success},
            {"unrest_delta", check.unrest_delta}, {"loyalty_delta", check.loyalty_delta}});
    for (const auto& commission : k.commissions)
        output["commissions"].push_back({{"turn", commission.turn}, {"item_name", commission.item_name},
            {"recipient", commission.recipient}, {"cost_bp", commission.cost_bp},
            {"exotic_crafters_available", commission.exotic_crafters_available}});
    for (const auto& outcome : k.magic_item_outcomes)
        output["magic_item_outcomes"].push_back({{"turn", outcome.turn},
            {"category", magic_item_category_name(outcome.category)}, {"sold", outcome.sold},
            {"commissioned", outcome.commissioned}, {"lost", outcome.lost},
            {"bp_received", outcome.bp_received}});
    for (const auto& sale : k.valuable_item_sales)
        output["valuable_item_sales"].push_back({{"turn", sale.turn},
            {"item_name", sale.item_name}, {"city_id", sale.city_id}, {"district", sale.district},
            {"item_value_gp", sale.item_value_gp},
            {"category", magic_item_category_name(sale.category)},
            {"economy_check_dc", sale.economy_check_dc},
            {"economy_check_result", sale.economy_check_result},
            {"sold", sale.sold}, {"bp_received", sale.bp_received}});
    for (const auto& event : k.kingdom_events)
        output["kingdom_events"].push_back({{"turn", event.turn}, {"d100_roll", event.d100_roll},
            {"kind", kingdom_event_kind_name(event.kind)}, {"resolved", event.resolved},
            {"continuous_active", event.continuous_active}, {"affected_leader", event.affected_leader},
            {"affected_city", event.affected_city}, {"affected_block", event.affected_block},
            {"affected_hex", event.affected_hex}, {"consequence_notes", event.consequence_notes},
            {"treasury_delta", event.treasury_delta}, {"unrest_delta", event.unrest_delta},
            {"kingdom_size_delta", event.kingdom_size_delta},
            {"hex_loss_confirmed", event.hex_loss_confirmed},
            {"anarchy_confirmed", event.anarchy_confirmed},
            {"consequences_applied", event.consequences_applied}});
    for (const auto& transaction : k.construction_transactions) {
        Json item = {{"turn", transaction.turn}, {"settlement_id", transaction.settlement_id},
            {"lines", Json::array()}, {"bp_cost", transaction.bp_cost},
            {"successful", transaction.successful},
            {"override_building_limit", transaction.override_building_limit},
            {"override_failed_checks", transaction.override_failed_checks},
            {"reversal", transaction.reversal}, {"reverses_index", transaction.reverses_index},
            {"failures", transaction.failures}, {"terrain_facts", transaction.terrain_facts},
            {"discount_sources", transaction.discount_sources}};
        for (const auto& line : transaction.lines)
            item["lines"].push_back({{"building_name", line.building_name}, {"count", line.count},
                {"upgrade_from", line.upgrade_from}, {"row", line.row}, {"column", line.column},
                {"width", line.width}, {"height", line.height},
                {"source_width", line.source_width}, {"source_height", line.source_height}});
        output["construction_transactions"].push_back(std::move(item));
    }
    for (const auto& army : k.armies) {
        const auto& request = army.request;
        output["armies"].push_back({
            {"turn", army.turn}, {"name", request.name}, {"unit_description", request.unit_description},
            {"size_category", request.size_category}, {"individual_cr", request.individual_cr},
            {"hd_average_tenths", request.hd_average_tenths}, {"speed", request.speed},
            {"leader", request.leader}, {"leader_charisma", request.leader_charisma}, {"ranged", request.ranged},
            {"mount_cr", request.mount_cr}, {"improved_weapons", request.improved_weapons},
            {"improved_armor", request.improved_armor}, {"healing_potions", request.healing_potions},
            {"ranged_weapons", request.ranged_weapons}, {"siege_engines", request.siege_engines},
            {"tactics", request.tactics}, {"special_abilities", request.special_abilities},
            {"prerequisites", request.prerequisites}, {"notes", request.notes},
            {"soldiers", army.soldiers}, {"army_cr", army.army_cr}, {"hit_points", army.hit_points},
            {"current_hit_points", army.current_hit_points}, {"defense_value", army.defense_value},
            {"offense_modifier", army.offense_modifier}, {"morale", army.morale},
            {"weekly_consumption", army.weekly_consumption}, {"bp_cost", army.bp_cost}});
    }
    for (const auto& note : k.calendar_notes)
        output["calendar_notes"].push_back({{"entry_id", note.entry_id}, {"holiday", note.holiday},
            {"kingdom_upgrades", note.kingdom_upgrades}, {"events", note.events}, {"other", note.other}});
    for (const auto& hex : k.greenbelt_map.hexes)
        output["greenbelt_map"]["hexes"].push_back({{"q", hex.q}, {"r", hex.r}, {"terrain", hex.terrain},
            {"claimed", hex.claimed}, {"claim_turn", hex.claim_turn}, {"settlement_id", hex.settlement_id},
            {"improvements", hex.improvements}, {"terrain_notes", hex.terrain_notes}});
    for (const auto& terrain : k.greenbelt_map.special_terrain)
        output["greenbelt_map"]["special_terrain"].push_back({{"q", terrain.q}, {"r", terrain.r},
            {"type", terrain.type}, {"name", terrain.name}, {"notes", terrain.notes}});
    for (const auto& improvement : k.greenbelt_map.custom_improvements)
        output["greenbelt_map"]["custom_improvements"].push_back({{"id", improvement.id},
            {"name", improvement.name}, {"effect_preview", improvement.effect_preview},
            {"cost_description", improvement.cost_description}, {"manual_only", improvement.manual_only},
            {"effects", improvement.effects}, {"costs", improvement.costs}});
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
    if (document_version < 1 || document_version > 10) throw std::runtime_error("Unsupported schema version: " + std::to_string(document_version));
    if (document_version >= 2 && !root.contains("rules")) throw std::runtime_error("Missing JSON field: rules");
    if (document_version >= 8 && (!root.contains("turn_progress") || !root.at("turn_progress").is_array()))
        throw std::runtime_error("turn_progress must be an array");
    if (document_version >= 9 && (!root.contains("calendar_notes") || !root.at("calendar_notes").is_array()))
        throw std::runtime_error("calendar_notes must be an array");
    k.schema_version = 10;
    k.name = required_string(root, "name");
    k.turn = required_int(root, "turn");
    k.treasury_bp = required_int(root, "treasury_bp");
    k.unrest = required_int(root, "unrest");
    if (root.contains("rule_profile")) k.rule_profile = rule_profile_from_name(required_string(root, "rule_profile"));
    else k.rule_profile = RuleProfile::Workbook;
    if (root.contains("use_optional_mine_rule")) k.use_optional_mine_rule = required_bool(root, "use_optional_mine_rule");
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
        k.settlements.back().id = item.contains("id") ? required_string(item, "id")
            : "settlement." + std::to_string(k.settlements.size());
        if (document_version >= 5)
            k.settlements.back().apply_catalog_stat_effects = required_bool(item, "apply_catalog_stat_effects");
        if (item.contains("city_layout")) {
            const auto& worksheet = item.at("city_layout");
            if (!worksheet.is_object()) throw std::runtime_error("Settlement city_layout must be an object");
            auto& city_layout = k.settlements.back().city_layout;
            if (worksheet.contains("north")) city_layout.north = city_boundary_surface_from_name(required_string(worksheet, "north"));
            if (worksheet.contains("south")) city_layout.south = city_boundary_surface_from_name(required_string(worksheet, "south"));
            if (worksheet.contains("west")) city_layout.west = city_boundary_surface_from_name(required_string(worksheet, "west"));
            if (worksheet.contains("east")) city_layout.east = city_boundary_surface_from_name(required_string(worksheet, "east"));
            city_layout.base_value = optional_int(worksheet, "base_value", 0);
            if (worksheet.contains("item_columns")) {
                const auto& columns = worksheet.at("item_columns");
                if (!columns.is_array() || columns.size() != 4) throw std::runtime_error("Settlement city_layout item_columns must contain four arrays");
                for (size_t index = 0; index < 4; ++index) {
                    if (!columns[index].is_array()) throw std::runtime_error("Settlement city_layout item column must be an array");
                    for (const auto& entry : columns[index]) {
                        if (!entry.is_string()) throw std::runtime_error("Settlement city_layout item must be a string");
                        city_layout.item_columns[index].push_back(entry.get<std::string>());
                    }
                }
            }
        }
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
        if (item.contains("random_magic_item_stock")) {
            const auto& stock_array = item.at("random_magic_item_stock");
            if (!stock_array.is_array()) throw std::runtime_error("Settlement random_magic_item_stock must be an array");
            for (const auto& stock : stock_array) {
                if (!stock.is_object()) throw std::runtime_error("Invalid settlement random magic-item stock entry");
                SettlementMagicItemStock entry;
                entry.tier = required_string(stock, "tier");
                entry.slot_index = required_int(stock, "slot_index");
                entry.state = stock.contains("state") ? required_string(stock, "state") : "empty";
                entry.item_name = stock.contains("item_name") ? required_string(stock, "item_name") : std::string{};
                entry.price_gp = optional_int(stock, "price_gp", 0);
                entry.source_table = stock.contains("source_table") ? required_string(stock, "source_table") : std::string{};
                entry.source_url = stock.contains("source_url") ? required_string(stock, "source_url") : std::string{};
                entry.generated_turn = optional_int(stock, "generated_turn", 0);
                entry.last_changed_turn = optional_int(stock, "last_changed_turn", 0);
                k.settlements.back().random_magic_item_stock.push_back(std::move(entry));
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
            if (item.contains("skipped_step_ids")) {
                if (!item.at("skipped_step_ids").is_array())
                    throw std::runtime_error("Turn checklist skipped step ids must be an array");
                for (const auto& step_id : item.at("skipped_step_ids")) {
                    if (!step_id.is_string()) throw std::runtime_error("Turn checklist skipped step id must be a string");
                    progress.skipped_step_ids.push_back(step_id.get<std::string>());
                }
            }
            k.turn_progress.push_back(std::move(progress));
        }
    }
    if (root.contains("turn_actions")) {
        if (!root.at("turn_actions").is_array()) throw std::runtime_error("turn_actions must be an array");
        for (const auto& item : root.at("turn_actions")) {
            if (!item.is_object()) throw std::runtime_error("Invalid turn action record");
            TurnActionRecord record;
            record.turn = required_int(item, "turn");
            record.action = read_turn_action(item, "action");
            record.count = required_int(item, "count");
            record.applied = {required_int(item, "treasury_delta"),
                required_int(item, "kingdom_size_delta"), required_int(item, "unrest_delta")};
            if (item.contains("reversal")) record.reversal = item.at("reversal").get<bool>();
            if (item.contains("reverses_index")) record.reverses_index = item.at("reverses_index").get<std::ptrdiff_t>();
            k.turn_actions.push_back(std::move(record));
        }
    }
    if (root.contains("tax_actions")) {
        if (!root.at("tax_actions").is_array()) throw std::runtime_error("tax_actions must be an array");
        for (const auto& item : root.at("tax_actions")) {
            if (!item.is_object()) throw std::runtime_error("Invalid tax action record");
            const auto profile = item.contains("rule_profile")
                ? pkkm::rule_profile_from_name(required_string(item, "rule_profile"))
                : pkkm::RuleProfile::Workbook;
            const int result_input = item.contains("result_input")
                ? required_int(item, "result_input")
                : required_int(item, "result_divided_by_three");
            k.tax_actions.push_back({required_int(item, "turn"), required_bool(item, "economy_check_success"),
                result_input, required_int(item, "other_bp_gained"),
                required_int(item, "treasury_delta"), profile});
        }
    }
    if (root.contains("gold_withdrawals")) {
        if (!root.at("gold_withdrawals").is_array()) throw std::runtime_error("gold_withdrawals must be an array");
        for (const auto& item : root.at("gold_withdrawals")) {
            if (!item.is_object()) throw std::runtime_error("Invalid gold withdrawal record");
            const bool loyalty_success = item.contains("loyalty_check_success")
                ? required_bool(item, "loyalty_check_success") : true;
            const int loyalty_failure_delta = item.contains("loyalty_failure_unrest_delta")
                ? required_int(item, "loyalty_failure_unrest_delta") : 0;
            k.gold_withdrawals.push_back({required_int(item, "turn"), required_int(item, "bp_withdrawn"),
                required_int(item, "gold_received"), required_int(item, "treasury_delta"),
                required_int(item, "unrest_delta"), loyalty_success, loyalty_failure_delta});
        }
    }
    if (root.contains("stability_checks")) {
        if (!root.at("stability_checks").is_array()) throw std::runtime_error("stability_checks must be an array");
        for (const auto& item : root.at("stability_checks")) {
            if (!item.is_object()) throw std::runtime_error("Invalid stability check record");
            const auto profile = item.contains("rule_profile")
                ? rule_profile_from_name(required_string(item, "rule_profile"))
                : RuleProfile::Workbook;
            k.stability_checks.push_back({required_int(item, "turn"), required_bool(item, "success"),
                required_int(item, "failure_by"), required_int(item, "rolled_unrest"),
                required_int(item, "treasury_delta"), required_int(item, "unrest_delta"), profile});
        }
    }
    if (root.contains("consumption_actions")) {
        if (!root.at("consumption_actions").is_array()) throw std::runtime_error("consumption_actions must be an array");
        for (const auto& item : root.at("consumption_actions")) {
            if (!item.is_object()) throw std::runtime_error("Invalid consumption record");
            k.consumption_actions.push_back({required_int(item, "turn"), required_int(item, "consumption_bp"),
                required_bool(item, "paid"), required_int(item, "treasury_delta"), required_int(item, "unrest_delta")});
        }
    }
    if (root.contains("unrest_modifications")) {
        if (!root.at("unrest_modifications").is_array()) throw std::runtime_error("unrest_modifications must be an array");
        for (const auto& item : root.at("unrest_modifications")) {
            if (!item.is_object()) throw std::runtime_error("Invalid unrest modification record");
            k.unrest_modifications.push_back({required_int(item, "turn"), required_int(item, "negative_stat_count"),
                required_int(item, "unrest_delta")});
        }
    }
    if (root.contains("gold_deposits")) {
        if (!root.at("gold_deposits").is_array()) throw std::runtime_error("gold_deposits must be an array");
        for (const auto& item : root.at("gold_deposits")) {
            if (!item.is_object()) throw std::runtime_error("Invalid gold deposit record");
            k.gold_deposits.push_back({required_int(item, "turn"), required_int(item, "gold_deposited"),
                required_int(item, "bp_received"), required_int(item, "treasury_delta")});
        }
    }
    if (root.contains("item_deposits")) {
        if (!root.at("item_deposits").is_array()) throw std::runtime_error("item_deposits must be an array");
        for (const auto& item : root.at("item_deposits")) {
            if (!item.is_object()) throw std::runtime_error("Invalid item deposit record");
            const auto profile = item.contains("rule_profile")
                ? rule_profile_from_name(required_string(item, "rule_profile"))
                : RuleProfile::Workbook;
            k.item_deposits.push_back({required_int(item, "turn"), required_int(item, "item_value"),
                required_int(item, "bp_received"), required_int(item, "treasury_delta"), profile});
        }
    }
    if (root.contains("loyalty_checks")) {
        if (!root.at("loyalty_checks").is_array()) throw std::runtime_error("loyalty_checks must be an array");
        for (const auto& item : root.at("loyalty_checks")) {
            if (!item.is_object()) throw std::runtime_error("Invalid loyalty check record");
            k.loyalty_checks.push_back({required_int(item, "turn"), required_bool(item, "success"),
                required_int(item, "unrest_delta"), required_int(item, "loyalty_delta")});
        }
    }
    if (root.contains("commissions")) {
        if (!root.at("commissions").is_array()) throw std::runtime_error("commissions must be an array");
        for (const auto& item : root.at("commissions")) {
            if (!item.is_object()) throw std::runtime_error("Invalid commission record");
            k.commissions.push_back({required_int(item, "turn"), required_string(item, "item_name"),
                required_string(item, "recipient"), required_int(item, "cost_bp"),
                required_int(item, "exotic_crafters_available")});
        }
    }
    if (root.contains("magic_item_outcomes")) {
        if (!root.at("magic_item_outcomes").is_array()) throw std::runtime_error("magic_item_outcomes must be an array");
        for (const auto& item : root.at("magic_item_outcomes")) {
            if (!item.is_object()) throw std::runtime_error("Invalid magic-item outcome record");
            k.magic_item_outcomes.push_back({required_int(item, "turn"),
                magic_item_category_from_name(required_string(item, "category")),
                required_bool(item, "sold"), required_bool(item, "commissioned"),
                required_bool(item, "lost"), required_int(item, "bp_received")});
        }
    }
    if (root.contains("valuable_item_sales")) {
        if (!root.at("valuable_item_sales").is_array()) throw std::runtime_error("valuable_item_sales must be an array");
        for (const auto& item : root.at("valuable_item_sales")) {
            if (!item.is_object()) throw std::runtime_error("Invalid valuable-item sale record");
            k.valuable_item_sales.push_back({required_int(item, "turn"),
                required_string(item, "item_name"), required_string(item, "city_id"),
                required_int(item, "district"), required_int(item, "item_value_gp"),
                magic_item_category_from_name(required_string(item, "category")),
                required_int(item, "economy_check_dc"), required_int(item, "economy_check_result"),
                required_bool(item, "sold"), required_int(item, "bp_received")});
        }
    }
    if (root.contains("kingdom_events")) {
        if (!root.at("kingdom_events").is_array()) throw std::runtime_error("kingdom_events must be an array");
        for (const auto& item : root.at("kingdom_events")) {
            if (!item.is_object()) throw std::runtime_error("Invalid Kingdom event record");
            const int roll = required_int(item, "d100_roll");
            const auto& definition = kingdom_event_from_d100(roll);
            const auto kind_name = required_string(item, "kind");
            if (kind_name != definition.name) throw std::runtime_error("Kingdom event kind does not match d100 roll");
            k.kingdom_events.push_back({required_int(item, "turn"), roll, definition.kind,
                required_bool(item, "resolved"), required_bool(item, "continuous_active"),
                required_string(item, "affected_leader"), required_string(item, "affected_city"),
                required_string(item, "affected_block"), required_string(item, "affected_hex"),
                item.value("consequence_notes", std::string{}), item.value("treasury_delta", 0),
                item.value("unrest_delta", 0), item.value("kingdom_size_delta", 0),
                item.value("hex_loss_confirmed", false), item.value("anarchy_confirmed", false)});
            k.kingdom_events.back().consequences_applied = item.value("consequences_applied", false);
        }
    }
    if (root.contains("construction_transactions")) {
        if (!root.at("construction_transactions").is_array())
            throw std::runtime_error("construction_transactions must be an array");
        for (const auto& item : root.at("construction_transactions")) {
            if (!item.is_object() || !item.contains("lines") || !item.at("lines").is_array())
                throw std::runtime_error("Invalid construction transaction");
            ConstructionRecord record;
            record.turn = required_int(item, "turn");
            record.settlement_id = required_string(item, "settlement_id");
            record.bp_cost = required_int(item, "bp_cost");
            record.successful = required_bool(item, "successful");
            record.override_building_limit = required_bool(item, "override_building_limit");
            record.override_failed_checks = required_bool(item, "override_failed_checks");
            record.reversal = required_bool(item, "reversal");
            record.reverses_index = item.contains("reverses_index")
                ? item.at("reverses_index").get<std::ptrdiff_t>() : -1;
            if (item.contains("failures")) {
                if (!item.at("failures").is_array()) throw std::runtime_error("Construction failures must be an array");
                for (const auto& failure : item.at("failures")) {
                    if (!failure.is_string()) throw std::runtime_error("Construction failure must be a string");
                    record.failures.push_back(failure.get<std::string>());
                }
            }
            if (item.contains("terrain_facts")) {
                if (!item.at("terrain_facts").is_array()) throw std::runtime_error("Construction terrain_facts must be an array");
                for (const auto& fact : item.at("terrain_facts")) {
                    if (!fact.is_string()) throw std::runtime_error("Construction terrain fact must be a string");
                    record.terrain_facts.push_back(fact.get<std::string>());
                }
            }
            if (item.contains("discount_sources")) {
                if (!item.at("discount_sources").is_array()) throw std::runtime_error("Construction discount_sources must be an array");
                for (const auto& source : item.at("discount_sources")) {
                    if (!source.is_string()) throw std::runtime_error("Construction discount source must be a string");
                    record.discount_sources.push_back(source.get<std::string>());
                }
            }
            for (const auto& line : item.at("lines")) {
                if (!line.is_object()) throw std::runtime_error("Invalid construction line");
                record.lines.push_back({required_string(line, "building_name"), required_int(line, "count"),
                    line.value("upgrade_from", std::string{}), required_int(line, "row"),
                    required_int(line, "column"), required_int(line, "width"), required_int(line, "height"),
                    line.value("source_width", 0), line.value("source_height", 0)});
                record.lines.back().upgrade_from = line.value("upgrade_from", std::string{});
            }
            k.construction_transactions.push_back(std::move(record));
        }
    }
    if (root.contains("armies")) {
        if (!root.at("armies").is_array()) throw std::runtime_error("armies must be an array");
        for (const auto& item : root.at("armies")) {
            if (!item.is_object()) throw std::runtime_error("Invalid army record");
            ArmyRecord record;
            record.turn = required_int(item, "turn");
            record.request.name = required_string(item, "name");
            record.request.unit_description = required_string(item, "unit_description");
            record.request.size_category = required_string(item, "size_category");
            record.request.individual_cr = required_int(item, "individual_cr");
            record.request.hd_average_tenths = required_int(item, "hd_average_tenths");
            record.request.speed = required_int(item, "speed");
            record.request.leader = item.value("leader", std::string{});
            record.request.leader_charisma = item.value("leader_charisma", 0);
            record.request.ranged = item.value("ranged", false);
            record.request.mount_cr = item.value("mount_cr", 0);
            record.request.improved_weapons = item.value("improved_weapons", 0);
            record.request.improved_armor = item.value("improved_armor", 0);
            record.request.healing_potions = item.value("healing_potions", 0);
            record.request.ranged_weapons = item.value("ranged_weapons", 0);
            record.request.siege_engines = item.value("siege_engines", 0);
            record.request.tactics = item.value("tactics", std::vector<std::string>{});
            record.request.special_abilities = item.value("special_abilities", std::vector<std::string>{});
            record.request.prerequisites = item.value("prerequisites", std::vector<std::string>{});
            record.request.notes = item.value("notes", std::string{});
            record.soldiers = required_int(item, "soldiers");
            record.army_cr = required_int(item, "army_cr");
            record.hit_points = required_int(item, "hit_points");
            record.current_hit_points = required_int(item, "current_hit_points");
            record.defense_value = required_int(item, "defense_value");
            record.offense_modifier = required_int(item, "offense_modifier");
            record.morale = required_int(item, "morale");
            record.weekly_consumption = required_int(item, "weekly_consumption");
            record.bp_cost = required_int(item, "bp_cost");
            k.armies.push_back(std::move(record));
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
    if (document_version >= 10) {
        if (!root.contains("greenbelt_map") || !root.at("greenbelt_map").is_object())
            throw std::runtime_error("greenbelt_map must be an object");
        const auto& greenbelt = root.at("greenbelt_map");
        k.greenbelt_map.asset_path = required_string(greenbelt, "asset_path");
        k.greenbelt_map.image_width = required_int(greenbelt, "image_width");
        k.greenbelt_map.image_height = required_int(greenbelt, "image_height");
        const auto& calibration = greenbelt.at("calibration");
        k.greenbelt_map.calibration = {required_number(calibration, "origin_x"),
            required_number(calibration, "origin_y"), required_number(calibration, "hex_width"),
            required_number(calibration, "hex_height"), required_number(calibration, "scale")};
        if (!greenbelt.at("hexes").is_array()) throw std::runtime_error("greenbelt_map hexes must be an array");
        for (const auto& item : greenbelt.at("hexes")) {
            if (!item.is_object() || !item.at("improvements").is_array())
                throw std::runtime_error("Invalid Greenbelt hex");
            GreenbeltHex hex;
            hex.q = required_int(item, "q");
            hex.r = required_int(item, "r");
            hex.terrain = required_string(item, "terrain");
            hex.claimed = required_bool(item, "claimed");
            hex.claim_turn = required_int(item, "claim_turn");
            hex.settlement_id = required_string(item, "settlement_id");
            hex.terrain_notes = item.contains("terrain_notes")
                ? required_string(item, "terrain_notes") : std::string{};
            for (const auto& improvement : item.at("improvements")) {
                if (!improvement.is_string()) throw std::runtime_error("Greenbelt improvement ID must be a string");
                hex.improvements.push_back(improvement.get<std::string>());
            }
            k.greenbelt_map.hexes.push_back(std::move(hex));
        }
        if (greenbelt.contains("special_terrain")) {
            if (!greenbelt.at("special_terrain").is_array()) throw std::runtime_error("greenbelt_map special_terrain must be an array");
            for (const auto& item : greenbelt.at("special_terrain")) {
                if (!item.is_object()) throw std::runtime_error("Invalid Greenbelt special terrain");
                k.greenbelt_map.special_terrain.push_back({required_int(item, "q"), required_int(item, "r"),
                    required_string(item, "type"), required_string(item, "name"), required_string(item, "notes")});
            }
        }
        if (greenbelt.contains("custom_improvements")) {
            if (!greenbelt.at("custom_improvements").is_array()) throw std::runtime_error("greenbelt_map custom_improvements must be an array");
            for (const auto& item : greenbelt.at("custom_improvements")) {
                if (!item.is_object()) throw std::runtime_error("Invalid Greenbelt custom improvement");
                GreenbeltCustomImprovement improvement{
                    required_string(item, "id"), required_string(item, "name"),
                    required_string(item, "effect_preview"), required_string(item, "cost_description"),
                    required_bool(item, "manual_only")};
                if (item.contains("effects")) {
                    if (!item.at("effects").is_array()) throw std::runtime_error("Greenbelt custom improvement effects must be an array");
                    for (const auto& effect : item.at("effects")) {
                        if (!effect.is_string()) throw std::runtime_error("Greenbelt custom improvement effect must be a string");
                        improvement.effects.push_back(effect.get<std::string>());
                    }
                } else if (!improvement.effect_preview.empty()) {
                    improvement.effects.push_back(improvement.effect_preview);
                }
                if (item.contains("costs")) {
                    if (!item.at("costs").is_array()) throw std::runtime_error("Greenbelt custom improvement costs must be an array");
                    for (const auto& cost : item.at("costs")) {
                        if (!cost.is_string()) throw std::runtime_error("Greenbelt custom improvement cost must be a string");
                        improvement.costs.push_back(cost.get<std::string>());
                    }
                } else if (!improvement.cost_description.empty()) {
                    improvement.costs.push_back(improvement.cost_description);
                }
                k.greenbelt_map.custom_improvements.push_back(std::move(improvement));
            }
        }
    }
    validate(k);
    return k;
}
}

