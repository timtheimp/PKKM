#pragma once
#include <string>
#include <vector>
namespace pkkm {
struct BuildingCatalogEntry;
struct Building { std::string name; int count = 0; };
// Placement coordinates are zero-based; footprint is explicit, never inferred from catalog lots.
struct MapPlacement { std::string building_name; int row = 0; int column = 0; int width = 1; int height = 1; };
// Labels are visual annotations (terrain, gate, corridor text), not rule-bearing effects.
struct MapLabel { std::string text; int row = 0; int column = 0; int width = 1; int height = 1; };
struct SettlementMap {
 int rows = 0; int columns = 0;
 std::vector<MapPlacement> placements;
 std::vector<MapLabel> labels;
};
enum class WorkbookMapTemplate { Capital, City, Example, MultiDistrict };
const char* workbook_map_template_name(WorkbookMapTemplate preset);
SettlementMap workbook_map_template(WorkbookMapTemplate preset);
struct Settlement {
 std::string name;
 int population = 0; int districts = 0;
 int economy = 0; int loyalty = 0; int stability = 0; int defense = 0;
 std::vector<Building> building_inventory;
 bool apply_catalog_stat_effects = false;
 SettlementMap map;
};
struct SettlementTotals {
 int count = 0; int population = 0; int districts = 0;
 int economy = 0; int loyalty = 0; int stability = 0; int defense = 0;
 int lots = 0;
};
SettlementTotals aggregate_settlements(const std::vector<Settlement>& settlements);
SettlementTotals aggregate_settlements(const std::vector<Settlement>& settlements,
 const std::vector<BuildingCatalogEntry>& catalog);
// Matches Kingdom!C:G and H:I:J: positive sources, deductions, then other modifier.
struct KingdomStatInput {
 int events = 0;
 int alignment = 0;
 int improvements = 0;
 int leadership = 0;
 int laws = 0;
 int unrest = 0;
 int vacancies = 0;
 int other = 0;
};
struct KingdomStatResult { int total = 0; int check_threshold_percent = 5; };
KingdomStatResult calculate_stat(const KingdomStatInput& input, int control_dc);
// Component values are signed contributions to total, including deductions.
struct KingdomStatBreakdown {
 int events = 0; int alignment = 0; int improvements = 0; int leadership = 0; int laws = 0;
 int unrest_adjustment = 0; int vacancy_adjustment = 0; int other = 0;
 int total = 0; int check_threshold_percent = 5;
};
KingdomStatBreakdown calculate_stat_breakdown(const KingdomStatInput& input, int control_dc);
struct KingdomAlignmentBonuses { int economy = 0; int loyalty = 0; int stability = 0; };
KingdomAlignmentBonuses alignment_bonuses(const std::string& alignment);
enum class PromotionLaw { None, Token, Standard, Aggressive, Expansionist };
enum class TaxationLaw { None, Light, Normal, Heavy, Overwhelming };
struct KingdomLawInput {
 PromotionLaw promotion = PromotionLaw::None;
 bool has_cathedral = false;
 TaxationLaw taxation = TaxationLaw::None;
 bool has_waterfront = false;
 int holiday_interval_months = 0; // 0 represents the workbook's ">None" sentinel.
};
struct KingdomLawEffects { int economy = 0; int loyalty = 0; int stability = 0; int consumption = 0; };
KingdomLawEffects calculate_laws(const KingdomLawInput& input);
enum class KingdomStatFocus { Economy, Loyalty, Stability };
struct BenefitCoverage { bool economy = false; bool loyalty = false; bool stability = false; };
struct LeadershipSlot { bool filled = false; int bonus = 0; };
struct KingdomLeadershipInput {
 int baron_bonus = 0;
 BenefitCoverage baron_benefits{true, false, false};
 LeadershipSlot councilor, general, grand_diplomat, high_priest, magister, marshal;
 LeadershipSlot royal_enforcer, spymaster, treasurer, warden;
 KingdomStatFocus spymaster_benefit = KingdomStatFocus::Economy;
};
struct KingdomLeadershipResult {
 int economy_bonus = 0; int loyalty_bonus = 0; int stability_bonus = 0;
 int economy_vacancy = 0; int loyalty_vacancy = 0; int stability_vacancy = 0;
};
KingdomLeadershipResult calculate_leadership(const KingdomLeadershipInput& input);
int control_dc(int kingdom_size, int total_districts, int other = 0);
int population_total(int kingdom_size, int city_count, int total_city_population);
struct KingdomStatBaseInput { int events = 0; int improvements = 0; int other = 0; };
struct KingdomRuleInputs {
 std::string alignment = "N";
 KingdomLawInput laws;
 KingdomLeadershipInput leadership;
 KingdomStatBaseInput economy, loyalty, stability;
 int kingdom_size = 0;
 int total_city_districts = 0;
 int control_dc_other = 0;
 int city_count = 0;
 int total_city_population = 0;
};
struct KingdomSummary {
 int control_dc = 20;
 KingdomStatBreakdown economy, loyalty, stability;
 KingdomLawEffects laws;
 KingdomLeadershipResult leadership;
 SettlementTotals settlement_totals;
 int population = 0;
};
KingdomSummary calculate_kingdom_summary(int unrest, const KingdomRuleInputs& input,
 const std::vector<Settlement>& settlements = std::vector<Settlement>{},
 const std::vector<BuildingCatalogEntry>* catalog = nullptr);
struct TurnChecklistStep {
 std::string id;
 std::string phase;
 std::string marker;
 std::string label;
 bool optional = false;
};
struct TurnProgress {
 int turn = 1;
 std::vector<std::string> completed_step_ids;
};
const std::vector<TurnChecklistStep>& turn_checklist_steps();
enum class StabilityFailureOutcome { FailByLessThanFour, UnresolvedExactlyFour, FailByAtLeastFive };
StabilityFailureOutcome classify_stability_failure(int failure_by);
enum class TurnAction { ClaimHex, AbandonHex, AbandonCity };
struct TurnActionOutcome { int treasury_delta = 0; int kingdom_size_delta = 0; int unrest_delta = 0; };
TurnActionOutcome resolve_turn_action(TurnAction action);
struct KingdomSizeLimits {
 int new_settlements = 0; int new_buildings = 0; bool unlimited_new_buildings = false;
 int improvements_per_hex = 0; int hex_claims = 0;
};
KingdomSizeLimits kingdom_size_limits(int kingdom_size);
struct TurnEconomyRates {
 int gold_per_withdrawn_bp = 2000; int gold_per_deposited_bp = 4000;
 int item_value_per_bp = 8000; int unrest_per_withdrawn_bp = 1;
 int gold_per_buy_for_kingdom_bp = 2000;
};
TurnEconomyRates turn_economy_rates();
int resolve_tax_bp_delta(bool economy_check_success, int result_divided_by_three, int other_bp_gained);
struct MagicItemWorkflow {
 bool buy_item_moves_to_pc = true;
 bool success_recycles_item = true;
 bool failure_recycles_item = false;
 int excess_checks_economy_delta = -1;
 int gold_per_buy_for_kingdom_bp = 2000;
 bool buy_for_kingdom_requires_use = true;
};
MagicItemWorkflow magic_item_workflow();
struct CalendarTemplateEntry {
 std::string id;
 int source_row = 0;
 std::string month;
 std::string year_marker;
};
struct CalendarNote {
 std::string entry_id;
 std::string holiday;
 std::string kingdom_upgrades;
 std::string events;
 std::string other;
};
const std::vector<CalendarTemplateEntry>& calendar_template_entries();
struct Kingdom {
 int schema_version = 9;
 std::string name = "New Kingdom";
 int turn = 1;
 int treasury_bp = 0;
 int unrest = 0;
 KingdomRuleInputs rule_inputs;
 std::vector<Settlement> settlements;
 std::vector<Building> buildings;
 std::vector<TurnProgress> turn_progress;
 std::vector<CalendarNote> calendar_notes;
};
void validate(const Kingdom& kingdom);
void save(const Kingdom& kingdom, const std::string& path);
Kingdom load(const std::string& path);
}
