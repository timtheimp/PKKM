#pragma once
#include <cstddef>
#include <string>
#include <vector>
namespace pkkm {
struct BuildingCatalogEntry;
struct Building { std::string name; int count = 0; };
// Placement coordinates are zero-based; saved dimensions are explicit, while the editor may seed editable defaults from catalog lot counts.
struct MapPlacement { std::string building_name; int row = 0; int column = 0; int width = 1; int height = 1; };
struct MapFootprint { int width = 1; int height = 1; };
MapFootprint rotate_map_footprint(MapFootprint footprint);
MapFootprint default_map_footprint_for_lots(int lots);
// Labels are visual annotations (terrain, gate, corridor text), not rule-bearing effects.
struct MapLabel { std::string text; int row = 0; int column = 0; int width = 1; int height = 1; };
struct SettlementMap {
 int rows = 0; int columns = 0;
 std::vector<MapPlacement> placements;
 std::vector<MapLabel> labels;
 // Empty for a generic grid. Workbook/PDF templates persist an explicit identity.
 std::string template_id;
};
enum class WorkbookMapTemplate { Capital, City, Example, MultiDistrict, CityLayout };
const char* workbook_map_template_name(WorkbookMapTemplate preset);
SettlementMap workbook_map_template(WorkbookMapTemplate preset);
int city_layout_district_index(const SettlementMap& map, int row, int column);
// Counts occupied six-by-six city districts for the supported workbook map geometries.
// The PDF City Layout is one district containing nine visual two-by-two city squares.
int map_occupied_district_count(const SettlementMap& map);
enum class CityBoundarySurface { Land, Water };
const char* city_boundary_surface_name(CityBoundarySurface surface);
CityBoundarySurface city_boundary_surface_from_name(const std::string& name);
struct CityLayoutWorksheet {
 CityBoundarySurface north = CityBoundarySurface::Land;
 CityBoundarySurface south = CityBoundarySurface::Land;
 CityBoundarySurface west = CityBoundarySurface::Land;
 CityBoundarySurface east = CityBoundarySurface::Land;
 int base_value = 0;
 std::vector<std::string> item_columns[4];
};
struct SettlementMagicItemStock {
 std::string tier; // minor, medium, or major
 int slot_index = 0;
 std::string state = "empty"; // empty, generated, sold, or manual_resolution
 std::string item_name;
 int price_gp = 0;
 std::string source_table;
 std::string source_url;
 int generated_turn = 0;
 int last_changed_turn = 0;
};
struct Settlement {
 std::string name;
 int population = 0; int districts = 0;
 int economy = 0; int loyalty = 0; int stability = 0; int defense = 0;
 std::vector<Building> building_inventory;
 bool apply_catalog_stat_effects = false;
 SettlementMap map;
 std::string id;
 CityLayoutWorksheet city_layout;
 std::vector<SettlementMagicItemStock> random_magic_item_stock;
};
struct GreenbeltCalibration {
 double origin_x = 0.0; double origin_y = 0.0;
 double hex_width = 0.0; double hex_height = 0.0; double scale = 1.0;
};
struct GreenbeltHex {
 int q = 0; int r = 0;
 std::string terrain = "unresolved";
 bool claimed = false;
 int claim_turn = 0;
 std::string settlement_id;
 std::vector<std::string> improvements;
 std::string terrain_notes;
};
struct GreenbeltSpecialTerrain {
 int q = 0; int r = 0;
 std::string type = "unresolved";
 std::string name;
 std::string notes;
};
struct GreenbeltCustomImprovement {
 std::string id;
 std::string name;
 std::string effect_preview;
 std::string cost_description;
 bool manual_only = true;
 std::vector<std::string> effects;
 std::vector<std::string> costs;
};
struct GreenbeltMap {
 std::string asset_path;
 int image_width = 0; int image_height = 0;
 GreenbeltCalibration calibration;
 std::vector<GreenbeltHex> hexes;
 std::vector<GreenbeltSpecialTerrain> special_terrain;
 std::vector<GreenbeltCustomImprovement> custom_improvements;
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
 std::vector<std::string> skipped_step_ids;
};
const std::vector<TurnChecklistStep>& turn_checklist_steps();
const TurnChecklistStep* find_turn_checklist_step(const std::string& step_id);
enum class RuleProfile { Pdf, Workbook, CityInfo };
enum class StabilityFailureOutcome { FailByLessThanFour, FailByAtLeastFive };
StabilityFailureOutcome classify_stability_failure(int failure_by);
enum class StabilityCheckBranch { Success, FailByLessThanFour, FailByAtLeastFive };
struct StabilityCheckOutcome {
 int treasury_delta = 0;
 int unrest_delta = 0;
};
struct StabilityCheckPreview {
 bool success = false;
 StabilityFailureOutcome failure_outcome = StabilityFailureOutcome::FailByLessThanFour;
 StabilityCheckOutcome outcome;
};
StabilityCheckPreview preview_stability_check(RuleProfile profile, bool success,
 int current_unrest, int failure_by = 0, int rolled_unrest = 0);
StabilityCheckOutcome resolve_stability_check(bool success, int current_unrest,
 int failure_by = 0, int rolled_unrest = 0);
StabilityCheckOutcome resolve_stability_check(RuleProfile profile, bool success,
 int current_unrest, int failure_by = 0, int rolled_unrest = 0);
struct StabilityCheckRecord {
 int turn = 1;
 bool success = false;
 int failure_by = 0;
 int rolled_unrest = 0;
 int treasury_delta = 0;
 int unrest_delta = 0;
 RuleProfile rule_profile = RuleProfile::Pdf;
};

enum class TurnAction { ClaimHex, AbandonHex, AbandonCity };
const char* turn_action_checklist_step(TurnAction action);
struct TurnActionOutcome { int treasury_delta = 0; int kingdom_size_delta = 0; int unrest_delta = 0; };
TurnActionOutcome resolve_turn_action(TurnAction action);
struct TurnActionRecord {
 int turn = 1;
 TurnAction action = TurnAction::ClaimHex;
 int count = 1;
 TurnActionOutcome applied;
 bool reversal = false;
 std::ptrdiff_t reverses_index = -1;
};
struct KingdomSizeLimits {
 int new_settlements = 0; int new_buildings = 0; bool unlimited_new_buildings = false;
 int improvements_per_hex = 0; int hex_claims = 0;
};
KingdomSizeLimits kingdom_size_limits(int kingdom_size);
struct TurnEconomyRates {
 int gold_per_withdrawn_bp = 2000; int gold_per_deposited_bp = 4000;
 int item_value_per_bp = 4000; int unrest_per_withdrawn_bp = 1;
 int gold_per_buy_for_kingdom_bp = 2000;
};
TurnEconomyRates turn_economy_rates();
struct Kingdom;
int resolve_tax_bp_delta(bool economy_check_success, int result_input, int other_bp_gained);
int resolve_tax_bp_delta(RuleProfile profile, bool economy_check_success,
    int result_input, int other_bp_gained);
struct TaxCollectionRecord {
 int turn = 1;
 bool economy_check_success = false;
 // Caller-supplied tax result input; the active rule profile determines
 // whether it is divided by three or five.
 int result_input = 0;
 int other_bp_gained = 0;
 int treasury_delta = 0;
 RuleProfile rule_profile = RuleProfile::Pdf;
};
void apply_tax_action(Kingdom& kingdom, bool economy_check_success,
 int result_input, int other_bp_gained);
void apply_stability_check(Kingdom& kingdom, bool success, int failure_by = 0, int rolled_unrest = 0);
struct ConsumptionRecord {
 int turn = 1;
 int consumption_bp = 0;
 bool paid = false;
 int treasury_delta = 0;
 int unrest_delta = 0;
};
struct FoodResourcePreview {
 int starting_consumption = 0;
 int farm_reduction = 0;
 int extra_reduction = 0;
 int effective_consumption = 0;
 int surplus_bp = 0;
};
FoodResourcePreview preview_food_resource(int starting_consumption, int farm_count,
 int extra_reduction);
void apply_consumption_action(Kingdom& kingdom, int consumption_bp);
int preview_quarry_income(int quarry_count);
struct OptionalMinePreview {
 int starting_bp = 0;
 int months_elapsed = 0;
 int remaining_bp = 0;
 int player_share_bp = 0;
 int unresolved_remainder_bp = 0;
 bool exhausted = false;
 bool remainder_manual = true;
 bool zero_behavior_manual = true;
};
OptionalMinePreview preview_optional_mine_rule(int starting_bp, int months_elapsed);
struct UpkeepPreview {
 int starting_treasury = 0;
 int starting_unrest = 0;
 bool skipped_for_no_hexes = false;
 int stability_treasury_delta = 0;
 int stability_unrest_delta = 0;
 int consumption_treasury_delta = 0;
 int consumption_unrest_delta = 0;
 bool consumption_paid = false;
 bool negative_treasury_threat = false;
 int negative_treasury_unrest_delta = 0;
 int royal_assassin_unrest_delta = 0;
 int projected_treasury = 0;
 int projected_unrest = 0;
 bool threshold_threat_before_royal_assassin = false;
 bool anarchy_threat_before_royal_assassin = false;
 bool loses_hex = false;
 bool anarchy = false;
};
UpkeepPreview preview_upkeep(const Kingdom& kingdom, bool stability_success,
    int consumption_bp, bool royal_assassin_employed, int failure_by = 0,
    int rolled_unrest = 0);
struct UnrestModificationRecord {
 int turn = 1;
 int negative_stat_count = 0;
 int unrest_delta = 0;
};
struct UnrestModificationPreview {
 int current_unrest = 0;
 int unrest_delta = 0;
 int projected_unrest = 0;
 bool loses_hex = false;
 bool anarchy = false;
};
UnrestModificationPreview preview_unrest_modification(const Kingdom& kingdom, int negative_stat_count);
void apply_unrest_modification(Kingdom& kingdom, int negative_stat_count);
struct GoldWithdrawalRecord {
 int turn = 1;
 int bp_withdrawn = 0;
 int gold_received = 0;
 int treasury_delta = 0;
 int unrest_delta = 0;
 bool loyalty_check_success = false;
 int loyalty_failure_unrest_delta = 0;
};
void apply_gold_withdrawal(Kingdom& kingdom, int bp_withdrawn, bool loyalty_check_success);
struct GoldDepositRecord {
 int turn = 1;
 int gold_deposited = 0;
 int bp_received = 0;
 int treasury_delta = 0;
};
int resolve_gold_deposit_bp(int gold_deposited);
void apply_gold_deposit(Kingdom& kingdom, int gold_deposited);
struct ItemDepositRecord {
 int turn = 1;
 int item_value = 0;
 int bp_received = 0;
 int treasury_delta = 0;
 RuleProfile rule_profile = RuleProfile::Pdf;
};
int resolve_item_deposit_bp(int item_value);
int resolve_item_deposit_bp(RuleProfile profile, int item_value);
void apply_item_deposit(Kingdom& kingdom, int item_value);
struct LoyaltyCheckRecord {
 int turn = 1;
 bool success = false;
 int unrest_delta = 0;
 int loyalty_delta = 0;
};
void apply_loyalty_check(Kingdom& kingdom, bool success);
struct CommissionRecord {
 int turn = 1;
 std::string item_name;
 std::string recipient;
 int cost_bp = 0;
 int exotic_crafters_available = 0;
};
void apply_commission(Kingdom& kingdom, const std::string& item_name,
 const std::string& recipient, int cost_bp, int exotic_crafters_available);
enum class MagicItemCategory { Minor, Medium, Major };
struct ValuableItemSaleRequest {
 std::string item_name;
 std::string city_id;
 int district = 0;
 int item_value_gp = 0;
 MagicItemCategory category = MagicItemCategory::Minor;
 int economy_check_result = 0;
};
struct ValuableItemSaleRecord {
 int turn = 1;
 std::string item_name;
 std::string city_id;
 int district = 0;
 int item_value_gp = 0;
 MagicItemCategory category = MagicItemCategory::Minor;
 int economy_check_dc = 0;
 int economy_check_result = 0;
 bool sold = false;
 int bp_received = 0;
};
int valuable_item_sale_dc(MagicItemCategory category);
void apply_valuable_item_sale(Kingdom& kingdom, const ValuableItemSaleRequest& request);
int magic_item_sale_bp(MagicItemCategory category);
const char* magic_item_category_name(MagicItemCategory category);
MagicItemCategory magic_item_category_from_name(const std::string& name);
struct MagicItemOutcomeRecord {
 int turn = 1;
 MagicItemCategory category = MagicItemCategory::Minor;
 bool sold = false;
 bool commissioned = false;
 bool lost = false;
 int bp_received = 0;
};
void apply_magic_item_outcome(Kingdom& kingdom, MagicItemCategory category,
 bool sold, bool commissioned = false);
struct MagicItemWorkflow {
 bool buy_item_moves_to_pc = true;
 bool success_recycles_item = true;
 bool failure_recycles_item = false;
 int excess_checks_economy_delta = -1;
 int gold_per_buy_for_kingdom_bp = 2000;
 bool buy_for_kingdom_requires_use = true;
};
MagicItemWorkflow magic_item_workflow();
struct EventCadence {
 int no_event_last_month_percent = 75;
 int event_last_month_percent = 25;
};
EventCadence event_cadence();
enum class KingdomEventKind {
 AssassinationAttempt, BanditActivity, Disaster, EconomicBoom, Feud,
 FoodShortage, FoodSurplus, GoodWeather, MonsterAttack, NaturalBlessing,
 OutstandingSuccess, Plague, PoliticalCalm, PublicScandal, SensationalCrime,
 NewVassals, VisitingCelebrity
};
struct KingdomEventDefinition {
 int d100_min = 1;
 int d100_max = 100;
 KingdomEventKind kind = KingdomEventKind::AssassinationAttempt;
 bool continuous = false;
 const char* name = "";
};
const std::vector<KingdomEventDefinition>& kingdom_event_definitions();
const KingdomEventDefinition& kingdom_event_from_d100(int roll);
const char* kingdom_event_kind_name(KingdomEventKind kind);
struct KingdomEventRecord {
 int turn = 1;
 int d100_roll = 1;
 KingdomEventKind kind = KingdomEventKind::AssassinationAttempt;
 bool resolved = false;
 bool continuous_active = false;
 std::string affected_leader;
 std::string affected_city;
 std::string affected_block;
 std::string affected_hex;
 std::string consequence_notes;
 int treasury_delta = 0;
 int unrest_delta = 0;
 int kingdom_size_delta = 0;
 bool hex_loss_confirmed = false;
 bool anarchy_confirmed = false;
 bool consequences_applied = false;
};
struct ConstructionLine {
 std::string building_name;
 int count = 1;
 std::string upgrade_from;
 int row = 0;
 int column = 0;
 int width = 1;
 int height = 1;
 int source_width = 0;
 int source_height = 0;
};
struct ConstructionRequest {
 std::string settlement_id;
 std::vector<ConstructionLine> lines;
 bool override_building_limit = false;
 bool override_failed_checks = false;
 std::vector<std::string> terrain_facts;
};
struct ConstructionPreview {
 bool finalized = false;
 bool successful = false;
 int full_cost = 0;
 int discounted_cost = 0;
 int discounted_buildings = 0;
 int construction_count = 0;
 std::vector<std::string> failures;
};
struct ConstructionRecord {
 int turn = 1;
 std::string settlement_id;
 std::vector<ConstructionLine> lines;
 int bp_cost = 0;
 bool successful = false;
 bool override_building_limit = false;
 bool override_failed_checks = false;
 bool reversal = false;
 std::ptrdiff_t reverses_index = -1;
 std::vector<std::string> failures;
 std::vector<std::string> terrain_facts;
 std::vector<std::string> discount_sources;
};
struct Kingdom;
 struct ArmyCreationRequest {
 std::string name;
 std::string unit_description;
 std::string size_category = "medium";
 int individual_cr = 1;
 int hd_average_tenths = 45; // d6=35, d8=45, d10=55, d12=65
 int speed = 2;
 std::string leader;
 int leader_charisma = 0;
 bool ranged = false;
 int mount_cr = 0;
 int improved_weapons = 0; // 0 none, 1 masterwork, 2 magic
 int improved_armor = 0; // 0 none, 1 masterwork, 2 magic
 int healing_potions = 0;
 int ranged_weapons = 0;
 int siege_engines = 0;
 std::vector<std::string> tactics;
 std::vector<std::string> special_abilities;
 std::vector<std::string> prerequisites;
 std::string notes;
};
struct ArmyRecord {
 int turn = 1;
 ArmyCreationRequest request;
 int soldiers = 0;
 int army_cr = 0;
 int hit_points = 0;
 int current_hit_points = 0;
 int defense_value = 0;
 int offense_modifier = 0;
 int morale = 0;
 int weekly_consumption = 0;
 int bp_cost = 0;
};
void apply_army_creation(Kingdom& kingdom, const ArmyCreationRequest& request);
std::size_t record_kingdom_event(Kingdom& kingdom, int d100_roll);
void apply_manual_event_consequence(Kingdom& kingdom, std::size_t event_index);
struct TurnTransitionPreview {
 int current_turn = 1;
 int next_turn = 2;
 int event_trigger_percent = 75;
 bool event_resolution_manual = true;
};
TurnTransitionPreview preview_turn_transition(const Kingdom& kingdom, bool event_occurred_last_turn);
struct TurnResolutionRequest {
 bool stability_success = false;
 int failure_by = 0;
 int rolled_unrest = 0;
 int consumption_bp = 1;
 bool royal_assassin_employed = false;
};
struct TurnResolutionPreview {
 int current_turn = 1;
 int next_turn = 2;
 UpkeepPreview upkeep;
 bool event_resolution_manual = true;
};
TurnResolutionPreview preview_turn_resolution(const Kingdom& kingdom,
 const TurnResolutionRequest& request);
void apply_turn_resolution(Kingdom& kingdom, const TurnResolutionRequest& request);
enum class EventResolution { NoEvent, ManualResolutionRequired };
EventResolution classify_event_resolution(bool event_triggered);
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
const char* rule_profile_name(RuleProfile profile);
RuleProfile rule_profile_from_name(const std::string& name);
struct Kingdom {
 int schema_version = 10;
 std::string name = "New Kingdom";
 int turn = 1;
 int treasury_bp = 0;
 int unrest = 0;
 RuleProfile rule_profile = RuleProfile::Pdf;
 bool use_optional_mine_rule = false;
 KingdomRuleInputs rule_inputs;
 std::vector<Settlement> settlements;
 std::vector<Building> buildings;
 std::vector<TurnProgress> turn_progress;
 std::vector<TurnActionRecord> turn_actions;
 std::vector<TaxCollectionRecord> tax_actions;
 std::vector<StabilityCheckRecord> stability_checks;
 std::vector<ConsumptionRecord> consumption_actions;
 std::vector<UnrestModificationRecord> unrest_modifications;
 std::vector<GoldWithdrawalRecord> gold_withdrawals;
 std::vector<GoldDepositRecord> gold_deposits;
 std::vector<ItemDepositRecord> item_deposits;
 std::vector<LoyaltyCheckRecord> loyalty_checks;
 std::vector<CommissionRecord> commissions;
 std::vector<MagicItemOutcomeRecord> magic_item_outcomes;
 std::vector<ValuableItemSaleRecord> valuable_item_sales;
 std::vector<KingdomEventRecord> kingdom_events;
 std::vector<ConstructionRecord> construction_transactions;
 std::vector<ArmyRecord> armies;
 std::vector<CalendarNote> calendar_notes;
 GreenbeltMap greenbelt_map;
};
struct TurnChecklistStatus {
 int turn = 1;
 std::vector<TurnChecklistStep> steps;
 std::vector<std::string> completed_step_ids;
 std::vector<std::string> skipped_step_ids;
 std::string next_step_id;
};
TurnChecklistStatus turn_checklist_status(const Kingdom& kingdom, int turn);
bool complete_current_turn_checklist_step(Kingdom& kingdom, int turn, const std::string& step_id);
bool skip_current_turn_checklist_step(Kingdom& kingdom, int turn, const std::string& step_id);
void set_turn_checklist_step(Kingdom& kingdom, int turn, const std::string& step_id, bool completed);
void skip_turn_checklist_step(Kingdom& kingdom, int turn, const std::string& step_id);
void advance_turn(Kingdom& kingdom);
void apply_turn_action(Kingdom& kingdom, TurnAction action, int count = 1);
void reverse_last_turn_action(Kingdom& kingdom);
void reverse_turn_action(Kingdom& kingdom, std::size_t index);
ConstructionPreview apply_construction(Kingdom& kingdom,
 const ConstructionRequest& request,
 const std::vector<BuildingCatalogEntry>& catalog);
void reverse_construction(Kingdom& kingdom, std::size_t index,
 const std::vector<BuildingCatalogEntry>& catalog, bool override_reversal);
void validate(const Kingdom& kingdom);
void save(const Kingdom& kingdom, const std::string& path);
Kingdom load(const std::string& path);
}
