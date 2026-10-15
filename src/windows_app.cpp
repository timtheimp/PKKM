#define UNICODE
#define _UNICODE
#define NOMINMAX
#include <windows.h>
#include <commdlg.h>
#include <gdiplus.h>
#include "pkkm/kingdom.hpp"
#include "pkkm/buildings.hpp"
#include "pkkm/greenbelt.hpp"
#include "pkkm/map_editor.hpp"
#include <algorithm>
#include <cctype>
#include <cmath>
#include <filesystem>
#include <functional>
#include <limits>
#include <random>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {
constexpr int kOpen = 1;
constexpr int kSave = 2;
constexpr int kNew = 3;
constexpr int kEditInputs = 4;
constexpr int kManageSettlements = 5;
constexpr int kTurnChecklist = 6;
constexpr int kCalendarTracker = 7;
constexpr int kGreenbeltMap = 8;
constexpr int kTurnActions = 9;
constexpr int kDisplay = 10;
constexpr int kAdvanceTurn = 11;
constexpr int kEditorApply = 3001;
constexpr int kEditorCancel = 3002;
constexpr int kEditorClose = 3003;
constexpr int kFieldIdBase = 4000;
constexpr int kColumnWidth = 325;
constexpr int kSettlementList = 5100;
constexpr int kSettlementName = 5101;
constexpr int kSettlementPopulation = 5102;
constexpr int kSettlementDistricts = 5103;
constexpr int kSettlementEconomy = 5104;
constexpr int kSettlementLoyalty = 5105;
constexpr int kSettlementStability = 5106;
constexpr int kSettlementDefense = 5107;
constexpr int kSettlementApplyCatalogStats = 5108;
constexpr int kInventoryList = 5110;
constexpr int kCatalogChoice = 5111;
constexpr int kBuildingCount = 5112;
constexpr int kAddBuilding = 5113;
constexpr int kRemoveBuilding = 5114;
constexpr int kAddSettlement = 5115;
constexpr int kRemoveSettlement = 5116;
constexpr int kApplySettlements = 5117;
constexpr int kCancelSettlements = 5118;
constexpr int kCloseSettlements = 5121;
constexpr int kEditMap = 5119;
constexpr int kMapSummary = 5120;
constexpr int kChecklistApply = 6201;
constexpr int kChecklistCancel = 6202;
constexpr int kChecklistClose = 6203;
constexpr int kChecklistEventLastTurn = 6204;
constexpr int kChecklistTransitionStatus = 6205;
constexpr int kChecklistControlBase = 6300;
constexpr int kActionChoice = 6400;
constexpr int kActionCount = 6401;
constexpr int kActionApply = 6402;
constexpr int kActionCancel = 6403;
constexpr int kActionClose = 6404;
constexpr int kActionHistory = 6405;
constexpr int kActionReverse = 6406;
constexpr int kActionFilter = 6407;
constexpr int kActionDetails = 6408;
constexpr int kActionFilterOriginal = 6409;
constexpr int kActionFilterReversal = 6410;
constexpr int kTaxSuccess = 6411;
constexpr int kTaxResult = 6412;
constexpr int kTaxOther = 6413;
constexpr int kTaxApply = 6414;
constexpr int kTaxStatus = 6415;
constexpr int kTaxHistory = 6416;
constexpr int kWithdrawalBp = 6417;
constexpr int kWithdrawalApply = 6418;
constexpr int kWithdrawalStatus = 6419;
constexpr int kWithdrawalHistory = 6420;
constexpr int kDepositGold = 6421;
constexpr int kDepositApply = 6422;
constexpr int kDepositStatus = 6423;
constexpr int kDepositHistory = 6424;
constexpr int kDepositItemValue = 6439;
constexpr int kDepositItemApply = 6440;
constexpr int kDepositItemStatus = 6441;
constexpr int kDepositItemHistory = 6442;
constexpr int kLoyaltySuccess = 6443;
constexpr int kLoyaltyApply = 6444;
constexpr int kLoyaltyStatus = 6445;
constexpr int kLoyaltyHistory = 6446;
constexpr int kCommissionItem = 6447;
constexpr int kCommissionRecipient = 6448;
constexpr int kCommissionCost = 6449;
constexpr int kCommissionCrafters = 6450;
constexpr int kCommissionApply = 6451;
constexpr int kCommissionStatus = 6452;
constexpr int kCommissionHistory = 6453;
constexpr int kRuleProfileChoice = 6454;
constexpr int kWithdrawalLoyaltySuccess = 6455;
constexpr int kMagicItemCategory = 6456;
constexpr int kMagicItemSold = 6457;
constexpr int kMagicItemCommissioned = 6458;
constexpr int kMagicItemApply = 6459;
constexpr int kMagicItemStatus = 6460;
constexpr int kMagicItemHistory = 6461;
constexpr int kStabilitySuccess = 6425;
constexpr int kStabilityFailureBy = 6426;
constexpr int kStabilityRoll = 6427;
constexpr int kStabilityApply = 6428;
constexpr int kStabilityStatus = 6429;
constexpr int kStabilityHistory = 6430;
constexpr int kConsumptionBp = 6431;
constexpr int kConsumptionApply = 6432;
constexpr int kConsumptionStatus = 6433;
constexpr int kConsumptionHistory = 6434;
constexpr int kOptionalMineRule = 6462;
constexpr int kRoyalAssassinEmployed = 6463;
constexpr int kUpkeepPreview = 6464;
constexpr int kUpkeepStatus = 6465;
constexpr int kFoodStartingConsumption = 6466;
constexpr int kFoodFarmCount = 6467;
constexpr int kFoodExtraReduction = 6468;
constexpr int kFoodPreview = 6469;
constexpr int kFoodStatus = 6470;
constexpr int kQuarryCount = 6471;
constexpr int kQuarryPreview = 6472;
constexpr int kQuarryStatus = 6473;
constexpr int kMineStartingValue = 6474;
constexpr int kMineMonthsElapsed = 6475;
constexpr int kMinePreview = 6476;
constexpr int kMineStatus = 6477;
constexpr int kEventRoll = 6478;
constexpr int kEventAutoRoll = 6479;
constexpr int kEventResolved = 6480;
constexpr int kEventLeader = 6481;
constexpr int kEventCity = 6482;
constexpr int kEventBlock = 6483;
constexpr int kEventHex = 6484;
constexpr int kEventPreview = 6485;
constexpr int kEventApply = 6486;
constexpr int kEventStatus = 6487;
constexpr int kEventHistory = 6488;
constexpr int kEventConsequenceNotes = 6489;
constexpr int kEventTreasuryDelta = 6490;
constexpr int kEventUnrestDelta = 6491;
constexpr int kEventSizeDelta = 6492;
constexpr int kEventHexLoss = 6493;
constexpr int kEventAnarchy = 6494;
constexpr int kEventApplyConsequence = 6495;
constexpr int kEventDetails = 6496;
constexpr int kUnrestNegativeStats = 6435;
constexpr int kUnrestApply = 6436;
constexpr int kUnrestStatus = 6437;
constexpr int kUnrestHistory = 6438;
constexpr int kCalendarEntryList = 7100;
constexpr int kCalendarEntryTitle = 7101;
constexpr int kCalendarHoliday = 7102;
constexpr int kCalendarUpgrades = 7103;
constexpr int kCalendarEvents = 7104;
constexpr int kCalendarOther = 7105;
constexpr int kCalendarApply = 7201;
constexpr int kCalendarCancel = 7202;
constexpr int kCalendarClose = 7203;
constexpr int kGreenbeltHexList = 8100;
constexpr int kGreenbeltQ = 8101;
constexpr int kGreenbeltR = 8102;
constexpr int kGreenbeltTerrain = 8103;
constexpr int kGreenbeltClaimed = 8104;
constexpr int kGreenbeltClaimTurn = 8105;
constexpr int kGreenbeltSettlement = 8106;
constexpr int kGreenbeltImprovement = 8107;
constexpr int kGreenbeltAddImprovement = 8108;
constexpr int kGreenbeltPreview = 8109;
constexpr int kGreenbeltOriginX = 8110;
constexpr int kGreenbeltOriginY = 8111;
constexpr int kGreenbeltHexWidth = 8112;
constexpr int kGreenbeltHexHeight = 8113;
constexpr int kGreenbeltScale = 8114;
constexpr int kGreenbeltSpecialList = 8115;
constexpr int kGreenbeltSpecialQ = 8116;
constexpr int kGreenbeltSpecialR = 8117;
constexpr int kGreenbeltSpecialType = 8118;
constexpr int kGreenbeltSpecialName = 8119;
constexpr int kGreenbeltSpecialNotes = 8120;
constexpr int kGreenbeltSpecialSave = 8121;
constexpr int kGreenbeltSpecialRemove = 8122;
constexpr int kGreenbeltCustomList = 8123;
constexpr int kGreenbeltCustomId = 8124;
constexpr int kGreenbeltCustomName = 8125;
constexpr int kGreenbeltCustomEffect = 8126;
constexpr int kGreenbeltCustomCost = 8127;
constexpr int kGreenbeltCustomManual = 8128;
constexpr int kGreenbeltCustomSave = 8129;
constexpr int kGreenbeltCustomRemove = 8130;
constexpr int kGreenbeltAddHex = 8131;
constexpr int kGreenbeltAutodetected = 8132;
constexpr int kGreenbeltOpenMap = 8133;
constexpr int kGreenbeltZoomIn = 8134;
constexpr int kGreenbeltZoomOut = 8135;
constexpr int kGreenbeltZoomFit = 8136;
constexpr int kGreenbeltPopoutClose = 8137;
constexpr int kGreenbeltAlignMode = 8138;
constexpr int kGreenbeltPanLeft = 8139;
constexpr int kGreenbeltPanRight = 8140;
constexpr int kGreenbeltPanUp = 8141;
constexpr int kGreenbeltPanDown = 8142;
constexpr int kGreenbeltGrowWidth = 8143;
constexpr int kGreenbeltShrinkWidth = 8144;
constexpr int kGreenbeltGrowHeight = 8145;
constexpr int kGreenbeltShrinkHeight = 8146;
constexpr int kGreenbeltResetPan = 8147;
constexpr int kGreenbeltAlignmentStatus = 8148;
constexpr int kGreenbeltTerrainNotes = 8149;
constexpr int kGreenbeltCustomOption = 8150;
constexpr int kGreenbeltCustomPopupId = 8151;
constexpr int kGreenbeltCustomPopupName = 8152;
constexpr int kGreenbeltCustomPopupEffect = 8153;
constexpr int kGreenbeltCustomPopupCost = 8154;
constexpr int kGreenbeltCustomPopupManual = 8155;
constexpr int kGreenbeltCustomPopupAdd = 8156;
constexpr int kGreenbeltCustomPopupCancel = 8157;
constexpr int kGreenbeltCustomPopupEffectCombo = 8158;
constexpr int kGreenbeltCustomPopupEffectList = 8159;
constexpr int kGreenbeltCustomPopupEffectAdd = 8160;
constexpr int kGreenbeltCustomPopupEffectRemove = 8161;
constexpr int kGreenbeltCustomPopupCostCombo = 8162;
constexpr int kGreenbeltCustomPopupCostList = 8163;
constexpr int kGreenbeltCustomPopupCostAdd = 8164;
constexpr int kGreenbeltCustomPopupCostRemove = 8165;
constexpr int kGreenbeltCustomPopupEffectUpdate = 8166;
constexpr int kGreenbeltCustomPopupCostUpdate = 8167;
constexpr int kGreenbeltFactWater = 8168;
constexpr int kGreenbeltFactRiver = 8169;
constexpr int kGreenbeltFactCanal = 8170;
constexpr int kGreenbeltFactEndpoint = 8171;
constexpr int kGreenbeltFactRoad = 8172;
constexpr int kGreenbeltFactCrossing = 8173;
constexpr int kGreenbeltFactFarms = 8174;
constexpr int kGreenbeltFactSize = 8175;
constexpr int kGreenbeltFactRoute = 8176;
constexpr int kGreenbeltRefreshPreview = 8177;
constexpr int kGreenbeltApply = 8201;
constexpr int kGreenbeltCancel = 8202;
constexpr int kGreenbeltClose = 8203;
constexpr int kGreenbeltViewportX = 20;
constexpr int kGreenbeltViewportY = 104;
constexpr int kGreenbeltViewportWidth = 700;
constexpr int kGreenbeltViewportHeight = 250;
constexpr int kGreenbeltContentHeight = 1310;
constexpr int kTurnActionsContentHeight = 3250;

pkkm::Kingdom kingdom;
pkkm::Kingdom edit_draft;
std::wstring current_path;
HWND editor_window = nullptr;
HWND settlement_window = nullptr;
HWND checklist_window = nullptr;
HWND action_window = nullptr;
HWND calendar_window = nullptr;
HWND greenbelt_window = nullptr;
HWND greenbelt_popout_window = nullptr;
HWND greenbelt_custom_popup_window = nullptr;
int greenbelt_custom_popup_edit_index = -1;
double greenbelt_popout_zoom = 1.0;
double greenbelt_popout_pan_x = 0.0;
double greenbelt_popout_pan_y = 0.0;
bool greenbelt_popout_dragging = false;
POINT greenbelt_popout_drag_start{};
double greenbelt_popout_drag_origin_x = 0.0;
double greenbelt_popout_drag_origin_y = 0.0;
void preview_event_dialog(HWND window);
void apply_event_dialog(HWND window);
void apply_latest_event_consequence_dialog(HWND window);
bool greenbelt_alignment_mode = false;
HWND greenbelt_canvas = nullptr;
HINSTANCE app_instance = nullptr;
bool dirty = false;
std::vector<pkkm::Settlement> settlement_draft;
std::vector<pkkm::BuildingCatalogEntry> settlement_catalog;
std::vector<pkkm::BuildingCatalogEntry> building_catalog;
std::vector<HWND> checklist_controls;
std::vector<std::string> checklist_step_ids;
std::vector<pkkm::CalendarNote> calendar_note_draft;
int calendar_active_index = -1;
int settlement_active_index = -1;
int greenbelt_active_index = -1;
int greenbelt_special_active_index = -1;
int greenbelt_custom_active_index = -1;
pkkm::GreenbeltMap greenbelt_draft;
std::vector<pkkm::GreenbeltImprovementDefinition> greenbelt_catalog;
ULONG_PTR gdiplus_token = 0;
int greenbelt_scroll_y = 0;
int greenbelt_popout_scroll_y = 0;
int turn_actions_scroll_y = 0;
int turn_action_filter = 0;

void open_greenbelt_popout(HWND owner);
LRESULT CALLBACK greenbelt_custom_popup_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);

void set_minimum_track_size(LPARAM lparam, int width, int height) {
    auto* info = reinterpret_cast<MINMAXINFO*>(lparam);
    info->ptMinTrackSize.x = width;
    info->ptMinTrackSize.y = height;
}

enum class FieldKind { Text, Integer, Boolean, Choice };
struct EditorField {
    std::wstring label;
    int column = 0;
    FieldKind kind = FieldKind::Text;
    HWND control = nullptr;
    std::function<std::wstring()> read_text;
    std::function<void(const std::wstring&)> write_text;
    std::function<bool()> read_bool;
    std::function<void(bool)> write_bool;
    std::vector<std::wstring> options;
    std::vector<int> option_values;
    std::function<int()> read_choice;
    std::function<void(int)> write_choice;
};
std::vector<EditorField> editor_fields;

std::string narrow(const std::wstring& value) {
    if (value.empty()) return {};
    const int size = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, value.data(),
        static_cast<int>(value.size()), nullptr, 0, nullptr, nullptr);
    if (size <= 0) throw std::runtime_error("Could not convert text to UTF-8");
    std::string result(static_cast<size_t>(size), '\0');
    if (WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, value.data(), static_cast<int>(value.size()),
            result.data(), size, nullptr, nullptr) <= 0) {
        throw std::runtime_error("Could not convert text to UTF-8");
    }
    return result;
}

std::wstring widen(const std::string& value) {
    if (value.empty()) return {};
    const int size = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, value.data(),
        static_cast<int>(value.size()), nullptr, 0);
    if (size <= 0) throw std::runtime_error("Could not convert text from UTF-8");
    std::wstring result(static_cast<size_t>(size), L'\0');
    if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, value.data(), static_cast<int>(value.size()),
            result.data(), size) <= 0) {
        throw std::runtime_error("Could not convert text from UTF-8");
    }
    return result;
}

std::string next_settlement_id(const std::vector<pkkm::Settlement>& settlements) {
    for (size_t candidate = 1;; ++candidate) {
        const std::string id = "settlement." + std::to_string(candidate);
        const auto found = std::find_if(settlements.begin(), settlements.end(),
            [&id](const auto& settlement) { return settlement.id == id; });
        if (found == settlements.end()) return id;
    }
}

void ensure_settlement_ids(std::vector<pkkm::Settlement>& settlements) {
    for (auto& settlement : settlements)
        if (settlement.id.empty())
            settlement.id = next_settlement_id(settlements);
}

int parse_integer(const std::wstring& text) {
    if (text.empty())
        throw std::invalid_argument("Enter a whole number within the supported range.");
    size_t consumed = 0;
    const long long parsed = std::stoll(text, &consumed);
    if (consumed != text.size() || parsed < INT_MIN || parsed > INT_MAX) {
        throw std::invalid_argument("Enter a whole number within the supported range.");
    }
    return static_cast<int>(parsed);
}

double parse_decimal(const std::wstring& text) {
    size_t consumed = 0;
    const double parsed = std::stod(text, &consumed);
    if (consumed != text.size() || !std::isfinite(parsed))
        throw std::invalid_argument("Enter a finite decimal number.");
    return parsed;
}

void add_text_field(const wchar_t* label, int column, std::string& value) {
    auto* target = &value;
    EditorField field;
    field.label = label;
    field.column = column;
    field.kind = FieldKind::Text;
    field.read_text = [target] { return widen(*target); };
    field.write_text = [target](const std::wstring& text) { *target = narrow(text); };
    editor_fields.push_back(std::move(field));
}

void add_integer_field(const wchar_t* label, int column, int& value) {
    auto* target = &value;
    EditorField field;
    field.label = label;
    field.column = column;
    field.kind = FieldKind::Integer;
    field.read_text = [target] { return std::to_wstring(*target); };
    field.write_text = [target](const std::wstring& text) { *target = parse_integer(text); };
    editor_fields.push_back(std::move(field));
}

void add_boolean_field(const wchar_t* label, int column, bool& value) {
    auto* target = &value;
    EditorField field;
    field.label = label;
    field.column = column;
    field.kind = FieldKind::Boolean;
    field.read_bool = [target] { return *target; };
    field.write_bool = [target](bool checked) { *target = checked; };
    editor_fields.push_back(std::move(field));
}

void add_choice_field(const wchar_t* label, int column, std::vector<std::wstring> options,
                      std::vector<int> values, std::function<int()> read,
                      std::function<void(int)> write) {
    EditorField field;
    field.label = label;
    field.column = column;
    field.kind = FieldKind::Choice;
    field.options = std::move(options);
    field.option_values = std::move(values);
    field.read_choice = std::move(read);
    field.write_choice = std::move(write);
    editor_fields.push_back(std::move(field));
}

void prepare_editor_fields() {
    edit_draft = kingdom;
    editor_fields.clear();
    auto& rules = edit_draft.rule_inputs;
    add_text_field(L"Kingdom name", 0, edit_draft.name);
    add_integer_field(L"Turn", 0, edit_draft.turn);
    add_integer_field(L"Treasury (BP)", 0, edit_draft.treasury_bp);
    add_integer_field(L"Unrest", 0, edit_draft.unrest);
    add_text_field(L"Alignment code", 0, rules.alignment);
    add_integer_field(L"Kingdom size", 0, rules.kingdom_size);
    add_integer_field(L"City districts", 0, rules.total_city_districts);
    add_integer_field(L"Other Control DC", 0, rules.control_dc_other);
    add_integer_field(L"City count", 0, rules.city_count);
    add_integer_field(L"City population", 0, rules.total_city_population);
    add_integer_field(L"Economy events", 0, rules.economy.events);
    add_integer_field(L"Economy improvements", 0, rules.economy.improvements);
    add_integer_field(L"Economy other", 0, rules.economy.other);
    add_integer_field(L"Loyalty events", 0, rules.loyalty.events);
    add_integer_field(L"Loyalty improvements", 0, rules.loyalty.improvements);
    add_integer_field(L"Loyalty other", 0, rules.loyalty.other);
    add_integer_field(L"Stability events", 0, rules.stability.events);
    add_integer_field(L"Stability improvements", 0, rules.stability.improvements);
    add_integer_field(L"Stability other", 0, rules.stability.other);

    const std::vector<std::wstring> promotion_names{L"None", L"Token", L"Standard", L"Aggressive", L"Expansionist"};
    add_choice_field(L"Promotion law", 1, promotion_names, {0, 1, 2, 3, 4},
        [&rules] { return static_cast<int>(rules.laws.promotion); },
        [&rules](int value) { rules.laws.promotion = static_cast<pkkm::PromotionLaw>(value); });
    add_boolean_field(L"Cathedral present", 1, rules.laws.has_cathedral);
    const std::vector<std::wstring> taxation_names{L"None", L"Light", L"Normal", L"Heavy", L"Overwhelming"};
    add_choice_field(L"Taxation", 1, taxation_names, {0, 1, 2, 3, 4},
        [&rules] { return static_cast<int>(rules.laws.taxation); },
        [&rules](int value) { rules.laws.taxation = static_cast<pkkm::TaxationLaw>(value); });
    add_boolean_field(L"Waterfront present", 1, rules.laws.has_waterfront);
    add_choice_field(L"Holiday interval", 1, {L">None", L"1 month", L"6 months", L"12 months", L"24 months"},
        {0, 1, 6, 12, 24}, [&rules] { return rules.laws.holiday_interval_months; },
        [&rules](int value) { rules.laws.holiday_interval_months = value; });
    add_integer_field(L"Baron bonus", 1, rules.leadership.baron_bonus);
    add_boolean_field(L"Baron: Economy", 1, rules.leadership.baron_benefits.economy);
    add_boolean_field(L"Baron: Loyalty", 1, rules.leadership.baron_benefits.loyalty);
    add_boolean_field(L"Baron: Stability", 1, rules.leadership.baron_benefits.stability);
    add_choice_field(L"Spymaster benefit", 1, {L"Economy", L"Loyalty", L"Stability"}, {0, 1, 2},
        [&rules] { return static_cast<int>(rules.leadership.spymaster_benefit); },
        [&rules](int value) { rules.leadership.spymaster_benefit = static_cast<pkkm::KingdomStatFocus>(value); });

    auto add_slot = [](const wchar_t* role, pkkm::LeadershipSlot& slot) {
        std::wstring filled_label(role);
        filled_label += L" filled";
        add_boolean_field(filled_label.c_str(), 2, slot.filled);
        std::wstring bonus_label(role);
        bonus_label += L" bonus";
        add_integer_field(bonus_label.c_str(), 2, slot.bonus);
    };
    auto& leaders = rules.leadership;
    add_slot(L"Councilor", leaders.councilor);
    add_slot(L"General", leaders.general);
    add_slot(L"Grand Diplomat", leaders.grand_diplomat);
    add_slot(L"High Priest", leaders.high_priest);
    add_slot(L"Magister", leaders.magister);
    add_slot(L"Marshal", leaders.marshal);
    add_slot(L"Royal Enforcer", leaders.royal_enforcer);
    add_slot(L"Spymaster", leaders.spymaster);
    add_slot(L"Treasurer", leaders.treasurer);
    add_slot(L"Warden", leaders.warden);
}

void refresh(HWND display) {
    const auto summary = pkkm::calculate_kingdom_summary(
        kingdom.unrest, kingdom.rule_inputs, kingdom.settlements, &building_catalog);
    const auto signed_component = [](int value) {
        return value > 0 ? L"+" + std::to_wstring(value) : std::to_wstring(value);
    };
    const auto breakdown_line = [&signed_component](const pkkm::KingdomStatBreakdown& stat) {
        return L"\r\n  Modifiers: events " + signed_component(stat.events)
            + L", alignment " + signed_component(stat.alignment)
            + L", improvements " + signed_component(stat.improvements)
            + L", leadership " + signed_component(stat.leadership)
            + L", laws " + signed_component(stat.laws)
            + L", unrest " + signed_component(stat.unrest_adjustment)
            + L", vacancies " + signed_component(stat.vacancy_adjustment)
            + L", other " + signed_component(stat.other);
    };
    std::wstring text = widen(kingdom.name) + L" - Turn " + std::to_wstring(kingdom.turn)
        + L"\r\nTreasury: " + std::to_wstring(kingdom.treasury_bp) + L" BP"
        + L"\r\nUnrest: " + std::to_wstring(kingdom.unrest)
        + L"\r\nCalendar notes: " + std::to_wstring(kingdom.calendar_notes.size())
        + L" of " + std::to_wstring(pkkm::calendar_template_entries().size()) + L" source month entries"
        + L"\r\nAlignment: " + widen(kingdom.rule_inputs.alignment)
        + L"\r\nControl DC: " + std::to_wstring(summary.control_dc)
        + L" | Population: " + std::to_wstring(summary.population)
        + L"\r\n\r\nKingdom Statistics\r\nEconomy: " + std::to_wstring(summary.economy.total)
        + L" (" + std::to_wstring(summary.economy.check_threshold_percent) + L"% check)"
        + L"\r\nLoyalty: " + std::to_wstring(summary.loyalty.total)
        + L" (" + std::to_wstring(summary.loyalty.check_threshold_percent) + L"% check)"
        + L"\r\nStability: " + std::to_wstring(summary.stability.total)
        + L" (" + std::to_wstring(summary.stability.check_threshold_percent) + L"% check)"
        + breakdown_line(summary.economy) + breakdown_line(summary.loyalty) + breakdown_line(summary.stability)
        + L"\r\n\r\nSettlements\r\n";
    if (summary.settlement_totals.count > 0) {
        const auto& totals = summary.settlement_totals;
        text += L"Totals: " + std::to_wstring(totals.count) + L" settlements | population "
            + std::to_wstring(totals.population) + L" | districts " + std::to_wstring(totals.districts)
            + L" | lots " + std::to_wstring(totals.lots)
            + L" | Economy " + signed_component(totals.economy)
            + L" | Loyalty " + signed_component(totals.loyalty)
            + L" | Stability " + signed_component(totals.stability)
            + L" | Defense " + signed_component(totals.defense) + L"\r\n";
    }
    if (kingdom.settlements.empty()) text += L"(none)\r\n";
    for (const auto& settlement : kingdom.settlements) {
        const auto size = pkkm::calculate_settlement_size(settlement, building_catalog);
        const auto market = pkkm::calculate_settlement_market_reference(settlement, building_catalog);
        const auto catalog_cost = pkkm::calculate_settlement_catalog_cost(settlement, building_catalog);
        const auto magic_items = pkkm::preview_settlement_magic_items(settlement, building_catalog);
        text += L"- " + widen(settlement.name) + L" - population "
            + std::to_wstring(size.population) + L", districts " + std::to_wstring(size.districts)
            + L", lots " + std::to_wstring(size.lots)
            + L", Economy " + signed_component(settlement.economy)
            + L", Loyalty " + signed_component(settlement.loyalty)
            + L", Stability " + signed_component(settlement.stability)
            + L", Defense " + signed_component(settlement.defense) + L"\n"
            + L"  Market reference only: base value " + std::to_wstring(market.base_value)
            + L", buy limit " + std::to_wstring(market.buy_limit)
            + L", sell limit " + std::to_wstring(market.sell_limit)
            + L", spellcasting level " + std::to_wstring(market.spellcasting_level) + L"\n"
            + L"  Catalog cost reference only: " + std::to_wstring(catalog_cost) + L" BP\n"
            + L"  Magic-item slots preview: minor " + std::to_wstring(magic_items.minor_slots)
            + L", medium " + std::to_wstring(magic_items.medium_slots)
            + L", major " + std::to_wstring(magic_items.major_slots) + L"\n";
        if (!settlement.building_inventory.empty()) {
            text += L"  Building inventory: ";
            for (size_t index = 0; index < settlement.building_inventory.size(); ++index) {
                if (index > 0) text += L", ";
                const auto& building = settlement.building_inventory[index];
                text += widen(building.name) + L" x " + std::to_wstring(building.count);
            }
            text += L"\n";
            for (const auto& building : settlement.building_inventory) {
                const auto found = std::find_if(building_catalog.begin(), building_catalog.end(), [&building](const pkkm::BuildingCatalogEntry& entry) {
                    return entry.kind == pkkm::CatalogRowKind::Improvement && entry.name == building.name;
                });
                if (found == building_catalog.end()) continue;
                if (found->cost != 0 || found->lots != 0 || !found->upgrade_from.empty() || !found->upgrade_to.empty()
                    || !found->discounts.empty() || !found->magic_item.empty()) {
                    text += L"  Catalog reference: " + widen(found->name)
                        + L" | source row " + std::to_wstring(found->source_row)
                        + L" | cost " + std::to_wstring(found->cost) + L" BP"
                        + L" | lots " + std::to_wstring(found->lots);
                    if (!found->upgrade_from.empty()) text += L" | upgrades from " + widen(found->upgrade_from);
                    if (!found->upgrade_to.empty()) text += L" | upgrades to " + widen(found->upgrade_to);
                    if (!found->discounts.empty()) text += L" | discounts " + widen(found->discounts);
                    if (!found->magic_item.empty()) text += L" | magic item " + widen(found->magic_item);
                    text += L"\n";
                }
            }
            const auto effects = pkkm::calculate_settlement_building_totals(settlement, building_catalog);
            if (settlement.apply_catalog_stat_effects) {
                text += L"  Catalog stat bonuses applied to kingdom totals: Economy " + signed_component(effects.economy)
                    + L" | Loyalty " + signed_component(effects.loyalty)
                    + L" | Stability " + signed_component(effects.stability)
                    + L" | Defense/Unrest preview only: Defense " + signed_component(effects.defense)
                    + L" | Unrest " + signed_component(effects.unrest)
                    + L" | Other source previews: Corruption " + signed_component(effects.corruption)
                    + L" | Crime " + signed_component(effects.crime)
                    + L" | Law " + signed_component(effects.law)
                    + L" | Lore " + signed_component(effects.lore)
                    + L" | Society " + signed_component(effects.society)
                    + L" | Productivity " + signed_component(effects.productivity)
                    + L" | Fame " + signed_component(effects.fame) + L"\r\n";
            } else {
                text += L"  Catalog effects preview (not applied): Economy " + signed_component(effects.economy)
                    + L" | Loyalty " + signed_component(effects.loyalty)
                    + L" | Stability " + signed_component(effects.stability)
                    + L" | Defense " + signed_component(effects.defense)
                    + L" | Unrest " + signed_component(effects.unrest)
                    + L" | Other source previews: Corruption " + signed_component(effects.corruption)
                    + L" | Crime " + signed_component(effects.crime)
                    + L" | Law " + signed_component(effects.law)
                    + L" | Lore " + signed_component(effects.lore)
                    + L" | Society " + signed_component(effects.society)
                    + L" | Productivity " + signed_component(effects.productivity)
                    + L" | Fame " + signed_component(effects.fame) + L"\r\n";
            }
        }
    }
    SetWindowTextW(display, text.c_str());
    const HWND main = GetParent(display);
    SetWindowTextW(main, dirty ? L"PKKM - Kingdom Manager *" : L"PKKM - Kingdom Manager");
}

std::wstring choose_file(HWND owner, bool save) {
    wchar_t file[MAX_PATH] = L"";
    OPENFILENAMEW dialog{};
    dialog.lStructSize = sizeof(dialog);
    dialog.hwndOwner = owner;
    dialog.lpstrFilter = L"PKKM Kingdom JSON (*.json)\0*.json\0All files\0*.*\0\0";
    dialog.lpstrFile = file;
    dialog.nMaxFile = MAX_PATH;
    dialog.Flags = OFN_PATHMUSTEXIST | (save ? OFN_OVERWRITEPROMPT : OFN_FILEMUSTEXIST);
    dialog.lpstrDefExt = L"json";
    return (save ? GetSaveFileNameW(&dialog) : GetOpenFileNameW(&dialog)) ? file : L"";
}

void create_editor_controls(HWND window) {
    CreateWindowExW(0, L"STATIC", L"Edit the modeled kingdom inputs. Apply recalculates the dashboard; use File > Save to write the changes.",
        WS_CHILD | WS_VISIBLE, 20, 12, 960, 24, window, nullptr, app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"Kingdom and statistics", WS_CHILD | WS_VISIBLE,
        20, 36, 300, 20, window, nullptr, app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"Laws and benefits", WS_CHILD | WS_VISIBLE,
        345, 36, 300, 20, window, nullptr, app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"Leadership", WS_CHILD | WS_VISIBLE,
        670, 36, 300, 20, window, nullptr, app_instance, nullptr);
    int rows[3] = {0, 0, 0};
    for (size_t index = 0; index < editor_fields.size(); ++index) {
        auto& field = editor_fields[index];
        const int x = 20 + field.column * kColumnWidth;
        const int y = 62 + rows[field.column]++ * 26;
        const int control_id = kFieldIdBase + static_cast<int>(index);
        if (field.kind == FieldKind::Boolean) {
            field.control = CreateWindowExW(0, L"BUTTON", field.label.c_str(),
                WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, x, y, 300, 23, window,
                reinterpret_cast<HMENU>(static_cast<INT_PTR>(control_id)), app_instance, nullptr);
            SendMessageW(field.control, BM_SETCHECK, field.read_bool() ? BST_CHECKED : BST_UNCHECKED, 0);
            continue;
        }
        CreateWindowExW(0, L"STATIC", field.label.c_str(), WS_CHILD | WS_VISIBLE | SS_LEFT,
            x, y + 4, 122, 20, window, nullptr, app_instance, nullptr);
        const int input_x = x + 124;
        if (field.kind == FieldKind::Choice) {
            field.control = CreateWindowExW(0, L"COMBOBOX", L"",
                WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL, input_x, y, 178, 220,
                window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(control_id)), app_instance, nullptr);
            int selection = CB_ERR;
            const int current_value = field.read_choice();
            for (size_t option = 0; option < field.options.size(); ++option) {
                SendMessageW(field.control, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(field.options[option].c_str()));
                if (field.option_values[option] == current_value) selection = static_cast<int>(option);
            }
            SendMessageW(field.control, CB_SETCURSEL, selection, 0);
        } else {
            field.control = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", field.read_text().c_str(),
                WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, input_x, y, 178, 23, window,
                reinterpret_cast<HMENU>(static_cast<INT_PTR>(control_id)), app_instance, nullptr);
        }
    }
    CreateWindowExW(0, L"BUTTON", L"Apply", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        712, 615, 88, 30, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kEditorApply)), app_instance, nullptr);
    CreateWindowExW(0, L"BUTTON", L"Cancel", WS_CHILD | WS_VISIBLE,
        810, 615, 88, 30, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kEditorCancel)), app_instance, nullptr);
    CreateWindowExW(0, L"BUTTON", L"Close", WS_CHILD | WS_VISIBLE,
        908, 615, 88, 30, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kEditorClose)), app_instance, nullptr);
}

void refresh_editor_controls() {
    for (auto& field : editor_fields) {
        if (field.kind == FieldKind::Boolean) {
            SendMessageW(field.control, BM_SETCHECK, field.read_bool() ? BST_CHECKED : BST_UNCHECKED, 0);
        } else if (field.kind == FieldKind::Choice) {
            const int current_value = field.read_choice();
            LRESULT selection = CB_ERR;
            for (size_t index = 0; index < field.option_values.size(); ++index) {
                if (field.option_values[index] == current_value) {
                    selection = static_cast<LRESULT>(index);
                    break;
                }
            }
            SendMessageW(field.control, CB_SETCURSEL, selection, 0);
        } else {
            SetWindowTextW(field.control, field.read_text().c_str());
        }
    }
}

void cancel_editor() {
    edit_draft = kingdom;
    refresh_editor_controls();
}

void apply_editor(HWND window) {
    try {
        for (auto& field : editor_fields) {
            switch (field.kind) {
                case FieldKind::Text:
                case FieldKind::Integer: {
                    const int length = GetWindowTextLengthW(field.control);
                    std::wstring text(static_cast<size_t>(length) + 1, L'\0');
                    GetWindowTextW(field.control, text.data(), length + 1);
                    text.resize(static_cast<size_t>(length));
                    field.write_text(text);
                    break;
                }
                case FieldKind::Boolean:
                    field.write_bool(SendMessageW(field.control, BM_GETCHECK, 0, 0) == BST_CHECKED);
                    break;
                case FieldKind::Choice: {
                    const LRESULT selected = SendMessageW(field.control, CB_GETCURSEL, 0, 0);
                    if (selected == CB_ERR) throw std::invalid_argument("Select a value for every drop-down field.");
                    field.write_choice(field.option_values[static_cast<size_t>(selected)]);
                    break;
                }
            }
        }
        pkkm::validate(edit_draft);
        (void)pkkm::calculate_kingdom_summary(
            edit_draft.unrest, edit_draft.rule_inputs, edit_draft.settlements, &building_catalog);
        kingdom = edit_draft;
        dirty = true;
        edit_draft = kingdom;
        refresh_editor_controls();
        refresh(GetDlgItem(GetParent(window), kDisplay));
    } catch (const std::exception& error) {
        MessageBoxW(window, widen(error.what()).c_str(), L"Invalid kingdom input", MB_OK | MB_ICONWARNING);
    }
}

std::string bundled_catalog_path() {
    wchar_t module_path[32768]{};
    const DWORD length = GetModuleFileNameW(nullptr, module_path,
        static_cast<DWORD>(sizeof(module_path) / sizeof(module_path[0])));
    if (length == 0 || length >= sizeof(module_path) / sizeof(module_path[0]))
        throw std::runtime_error("Could not locate the application directory");
    const auto path = std::filesystem::path(std::wstring(module_path, length)).parent_path()
        / L"building_catalog.json";
    return path.u8string();
}

std::string bundled_greenbelt_path() {
    wchar_t module_path[32768]{};
    const DWORD length = GetModuleFileNameW(nullptr, module_path,
        static_cast<DWORD>(sizeof(module_path) / sizeof(module_path[0])));
    if (length == 0 || length >= sizeof(module_path) / sizeof(module_path[0]))
        throw std::runtime_error("Could not locate the application directory");
    const auto path = std::filesystem::path(std::wstring(module_path, length)).parent_path()
        / L"Kingmaker-Greenbelt-Map.jpg";
    return path.u8string();
}

void manager_label(HWND window, const wchar_t* text, int x, int y, int width = 130) {
    CreateWindowExW(0, L"STATIC", text, WS_CHILD | WS_VISIBLE | SS_LEFT,
        x, y + 4, width, 22, window, nullptr, app_instance, nullptr);
}

void manager_edit(HWND window, int id, const wchar_t* initial, int x, int y, int width = 130) {
    CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", initial,
        WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, x, y, width, 24, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), app_instance, nullptr);
}

void manager_value(HWND window, int id, const wchar_t* initial, int x, int y, int width = 130) {
    CreateWindowExW(0, L"STATIC", initial, WS_CHILD | WS_VISIBLE | SS_LEFT,
        x, y + 4, width, 22, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), app_instance, nullptr);
}

std::wstring manager_text(HWND window, int id) {
    const HWND control = GetDlgItem(window, id);
    const int length = GetWindowTextLengthW(control);
    std::wstring text(static_cast<size_t>(length) + 1, L'\0');
    GetWindowTextW(control, text.data(), length + 1);
    text.resize(static_cast<size_t>(length));
    return text;
}

void set_manager_text(HWND window, int id, const std::wstring& text) {
    SetWindowTextW(GetDlgItem(window, id), text.c_str());
}

void create_settlement_controls(HWND window) {
    CreateWindowExW(0, L"STATIC",
        L"Manage settlements and inventories. Catalog Economy/Loyalty/Stability bonuses are opt-in; saved totals remain unchanged.",
        WS_CHILD | WS_VISIBLE, 18, 10, 1010, 25, window, nullptr, app_instance, nullptr);
    manager_label(window, L"Settlements", 20, 40, 180);
    CreateWindowExW(WS_EX_CLIENTEDGE, L"LISTBOX", L"",
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY, 20, 68, 200, 420, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kSettlementList)), app_instance, nullptr);
    CreateWindowExW(0, L"BUTTON", L"Add Settlement", WS_CHILD | WS_VISIBLE,
        20, 500, 98, 28, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kAddSettlement)), app_instance, nullptr);
    CreateWindowExW(0, L"BUTTON", L"Remove", WS_CHILD | WS_VISIBLE,
        122, 500, 98, 28, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kRemoveSettlement)), app_instance, nullptr);

    manager_label(window, L"Settlement details", 250, 40, 250);
    manager_label(window, L"Name", 250, 68); manager_edit(window, kSettlementName, L"", 380, 68);
    manager_label(window, L"Population override", 250, 100); manager_edit(window, kSettlementPopulation, L"0", 380, 100);
    manager_label(window, L"District override", 250, 132); manager_edit(window, kSettlementDistricts, L"0", 380, 132);
    manager_label(window, L"Saved Economy total", 250, 164); manager_edit(window, kSettlementEconomy, L"0", 380, 164);
    manager_label(window, L"Saved Loyalty total", 250, 196); manager_edit(window, kSettlementLoyalty, L"0", 380, 196);
    manager_label(window, L"Saved Stability total", 250, 228); manager_edit(window, kSettlementStability, L"0", 380, 228);
    manager_label(window, L"Saved Defense total", 250, 260); manager_edit(window, kSettlementDefense, L"0", 380, 260);
    CreateWindowExW(0, L"BUTTON", L"Apply catalog stat bonuses to kingdom totals",
        WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, 250, 294, 280, 24, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kSettlementApplyCatalogStats)), app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"Off by default. Saved totals are never rewritten.",
        WS_CHILD | WS_VISIBLE | SS_LEFT, 250, 322, 280, 36, window, nullptr, app_instance, nullptr);
    CreateWindowExW(0, L"BUTTON", L"Edit Map...", WS_CHILD | WS_VISIBLE | WS_TABSTOP,
        250, 365, 125, 28, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kEditMap)), app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"Map: none", WS_CHILD | WS_VISIBLE | SS_LEFT,
        385, 365, 145, 28, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kMapSummary)), app_instance, nullptr);

    manager_label(window, L"Building inventory", 540, 40, 240);
    CreateWindowExW(WS_EX_CLIENTEDGE, L"LISTBOX", L"",
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY, 540, 68, 480, 325, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kInventoryList)), app_instance, nullptr);
    manager_label(window, L"Catalog improvement", 540, 405, 250);
    manager_label(window, L"Qty", 865, 405, 45);
    CreateWindowExW(0, L"COMBOBOX", L"",
        WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL, 540, 434, 310, 190,
        window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kCatalogChoice)), app_instance, nullptr);
    manager_edit(window, kBuildingCount, L"1", 865, 434, 55);
    CreateWindowExW(0, L"BUTTON", L"Add", WS_CHILD | WS_VISIBLE,
        930, 434, 90, 26, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kAddBuilding)), app_instance, nullptr);
    CreateWindowExW(0, L"BUTTON", L"Remove Selected Building", WS_CHILD | WS_VISIBLE,
        540, 472, 190, 28, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kRemoveBuilding)), app_instance, nullptr);
    CreateWindowExW(0, L"BUTTON", L"Apply", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        732, 575, 88, 30, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kApplySettlements)), app_instance, nullptr);
    CreateWindowExW(0, L"BUTTON", L"Cancel", WS_CHILD | WS_VISIBLE,
        830, 575, 88, 30, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kCancelSettlements)), app_instance, nullptr);
    CreateWindowExW(0, L"BUTTON", L"Close", WS_CHILD | WS_VISIBLE,
        928, 575, 88, 30, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kCloseSettlements)), app_instance, nullptr);
}

void refresh_settlement_list(HWND window, int selection) {
    const HWND list = GetDlgItem(window, kSettlementList);
    SendMessageW(list, LB_RESETCONTENT, 0, 0);
    for (const auto& settlement : settlement_draft) {
        const auto name = widen(settlement.name);
        SendMessageW(list, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(name.c_str()));
    }
    if (selection >= 0 && selection < static_cast<int>(settlement_draft.size()))
        SendMessageW(list, LB_SETCURSEL, selection, 0);
}

const pkkm::BuildingCatalogEntry* catalog_entry(const std::string& name) {
    for (const auto& entry : settlement_catalog) if (entry.name == name) return &entry;
    return nullptr;
}

void refresh_inventory_list(HWND window) {
    const HWND list = GetDlgItem(window, kInventoryList);
    SendMessageW(list, LB_RESETCONTENT, 0, 0);
    if (settlement_active_index < 0 || settlement_active_index >= static_cast<int>(settlement_draft.size())) return;
    const auto& inventory = settlement_draft[static_cast<size_t>(settlement_active_index)].building_inventory;
    for (const auto& building : inventory) {
        std::wstring label = widen(building.name) + L" x " + std::to_wstring(building.count);
        if (const auto* definition = catalog_entry(building.name)) {
            label += L" | " + std::to_wstring(definition->cost) + L" BP | "
                + std::to_wstring(definition->lots) + L" lots each";
        }
        SendMessageW(list, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(label.c_str()));
    }
}

void refresh_settlement_form(HWND window) {
    const bool has_selection = settlement_active_index >= 0
        && settlement_active_index < static_cast<int>(settlement_draft.size());
    const int field_ids[] = {kSettlementName, kSettlementPopulation, kSettlementDistricts,
        kSettlementEconomy, kSettlementLoyalty, kSettlementStability, kSettlementDefense,
        kSettlementApplyCatalogStats, kInventoryList, kCatalogChoice, kBuildingCount, kAddBuilding,
        kRemoveBuilding, kEditMap};
    for (const int id : field_ids) EnableWindow(GetDlgItem(window, id), has_selection);
    if (!has_selection) {
        for (const int id : {kSettlementName, kSettlementPopulation, kSettlementDistricts,
                kSettlementEconomy, kSettlementLoyalty, kSettlementStability, kSettlementDefense})
            set_manager_text(window, id, L"");
        SendMessageW(GetDlgItem(window, kSettlementApplyCatalogStats), BM_SETCHECK, BST_UNCHECKED, 0);
        set_manager_text(window, kMapSummary, L"Map: none");
        refresh_inventory_list(window);
        return;
    }
    const auto& settlement = settlement_draft[static_cast<size_t>(settlement_active_index)];
    set_manager_text(window, kSettlementName, widen(settlement.name));
    set_manager_text(window, kSettlementPopulation, std::to_wstring(settlement.population));
    set_manager_text(window, kSettlementDistricts, std::to_wstring(settlement.districts));
    set_manager_text(window, kSettlementEconomy, std::to_wstring(settlement.economy));
    set_manager_text(window, kSettlementLoyalty, std::to_wstring(settlement.loyalty));
    set_manager_text(window, kSettlementStability, std::to_wstring(settlement.stability));
    set_manager_text(window, kSettlementDefense, std::to_wstring(settlement.defense));
    SendMessageW(GetDlgItem(window, kSettlementApplyCatalogStats), BM_SETCHECK,
        settlement.apply_catalog_stat_effects ? BST_CHECKED : BST_UNCHECKED, 0);
    set_manager_text(window, kMapSummary, settlement.map.rows > 0 && settlement.map.columns > 0
        ? L"Map: " + std::to_wstring(settlement.map.rows) + L"x" + std::to_wstring(settlement.map.columns)
            + L" | B" + std::to_wstring(settlement.map.placements.size())
            + L" L" + std::to_wstring(settlement.map.labels.size())
        : L"Map: none");
    refresh_inventory_list(window);
}

void save_active_settlement(HWND window) {
    if (settlement_active_index < 0 || settlement_active_index >= static_cast<int>(settlement_draft.size())) return;
    auto& settlement = settlement_draft[static_cast<size_t>(settlement_active_index)];
    settlement.name = narrow(manager_text(window, kSettlementName));
    settlement.population = parse_integer(manager_text(window, kSettlementPopulation));
    settlement.districts = parse_integer(manager_text(window, kSettlementDistricts));
    settlement.economy = parse_integer(manager_text(window, kSettlementEconomy));
    settlement.loyalty = parse_integer(manager_text(window, kSettlementLoyalty));
    settlement.stability = parse_integer(manager_text(window, kSettlementStability));
    settlement.defense = parse_integer(manager_text(window, kSettlementDefense));
    settlement.apply_catalog_stat_effects =
        SendMessageW(GetDlgItem(window, kSettlementApplyCatalogStats), BM_GETCHECK, 0, 0) == BST_CHECKED;
    if (settlement.name.empty() || settlement.population < 0 || settlement.districts < 0)
        throw std::invalid_argument("Settlement name must be set; population and districts cannot be negative.");
}

void handle_settlement_selection(HWND window) {
    const int selected = static_cast<int>(SendMessageW(GetDlgItem(window, kSettlementList), LB_GETCURSEL, 0, 0));
    if (selected == settlement_active_index || selected < 0) return;
    const int previous = settlement_active_index;
    try {
        save_active_settlement(window);
        settlement_active_index = selected;
        refresh_settlement_list(window, selected);
        refresh_settlement_form(window);
    } catch (const std::exception& error) {
        SendMessageW(GetDlgItem(window, kSettlementList), LB_SETCURSEL, previous, 0);
        MessageBoxW(window, widen(error.what()).c_str(), L"Invalid settlement", MB_OK | MB_ICONWARNING);
    }
}

void add_settlement(HWND window) {
    save_active_settlement(window);
    pkkm::Settlement settlement;
    settlement.name = "Settlement " + std::to_string(settlement_draft.size() + 1);
    settlement.id = next_settlement_id(settlement_draft);
    settlement_draft.push_back(std::move(settlement));
    settlement_active_index = static_cast<int>(settlement_draft.size()) - 1;
    refresh_settlement_list(window, settlement_active_index);
    refresh_settlement_form(window);
    SetFocus(GetDlgItem(window, kSettlementName));
}

void remove_settlement(HWND window) {
    save_active_settlement(window);
    if (settlement_active_index < 0 || settlement_active_index >= static_cast<int>(settlement_draft.size())) return;
    if (MessageBoxW(window, L"Remove the selected settlement and its inventory from this draft?",
            L"Remove settlement", MB_YESNO | MB_ICONQUESTION) != IDYES) return;
    settlement_draft.erase(settlement_draft.begin() + settlement_active_index);
    settlement_active_index = settlement_draft.empty() ? -1
        : std::min(settlement_active_index, static_cast<int>(settlement_draft.size()) - 1);
    refresh_settlement_list(window, settlement_active_index);
    refresh_settlement_form(window);
}

void add_inventory_building(HWND window) {
    if (settlement_active_index < 0) throw std::invalid_argument("Add or select a settlement first.");
    save_active_settlement(window);
    const int selected = static_cast<int>(SendMessageW(GetDlgItem(window, kCatalogChoice), CB_GETCURSEL, 0, 0));
    if (selected < 0 || selected >= static_cast<int>(settlement_catalog.size()))
        throw std::invalid_argument("Select an improvement from the catalog.");
    const int count = parse_integer(manager_text(window, kBuildingCount));
    if (count < 1) throw std::invalid_argument("Building quantity must be at least 1.");
    const std::string& name = settlement_catalog[static_cast<size_t>(selected)].name;
    auto& inventory = settlement_draft[static_cast<size_t>(settlement_active_index)].building_inventory;
    const auto found = std::find_if(inventory.begin(), inventory.end(), [&name](const pkkm::Building& item) {
        return item.name == name;
    });
    if (found == inventory.end()) inventory.push_back({name, count});
    else {
        if (count > std::numeric_limits<int>::max() - found->count)
            throw std::overflow_error("Building quantity exceeds the supported range.");
        found->count += count;
    }
    set_manager_text(window, kBuildingCount, L"1");
    refresh_inventory_list(window);
}

void remove_inventory_building(HWND window) {
    if (settlement_active_index < 0) return;
    const int selected = static_cast<int>(SendMessageW(GetDlgItem(window, kInventoryList), LB_GETCURSEL, 0, 0));
    auto& inventory = settlement_draft[static_cast<size_t>(settlement_active_index)].building_inventory;
    if (selected < 0 || selected >= static_cast<int>(inventory.size())) return;
    inventory.erase(inventory.begin() + selected);
    refresh_inventory_list(window);
}

void apply_settlement_manager(HWND window) {
    try {
        save_active_settlement(window);
        ensure_settlement_ids(settlement_draft);
        pkkm::Kingdom candidate = kingdom;
        candidate.settlements = settlement_draft;
        pkkm::validate(candidate);
        kingdom.settlements = settlement_draft;
        dirty = true;
        settlement_draft = kingdom.settlements;
        const HWND owner = GetWindow(window, GW_OWNER);
        if (owner) refresh(GetDlgItem(owner, kDisplay));
    } catch (const std::exception& error) {
        MessageBoxW(window, widen(error.what()).c_str(), L"Invalid settlement data", MB_OK | MB_ICONWARNING);
    }
}

void cancel_settlement_manager(HWND window) {
    const int selection = settlement_active_index;
    settlement_draft = kingdom.settlements;
    settlement_active_index = settlement_draft.empty() ? -1
        : std::min(selection, static_cast<int>(settlement_draft.size()) - 1);
    refresh_settlement_list(window, settlement_active_index);
    refresh_settlement_form(window);
}

LRESULT CALLBACK settlement_manager_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
    if (message == WM_GETMINMAXINFO) {
        set_minimum_track_size(lparam, 760, 520);
        return 0;
    }
    switch (message) {
        case WM_CREATE: {
            create_settlement_controls(window);
            const HWND combo = GetDlgItem(window, kCatalogChoice);
            for (const auto& entry : settlement_catalog) {
                const auto label = widen(entry.name) + L" | " + std::to_wstring(entry.cost) + L" BP | "
                    + std::to_wstring(entry.lots) + L" lots";
                SendMessageW(combo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(label.c_str()));
            }
            if (!settlement_catalog.empty()) SendMessageW(combo, CB_SETCURSEL, 0, 0);
            settlement_active_index = settlement_draft.empty() ? -1 : 0;
            refresh_settlement_list(window, settlement_active_index);
            refresh_settlement_form(window);
            return 0;
        }
        case WM_COMMAND: {
            const int id = LOWORD(wparam);
            try {
                if (id == kSettlementList && HIWORD(wparam) == LBN_SELCHANGE) handle_settlement_selection(window);
                else if (id == kAddSettlement) add_settlement(window);
                else if (id == kRemoveSettlement) remove_settlement(window);
                else if (id == kAddBuilding) add_inventory_building(window);
                else if (id == kRemoveBuilding) remove_inventory_building(window);
                else if (id == kEditMap) {
                    save_active_settlement(window);
                    if (settlement_active_index < 0) throw std::invalid_argument("Select a settlement first.");
                    const int index = settlement_active_index;
                    const auto& settlement = settlement_draft[static_cast<size_t>(index)];
                    pkkm::ui::open_map_editor(window, app_instance, settlement.map,
                        settlement.building_inventory, [window, index](pkkm::SettlementMap map) {
                            if (IsWindow(window) && index >= 0 && index < static_cast<int>(settlement_draft.size())) {
                                settlement_draft[static_cast<size_t>(index)].map = std::move(map);
                                refresh_settlement_form(window);
                            }
                        });
                }
                else if (id == kApplySettlements) apply_settlement_manager(window);
                else if (id == kCancelSettlements) cancel_settlement_manager(window);
                else if (id == kCloseSettlements) DestroyWindow(window);
            } catch (const std::exception& error) {
                MessageBoxW(window, widen(error.what()).c_str(), L"Settlement editor", MB_OK | MB_ICONWARNING);
            }
            return 0;
        }
        case WM_CLOSE:
            DestroyWindow(window);
            return 0;
        case WM_DESTROY: {
            pkkm::ui::close_map_editor(window);
            settlement_window = nullptr;
            settlement_draft.clear();
            settlement_catalog.clear();
            settlement_active_index = -1;
            const HWND owner = GetWindow(window, GW_OWNER);
            if (owner && IsWindow(owner)) {
                EnableWindow(owner, TRUE);
                SetForegroundWindow(owner);
            }
            return 0;
        }
    }
    return DefWindowProcW(window, message, wparam, lparam);
}

void command(HWND window, int id) {
    try {
        if (id == kOpen) {
            const auto path = choose_file(window, false);
            if (!path.empty()) {
                kingdom = pkkm::load(narrow(path));
                current_path = path;
                dirty = false;
                refresh(GetDlgItem(window, kDisplay));
            }
        } else if (id == kSave) {
            const auto path = current_path.empty() ? choose_file(window, true) : current_path;
            if (!path.empty()) {
                pkkm::save(kingdom, narrow(path));
                current_path = path;
                dirty = false;
                refresh(GetDlgItem(window, kDisplay));
                MessageBoxW(window, L"Kingdom saved.", L"PKKM", MB_OK | MB_ICONINFORMATION);
            }
        } else if (id == kNew) {
            kingdom = pkkm::Kingdom{};
            current_path.clear();
            dirty = false;
            refresh(GetDlgItem(window, kDisplay));
        } else if (id == kAdvanceTurn) {
            const auto prompt = L"Advance from Turn " + std::to_wstring(kingdom.turn)
                + L" to Turn " + std::to_wstring(kingdom.turn + 1)
                + L"? Previous checklist history will be preserved. No event outcomes or automatic rules will be resolved.";
            if (MessageBoxW(window, prompt.c_str(), L"Advance Turn", MB_YESNO | MB_ICONQUESTION) == IDYES) {
                pkkm::advance_turn(kingdom);
                dirty = true;
                refresh(GetDlgItem(window, kDisplay));
            }
        } else if (id == kEditInputs) {
            if (editor_window && IsWindow(editor_window)) {
                SetForegroundWindow(editor_window);
                return;
            }
            prepare_editor_fields();
            editor_window = CreateWindowExW(WS_EX_DLGMODALFRAME, L"PKKMEditor", L"Edit Kingdom Inputs",
                WS_CAPTION | WS_SYSMENU | WS_POPUP | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, 1025, 700,
                window, nullptr, app_instance, nullptr);
            if (editor_window) {
                EnableWindow(window, FALSE);
                ShowWindow(editor_window, SW_SHOW);
                UpdateWindow(editor_window);
            }
        } else if (id == kManageSettlements) {
            if (settlement_window && IsWindow(settlement_window)) {
                SetForegroundWindow(settlement_window);
                return;
            }
            settlement_draft = kingdom.settlements;
            settlement_catalog = pkkm::search_building_catalog(building_catalog, "");
            settlement_window = CreateWindowExW(WS_EX_DLGMODALFRAME, L"PKKMSettlements", L"Manage Settlements",
                WS_CAPTION | WS_SYSMENU | WS_POPUP | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX, CW_USEDEFAULT, CW_USEDEFAULT, 1075, 660,
                window, nullptr, app_instance, nullptr);
            if (!settlement_window) throw std::runtime_error("Could not open the settlement manager");
            EnableWindow(window, FALSE);
            ShowWindow(settlement_window, SW_SHOW);
            UpdateWindow(settlement_window);
        } else if (id == kTurnChecklist) {
            if (checklist_window && IsWindow(checklist_window)) {
                SetForegroundWindow(checklist_window);
                return;
            }
            checklist_controls.clear();
            checklist_step_ids.clear();
            checklist_window = CreateWindowExW(WS_EX_DLGMODALFRAME, L"PKKMTurnChecklist",
                L"Turn Checklist", WS_CAPTION | WS_SYSMENU | WS_POPUP | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,
                CW_USEDEFAULT, CW_USEDEFAULT, 1010, 620, window, nullptr, app_instance, nullptr);
            if (!checklist_window) throw std::runtime_error("Could not open the turn checklist");
            EnableWindow(window, FALSE);
            ShowWindow(checklist_window, SW_SHOW);
            UpdateWindow(checklist_window);
        } else if (id == kTurnActions) {
            if (action_window && IsWindow(action_window)) {
                SetForegroundWindow(action_window);
                return;
            }
            action_window = CreateWindowExW(WS_EX_DLGMODALFRAME, L"PKKMTurnActions",
                L"Turn Actions", WS_CAPTION | WS_SYSMENU | WS_POPUP | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_VSCROLL,
                                CW_USEDEFAULT, CW_USEDEFAULT, 620, 720, window, nullptr, app_instance, nullptr);
            if (!action_window) throw std::runtime_error("Could not open the Turn Actions manager");
            EnableWindow(window, FALSE);
            ShowWindow(action_window, SW_SHOW);
            UpdateWindow(action_window);
        } else if (id == kCalendarTracker) {
            if (calendar_window && IsWindow(calendar_window)) {
                SetForegroundWindow(calendar_window);
                return;
            }
            calendar_note_draft = kingdom.calendar_notes;
            calendar_active_index = -1;
            calendar_window = CreateWindowExW(WS_EX_DLGMODALFRAME, L"PKKMCalendarTracker",
                L"Calendar Notes", WS_CAPTION | WS_SYSMENU | WS_POPUP | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,
                CW_USEDEFAULT, CW_USEDEFAULT, 1050, 760, window, nullptr, app_instance, nullptr);
            if (!calendar_window) throw std::runtime_error("Could not open the calendar notes tracker");
            EnableWindow(window, FALSE);
            ShowWindow(calendar_window, SW_SHOW);
            UpdateWindow(calendar_window);
        } else if (id == kGreenbeltMap) {
            if (greenbelt_popout_window && IsWindow(greenbelt_popout_window)) {
                SetForegroundWindow(greenbelt_popout_window);
                return;
            }
            ensure_settlement_ids(kingdom.settlements);
            greenbelt_draft = kingdom.greenbelt_map;
            if (greenbelt_draft.asset_path.empty()) {
                greenbelt_draft.asset_path = "Kingmaker-Greenbelt-Map.jpg";
                greenbelt_draft.image_width = 7865;
                greenbelt_draft.image_height = 2610;
            }
            greenbelt_catalog = pkkm::greenbelt_improvement_catalog();
            greenbelt_active_index = greenbelt_draft.hexes.empty() ? -1 : 0;
            greenbelt_special_active_index = greenbelt_draft.special_terrain.empty() ? -1 : 0;
            greenbelt_custom_active_index = greenbelt_draft.custom_improvements.empty() ? -1 : 0;
            greenbelt_scroll_y = 0;
            open_greenbelt_popout(window);
        }
    } catch (const std::exception& error) {
        MessageBoxW(window, widen(error.what()).c_str(), L"PKKM error", MB_OK | MB_ICONERROR);
    }
}

void create_turn_checklist_controls(HWND window) {
    const auto record = std::find_if(kingdom.turn_progress.begin(), kingdom.turn_progress.end(),
        [](const auto& progress) { return progress.turn == kingdom.turn; });
    const std::vector<std::string> completed = record == kingdom.turn_progress.end()
        ? std::vector<std::string>{} : record->completed_step_ids;
    CreateWindowExW(0, L"STATIC",
        L"Manual checklist only; PKKM does not resolve checks or turn outcomes.\r\nApply updates this file in memory; use File > Save to persist. Stability failure by exactly 4 is not used by this rule set.",
        WS_CHILD | WS_VISIBLE | SS_LEFT, 20, 12, 950, 40, window, nullptr, app_instance, nullptr);
    const auto& steps = pkkm::turn_checklist_steps();
    std::vector<std::string> phase_names;
    std::vector<int> phase_counts;
    for (const auto& step : steps) {
        auto phase = std::find(phase_names.begin(), phase_names.end(), step.phase);
        if (phase == phase_names.end()) {
            phase_names.push_back(step.phase);
            phase_counts.push_back(0);
        }
    }
    for (size_t phase_index = 0; phase_index < phase_names.size(); ++phase_index) {
        const int x = phase_index % 2 == 0 ? 24 : 500;
        const int y = phase_index < 2 ? 60 : 300;
        CreateWindowExW(0, L"STATIC", widen(phase_names[phase_index]).c_str(),
            WS_CHILD | WS_VISIBLE | SS_LEFT, x, y, 450, 22, window, nullptr, app_instance, nullptr);
    }
    for (const auto& step : steps) {
        const auto phase = std::find(phase_names.begin(), phase_names.end(), step.phase);
        const size_t phase_index = static_cast<size_t>(phase - phase_names.begin());
        const int x = phase_index % 2 == 0 ? 24 : 500;
        const int y = (phase_index < 2 ? 85 : 325) + phase_counts[phase_index]++ * 26;
        std::wstring label = widen(step.marker) + L": " + widen(step.label);
        const int control_id = kChecklistControlBase + static_cast<int>(checklist_controls.size());
        HWND control = CreateWindowExW(0, L"BUTTON", label.c_str(),
            WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX, x, y, 450, 23, window,
            reinterpret_cast<HMENU>(static_cast<INT_PTR>(control_id)), app_instance, nullptr);
        const bool checked = std::find(completed.begin(), completed.end(), step.id) != completed.end();
        SendMessageW(control, BM_SETCHECK, checked ? BST_CHECKED : BST_UNCHECKED, 0);
        checklist_controls.push_back(control);
        checklist_step_ids.push_back(step.id);
    }
    CreateWindowExW(0, L"BUTTON", L"Event occurred last turn", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        24, 535, 220, 24, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kChecklistEventLastTurn)), app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"", WS_CHILD | WS_VISIBLE | SS_LEFT,
        250, 535, 420, 24, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kChecklistTransitionStatus)), app_instance, nullptr);
    CreateWindowExW(0, L"BUTTON", L"Apply", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        692, 535, 88, 30, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kChecklistApply)), app_instance, nullptr);
    CreateWindowExW(0, L"BUTTON", L"Cancel", WS_CHILD | WS_VISIBLE,
        790, 535, 88, 30, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kChecklistCancel)), app_instance, nullptr);
    CreateWindowExW(0, L"BUTTON", L"Close", WS_CHILD | WS_VISIBLE,
        888, 535, 88, 30, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kChecklistClose)), app_instance, nullptr);
}

void refresh_turn_transition_preview(HWND window) {
    const bool event_occurred_last_turn = SendMessageW(
        GetDlgItem(window, kChecklistEventLastTurn), BM_GETCHECK, 0, 0) == BST_CHECKED;
    const auto preview = pkkm::preview_turn_transition(kingdom, event_occurred_last_turn);
    const auto status = L"Next turn: " + std::to_wstring(preview.next_turn)
        + L" | Event chance: " + std::to_wstring(preview.event_trigger_percent)
        + L"% | Event resolution: manual";
    SetWindowTextW(GetDlgItem(window, kChecklistTransitionStatus), status.c_str());
}

void refresh_turn_checklist_controls() {
    const auto record = std::find_if(kingdom.turn_progress.begin(), kingdom.turn_progress.end(),
        [](const auto& progress) { return progress.turn == kingdom.turn; });
    const std::vector<std::string> completed = record == kingdom.turn_progress.end()
        ? std::vector<std::string>{} : record->completed_step_ids;
    for (size_t index = 0; index < checklist_controls.size(); ++index) {
        const bool checked = std::find(completed.begin(), completed.end(), checklist_step_ids[index])
            != completed.end();
        SendMessageW(checklist_controls[index], BM_SETCHECK, checked ? BST_CHECKED : BST_UNCHECKED, 0);
    }
}

void apply_turn_checklist(HWND window) {
    std::vector<std::string> completed;
    for (size_t index = 0; index < checklist_controls.size(); ++index) {
        if (SendMessageW(checklist_controls[index], BM_GETCHECK, 0, 0) == BST_CHECKED)
            completed.push_back(checklist_step_ids[index]);
    }
    auto record = std::find_if(kingdom.turn_progress.begin(), kingdom.turn_progress.end(),
        [](const auto& progress) { return progress.turn == kingdom.turn; });
    bool changed = false;
    if (completed.empty()) {
        if (record != kingdom.turn_progress.end()) {
            kingdom.turn_progress.erase(record);
            changed = true;
        }
    } else if (record == kingdom.turn_progress.end()) {
        kingdom.turn_progress.push_back({kingdom.turn, std::move(completed)});
        changed = true;
    } else if (record->completed_step_ids != completed) {
        record->completed_step_ids = std::move(completed);
        changed = true;
    }
    if (changed) {
        pkkm::validate(kingdom);
        dirty = true;
        refresh(GetDlgItem(GetParent(window), kDisplay));
    }
}

void create_turn_action_controls(HWND window) {
    CreateWindowExW(0, L"STATIC", L"Current turn actions are explicit only. No event or date automation is performed.",
        WS_CHILD | WS_VISIBLE | SS_LEFT, 20, 18, 390, 24, window, nullptr, app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"Rules", WS_CHILD | WS_VISIBLE | SS_LEFT,
        420, 18, 45, 22, window, nullptr, app_instance, nullptr);
    HWND profile = CreateWindowExW(0, L"COMBOBOX", L"", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
        468, 14, 105, 120, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kRuleProfileChoice)), app_instance, nullptr);
    SendMessageW(profile, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Workbook"));
    SendMessageW(profile, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"City Info"));
    SendMessageW(profile, CB_SETCURSEL, kingdom.rule_profile == pkkm::RuleProfile::CityInfo ? 1 : 0, 0);
    CreateWindowExW(0, L"STATIC", L"Action", WS_CHILD | WS_VISIBLE | SS_LEFT,
        20, 62, 120, 22, window, nullptr, app_instance, nullptr);
    HWND choice = CreateWindowExW(0, L"COMBOBOX", L"", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
        150, 58, 270, 130, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kActionChoice)), app_instance, nullptr);
    for (const wchar_t* label : {L"Claim Hex (-1 BP, +1 kingdom size)",
                                 L"Abandon Hex (-1 kingdom size, +1 unrest)",
                                 L"Abandon City (-1 kingdom size, +4 unrest)"})
        SendMessageW(choice, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(label));
    SendMessageW(choice, CB_SETCURSEL, 0, 0);
    CreateWindowExW(0, L"STATIC", L"Count", WS_CHILD | WS_VISIBLE | SS_LEFT,
        20, 104, 120, 22, window, nullptr, app_instance, nullptr);
    CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"1", WS_CHILD | WS_VISIBLE | ES_NUMBER,
        150, 100, 100, 24, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kActionCount)), app_instance, nullptr);
    CreateWindowExW(0, L"BUTTON", L"Apply Action", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        120, 150, 125, 30, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kActionApply)), app_instance, nullptr);
    CreateWindowExW(0, L"BUTTON", L"Reverse Selected", WS_CHILD | WS_VISIBLE,
        255, 150, 125, 30, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kActionReverse)), app_instance, nullptr);
    CreateWindowExW(0, L"BUTTON", L"Cancel", WS_CHILD | WS_VISIBLE,
        390, 150, 90, 30, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kActionCancel)), app_instance, nullptr);
    CreateWindowExW(0, L"BUTTON", L"Close", WS_CHILD | WS_VISIBLE,
        490, 150, 60, 30, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kActionClose)), app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"Recorded actions", WS_CHILD | WS_VISIBLE | SS_LEFT,
        20, 205, 200, 22, window, nullptr, app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"Filter", WS_CHILD | WS_VISIBLE | SS_LEFT,
        280, 190, 70, 22, window, nullptr, app_instance, nullptr);
    CreateWindowExW(0, L"BUTTON", L"All records", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON | WS_GROUP,
        345, 185, 115, 24, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kActionFilter)), app_instance, nullptr);
    CreateWindowExW(0, L"BUTTON", L"Original actions", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
        345, 211, 140, 24, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kActionFilterOriginal)), app_instance, nullptr);
    CreateWindowExW(0, L"BUTTON", L"Reversal records", WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
        345, 237, 140, 24, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kActionFilterReversal)), app_instance, nullptr);
    SendMessageW(GetDlgItem(window, kActionFilter), BM_SETCHECK, BST_CHECKED, 0);
    CreateWindowExW(WS_EX_CLIENTEDGE, L"LISTBOX", L"",
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOINTEGRALHEIGHT,
        20, 270, 560, 110, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kActionHistory)), app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"Selected action details", WS_CHILD | WS_VISIBLE | SS_LEFT,
        20, 390, 250, 22, window, nullptr, app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"Select a recorded action to inspect its audit details.",
        WS_CHILD | WS_VISIBLE | SS_LEFT,
        20, 415, 560, 35, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kActionDetails)), app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"Taxation (explicit branch; no automatic rounding)",
        WS_CHILD | WS_VISIBLE | SS_LEFT, 20, 465, 360, 22, window, nullptr, app_instance, nullptr);
    CreateWindowExW(0, L"BUTTON", L"Economy check succeeded", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        20, 492, 210, 24, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kTaxSuccess)), app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"Result input (profile divisor)", WS_CHILD | WS_VISIBLE | SS_LEFT,
        215, 492, 75, 22, window, nullptr, app_instance, nullptr);
    CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"0", WS_CHILD | WS_VISIBLE | ES_NUMBER,
        290, 490, 65, 24, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kTaxResult)), app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"Other BP", WS_CHILD | WS_VISIBLE | SS_LEFT,
        365, 492, 65, 22, window, nullptr, app_instance, nullptr);
    CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"0", WS_CHILD | WS_VISIBLE | ES_NUMBER,
        430, 490, 60, 24, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kTaxOther)), app_instance, nullptr);
    CreateWindowExW(0, L"BUTTON", L"Apply Taxes", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        430, 488, 95, 30, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kTaxApply)), app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"No tax action applied this session.", WS_CHILD | WS_VISIBLE | SS_LEFT,
        20, 525, 575, 22, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kTaxStatus)), app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"Tax collection history", WS_CHILD | WS_VISIBLE | SS_LEFT,
        20, 555, 250, 22, window, nullptr, app_instance, nullptr);
    CreateWindowExW(WS_EX_CLIENTEDGE, L"LISTBOX", nullptr,
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOINTEGRALHEIGHT,
        20, 580, 575, 75, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kTaxHistory)), app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"Gold withdrawal (2,000 gp per BP; +1 unrest per BP; failed Loyalty adds BP withdrawn again)",
        WS_CHILD | WS_VISIBLE | SS_LEFT, 20, 665, 450, 22, window, nullptr, app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"BP to withdraw", WS_CHILD | WS_VISIBLE | SS_LEFT,
        20, 695, 110, 22, window, nullptr, app_instance, nullptr);
    CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"1", WS_CHILD | WS_VISIBLE | ES_NUMBER,
        135, 693, 80, 24, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kWithdrawalBp)), app_instance, nullptr);
    CreateWindowExW(0, L"BUTTON", L"Apply Withdrawal", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        230, 690, 145, 30, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kWithdrawalApply)), app_instance, nullptr);
    CreateWindowExW(0, L"BUTTON", L"Loyalty Check succeeded", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        230, 722, 185, 24, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kWithdrawalLoyaltySuccess)), app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"No gold withdrawal applied this session.", WS_CHILD | WS_VISIBLE | SS_LEFT,
        20, 752, 575, 22, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kWithdrawalStatus)), app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"Gold withdrawal history", WS_CHILD | WS_VISIBLE | SS_LEFT,
        20, 782, 250, 22, window, nullptr, app_instance, nullptr);
    CreateWindowExW(WS_EX_CLIENTEDGE, L"LISTBOX", nullptr,
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOINTEGRALHEIGHT,
        20, 807, 575, 55, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kWithdrawalHistory)), app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"Gold/item deposit (profile conversion; exact units only)",
        WS_CHILD | WS_VISIBLE | SS_LEFT, 20, 875, 450, 22, window, nullptr, app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"Gold to deposit", WS_CHILD | WS_VISIBLE | SS_LEFT,
        20, 905, 110, 22, window, nullptr, app_instance, nullptr);
    CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"4000", WS_CHILD | WS_VISIBLE | ES_NUMBER,
        135, 903, 100, 24, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kDepositGold)), app_instance, nullptr);
    CreateWindowExW(0, L"BUTTON", L"Apply Deposit", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        250, 900, 125, 30, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kDepositApply)), app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"No gold deposit applied this session.", WS_CHILD | WS_VISIBLE | SS_LEFT,
        20, 940, 340, 22, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kDepositStatus)), app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"Gold deposit history", WS_CHILD | WS_VISIBLE | SS_LEFT,
        20, 970, 250, 22, window, nullptr, app_instance, nullptr);
    CreateWindowExW(WS_EX_CLIENTEDGE, L"LISTBOX", nullptr,
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOINTEGRALHEIGHT,
        20, 995, 350, 75, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kDepositHistory)), app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"Item base value", WS_CHILD | WS_VISIBLE | SS_LEFT,
        380, 875, 105, 22, window, nullptr, app_instance, nullptr);
    CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"8000", WS_CHILD | WS_VISIBLE | ES_NUMBER,
        485, 873, 75, 24, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kDepositItemValue)), app_instance, nullptr);
    CreateWindowExW(0, L"BUTTON", L"Apply Item", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        380, 905, 115, 30, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kDepositItemApply)), app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"No item deposit applied.", WS_CHILD | WS_VISIBLE | SS_LEFT,
        380, 945, 200, 22, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kDepositItemStatus)), app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"Loyalty Check", WS_CHILD | WS_VISIBLE | SS_LEFT,
        380, 970, 150, 22, window, nullptr, app_instance, nullptr);
    CreateWindowExW(0, L"BUTTON", L"Check succeeded", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        380, 995, 135, 24, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kLoyaltySuccess)), app_instance, nullptr);
    CreateWindowExW(0, L"BUTTON", L"Apply Loyalty", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        380, 1025, 125, 30, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kLoyaltyApply)), app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"No Loyalty Check applied.", WS_CHILD | WS_VISIBLE | SS_LEFT,
        380, 1060, 200, 22, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kLoyaltyStatus)), app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"Stability Check (failure by 4 is not used by this rule set)",
        WS_CHILD | WS_VISIBLE | SS_LEFT, 20, 1085, 560, 22, window, nullptr, app_instance, nullptr);
    CreateWindowExW(0, L"BUTTON", L"Check succeeded", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        20, 1112, 160, 24, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kStabilitySuccess)), app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"Failure by", WS_CHILD | WS_VISIBLE | SS_LEFT,
        195, 1114, 75, 22, window, nullptr, app_instance, nullptr);
    CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"0", WS_CHILD | WS_VISIBLE | ES_NUMBER,
        270, 1111, 65, 24, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kStabilityFailureBy)), app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"Ignored roll input", WS_CHILD | WS_VISIBLE | SS_LEFT,
        350, 1114, 75, 22, window, nullptr, app_instance, nullptr);
    CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"0", WS_CHILD | WS_VISIBLE | ES_NUMBER,
        425, 1111, 65, 24, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kStabilityRoll)), app_instance, nullptr);
    CreateWindowExW(0, L"BUTTON", L"Apply Stability", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        250, 1145, 140, 30, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kStabilityApply)), app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"No Stability Check applied this session.", WS_CHILD | WS_VISIBLE | SS_LEFT,
        20, 1180, 575, 22, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kStabilityStatus)), app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"Stability Check history", WS_CHILD | WS_VISIBLE | SS_LEFT,
        20, 1210, 250, 22, window, nullptr, app_instance, nullptr);
    CreateWindowExW(WS_EX_CLIENTEDGE, L"LISTBOX", nullptr,
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOINTEGRALHEIGHT,
        20, 1235, 575, 75, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kStabilityHistory)), app_instance, nullptr);
    CreateWindowExW(0, L"BUTTON", L"Use optional GM mine rule", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        20, 1310, 220, 24, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kOptionalMineRule)), app_instance, nullptr);
    SendMessageW(GetDlgItem(window, kOptionalMineRule), BM_SETCHECK,
        kingdom.use_optional_mine_rule ? BST_CHECKED : BST_UNCHECKED, 0);
    CreateWindowExW(0, L"STATIC", L"Pay Consumption (amount is caller-supplied)",
        WS_CHILD | WS_VISIBLE | SS_LEFT, 20, 1325, 400, 22, window, nullptr, app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"Consumption BP", WS_CHILD | WS_VISIBLE | SS_LEFT,
        20, 1355, 110, 22, window, nullptr, app_instance, nullptr);
    CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"1", WS_CHILD | WS_VISIBLE | ES_NUMBER,
        135, 1353, 80, 24, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kConsumptionBp)), app_instance, nullptr);
    CreateWindowExW(0, L"BUTTON", L"Apply Consumption", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        230, 1350, 150, 30, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kConsumptionApply)), app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"No consumption action applied this session.", WS_CHILD | WS_VISIBLE | SS_LEFT,
        20, 1385, 575, 22, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kConsumptionStatus)), app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"Consumption history", WS_CHILD | WS_VISIBLE | SS_LEFT,
        20, 1415, 250, 22, window, nullptr, app_instance, nullptr);
    CreateWindowExW(WS_EX_CLIENTEDGE, L"LISTBOX", nullptr,
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOINTEGRALHEIGHT,
        20, 1440, 575, 75, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kConsumptionHistory)), app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"Modify Unrest (caller supplies negative-stat count)",
        WS_CHILD | WS_VISIBLE | SS_LEFT, 20, 1530, 450, 22, window, nullptr, app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"Negative stats", WS_CHILD | WS_VISIBLE | SS_LEFT,
        20, 1560, 110, 22, window, nullptr, app_instance, nullptr);
    CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"0", WS_CHILD | WS_VISIBLE | ES_NUMBER,
        135, 1558, 80, 24, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kUnrestNegativeStats)), app_instance, nullptr);
    CreateWindowExW(0, L"BUTTON", L"Apply Unrest", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        230, 1555, 125, 30, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kUnrestApply)), app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"No unrest modification applied this session.", WS_CHILD | WS_VISIBLE | SS_LEFT,
        20, 1590, 575, 22, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kUnrestStatus)), app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"Unrest modification history", WS_CHILD | WS_VISIBLE | SS_LEFT,
        20, 1620, 250, 22, window, nullptr, app_instance, nullptr);
    CreateWindowExW(WS_EX_CLIENTEDGE, L"LISTBOX", nullptr,
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOINTEGRALHEIGHT,
        20, 1645, 575, 75, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kUnrestHistory)), app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"Commissioned item (manual record; distribution remains manual)",
        WS_CHILD | WS_VISIBLE | SS_LEFT, 20, 1740, 520, 22, window, nullptr, app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"Item name", WS_CHILD | WS_VISIBLE | SS_LEFT,
        20, 1770, 80, 22, window, nullptr, app_instance, nullptr);
    CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
        105, 1768, 180, 24, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kCommissionItem)), app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"Recipient", WS_CHILD | WS_VISIBLE | SS_LEFT,
        300, 1770, 70, 22, window, nullptr, app_instance, nullptr);
    CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
        375, 1768, 180, 24, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kCommissionRecipient)), app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"Cost BP", WS_CHILD | WS_VISIBLE | SS_LEFT,
        20, 1805, 80, 22, window, nullptr, app_instance, nullptr);
    CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"1", WS_CHILD | WS_VISIBLE | ES_NUMBER,
        105, 1803, 80, 24, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kCommissionCost)), app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"Available exotic crafters", WS_CHILD | WS_VISIBLE | SS_LEFT,
        200, 1805, 155, 22, window, nullptr, app_instance, nullptr);
    CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"1", WS_CHILD | WS_VISIBLE | ES_NUMBER,
        360, 1803, 80, 24, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kCommissionCrafters)), app_instance, nullptr);
    CreateWindowExW(0, L"BUTTON", L"Apply Commission", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        450, 1800, 130, 30, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kCommissionApply)), app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"No commission applied this session.", WS_CHILD | WS_VISIBLE | SS_LEFT,
        20, 1840, 575, 22, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kCommissionStatus)), app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"Commission history", WS_CHILD | WS_VISIBLE | SS_LEFT,
        20, 1870, 250, 22, window, nullptr, app_instance, nullptr);
    CreateWindowExW(WS_EX_CLIENTEDGE, L"LISTBOX", nullptr,
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOINTEGRALHEIGHT,
        20, 1895, 575, 75, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kCommissionHistory)), app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"Magic-item outcome (manual valuation record)",
        WS_CHILD | WS_VISIBLE | SS_LEFT, 20, 1990, 520, 22, window, nullptr, app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"Category", WS_CHILD | WS_VISIBLE | SS_LEFT,
        20, 2020, 70, 22, window, nullptr, app_instance, nullptr);
    HWND category = CreateWindowExW(0, L"COMBOBOX", L"", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
        95, 2017, 100, 120, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kMagicItemCategory)), app_instance, nullptr);
    SendMessageW(category, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Minor"));
    SendMessageW(category, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Medium"));
    SendMessageW(category, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Major"));
    SendMessageW(category, CB_SETCURSEL, 0, 0);
    CreateWindowExW(0, L"BUTTON", L"Sold", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        210, 2017, 75, 24, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kMagicItemSold)), app_instance, nullptr);
    CreateWindowExW(0, L"BUTTON", L"Commissioned", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        295, 2017, 105, 24, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kMagicItemCommissioned)), app_instance, nullptr);
    CreateWindowExW(0, L"BUTTON", L"Apply Item Outcome", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        410, 2014, 170, 30, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kMagicItemApply)), app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"No magic-item outcome applied this session.", WS_CHILD | WS_VISIBLE | SS_LEFT,
        20, 2052, 575, 22, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kMagicItemStatus)), app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"Magic-item outcome history", WS_CHILD | WS_VISIBLE | SS_LEFT,
        20, 2082, 280, 22, window, nullptr, app_instance, nullptr);
    CreateWindowExW(WS_EX_CLIENTEDGE, L"LISTBOX", nullptr,
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOINTEGRALHEIGHT,
        20, 2107, 575, 85, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kMagicItemHistory)), app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"Upkeep preview (non-mutating)",
        WS_CHILD | WS_VISIBLE | SS_LEFT, 20, 2215, 300, 22, window, nullptr, app_instance, nullptr);
    CreateWindowExW(0, L"BUTTON", L"Royal Assassin employed", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        20, 2245, 180, 24, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kRoyalAssassinEmployed)), app_instance, nullptr);
    CreateWindowExW(0, L"BUTTON", L"Preview Upkeep", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        230, 2240, 135, 30, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kUpkeepPreview)), app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"No Upkeep preview generated this session.", WS_CHILD | WS_VISIBLE | SS_LEFT,
        20, 2275, 575, 22, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kUpkeepStatus)), app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"Food resource preview (non-mutating; each Farm reduces Consumption by 2)",
        WS_CHILD | WS_VISIBLE | SS_LEFT, 20, 2325, 560, 22, window, nullptr, app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"Starting Consumption", WS_CHILD | WS_VISIBLE | SS_LEFT,
        20, 2355, 125, 22, window, nullptr, app_instance, nullptr);
    CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"0", WS_CHILD | WS_VISIBLE | ES_NUMBER,
        150, 2353, 70, 24, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kFoodStartingConsumption)), app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"Farms", WS_CHILD | WS_VISIBLE | SS_LEFT,
        235, 2355, 45, 22, window, nullptr, app_instance, nullptr);
    CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"0", WS_CHILD | WS_VISIBLE | ES_NUMBER,
        285, 2353, 60, 24, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kFoodFarmCount)), app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"Extra reduction", WS_CHILD | WS_VISIBLE | SS_LEFT,
        360, 2355, 95, 22, window, nullptr, app_instance, nullptr);
    CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"0", WS_CHILD | WS_VISIBLE | ES_NUMBER,
        460, 2353, 60, 24, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kFoodExtraReduction)), app_instance, nullptr);
    CreateWindowExW(0, L"BUTTON", L"Preview Food", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        20, 2388, 125, 30, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kFoodPreview)), app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"No food-resource preview generated this session.",
        WS_CHILD | WS_VISIBLE | SS_LEFT, 160, 2392, 420, 22, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kFoodStatus)), app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"Quarry income preview (non-mutating; +1 BP per Quarry)",
        WS_CHILD | WS_VISIBLE | SS_LEFT, 20, 2435, 500, 22, window, nullptr, app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"Quarries", WS_CHILD | WS_VISIBLE | SS_LEFT,
        20, 2465, 65, 22, window, nullptr, app_instance, nullptr);
    CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"0", WS_CHILD | WS_VISIBLE | ES_NUMBER,
        90, 2463, 70, 24, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kQuarryCount)), app_instance, nullptr);
    CreateWindowExW(0, L"BUTTON", L"Preview Quarry", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        180, 2458, 135, 30, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kQuarryPreview)), app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"No quarry-income preview generated this session.",
        WS_CHILD | WS_VISIBLE | SS_LEFT, 325, 2462, 255, 22, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kQuarryStatus)), app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"Optional GM mine preview (non-mutating; unresolved accounting remains manual)",
        WS_CHILD | WS_VISIBLE | SS_LEFT, 20, 2490, 560, 22, window, nullptr, app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"Starting BP", WS_CHILD | WS_VISIBLE | SS_LEFT,
        20, 2518, 75, 22, window, nullptr, app_instance, nullptr);
    CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"20", WS_CHILD | WS_VISIBLE | ES_NUMBER,
        100, 2516, 70, 24, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kMineStartingValue)), app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"Months elapsed", WS_CHILD | WS_VISIBLE | SS_LEFT,
        190, 2518, 100, 22, window, nullptr, app_instance, nullptr);
    CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"0", WS_CHILD | WS_VISIBLE | ES_NUMBER,
        295, 2516, 70, 24, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kMineMonthsElapsed)), app_instance, nullptr);
    CreateWindowExW(0, L"BUTTON", L"Preview Mine", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        385, 2513, 120, 30, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kMinePreview)), app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"No mine preview generated this session.",
        WS_CHILD | WS_VISIBLE | SS_LEFT, 20, 2550, 560, 22, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kMineStatus)), app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"GM event record (roll/effects remain GM-controlled)",
        WS_CHILD | WS_VISIBLE | SS_LEFT, 20, 2590, 560, 22, window, nullptr, app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"d100 roll", WS_CHILD | WS_VISIBLE | SS_LEFT,
        20, 2620, 70, 22, window, nullptr, app_instance, nullptr);
    CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_NUMBER,
        95, 2618, 60, 24, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kEventRoll)), app_instance, nullptr);
    CreateWindowExW(0, L"BUTTON", L"Automatic roll (opt-in)", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        170, 2618, 160, 24, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kEventAutoRoll)), app_instance, nullptr);
    CreateWindowExW(0, L"BUTTON", L"Resolved", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        340, 2618, 90, 24, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kEventResolved)), app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"Leader", WS_CHILD | WS_VISIBLE | SS_LEFT,
        20, 2655, 55, 22, window, nullptr, app_instance, nullptr);
    CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
        80, 2653, 130, 24, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kEventLeader)), app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"City", WS_CHILD | WS_VISIBLE | SS_LEFT,
        225, 2655, 35, 22, window, nullptr, app_instance, nullptr);
    CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
        265, 2653, 130, 24, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kEventCity)), app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"Block", WS_CHILD | WS_VISIBLE | SS_LEFT,
        410, 2655, 45, 22, window, nullptr, app_instance, nullptr);
    CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
        460, 2653, 120, 24, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kEventBlock)), app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"Hex", WS_CHILD | WS_VISIBLE | SS_LEFT,
        20, 2690, 35, 22, window, nullptr, app_instance, nullptr);
    CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
        60, 2688, 150, 24, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kEventHex)), app_instance, nullptr);
    CreateWindowExW(0, L"BUTTON", L"Preview Event", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        225, 2685, 125, 30, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kEventPreview)), app_instance, nullptr);
    CreateWindowExW(0, L"BUTTON", L"Record Event", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        365, 2685, 125, 30, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kEventApply)), app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"No event record created this session.",
        WS_CHILD | WS_VISIBLE | SS_LEFT, 20, 2725, 560, 38, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kEventStatus)), app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"Event history", WS_CHILD | WS_VISIBLE | SS_LEFT,
        20, 2770, 200, 22, window, nullptr, app_instance, nullptr);
    CreateWindowExW(WS_EX_CLIENTEDGE, L"LISTBOX", L"",
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY | LBS_NOINTEGRALHEIGHT,
        20, 2795, 560, 100, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kEventHistory)), app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"Select an event to inspect its consequence details.",
        WS_CHILD | WS_VISIBLE | SS_LEFT, 20, 2900, 560, 22, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kEventDetails)), app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"Manual consequence record (not applied automatically)",
        WS_CHILD | WS_VISIBLE | SS_LEFT, 20, 2930, 560, 22, window, nullptr, app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"Notes", WS_CHILD | WS_VISIBLE | SS_LEFT,
        20, 2960, 45, 22, window, nullptr, app_instance, nullptr);
    CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
        70, 2958, 510, 24, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kEventConsequenceNotes)), app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"Treasury", WS_CHILD | WS_VISIBLE | SS_LEFT,
        20, 2995, 65, 22, window, nullptr, app_instance, nullptr);
    CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"0", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
        90, 2993, 60, 24, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kEventTreasuryDelta)), app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"Unrest", WS_CHILD | WS_VISIBLE | SS_LEFT,
        170, 2995, 55, 22, window, nullptr, app_instance, nullptr);
    CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"0", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
        230, 2993, 60, 24, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kEventUnrestDelta)), app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"Size", WS_CHILD | WS_VISIBLE | SS_LEFT,
        310, 2995, 35, 22, window, nullptr, app_instance, nullptr);
    CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"0", WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL,
        350, 2993, 60, 24, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kEventSizeDelta)), app_instance, nullptr);
    CreateWindowExW(0, L"BUTTON", L"Hex loss completed", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        20, 3035, 145, 24, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kEventHexLoss)), app_instance, nullptr);
    CreateWindowExW(0, L"BUTTON", L"Anarchy completed", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
        180, 3035, 135, 24, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kEventAnarchy)), app_instance, nullptr);
    CreateWindowExW(0, L"BUTTON", L"Apply selected numeric consequence", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
        330, 3032, 250, 30, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kEventApplyConsequence)), app_instance, nullptr);
}

const wchar_t* turn_action_label(pkkm::TurnAction action) {
    switch (action) {
        case pkkm::TurnAction::ClaimHex: return L"Claim Hex";
        case pkkm::TurnAction::AbandonHex: return L"Abandon Hex";
        case pkkm::TurnAction::AbandonCity: return L"Abandon City";
    }
    return L"Unknown action";
}

void refresh_turn_action_history(HWND window) {
    HWND list = GetDlgItem(window, kActionHistory);
    SendMessageW(list, LB_RESETCONTENT, 0, 0);
    const int filter = turn_action_filter;
    for (std::size_t index = 0; index < kingdom.turn_actions.size(); ++index) {
        const auto& record = kingdom.turn_actions[index];
        if ((filter == 1 && record.reversal) || (filter == 2 && !record.reversal)) continue;
        const auto& delta = record.applied;
        const std::wstring prefix = record.reversal
            ? L"REVERSAL of #" + std::to_wstring(record.reverses_index + 1) + L" - " : L"";
        const auto line = prefix + L"Turn " + std::to_wstring(record.turn) + L": "
            + turn_action_label(record.action) + L" x" + std::to_wstring(record.count)
            + L" | BP " + std::to_wstring(delta.treasury_delta)
            + L" | Size " + std::to_wstring(delta.kingdom_size_delta)
            + L" | Unrest " + std::to_wstring(delta.unrest_delta);
        const auto item = SendMessageW(list, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(line.c_str()));
        SendMessageW(list, LB_SETITEMDATA, item, static_cast<LPARAM>(index));
    }
    SendMessageW(list, LB_SETCURSEL, static_cast<WPARAM>(-1), 0);
    SetWindowTextW(GetDlgItem(window, kActionDetails), L"Select a recorded action to inspect its audit details.");
}

void refresh_turn_action_details(HWND window) {
    HWND list = GetDlgItem(window, kActionHistory);
    const int selected = static_cast<int>(SendMessageW(list, LB_GETCURSEL, 0, 0));
    if (selected < 0) {
        SetWindowTextW(GetDlgItem(window, kActionDetails), L"Select a recorded action to inspect its audit details.");
        return;
    }
    const auto index = static_cast<std::size_t>(SendMessageW(list, LB_GETITEMDATA, selected, 0));
    if (index >= kingdom.turn_actions.size()) return;
    const auto& record = kingdom.turn_actions[index];
    const auto& delta = record.applied;
    std::wstring details = L"Record #" + std::to_wstring(index + 1)
        + L" | Turn " + std::to_wstring(record.turn)
        + L" | Action: " + turn_action_label(record.action)
        + L" | Count: " + std::to_wstring(record.count)
        + L" | BP " + std::to_wstring(delta.treasury_delta)
        + L" | Size " + std::to_wstring(delta.kingdom_size_delta)
        + L" | Unrest " + std::to_wstring(delta.unrest_delta);
    if (record.reversal)
        details += L" | Status: reversal of record #" + std::to_wstring(record.reverses_index + 1);
    else
        details += L" | Status: original action";
    SetWindowTextW(GetDlgItem(window, kActionDetails), details.c_str());
}

void refresh_tax_history(HWND window) {
    HWND list = GetDlgItem(window, kTaxHistory);
    SendMessageW(list, LB_RESETCONTENT, 0, 0);
    for (const auto& record : kingdom.tax_actions) {
        const bool city_info = record.rule_profile == pkkm::RuleProfile::CityInfo;
        const std::wstring profile = city_info ? L"City Info" : L"Workbook";
        const std::wstring input_label = city_info ? L"Economy result/5 " : L"Result/3 ";
        const std::wstring line = L"Turn " + std::to_wstring(record.turn)
            + L" | " + (record.economy_check_success ? L"Economy success" : L"Economy failure")
            + L" | " + input_label + std::to_wstring(record.result_input)
            + L" | Other BP " + std::to_wstring(record.other_bp_gained)
            + L" | Treasury " + std::to_wstring(record.treasury_delta)
            + L" | Profile " + profile;
        SendMessageW(list, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(line.c_str()));
    }
}

void refresh_gold_withdrawal_history(HWND window) {
    HWND list = GetDlgItem(window, kWithdrawalHistory);
    SendMessageW(list, LB_RESETCONTENT, 0, 0);
    for (const auto& record : kingdom.gold_withdrawals) {
        const std::wstring line = L"Turn " + std::to_wstring(record.turn)
            + L" | BP " + std::to_wstring(record.bp_withdrawn)
            + L" | Gold received " + std::to_wstring(record.gold_received)
            + L" | Treasury " + std::to_wstring(record.treasury_delta)
            + L" | Unrest " + std::to_wstring(record.unrest_delta)
            + L" | Loyalty " + (record.loyalty_check_success ? L"success" : L"failure");
        SendMessageW(list, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(line.c_str()));
    }
}

void refresh_gold_deposit_history(HWND window) {
    HWND list = GetDlgItem(window, kDepositHistory);
    SendMessageW(list, LB_RESETCONTENT, 0, 0);
    for (const auto& record : kingdom.gold_deposits) {
        const std::wstring line = L"Turn " + std::to_wstring(record.turn)
            + L" | Gold deposited " + std::to_wstring(record.gold_deposited)
            + L" | BP received " + std::to_wstring(record.bp_received)
            + L" | Treasury " + std::to_wstring(record.treasury_delta);
        SendMessageW(list, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(line.c_str()));
    }
}

void refresh_stability_history(HWND window) {
    HWND list = GetDlgItem(window, kStabilityHistory);
    SendMessageW(list, LB_RESETCONTENT, 0, 0);
    for (const auto& record : kingdom.stability_checks) {
        const std::wstring branch = record.success ? L"Success" : L"Failure by " + std::to_wstring(record.failure_by);
        const std::wstring line = L"Turn " + std::to_wstring(record.turn)
            + L" | " + branch
            + L" | BP " + std::to_wstring(record.treasury_delta)
            + L" | Unrest " + std::to_wstring(record.unrest_delta);
        SendMessageW(list, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(line.c_str()));
    }
}

void refresh_consumption_history(HWND window) {
    HWND list = GetDlgItem(window, kConsumptionHistory);
    SendMessageW(list, LB_RESETCONTENT, 0, 0);
    for (const auto& record : kingdom.consumption_actions) {
        const std::wstring line = L"Turn " + std::to_wstring(record.turn)
            + L" | BP " + std::to_wstring(record.consumption_bp)
            + L" | " + (record.paid ? L"Paid" : L"Insufficient BP")
            + L" | Treasury " + std::to_wstring(record.treasury_delta)
            + L" | Unrest " + std::to_wstring(record.unrest_delta);
        SendMessageW(list, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(line.c_str()));
    }
}

void refresh_unrest_history(HWND window) {
    HWND list = GetDlgItem(window, kUnrestHistory);
    SendMessageW(list, LB_RESETCONTENT, 0, 0);
    for (const auto& record : kingdom.unrest_modifications) {
        const std::wstring line = L"Turn " + std::to_wstring(record.turn)
            + L" | Negative stats " + std::to_wstring(record.negative_stat_count)
            + L" | Unrest " + std::to_wstring(record.unrest_delta);
        SendMessageW(list, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(line.c_str()));
    }
}

void refresh_commission_history(HWND window) {
    HWND list = GetDlgItem(window, kCommissionHistory);
    SendMessageW(list, LB_RESETCONTENT, 0, 0);
    for (const auto& record : kingdom.commissions) {
        const std::wstring line = L"Turn " + std::to_wstring(record.turn)
            + L" | " + widen(record.item_name)
            + L" | Recipient " + widen(record.recipient)
            + L" | Cost " + std::to_wstring(record.cost_bp) + L" BP"
            + L" | Crafters " + std::to_wstring(record.exotic_crafters_available);
        SendMessageW(list, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(line.c_str()));
    }
}

void refresh_magic_item_history(HWND window) {
    HWND list = GetDlgItem(window, kMagicItemHistory);
    SendMessageW(list, LB_RESETCONTENT, 0, 0);
    for (const auto& record : kingdom.magic_item_outcomes) {
        const std::wstring line = L"Turn " + std::to_wstring(record.turn)
            + L" | " + widen(pkkm::magic_item_category_name(record.category))
            + L" | " + (record.commissioned ? L"Commissioned" : record.sold ? L"Sold" : L"Unsold / lost")
            + L" | BP " + std::to_wstring(record.bp_received);
        SendMessageW(list, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(line.c_str()));
    }
}

void refresh_event_history(HWND window) {
    HWND list = GetDlgItem(window, kEventHistory);
    const LRESULT selected_item = SendMessageW(list, LB_GETCURSEL, 0, 0);
    const std::size_t selected_index = selected_item == LB_ERR
        ? std::numeric_limits<std::size_t>::max()
        : static_cast<std::size_t>(SendMessageW(list, LB_GETITEMDATA, selected_item, 0));
    SendMessageW(list, LB_RESETCONTENT, 0, 0);
    for (std::size_t index = 0; index < kingdom.kingdom_events.size(); ++index) {
        const auto& event = kingdom.kingdom_events[index];
        const std::wstring line = L"Turn " + std::to_wstring(event.turn)
            + L" | d100 " + std::to_wstring(event.d100_roll)
            + L" | " + widen(pkkm::kingdom_event_kind_name(event.kind))
            + L" | " + (event.resolved ? L"Resolved" : L"Unresolved")
            + L" | Continuous " + (event.continuous_active ? L"active" : L"inactive")
            + L" | Leader " + widen(event.affected_leader)
            + L" | City " + widen(event.affected_city)
            + L" | Block " + widen(event.affected_block)
            + L" | Hex " + widen(event.affected_hex)
            + L" | BP " + std::to_wstring(event.treasury_delta)
            + L" | Unrest " + std::to_wstring(event.unrest_delta)
            + L" | Size " + std::to_wstring(event.kingdom_size_delta)
            + L" | Hex loss " + (event.hex_loss_confirmed ? L"yes" : L"no")
            + L" | Anarchy " + (event.anarchy_confirmed ? L"yes" : L"no")
            + L" | Applied " + (event.consequences_applied ? L"yes" : L"no");
        const auto item = SendMessageW(list, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(line.c_str()));
        SendMessageW(list, LB_SETITEMDATA, item,
            static_cast<LPARAM>(index));
        if (index == selected_index)
            SendMessageW(list, LB_SETCURSEL, item, 0);
    }
}

void refresh_event_details(HWND window) {
    HWND list = GetDlgItem(window, kEventHistory);
    const int selected = static_cast<int>(SendMessageW(list, LB_GETCURSEL, 0, 0));
    if (selected == LB_ERR) {
        set_manager_text(window, kEventDetails, L"Select an event to inspect its consequence details.");
        return;
    }
    const auto index = static_cast<std::size_t>(SendMessageW(list, LB_GETITEMDATA, selected, 0));
    if (index >= kingdom.kingdom_events.size()) return;
    const auto& event = kingdom.kingdom_events[index];
    set_manager_text(window, kEventDetails,
        L"Selected event " + std::to_wstring(index + 1) + L" | Treasury "
        + std::to_wstring(kingdom.treasury_bp) + L" -> "
        + std::to_wstring(static_cast<long long>(kingdom.treasury_bp) + event.treasury_delta)
        + L" | Unrest " + std::to_wstring(kingdom.unrest) + L" -> "
        + std::to_wstring(static_cast<long long>(kingdom.unrest) + event.unrest_delta)
        + L" | Applied " + (event.consequences_applied ? L"yes" : L"no"));
}

void apply_magic_item_dialog(HWND window) {
    const int selected = static_cast<int>(SendMessageW(
        GetDlgItem(window, kMagicItemCategory), CB_GETCURSEL, 0, 0));
    if (selected < 0 || selected > 2) throw std::invalid_argument("Select a magic-item category.");
    const bool sold = SendMessageW(GetDlgItem(window, kMagicItemSold), BM_GETCHECK, 0, 0) == BST_CHECKED;
    const bool commissioned = SendMessageW(GetDlgItem(window, kMagicItemCommissioned), BM_GETCHECK, 0, 0) == BST_CHECKED;
    pkkm::apply_magic_item_outcome(kingdom, static_cast<pkkm::MagicItemCategory>(selected), sold, commissioned);
    dirty = true;
    refresh(GetDlgItem(GetParent(window), kDisplay));
    const auto& record = kingdom.magic_item_outcomes.back();
    const std::wstring status = L"Item outcome recorded on Turn " + std::to_wstring(record.turn)
        + L" | " + widen(pkkm::magic_item_category_name(record.category))
        + L" | BP received " + std::to_wstring(record.bp_received)
        + (record.lost ? L" | Ordinary item lost" : record.commissioned ? L" | Commissioned exemption" : L" | Sold");
    SetWindowTextW(GetDlgItem(window, kMagicItemStatus), status.c_str());
    refresh_magic_item_history(window);
    SetWindowTextW(window, L"Turn Actions - Magic-Item Outcome Recorded");
}

void apply_commission_dialog(HWND window) {
    const std::string item_name = narrow(manager_text(window, kCommissionItem));
    const std::string recipient = narrow(manager_text(window, kCommissionRecipient));
    const int cost_bp = parse_integer(manager_text(window, kCommissionCost));
    const int exotic_crafters = parse_integer(manager_text(window, kCommissionCrafters));
    pkkm::apply_commission(kingdom, item_name, recipient, cost_bp, exotic_crafters);
    dirty = true;
    refresh(GetDlgItem(GetParent(window), kDisplay));
    const auto& record = kingdom.commissions.back();
    const std::wstring status = L"Commission recorded on Turn " + std::to_wstring(record.turn)
        + L" | " + widen(record.item_name)
        + L" | Recipient " + widen(record.recipient)
        + L" | BP " + std::to_wstring(record.cost_bp)
        + L" | Distribution remains manual";
    SetWindowTextW(GetDlgItem(window, kCommissionStatus), status.c_str());
    refresh_commission_history(window);
    SetWindowTextW(window, L"Turn Actions - Commission Recorded");
}

void reverse_turn_action_dialog(HWND window) {
    HWND list = GetDlgItem(window, kActionHistory);
    const int selected = static_cast<int>(SendMessageW(
        GetDlgItem(window, kActionHistory), LB_GETCURSEL, 0, 0));
    if (selected < 0)
        throw std::invalid_argument("Select a recorded Turn Action to reverse.");
    if (MessageBoxW(window,
            L"Reverse the selected Turn Action? The original record will remain and an inverse audit record will be appended.",
            L"Confirm Turn Action Reversal", MB_YESNO | MB_ICONWARNING) != IDYES)
        return;
    const auto index = static_cast<std::size_t>(SendMessageW(list, LB_GETITEMDATA, selected, 0));
    pkkm::reverse_turn_action(kingdom, index);
    dirty = true;
    refresh(GetDlgItem(GetParent(window), kDisplay));
    refresh_turn_action_history(window);
    SetWindowTextW(window, L"Turn Actions - Reversed");
}

void apply_turn_action_dialog(HWND window) {
    const int selected = static_cast<int>(SendMessageW(GetDlgItem(window, kActionChoice), CB_GETCURSEL, 0, 0));
    if (selected < 0 || selected > 2) throw std::invalid_argument("Select a Turn action.");
    const int count = parse_integer(manager_text(window, kActionCount));
    pkkm::apply_turn_action(kingdom, static_cast<pkkm::TurnAction>(selected), count);
    dirty = true;
    refresh(GetDlgItem(GetParent(window), kDisplay));
    refresh_turn_action_history(window);
    SetWindowTextW(window, L"Turn Actions - Applied");
}

void apply_tax_action_dialog(HWND window) {
    const bool success = SendMessageW(GetDlgItem(window, kTaxSuccess), BM_GETCHECK, 0, 0) == BST_CHECKED;
    const int result_input = parse_integer(manager_text(window, kTaxResult));
    const int other_bp_gained = parse_integer(manager_text(window, kTaxOther));
    pkkm::apply_tax_action(kingdom, success, result_input, other_bp_gained);
    dirty = true;
    refresh(GetDlgItem(GetParent(window), kDisplay));
    const int delta = pkkm::resolve_tax_bp_delta(
        kingdom.rule_profile, success, result_input, other_bp_gained);
    const std::wstring status = L"Tax action applied on Turn " + std::to_wstring(kingdom.turn)
        + L" | BP delta " + std::to_wstring(delta)
        + (success ? L" | Economy branch: success" : L" | Economy branch: failure");
    SetWindowTextW(GetDlgItem(window, kTaxStatus), status.c_str());
    refresh_tax_history(window);
    SetWindowTextW(window, L"Turn Actions - Tax Applied");
}

void apply_gold_withdrawal_dialog(HWND window) {
    const int bp_withdrawn = parse_integer(manager_text(window, kWithdrawalBp));
    const bool loyalty_success = SendMessageW(
        GetDlgItem(window, kWithdrawalLoyaltySuccess), BM_GETCHECK, 0, 0) == BST_CHECKED;
    pkkm::apply_gold_withdrawal(kingdom, bp_withdrawn, loyalty_success);
    dirty = true;
    refresh(GetDlgItem(GetParent(window), kDisplay));
    const auto& record = kingdom.gold_withdrawals.back();
    const std::wstring status = L"Withdrawal applied on Turn " + std::to_wstring(record.turn)
        + L" | Gold received " + std::to_wstring(record.gold_received)
        + L" | Treasury " + std::to_wstring(record.treasury_delta)
        + L" | Unrest " + std::to_wstring(record.unrest_delta)
        + L" | Loyalty " + (record.loyalty_check_success ? L"success" : L"failure");
    SetWindowTextW(GetDlgItem(window, kWithdrawalStatus), status.c_str());
    refresh_gold_withdrawal_history(window);
    SetWindowTextW(window, L"Turn Actions - Withdrawal Applied");
}

void apply_gold_deposit_dialog(HWND window) {
    const int gold_deposited = parse_integer(manager_text(window, kDepositGold));
    pkkm::apply_gold_deposit(kingdom, gold_deposited);
    dirty = true;
    refresh(GetDlgItem(GetParent(window), kDisplay));
    const auto& record = kingdom.gold_deposits.back();
    const std::wstring status = L"Deposit applied on Turn " + std::to_wstring(record.turn)
        + L" | Gold deposited " + std::to_wstring(record.gold_deposited)
        + L" | BP received " + std::to_wstring(record.bp_received)
        + L" | Treasury " + std::to_wstring(record.treasury_delta);
    SetWindowTextW(GetDlgItem(window, kDepositStatus), status.c_str());
    refresh_gold_deposit_history(window);
    SetWindowTextW(window, L"Turn Actions - Deposit Applied");
}

void apply_item_deposit_dialog(HWND window) {
    const int item_value = parse_integer(manager_text(window, kDepositItemValue));
    pkkm::apply_item_deposit(kingdom, item_value);
    dirty = true;
    refresh(GetDlgItem(GetParent(window), kDisplay));
    const auto& record = kingdom.item_deposits.back();
    const std::wstring status = L"Item deposit applied on Turn " + std::to_wstring(record.turn)
        + L" | Base value " + std::to_wstring(record.item_value)
        + L" | BP received " + std::to_wstring(record.bp_received);
    SetWindowTextW(GetDlgItem(window, kDepositItemStatus), status.c_str());
    SetWindowTextW(window, L"Turn Actions - Item Deposit Applied");
}

void apply_loyalty_dialog(HWND window) {
    const bool success = SendMessageW(GetDlgItem(window, kLoyaltySuccess), BM_GETCHECK, 0, 0) == BST_CHECKED;
    pkkm::apply_loyalty_check(kingdom, success);
    dirty = true;
    refresh(GetDlgItem(GetParent(window), kDisplay));
    const auto& record = kingdom.loyalty_checks.back();
    const std::wstring status = L"Loyalty Check applied on Turn " + std::to_wstring(record.turn)
        + L" | Unrest " + std::to_wstring(record.unrest_delta)
        + L" | Permanent Loyalty " + std::to_wstring(record.loyalty_delta);
    SetWindowTextW(GetDlgItem(window, kLoyaltyStatus), status.c_str());
    SetWindowTextW(window, L"Turn Actions - Loyalty Check Applied");
}

void apply_stability_dialog(HWND window) {
    const bool success = SendMessageW(GetDlgItem(window, kStabilitySuccess), BM_GETCHECK, 0, 0) == BST_CHECKED;
    const int failure_by = parse_integer(manager_text(window, kStabilityFailureBy));
    const int rolled_unrest = parse_integer(manager_text(window, kStabilityRoll));
    const auto preview = pkkm::preview_stability_check(
        kingdom.rule_profile, success, kingdom.unrest, failure_by, rolled_unrest);
    pkkm::apply_stability_check(kingdom, success, failure_by, rolled_unrest);
    dirty = true;
    refresh(GetDlgItem(GetParent(window), kDisplay));
    const auto& record = kingdom.stability_checks.back();
    const std::wstring status = L"Stability Check applied on Turn " + std::to_wstring(record.turn)
        + L" | BP " + std::to_wstring(record.treasury_delta)
        + L" | Unrest " + std::to_wstring(record.unrest_delta)
        + L" | Branch " + (preview.success ? L"success" : L"explicit failure");
    SetWindowTextW(GetDlgItem(window, kStabilityStatus), status.c_str());
    refresh_stability_history(window);
    SetWindowTextW(window, L"Turn Actions - Stability Applied");
}

void apply_consumption_dialog(HWND window) {
    const int consumption_bp = parse_integer(manager_text(window, kConsumptionBp));
    pkkm::apply_consumption_action(kingdom, consumption_bp);
    dirty = true;
    refresh(GetDlgItem(GetParent(window), kDisplay));
    const auto& record = kingdom.consumption_actions.back();
    const std::wstring status = L"Consumption applied on Turn " + std::to_wstring(record.turn)
        + L" | " + (record.paid ? L"Paid" : L"Insufficient BP; +2 unrest")
        + L" | Treasury " + std::to_wstring(record.treasury_delta)
        + L" | Unrest " + std::to_wstring(record.unrest_delta);
    SetWindowTextW(GetDlgItem(window, kConsumptionStatus), status.c_str());
    refresh_consumption_history(window);
    SetWindowTextW(window, L"Turn Actions - Consumption Applied");
}

void preview_upkeep_dialog(HWND window) {
    const bool stability_success = SendMessageW(
        GetDlgItem(window, kStabilitySuccess), BM_GETCHECK, 0, 0) == BST_CHECKED;
    const int failure_by = parse_integer(manager_text(window, kStabilityFailureBy));
    const int rolled_unrest = parse_integer(manager_text(window, kStabilityRoll));
    const int consumption_bp = parse_integer(manager_text(window, kConsumptionBp));
    const bool royal_assassin_employed = SendMessageW(
        GetDlgItem(window, kRoyalAssassinEmployed), BM_GETCHECK, 0, 0) == BST_CHECKED;
    const auto preview = pkkm::preview_upkeep(kingdom, stability_success, consumption_bp,
        royal_assassin_employed, failure_by, rolled_unrest);
    std::wstring status;
    if (preview.skipped_for_no_hexes) {
        status = L"Upkeep skipped: kingdom controls no hexes";
    } else {
        status = L"Upkeep preview | Treasury " + std::to_wstring(preview.projected_treasury)
            + L" | Unrest " + std::to_wstring(preview.projected_unrest)
            + L" | Consumption " + (preview.consumption_paid ? L"paid" : L"insufficient")
            + L" | Negative Treasury threat: "
            + (preview.negative_treasury_threat ? L"yes; +2 Unrest at phase end" : L"no")
            + L" | Threshold threat before Royal Assassin: hex "
            + (preview.threshold_threat_before_royal_assassin ? L"yes" : L"no")
            + L", anarchy "
            + (preview.anarchy_threat_before_royal_assassin ? L"yes" : L"no")
            + L" | Royal Assassin "
            + (preview.royal_assassin_unrest_delta ? L"-1 Unrest at phase end" : L"inactive")
            + L" | Lose hex: " + (preview.loses_hex ? L"yes" : L"no")
            + L" | Anarchy: " + (preview.anarchy ? L"yes" : L"no");
    }
    SetWindowTextW(GetDlgItem(window, kUpkeepStatus), status.c_str());
    const std::wstring title = L"Turn Actions - Upkeep Preview | " + status;
    SetWindowTextW(window, title.c_str());
}

void preview_food_resource_dialog(HWND window) {
    const int starting_consumption = parse_integer(manager_text(window, kFoodStartingConsumption));
    const int farm_count = parse_integer(manager_text(window, kFoodFarmCount));
    const int extra_reduction = parse_integer(manager_text(window, kFoodExtraReduction));
    const auto preview = pkkm::preview_food_resource(
        starting_consumption, farm_count, extra_reduction);
    const std::wstring status = L"Food preview | Effective Consumption "
        + std::to_wstring(preview.effective_consumption)
        + L" | Surplus BP " + std::to_wstring(preview.surplus_bp);
    SetWindowTextW(GetDlgItem(window, kFoodStatus), status.c_str());
    SetWindowTextW(window, L"Turn Actions - Food Resource Preview");
}

void preview_quarry_income_dialog(HWND window) {
    const int quarry_count = parse_integer(manager_text(window, kQuarryCount));
    const int income = pkkm::preview_quarry_income(quarry_count);
    const std::wstring status = L"Quarry preview | Income +"
        + std::to_wstring(income) + L" BP";
    SetWindowTextW(GetDlgItem(window, kQuarryStatus), status.c_str());
    SetWindowTextW(window, L"Turn Actions - Quarry Income Preview");
}

void preview_optional_mine_dialog(HWND window) {
    const int starting_bp = parse_integer(manager_text(window, kMineStartingValue));
    const int months_elapsed = parse_integer(manager_text(window, kMineMonthsElapsed));
    const auto preview = pkkm::preview_optional_mine_rule(starting_bp, months_elapsed);
    std::wstring status = L"Mine preview | Remaining " + std::to_wstring(preview.remaining_bp)
        + L" BP | Player share " + std::to_wstring(preview.player_share_bp)
        + L" BP | Remainder " + std::to_wstring(preview.unresolved_remainder_bp)
        + L" BP manual | Exhausted " + (preview.exhausted ? L"yes" : L"no")
        + L" | Zero behavior manual";
    SetWindowTextW(GetDlgItem(window, kMineStatus), status.c_str());
    SetWindowTextW(window, (L"Turn Actions - Optional Mine Preview | " + status).c_str());
}

void apply_unrest_dialog(HWND window) {
    const int negative_stat_count = parse_integer(manager_text(window, kUnrestNegativeStats));
    const auto preview = pkkm::preview_unrest_modification(kingdom, negative_stat_count);
    pkkm::apply_unrest_modification(kingdom, negative_stat_count);
    dirty = true;
    refresh(GetDlgItem(GetParent(window), kDisplay));
    const auto& record = kingdom.unrest_modifications.back();
    const std::wstring status = L"Unrest modification applied on Turn " + std::to_wstring(record.turn)
        + L" | Unrest +" + std::to_wstring(record.unrest_delta)
        + L" | Projected unrest " + std::to_wstring(preview.projected_unrest)
        + L" | Lose hex: " + (preview.loses_hex ? L"yes" : L"no")
        + L" | Anarchy: " + (preview.anarchy ? L"yes" : L"no")
        + L" | Consequences remain manual";
    SetWindowTextW(GetDlgItem(window, kUnrestStatus), status.c_str());
    refresh_unrest_history(window);
    SetWindowTextW(window, L"Turn Actions - Unrest Modified");
}

void update_turn_actions_scrollbar(HWND window) {
    RECT client{};
    GetClientRect(window, &client);
    SCROLLINFO info{sizeof(info), SIF_RANGE | SIF_PAGE | SIF_POS};
    info.nMin = 0;
    info.nMax = kTurnActionsContentHeight - 1;
    info.nPage = static_cast<UINT>(std::max(0L, client.bottom - client.top));
    const int max_position = std::max(0, info.nMax - static_cast<int>(info.nPage) + 1);
    const int previous_position = turn_actions_scroll_y;
    turn_actions_scroll_y = std::clamp(turn_actions_scroll_y, 0, max_position);
    if (turn_actions_scroll_y != previous_position) {
        ScrollWindowEx(window, 0, previous_position - turn_actions_scroll_y, nullptr, nullptr,
            nullptr, nullptr, SW_SCROLLCHILDREN | SW_INVALIDATE | SW_ERASE);
    }
    info.nPos = turn_actions_scroll_y;
    SetScrollInfo(window, SB_VERT, &info, TRUE);
}

void scroll_turn_actions_to(HWND window, int requested_position) {
    SCROLLINFO info{sizeof(info), SIF_RANGE | SIF_PAGE | SIF_POS};
    GetScrollInfo(window, SB_VERT, &info);
    const int max_position = std::max(0, info.nMax - static_cast<int>(info.nPage) + 1);
    const int position = std::clamp(requested_position, 0, max_position);
    const int delta = position - turn_actions_scroll_y;
    if (delta == 0) return;
    turn_actions_scroll_y = position;
    SetScrollPos(window, SB_VERT, turn_actions_scroll_y, TRUE);
    ScrollWindowEx(window, 0, -delta, nullptr, nullptr, nullptr, nullptr,
        SW_SCROLLCHILDREN | SW_INVALIDATE | SW_ERASE);
    InvalidateRect(window, nullptr, FALSE);
}

LRESULT CALLBACK turn_actions_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
    if (message == WM_GETMINMAXINFO) {
        set_minimum_track_size(lparam, 560, 460);
        return 0;
    }
    switch (message) {
        case WM_CREATE:
            turn_action_filter = 0;
            turn_actions_scroll_y = 0;
            create_turn_action_controls(window);
            refresh_turn_action_history(window);
            refresh_tax_history(window);
            refresh_gold_withdrawal_history(window);
            refresh_gold_deposit_history(window);
            refresh_stability_history(window);
            refresh_consumption_history(window);
            refresh_unrest_history(window);
            refresh_commission_history(window);
            refresh_magic_item_history(window);
            refresh_event_history(window);
            refresh_event_details(window);
            update_turn_actions_scrollbar(window);
            SetWindowTextW(window, (L"Turn Actions - Turn " + std::to_wstring(kingdom.turn)).c_str());
            return 0;
        case WM_SIZE:
            update_turn_actions_scrollbar(window);
            return 0;
        case WM_VSCROLL: {
            SCROLLINFO info{sizeof(info), SIF_RANGE | SIF_PAGE | SIF_POS | SIF_TRACKPOS};
            GetScrollInfo(window, SB_VERT, &info);
            int position = info.nPos;
            switch (LOWORD(wparam)) {
                case SB_TOP: position = info.nMin; break;
                case SB_BOTTOM: position = info.nMax; break;
                case SB_LINEUP: position -= 40; break;
                case SB_LINEDOWN: position += 40; break;
                case SB_PAGEUP: position -= static_cast<int>(info.nPage); break;
                case SB_PAGEDOWN: position += static_cast<int>(info.nPage); break;
                case SB_THUMBTRACK: position = info.nTrackPos; break;
                default: break;
            }
            scroll_turn_actions_to(window, position);
            return 0;
        }
        case WM_MOUSEWHEEL:
            scroll_turn_actions_to(window, turn_actions_scroll_y
                - static_cast<int>(GET_WHEEL_DELTA_WPARAM(wparam) / WHEEL_DELTA) * 40);
            return 0;
        case WM_COMMAND:
            try {
                if (LOWORD(wparam) == kRuleProfileChoice && HIWORD(wparam) == CBN_SELCHANGE) {
                    const int selected = static_cast<int>(SendMessageW(
                        GetDlgItem(window, kRuleProfileChoice), CB_GETCURSEL, 0, 0));
                    kingdom.rule_profile = selected == 1 ? pkkm::RuleProfile::CityInfo : pkkm::RuleProfile::Workbook;
                    dirty = true;
                    refresh(GetDlgItem(GetParent(window), kDisplay));
                    SetWindowTextW(window, L"Turn Actions - Rule Profile Updated");
                } else if (LOWORD(wparam) == kActionApply) apply_turn_action_dialog(window);
                else if (LOWORD(wparam) == kActionReverse) reverse_turn_action_dialog(window);
                else if (LOWORD(wparam) == kTaxApply) apply_tax_action_dialog(window);
                else if (LOWORD(wparam) == kWithdrawalApply) apply_gold_withdrawal_dialog(window);
                else if (LOWORD(wparam) == kDepositApply) apply_gold_deposit_dialog(window);
                else if (LOWORD(wparam) == kDepositItemApply) apply_item_deposit_dialog(window);
                else if (LOWORD(wparam) == kLoyaltyApply) apply_loyalty_dialog(window);
                else if (LOWORD(wparam) == kStabilityApply) apply_stability_dialog(window);
                else if (LOWORD(wparam) == kOptionalMineRule && HIWORD(wparam) == BN_CLICKED) {
                    kingdom.use_optional_mine_rule = !kingdom.use_optional_mine_rule;
                    SendMessageW(GetDlgItem(window, kOptionalMineRule), BM_SETCHECK,
                        kingdom.use_optional_mine_rule ? BST_CHECKED : BST_UNCHECKED, 0);
                    dirty = true;
                    SetWindowTextW(window, kingdom.use_optional_mine_rule
                        ? L"Turn Actions - Optional mine rule enabled"
                        : L"Turn Actions - Optional mine rule disabled");
                }
                else if (LOWORD(wparam) == kRoyalAssassinEmployed && HIWORD(wparam) == BN_CLICKED) {
                    const bool employed = SendMessageW(
                        GetDlgItem(window, kRoyalAssassinEmployed), BM_GETCHECK, 0, 0) == BST_CHECKED;
                    SendMessageW(GetDlgItem(window, kRoyalAssassinEmployed), BM_SETCHECK,
                        employed ? BST_CHECKED : BST_UNCHECKED, 0);
                    SetWindowTextW(window, employed
                        ? L"Turn Actions - Royal Assassin enabled"
                        : L"Turn Actions - Royal Assassin disabled");
                }
                else if (LOWORD(wparam) == kConsumptionApply) apply_consumption_dialog(window);
                else if (LOWORD(wparam) == kUpkeepPreview) preview_upkeep_dialog(window);
                else if (LOWORD(wparam) == kFoodPreview) preview_food_resource_dialog(window);
                else if (LOWORD(wparam) == kQuarryPreview) preview_quarry_income_dialog(window);
                else if (LOWORD(wparam) == kMinePreview) preview_optional_mine_dialog(window);
                else if (LOWORD(wparam) == kEventPreview) preview_event_dialog(window);
                else if (LOWORD(wparam) == kEventApply) apply_event_dialog(window);
                else if (LOWORD(wparam) == kEventApplyConsequence) apply_latest_event_consequence_dialog(window);
                else if (LOWORD(wparam) == kUnrestApply) apply_unrest_dialog(window);
                else if (LOWORD(wparam) == kCommissionApply) apply_commission_dialog(window);
                else if (LOWORD(wparam) == kMagicItemApply) apply_magic_item_dialog(window);
                else if (LOWORD(wparam) == kActionFilter ||
                         LOWORD(wparam) == kActionFilterOriginal ||
                         LOWORD(wparam) == kActionFilterReversal) {
                    turn_action_filter = LOWORD(wparam) == kActionFilter ? 0
                        : LOWORD(wparam) == kActionFilterOriginal ? 1 : 2;
                    refresh_turn_action_history(window);
                } else if (LOWORD(wparam) == kEventHistory && HIWORD(wparam) == LBN_SELCHANGE) {
                    refresh_event_details(window);
                } else if (LOWORD(wparam) == kActionHistory && HIWORD(wparam) == LBN_SELCHANGE) {
                    refresh_turn_action_details(window);
                }
                else if (LOWORD(wparam) == kActionCancel) {
                    SendMessageW(GetDlgItem(window, kActionChoice), CB_SETCURSEL, 0, 0);
                    SetWindowTextW(GetDlgItem(window, kActionCount), L"1");
                } else if (LOWORD(wparam) == kActionClose) DestroyWindow(window);
            } catch (const std::exception& error) {
                MessageBoxW(window, widen(error.what()).c_str(), L"Turn Action", MB_OK | MB_ICONWARNING);
            }
            return 0;
        case WM_CLOSE:
            DestroyWindow(window);
            return 0;
        case WM_DESTROY: {
            action_window = nullptr;
            const HWND owner = GetWindow(window, GW_OWNER);
            if (owner && IsWindow(owner)) {
                EnableWindow(owner, TRUE);
                SetForegroundWindow(owner);
            }
            return 0;
        }
    }
    return DefWindowProcW(window, message, wparam, lparam);
}

void refresh_greenbelt_preview(HWND window) {
    const std::wstring profile = widen(pkkm::rule_profile_name(kingdom.rule_profile));
    if (greenbelt_active_index < 0
        || greenbelt_active_index >= static_cast<int>(greenbelt_draft.hexes.size())) {
        set_manager_text(window, kGreenbeltPreview,
            L"Select a hex for an improvement preview. Rule profile: " + profile + L".");
        return;
    }
    const auto& hex = greenbelt_draft.hexes[static_cast<size_t>(greenbelt_active_index)];
    const int selected = static_cast<int>(SendMessageW(
        GetDlgItem(window, kGreenbeltImprovement), CB_GETCURSEL, 0, 0));
    if (selected <= 0 || selected > static_cast<int>(greenbelt_catalog.size())) {
        set_manager_text(window, kGreenbeltPreview,
            L"Select an improvement. Preview only; BP and kingdom totals remain unchanged. Rule profile: "
            + profile + L".");
        return;
    }
    const auto& item = greenbelt_catalog[static_cast<size_t>(selected - 1)];
    pkkm::GreenbeltImprovementPreviewFacts facts;
    facts.terrain = hex.terrain;
    facts.claimed = hex.claimed;
    facts.rule_profile = kingdom.rule_profile;
    const auto checked = [window](int id) {
        const HWND control = GetDlgItem(window, id);
        return control && SendMessageW(control, BM_GETCHECK, 0, 0) == BST_CHECKED;
    };
    const auto integer_or_zero = [window](int id) {
        const HWND control = GetDlgItem(window, id);
        return control ? parse_integer(manager_text(window, id)) : 0;
    };
    facts.water_adjacent = checked(kGreenbeltFactWater);
    facts.river_adjacent = checked(kGreenbeltFactRiver);
    facts.canal_adjacent = checked(kGreenbeltFactCanal);
    facts.settlement_endpoint = checked(kGreenbeltFactEndpoint);
    facts.connected_road = checked(kGreenbeltFactRoad);
    facts.river_crossing = checked(kGreenbeltFactCrossing);
    facts.neighboring_farms = integer_or_zero(kGreenbeltFactFarms);
    facts.kingdom_size = integer_or_zero(kGreenbeltFactSize);
    facts.route_length = integer_or_zero(kGreenbeltFactRoute);
    facts.existing_improvements = hex.improvements;
    facts.requested_upgrade = item.id;
    const auto preview = pkkm::preview_greenbelt_improvement(
        greenbelt_catalog, item.id, facts, false);
    std::wstring text = widen(preview.summary);
    for (const auto& warning : preview.warnings) text += L"\r\nWarning: " + widen(warning);
    set_manager_text(window, kGreenbeltPreview, text);
}

int event_roll_from_dialog(HWND window) {
    const std::wstring entered = manager_text(window, kEventRoll);
    if (!entered.empty()) return parse_integer(entered);
    if (SendMessageW(GetDlgItem(window, kEventAutoRoll), BM_GETCHECK, 0, 0) != BST_CHECKED)
        throw std::invalid_argument("Enter a player d100 roll or enable the opt-in automatic roll.");
    static std::random_device device;
    static std::mt19937 generator(device());
    std::uniform_int_distribution<int> distribution(1, 100);
    const int generated = distribution(generator);
    SetWindowTextW(GetDlgItem(window, kEventRoll), std::to_wstring(generated).c_str());
    return generated;
}

void preview_event_dialog(HWND window) {
    const int roll = event_roll_from_dialog(window);
    const auto& definition = pkkm::kingdom_event_from_d100(roll);
    const bool active = definition.continuous &&
        SendMessageW(GetDlgItem(window, kEventResolved), BM_GETCHECK, 0, 0) != BST_CHECKED;
    std::wstring status = L"Turn " + std::to_wstring(kingdom.turn) + L" | d100 "
        + std::to_wstring(roll) + L" | " + widen(definition.name)
        + (definition.continuous ? L" | Continuous event" : L" | One-time event")
        + (active ? L" | Continuous status active" : L" | Continuous status inactive")
        + L" | No effects applied";
    set_manager_text(window, kEventStatus, status);
}

void apply_event_dialog(HWND window) {
    const int roll = event_roll_from_dialog(window);
    const auto& definition = pkkm::kingdom_event_from_d100(roll);
    const bool resolved = SendMessageW(GetDlgItem(window, kEventResolved), BM_GETCHECK, 0, 0) == BST_CHECKED;
    const bool active = definition.continuous && !resolved;
    pkkm::KingdomEventRecord record;
    record.turn = kingdom.turn;
    record.d100_roll = roll;
    record.kind = definition.kind;
    record.resolved = resolved;
    record.continuous_active = active;
    record.affected_leader = narrow(manager_text(window, kEventLeader));
    record.affected_city = narrow(manager_text(window, kEventCity));
    record.affected_block = narrow(manager_text(window, kEventBlock));
    record.affected_hex = narrow(manager_text(window, kEventHex));
    record.consequence_notes = narrow(manager_text(window, kEventConsequenceNotes));
    record.treasury_delta = parse_integer(manager_text(window, kEventTreasuryDelta));
    record.unrest_delta = parse_integer(manager_text(window, kEventUnrestDelta));
    record.kingdom_size_delta = parse_integer(manager_text(window, kEventSizeDelta));
    record.hex_loss_confirmed = SendMessageW(GetDlgItem(window, kEventHexLoss), BM_GETCHECK, 0, 0) == BST_CHECKED;
    record.anarchy_confirmed = SendMessageW(GetDlgItem(window, kEventAnarchy), BM_GETCHECK, 0, 0) == BST_CHECKED;
    kingdom.kingdom_events.push_back(std::move(record));
    dirty = true;
    set_manager_text(window, kEventStatus,
        L"Recorded turn " + std::to_wstring(kingdom.turn) + L" | d100 " + std::to_wstring(roll)
        + L" | " + widen(definition.name) + L" | GM selections preserved | No effects applied");
    refresh_event_history(window);
    SetWindowTextW(window, L"Turn Actions - Event Recorded");
}

void apply_latest_event_consequence_dialog(HWND window) {
    HWND list = GetDlgItem(window, kEventHistory);
    const int selected = static_cast<int>(SendMessageW(list, LB_GETCURSEL, 0, 0));
    if (selected == LB_ERR)
        throw std::invalid_argument("Select an event history record before applying a manual consequence.");
    const auto index = static_cast<std::size_t>(SendMessageW(list, LB_GETITEMDATA, selected, 0));
    if (index >= kingdom.kingdom_events.size())
        throw std::invalid_argument("Selected event history record is no longer available.");
    const auto& pending = kingdom.kingdom_events[index];
    const std::wstring prompt = L"Apply Treasury delta " + std::to_wstring(pending.treasury_delta)
        + L" and Unrest delta " + std::to_wstring(pending.unrest_delta)
        + L" to the kingdom? Kingdom Size, hex loss, and Anarchy remain manual.";
    if (MessageBoxW(window, prompt.c_str(), L"Confirm Event Consequence", MB_YESNO | MB_ICONWARNING) != IDYES)
        return;
    pkkm::apply_manual_event_consequence(kingdom, index);
    dirty = true;
    refresh(GetDlgItem(GetParent(window), kDisplay));
    refresh_event_history(window);
    refresh_event_details(window);
    const auto& event = kingdom.kingdom_events[index];
    set_manager_text(window, kEventStatus,
        L"Applied selected numeric consequence | Treasury delta " + std::to_wstring(event.treasury_delta)
        + L" | Unrest delta " + std::to_wstring(event.unrest_delta)
        + L" | Kingdom Size/hex/anarchy decisions remain GM-controlled");
    SetWindowTextW(window, L"Turn Actions - Event Consequence Applied");
}

void load_greenbelt_form(HWND window) {
    const bool has_selection = greenbelt_active_index >= 0
        && greenbelt_active_index < static_cast<int>(greenbelt_draft.hexes.size());
    for (const int id : {kGreenbeltQ, kGreenbeltR, kGreenbeltTerrain, kGreenbeltTerrainNotes, kGreenbeltClaimed,
            kGreenbeltClaimTurn, kGreenbeltSettlement, kGreenbeltImprovement,
            kGreenbeltAddImprovement, kGreenbeltFactWater, kGreenbeltFactRiver, kGreenbeltFactCanal,
            kGreenbeltFactEndpoint, kGreenbeltFactRoad, kGreenbeltFactCrossing, kGreenbeltFactFarms,
            kGreenbeltFactSize, kGreenbeltFactRoute, kGreenbeltRefreshPreview})
        EnableWindow(GetDlgItem(window, id), has_selection);
    if (!has_selection) {
        for (const int id : {kGreenbeltQ, kGreenbeltR, kGreenbeltTerrain, kGreenbeltTerrainNotes, kGreenbeltClaimTurn})
            set_manager_text(window, id, L"");
        set_manager_text(window, kGreenbeltAutodetected,
            L"Auto-detection unavailable until a map hex is selected; output is advisory and editable.");
        SendMessageW(GetDlgItem(window, kGreenbeltClaimed), BM_SETCHECK, BST_UNCHECKED, 0);
        SendMessageW(GetDlgItem(window, kGreenbeltSettlement), CB_SETCURSEL, 0, 0);
        refresh_greenbelt_preview(window);
        return;
    }
    const auto& hex = greenbelt_draft.hexes[static_cast<size_t>(greenbelt_active_index)];
    set_manager_text(window, kGreenbeltQ, std::to_wstring(hex.q));
    set_manager_text(window, kGreenbeltR, std::to_wstring(hex.r));
    set_manager_text(window, kGreenbeltTerrain, widen(hex.terrain));
    set_manager_text(window, kGreenbeltTerrainNotes, widen(hex.terrain_notes));
    set_manager_text(window, kGreenbeltClaimTurn, std::to_wstring(hex.claim_turn));
    SendMessageW(GetDlgItem(window, kGreenbeltClaimed), BM_SETCHECK,
        hex.claimed ? BST_CHECKED : BST_UNCHECKED, 0);
    int settlement_selection = 0;
    const HWND settlement_combo = GetDlgItem(window, kGreenbeltSettlement);
    const int settlement_count = static_cast<int>(SendMessageW(
        settlement_combo, CB_GETCOUNT, 0, 0));
    for (int item = 0; item < settlement_count; ++item) {
        const LRESULT item_data = SendMessageW(settlement_combo, CB_GETITEMDATA, item, 0);
        if (item_data > 0 && item_data <= static_cast<LRESULT>(kingdom.settlements.size())
            && kingdom.settlements[static_cast<size_t>(item_data - 1)].id == hex.settlement_id) {
            settlement_selection = item;
            break;
        }
    }
    SendMessageW(GetDlgItem(window, kGreenbeltSettlement), CB_SETCURSEL, settlement_selection, 0);
    refresh_greenbelt_preview(window);
}

void refresh_greenbelt_hex_list(HWND window) {
    const HWND list = GetDlgItem(window, kGreenbeltHexList);
    if (list) {
        SendMessageW(list, LB_RESETCONTENT, 0, 0);
        for (const auto& hex : greenbelt_draft.hexes) {
            std::wstring label = L"(" + std::to_wstring(hex.q) + L", " + std::to_wstring(hex.r) + L") "
                + widen(hex.terrain) + (hex.claimed ? L" | claimed" : L" | unclaimed")
                + L" | I" + std::to_wstring(hex.improvements.size());
            if (!hex.settlement_id.empty()) {
                const auto found = std::find_if(kingdom.settlements.begin(), kingdom.settlements.end(),
                    [&hex](const auto& settlement) { return settlement.id == hex.settlement_id; });
                if (found != kingdom.settlements.end())
                    label += L" | settlement: " + widen(found->name);
            }
            SendMessageW(list, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(label.c_str()));
        }
        if (greenbelt_active_index >= 0
            && greenbelt_active_index < static_cast<int>(greenbelt_draft.hexes.size()))
            SendMessageW(list, LB_SETCURSEL, greenbelt_active_index, 0);
    }
    load_greenbelt_form(window);
}

void save_greenbelt_form(HWND window) {
    if (greenbelt_active_index < 0
        || greenbelt_active_index >= static_cast<int>(greenbelt_draft.hexes.size())) return;
    auto& hex = greenbelt_draft.hexes[static_cast<size_t>(greenbelt_active_index)];
    hex.terrain = narrow(manager_text(window, kGreenbeltTerrain));
    hex.terrain_notes = narrow(manager_text(window, kGreenbeltTerrainNotes));
    hex.claimed = SendMessageW(GetDlgItem(window, kGreenbeltClaimed), BM_GETCHECK, 0, 0) == BST_CHECKED;
    const std::wstring claim_turn_text = manager_text(window, kGreenbeltClaimTurn);
    hex.claim_turn = claim_turn_text.empty() ? 0 : parse_integer(claim_turn_text);
    const HWND settlement_combo = GetDlgItem(window, kGreenbeltSettlement);
    const int settlement_selection = static_cast<int>(SendMessageW(
        settlement_combo, CB_GETCURSEL, 0, 0));
    const LRESULT settlement_item = settlement_selection < 0 ? CB_ERR
        : SendMessageW(settlement_combo, CB_GETITEMDATA, settlement_selection, 0);
    hex.settlement_id = settlement_item <= 0
        || settlement_item > static_cast<LRESULT>(kingdom.settlements.size())
        ? std::string{}
        : kingdom.settlements[static_cast<size_t>(settlement_item - 1)].id;
    if (hex.terrain.empty()) throw std::invalid_argument("Greenbelt terrain must not be empty.");
}

void add_greenbelt_hex(HWND window) {
    if (greenbelt_active_index >= 0
        && greenbelt_active_index < static_cast<int>(greenbelt_draft.hexes.size()))
        save_greenbelt_form(window);
    greenbelt_draft.hexes.push_back(pkkm::make_next_greenbelt_hex(greenbelt_draft.hexes));
    greenbelt_active_index = static_cast<int>(greenbelt_draft.hexes.size()) - 1;
    set_manager_text(window, kGreenbeltAutodetected,
        L"Auto-detection was not run for this manually added hex. Enter or confirm terrain below.");
    refresh_greenbelt_hex_list(window);
}

struct GreenbeltDetectionResult {
    std::string terrain;
    std::wstring note;
};

GreenbeltDetectionResult detect_greenbelt_terrain(
    const std::filesystem::path& image_path, double center_x, double center_y,
    double radius_x, double radius_y) {
    try {
        Gdiplus::Image image(image_path.wstring().c_str());
        if (image.GetLastStatus() != Gdiplus::Ok || image.GetWidth() == 0 || image.GetHeight() == 0)
            return {{}, L"Auto-detection unavailable: the map image could not be loaded."};
        Gdiplus::Bitmap bitmap(image_path.wstring().c_str(), FALSE);
        int water = 0;
        int forest = 0;
        int mountain = 0;
        int samples = 0;
        const int min_x = std::max(0, static_cast<int>(std::floor(center_x - radius_x * 0.55)));
        const int max_x = std::min(static_cast<int>(image.GetWidth()) - 1,
            static_cast<int>(std::ceil(center_x + radius_x * 0.55)));
        const int min_y = std::max(0, static_cast<int>(std::floor(center_y - radius_y * 0.55)));
        const int max_y = std::min(static_cast<int>(image.GetHeight()) - 1,
            static_cast<int>(std::ceil(center_y + radius_y * 0.55)));
        const int step_x = std::max(1, (max_x - min_x) / 12);
        const int step_y = std::max(1, (max_y - min_y) / 12);
        for (int y = min_y; y <= max_y; y += step_y) {
            for (int x = min_x; x <= max_x; x += step_x) {
                Gdiplus::Color color;
                if (bitmap.GetPixel(x, y, &color) != Gdiplus::Ok) continue;
                const int red = color.GetRed();
                const int green = color.GetGreen();
                const int blue = color.GetBlue();
                if (blue > green + 18 && blue > red + 18) ++water;
                if (green > red + 18 && green > blue + 8 && green < 145) ++forest;
                if (std::abs(red - green) < 14 && std::abs(green - blue) < 14 && red < 150) ++mountain;
                ++samples;
            }
        }
        if (samples == 0) return {{}, L"Auto-detection unavailable: the selected map region is empty."};
        const auto fraction = [samples](int count) {
            return static_cast<double>(count) / static_cast<double>(samples);
        };
        if (fraction(water) >= 0.18)
            return {"water", L"Auto-detected: Water (confidence: low; confirm or correct)"};
        if (fraction(water) >= 0.04)
            return {"river", L"Auto-detected: River feature (confidence: low; confirm or correct)"};
        if (fraction(forest) >= 0.28)
            return {"forest", L"Auto-detected: Forest (confidence: low; confirm or correct)"};
        if (fraction(mountain) >= 0.30)
            return {"mountains", L"Auto-detected: Mountains (confidence: low; confirm or correct)"};
        return {{}, L"Auto-detection unavailable: no reliable terrain signature was found."};
    } catch (...) {
        return {{}, L"Auto-detection unavailable: the selected map region could not be read."};
    }
}

std::filesystem::path greenbelt_image_path() {
    std::filesystem::path image_path = std::filesystem::u8path(greenbelt_draft.asset_path);
    if (image_path.empty() || !image_path.is_absolute())
        image_path = std::filesystem::u8path(bundled_greenbelt_path());
    return image_path;
}

void seed_greenbelt_calibration_if_empty() {
    const auto& calibration = greenbelt_draft.calibration;
    if (calibration.hex_width > 0.0 && calibration.hex_height > 0.0) return;
    greenbelt_draft.calibration.origin_x = 132.0;
    greenbelt_draft.calibration.origin_y = 93.0;
    greenbelt_draft.calibration.hex_width = 279.0;
    greenbelt_draft.calibration.hex_height = 322.0;
    greenbelt_draft.calibration.scale = 1.0;
}

void select_greenbelt_hex_at(HWND window, int q, int r) {
    if (greenbelt_active_index >= 0
        && greenbelt_active_index < static_cast<int>(greenbelt_draft.hexes.size()))
        save_greenbelt_form(window);
    const auto found = std::find_if(greenbelt_draft.hexes.begin(), greenbelt_draft.hexes.end(),
        [q, r](const auto& hex) { return hex.q == q && hex.r == r; });
    if (found == greenbelt_draft.hexes.end()) return;
    greenbelt_active_index = static_cast<int>(std::distance(greenbelt_draft.hexes.begin(), found));
    set_manager_text(window, kGreenbeltAutodetected,
        L"Auto-detection is advisory. Existing terrain was preserved; confirm or edit it below.");
    refresh_greenbelt_hex_list(window);
    if (greenbelt_window && window != greenbelt_window && IsWindow(greenbelt_window))
        refresh_greenbelt_hex_list(greenbelt_window);
}

void toggle_greenbelt_hex_at(HWND window, int q, int r, double image_x, double image_y) {
    if (greenbelt_active_index >= 0
        && greenbelt_active_index < static_cast<int>(greenbelt_draft.hexes.size()))
        save_greenbelt_form(window);
    const auto found = std::find_if(greenbelt_draft.hexes.begin(), greenbelt_draft.hexes.end(),
        [q, r](const auto& hex) { return hex.q == q && hex.r == r; });
    if (found != greenbelt_draft.hexes.end()) {
        if (found->claimed || !found->settlement_id.empty()) {
            const int result = MessageBoxW(window,
                L"This hex is claimed or linked to a settlement. Remove it from the Greenbelt map?",
                L"Remove Greenbelt Hex", MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2);
            if (result != IDYES) return;
        }
        const int removed_index = static_cast<int>(std::distance(greenbelt_draft.hexes.begin(), found));
        greenbelt_draft.hexes.erase(found);
        if (greenbelt_draft.hexes.empty()) greenbelt_active_index = -1;
        else greenbelt_active_index = std::min(removed_index, static_cast<int>(greenbelt_draft.hexes.size()) - 1);
        refresh_greenbelt_hex_list(window);
        if (greenbelt_window && window != greenbelt_window && IsWindow(greenbelt_window))
            refresh_greenbelt_hex_list(greenbelt_window);
        return;
    }
    {
        pkkm::GreenbeltHex hex;
        hex.q = q;
        hex.r = r;
        const auto detected = detect_greenbelt_terrain(
            greenbelt_image_path(), image_x, image_y,
            greenbelt_draft.calibration.hex_width * greenbelt_draft.calibration.scale / 1.73205080756887729353,
            greenbelt_draft.calibration.hex_height * greenbelt_draft.calibration.scale * 0.5);
        hex.terrain = detected.terrain.empty() ? "unresolved" : detected.terrain;
        greenbelt_draft.hexes.push_back(std::move(hex));
        greenbelt_active_index = static_cast<int>(greenbelt_draft.hexes.size()) - 1;
        set_manager_text(window, kGreenbeltAutodetected, detected.note);
    }
    refresh_greenbelt_hex_list(window);
    if (greenbelt_window && window != greenbelt_window && IsWindow(greenbelt_window))
        refresh_greenbelt_hex_list(greenbelt_window);
}

void load_greenbelt_special_form(HWND window) {
    const bool selected = greenbelt_special_active_index >= 0
        && greenbelt_special_active_index < static_cast<int>(greenbelt_draft.special_terrain.size());
    for (const int id : {kGreenbeltSpecialQ, kGreenbeltSpecialR, kGreenbeltSpecialType,
            kGreenbeltSpecialName, kGreenbeltSpecialNotes, kGreenbeltSpecialSave,
            kGreenbeltSpecialRemove}) EnableWindow(GetDlgItem(window, id), TRUE);
    if (!selected) {
        set_manager_text(window, kGreenbeltSpecialQ, L"0");
        set_manager_text(window, kGreenbeltSpecialR, L"0");
        set_manager_text(window, kGreenbeltSpecialType, L"river");
        set_manager_text(window, kGreenbeltSpecialName, L"");
        set_manager_text(window, kGreenbeltSpecialNotes, L"");
        EnableWindow(GetDlgItem(window, kGreenbeltSpecialRemove), FALSE);
        return;
    }
    const auto& item = greenbelt_draft.special_terrain[static_cast<size_t>(greenbelt_special_active_index)];
    set_manager_text(window, kGreenbeltSpecialQ, std::to_wstring(item.q));
    set_manager_text(window, kGreenbeltSpecialR, std::to_wstring(item.r));
    set_manager_text(window, kGreenbeltSpecialType, widen(item.type));
    set_manager_text(window, kGreenbeltSpecialName, widen(item.name));
    set_manager_text(window, kGreenbeltSpecialNotes, widen(item.notes));
}

void refresh_greenbelt_special_list(HWND window) {
    const HWND list = GetDlgItem(window, kGreenbeltSpecialList);
    SendMessageW(list, LB_RESETCONTENT, 0, 0);
    for (const auto& item : greenbelt_draft.special_terrain) {
        const auto label = L"(" + std::to_wstring(item.q) + L", " + std::to_wstring(item.r)
            + L") " + widen(item.type) + L": " + widen(item.name);
        SendMessageW(list, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(label.c_str()));
    }
    if (greenbelt_special_active_index >= 0
        && greenbelt_special_active_index < static_cast<int>(greenbelt_draft.special_terrain.size()))
        SendMessageW(list, LB_SETCURSEL, greenbelt_special_active_index, 0);
    load_greenbelt_special_form(window);
}

void save_greenbelt_special_form(HWND window) {
    pkkm::GreenbeltSpecialTerrain item;
    item.q = parse_integer(manager_text(window, kGreenbeltSpecialQ));
    item.r = parse_integer(manager_text(window, kGreenbeltSpecialR));
    item.type = narrow(manager_text(window, kGreenbeltSpecialType));
    item.name = narrow(manager_text(window, kGreenbeltSpecialName));
    item.notes = narrow(manager_text(window, kGreenbeltSpecialNotes));
    if (item.type.empty() || item.name.empty())
        throw std::invalid_argument("Special terrain needs a type and name.");
    if (greenbelt_special_active_index < 0) {
        greenbelt_draft.special_terrain.push_back(std::move(item));
        greenbelt_special_active_index = static_cast<int>(greenbelt_draft.special_terrain.size()) - 1;
    } else {
        greenbelt_draft.special_terrain[static_cast<size_t>(greenbelt_special_active_index)] = std::move(item);
    }
    refresh_greenbelt_special_list(window);
}

void remove_greenbelt_special(HWND window) {
    if (greenbelt_special_active_index < 0) return;
    greenbelt_draft.special_terrain.erase(greenbelt_draft.special_terrain.begin() + greenbelt_special_active_index);
    greenbelt_special_active_index = -1;
    refresh_greenbelt_special_list(window);
}

void load_greenbelt_custom_form(HWND window) {
    const bool selected = greenbelt_custom_active_index >= 0
        && greenbelt_custom_active_index < static_cast<int>(greenbelt_draft.custom_improvements.size());
    if (!selected) {
        set_manager_text(window, kGreenbeltCustomId, L"");
        set_manager_text(window, kGreenbeltCustomName, L"");
        set_manager_text(window, kGreenbeltCustomEffect, L"");
        set_manager_text(window, kGreenbeltCustomCost, L"");
        SendMessageW(GetDlgItem(window, kGreenbeltCustomManual), BM_SETCHECK, BST_CHECKED, 0);
        EnableWindow(GetDlgItem(window, kGreenbeltCustomRemove), FALSE);
        return;
    }
    const auto& item = greenbelt_draft.custom_improvements[static_cast<size_t>(greenbelt_custom_active_index)];
    set_manager_text(window, kGreenbeltCustomId, widen(item.id));
    set_manager_text(window, kGreenbeltCustomName, widen(item.name));
    set_manager_text(window, kGreenbeltCustomEffect, widen(item.effect_preview));
    set_manager_text(window, kGreenbeltCustomCost, widen(item.cost_description));
    SendMessageW(GetDlgItem(window, kGreenbeltCustomManual), BM_SETCHECK,
        item.manual_only ? BST_CHECKED : BST_UNCHECKED, 0);
    EnableWindow(GetDlgItem(window, kGreenbeltCustomRemove), TRUE);
}

void refresh_greenbelt_custom_list(HWND window) {
    const HWND list = GetDlgItem(window, kGreenbeltCustomList);
    SendMessageW(list, LB_RESETCONTENT, 0, 0);
    for (const auto& item : greenbelt_draft.custom_improvements) {
        const auto label = widen(item.id) + L": " + widen(item.name)
            + (item.manual_only ? L" [manual]" : L"");
        SendMessageW(list, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(label.c_str()));
    }
    if (greenbelt_custom_active_index >= 0
        && greenbelt_custom_active_index < static_cast<int>(greenbelt_draft.custom_improvements.size()))
        SendMessageW(list, LB_SETCURSEL, greenbelt_custom_active_index, 0);
    load_greenbelt_custom_form(window);
}

void save_greenbelt_custom_form(HWND window) {
    pkkm::GreenbeltCustomImprovement item;
    item.id = narrow(manager_text(window, kGreenbeltCustomId));
    item.name = narrow(manager_text(window, kGreenbeltCustomName));
    item.effect_preview = narrow(manager_text(window, kGreenbeltCustomEffect));
    item.cost_description = narrow(manager_text(window, kGreenbeltCustomCost));
    if (!item.effect_preview.empty()) item.effects.push_back(item.effect_preview);
    if (!item.cost_description.empty()) item.costs.push_back(item.cost_description);
    item.manual_only = SendMessageW(GetDlgItem(window, kGreenbeltCustomManual), BM_GETCHECK, 0, 0) == BST_CHECKED;
    if (item.id.empty() || item.name.empty())
        throw std::invalid_argument("Custom improvements need an ID and name.");
    if (greenbelt_custom_active_index < 0) {
        greenbelt_draft.custom_improvements.push_back(std::move(item));
        greenbelt_custom_active_index = static_cast<int>(greenbelt_draft.custom_improvements.size()) - 1;
    } else {
        greenbelt_draft.custom_improvements[static_cast<size_t>(greenbelt_custom_active_index)] = std::move(item);
    }
    refresh_greenbelt_custom_list(window);
}

void remove_greenbelt_custom(HWND window) {
    if (greenbelt_custom_active_index < 0) return;
    greenbelt_draft.custom_improvements.erase(
        greenbelt_draft.custom_improvements.begin() + greenbelt_custom_active_index);
    greenbelt_custom_active_index = -1;
    refresh_greenbelt_custom_list(window);
}

void update_greenbelt_scrollbar(HWND window) {
    RECT client{};
    GetClientRect(window, &client);
    SCROLLINFO info{sizeof(info), SIF_RANGE | SIF_PAGE | SIF_POS};
    info.nMin = 0;
    info.nMax = std::max(0, kGreenbeltContentHeight - 1);
    info.nPage = static_cast<UINT>(std::max(0L, client.bottom - client.top));
    const int max_position = std::max(0, info.nMax - static_cast<int>(info.nPage) + 1);
    greenbelt_scroll_y = std::clamp(greenbelt_scroll_y, 0, max_position);
    info.nPos = greenbelt_scroll_y;
    SetScrollInfo(window, SB_VERT, &info, TRUE);
}

void scroll_greenbelt_to(HWND window, int requested_position) {
    SCROLLINFO info{sizeof(info), SIF_RANGE | SIF_PAGE | SIF_POS};
    GetScrollInfo(window, SB_VERT, &info);
    const int max_position = std::max(0, info.nMax - static_cast<int>(info.nPage) + 1);
    const int position = std::clamp(requested_position, 0, max_position);
    const int delta = position - greenbelt_scroll_y;
    if (delta == 0) return;
    greenbelt_scroll_y = position;
    SetScrollPos(window, SB_VERT, greenbelt_scroll_y, TRUE);
    ScrollWindowEx(window, 0, -delta, nullptr, nullptr, nullptr, nullptr,
        SW_SCROLLCHILDREN | SW_INVALIDATE | SW_ERASE);
    InvalidateRect(window, nullptr, FALSE);
}

Gdiplus::Color greenbelt_improvement_color(const std::string& id) {
    if (id == "road" || id == "highway") return Gdiplus::Color(255, 130, 75, 35);
    if (id == "bridge" || id == "canal" || id == "aqueduct") return Gdiplus::Color(255, 30, 110, 210);
    if (id == "fort" || id == "watchtower") return Gdiplus::Color(255, 130, 45, 155);
    if (id == "farm" || id == "fishery") return Gdiplus::Color(255, 30, 130, 55);
    if (id == "mine" || id == "quarry") return Gdiplus::Color(255, 120, 75, 35);
    if (id == "sawmill" || id == "lumber_camp") return Gdiplus::Color(255, 85, 55, 25);
    if (id == "vineyard") return Gdiplus::Color(255, 145, 35, 115);
    return Gdiplus::Color(255, 70, 70, 70);
}

std::wstring greenbelt_improvement_label(const std::string& id) {
    const auto found = std::find_if(greenbelt_catalog.begin(), greenbelt_catalog.end(),
        [&id](const auto& item) { return item.id == id; });
    return found == greenbelt_catalog.end() ? widen(id) : widen(found->name);
}

std::wstring greenbelt_improvement_display_label(const std::string& id) {
    if (id == "road") return L"Road";
    if (id == "highway") return L"Highway";
    if (id == "bridge") return L"Bridge";
    if (id == "canal") return L"Canal";
    if (id == "aqueduct") return L"Aqueduct";
    if (id == "fort") return L"Fort";
    if (id == "watchtower") return L"Watchtower";
    if (id == "farm") return L"Farm";
    if (id == "fishery") return L"Fishery";
    if (id == "mine") return L"Mine";
    if (id == "quarry") return L"Quarry";
    if (id == "sawmill") return L"Sawmill";
    if (id == "lumber_camp") return L"Lumber";
    if (id == "vineyard") return L"Vineyard";
    return greenbelt_improvement_label(id);
}

void draw_greenbelt_improvement_marker(Gdiplus::Graphics& graphics,
    Gdiplus::Pen& pen, const std::string& id, float x, float y, float size) {
    const float half = size * 0.5F;
    if (id == "road" || id == "highway" || id == "bridge" || id == "canal" || id == "aqueduct") {
        graphics.DrawLine(&pen, x - half, y, x + half, y);
        if (id == "highway")
            graphics.DrawLine(&pen, x - half, y + 2.0F, x + half, y + 2.0F);
        if (id == "bridge" || id == "canal" || id == "aqueduct") {
            graphics.DrawLine(&pen, x - half * 0.55F, y - half, x - half * 0.55F, y + half);
            graphics.DrawLine(&pen, x + half * 0.55F, y - half, x + half * 0.55F, y + half);
        }
    } else if (id == "fort") {
        graphics.DrawRectangle(&pen, x - half, y - half, size, size);
    } else if (id == "watchtower") {
        Gdiplus::PointF tower[3] = {
            {x, y - half}, {x - half, y + half}, {x + half, y + half}};
        graphics.DrawPolygon(&pen, tower, 3);
    } else {
        graphics.DrawEllipse(&pen, x - half, y - half, size, size);
    }
}

void draw_greenbelt_settlement_label(Gdiplus::Graphics& graphics,
    const pkkm::GreenbeltCalibration& calibration, const pkkm::GreenbeltHex& hex,
    float left, float top, double image_scale) {
    if (hex.settlement_id.empty())
        return;
    const auto found = std::find_if(kingdom.settlements.begin(), kingdom.settlements.end(),
        [&hex](const auto& settlement) { return settlement.id == hex.settlement_id; });
    if (found == kingdom.settlements.end() || found->name.empty())
        return;
    const auto center = pkkm::greenbelt_hex_center(calibration, hex.q, hex.r);
    const float x = left + static_cast<float>(center.x * image_scale);
    const float y = top + static_cast<float>(center.y * image_scale)
        - static_cast<float>(calibration.hex_height * image_scale) * 0.28F;
    const float width = static_cast<float>(calibration.hex_width * image_scale) * 0.72F;
    const float height = std::max(10.0F, static_cast<float>(calibration.hex_height * image_scale) * 0.12F);
    const float font_size = std::max(5.0F, std::min(10.0F, height * 0.72F));
    Gdiplus::Font font(L"Segoe UI", font_size, Gdiplus::FontStyleBold);
    Gdiplus::StringFormat format;
    format.SetAlignment(Gdiplus::StringAlignmentCenter);
    format.SetLineAlignment(Gdiplus::StringAlignmentCenter);
    format.SetTrimming(Gdiplus::StringTrimmingEllipsisCharacter);
    Gdiplus::SolidBrush shadow(Gdiplus::Color(220, 255, 255, 255));
    Gdiplus::SolidBrush text(Gdiplus::Color(255, 20, 45, 110));
    const Gdiplus::RectF bounds(x - width * 0.5F, y - height * 0.5F, width, height);
    graphics.DrawString(widen(found->name).c_str(), -1, &font,
        Gdiplus::RectF(bounds.X + 1.0F, bounds.Y + 1.0F, bounds.Width, bounds.Height),
        &format, &shadow);
    graphics.DrawString(widen(found->name).c_str(), -1, &font, bounds, &format, &text);
}

void draw_greenbelt_improvements(Gdiplus::Graphics& graphics,
    const pkkm::GreenbeltCalibration& calibration, const pkkm::GreenbeltHex& hex,
    float left, float top, double image_scale) {
    const auto center = pkkm::greenbelt_hex_center(calibration, hex.q, hex.r);
    const Gdiplus::PointF point(left + static_cast<float>(center.x * image_scale),
        top + static_cast<float>(center.y * image_scale));
    if (hex.improvements.empty())
        return;
    const auto vertices = pkkm::greenbelt_hex_vertices(calibration, hex.q, hex.r);
    Gdiplus::GraphicsPath clip_path;
    Gdiplus::PointF polygon[6]{};
    for (size_t index = 0; index < vertices.size(); ++index) {
        polygon[index] = Gdiplus::PointF(
            left + static_cast<float>(vertices[index].x * image_scale),
            top + static_cast<float>(vertices[index].y * image_scale));
    }
    clip_path.AddPolygon(polygon, 6);
    const auto saved_state = graphics.Save();
    graphics.SetClip(&clip_path, Gdiplus::CombineModeIntersect);

    const float cell_width = static_cast<float>(calibration.hex_width * image_scale);
    const float cell_height = static_cast<float>(calibration.hex_height * image_scale);
    const int columns = cell_width >= 105.0F ? 2 : 1;
    const int rows = static_cast<int>((hex.improvements.size() + columns - 1) / columns);
    const float font_size = std::max(5.0F, std::min(9.0F,
        std::min(cell_width / static_cast<float>(columns * 10),
            cell_height / static_cast<float>(std::max(1, rows) * 2.0F))));
    Gdiplus::Font font(L"Segoe UI", font_size, Gdiplus::FontStyleBold);
    Gdiplus::StringFormat centered;
    centered.SetAlignment(Gdiplus::StringAlignmentNear);
    centered.SetLineAlignment(Gdiplus::StringAlignmentCenter);
    const float row_height = std::max(8.0F, font_size + 3.0F);
    const float label_width = std::max(18.0F, cell_width * 0.43F);
    const float total_height = row_height * static_cast<float>(rows);
    const float start_y = point.Y - total_height * 0.5F;
    for (size_t index = 0; index < hex.improvements.size(); ++index) {
        const std::string& id = hex.improvements[index];
        const Gdiplus::Color color = greenbelt_improvement_color(id);
        Gdiplus::SolidBrush brush(color);
        const int column = static_cast<int>(index % static_cast<size_t>(columns));
        const int row = static_cast<int>(index / static_cast<size_t>(columns));
        const float x = point.X + (column == 0 ? -label_width : 0.0F) + 2.0F;
        const float y = start_y + static_cast<float>(row) * row_height;
        Gdiplus::Pen symbol_pen(color, std::max(1.0F, font_size * 0.22F));
        draw_greenbelt_improvement_marker(graphics, symbol_pen, id,
            x + font_size * 0.55F, y + row_height * 0.5F, font_size * 0.8F);
        graphics.DrawString(greenbelt_improvement_display_label(id).c_str(), -1, &font,
            Gdiplus::RectF(x + font_size * 1.2F, y, label_width - font_size * 1.3F,
                row_height), &centered, &brush);
    }
    graphics.Restore(saved_state);
}

void paint_greenbelt_map(HWND window) {
    PAINTSTRUCT paint{};
    HDC device = BeginPaint(window, &paint);
    Gdiplus::Graphics graphics(device);
    const Gdiplus::RectF viewport(
        static_cast<float>(kGreenbeltViewportX),
        static_cast<float>(kGreenbeltViewportY - greenbelt_scroll_y),
        static_cast<float>(kGreenbeltViewportWidth),
        static_cast<float>(kGreenbeltViewportHeight));
    Gdiplus::SolidBrush background(Gdiplus::Color(245, 245, 245));
    graphics.FillRectangle(&background, viewport);
    try {
        const std::filesystem::path image_path = greenbelt_image_path();
        Gdiplus::Image image(image_path.wstring().c_str());
        if (image.GetLastStatus() == Gdiplus::Ok && image.GetWidth() > 0 && image.GetHeight() > 0) {
            const double image_scale = std::min(
                static_cast<double>(viewport.Width) / image.GetWidth(),
                static_cast<double>(viewport.Height) / image.GetHeight());
            const float width = static_cast<float>(image.GetWidth() * image_scale);
            const float height = static_cast<float>(image.GetHeight() * image_scale);
            const float left = viewport.X + (viewport.Width - width) * 0.5F;
            const float top = viewport.Y + (viewport.Height - height) * 0.5F;
            graphics.DrawImage(&image, Gdiplus::RectF(left, top, width, height));
            const auto to_view = [&](const pkkm::GreenbeltPoint& point) {
                return Gdiplus::PointF(left + static_cast<float>(point.x * image_scale),
                    top + static_cast<float>(point.y * image_scale));
            };
            Gdiplus::Pen claimed_pen(Gdiplus::Color(150, 90, 100, 150), 1.0F);
            Gdiplus::Pen settlement_pen(Gdiplus::Color(220, 70, 180, 230), 2.0F);
            Gdiplus::Pen selected_pen(Gdiplus::Color(255, 220, 20), 3.0F);
            Gdiplus::Pen perimeter_pen(Gdiplus::Color(20, 90, 220), 4.0F);
            Gdiplus::Pen grid_pen(Gdiplus::Color(255, 255, 255, 150), 1.0F);
            if (greenbelt_draft.calibration.hex_width > 0.0
                && greenbelt_draft.calibration.hex_height > 0.0) {
                for (int r = -20; r <= 30; ++r) {
                    for (int q = -20; q <= 40; ++q) {
                        if (!pkkm::greenbelt_hex_inside_image(greenbelt_draft.calibration,
                                q, r, static_cast<int>(image.GetWidth()), static_cast<int>(image.GetHeight())))
                            continue;
                        const auto grid_vertices = pkkm::greenbelt_hex_vertices(
                            greenbelt_draft.calibration, q, r);
                        Gdiplus::PointF grid_polygon[6]{};
                        for (size_t vertex = 0; vertex < grid_vertices.size(); ++vertex)
                            grid_polygon[vertex] = to_view(grid_vertices[vertex]);
                        graphics.DrawPolygon(&grid_pen, grid_polygon, 6);
                    }
                }
                const auto anchor = to_view(pkkm::greenbelt_hex_center(
                    greenbelt_draft.calibration, 0, 0));
                Gdiplus::Font anchor_font(L"Segoe UI", 11.0F, Gdiplus::FontStyleBold);
                Gdiplus::SolidBrush anchor_brush(Gdiplus::Color(255, 255, 0));
                graphics.DrawString(L"(0, 0)", -1, &anchor_font,
                    Gdiplus::PointF(anchor.X + 5.0F, anchor.Y + 5.0F), &anchor_brush);
            }
            for (size_t index = 0; index < greenbelt_draft.hexes.size(); ++index) {
                const auto& hex = greenbelt_draft.hexes[index];
                if (!pkkm::greenbelt_hex_inside_image(greenbelt_draft.calibration, hex.q, hex.r,
                        static_cast<int>(image.GetWidth()), static_cast<int>(image.GetHeight())))
                    continue;
                const auto vertices = pkkm::greenbelt_hex_vertices(
                    greenbelt_draft.calibration, hex.q, hex.r);
                Gdiplus::PointF polygon[6]{};
                for (size_t vertex = 0; vertex < vertices.size(); ++vertex)
                    polygon[vertex] = to_view(vertices[vertex]);
                Gdiplus::Pen* hex_pen = nullptr;
                if (index == static_cast<size_t>(greenbelt_active_index))
                    hex_pen = &selected_pen;
                else if (!hex.settlement_id.empty())
                    hex_pen = &settlement_pen;
                else if (hex.claimed)
                    hex_pen = &claimed_pen;
                if (hex_pen) graphics.DrawPolygon(hex_pen, polygon, 6);
                draw_greenbelt_settlement_label(graphics, greenbelt_draft.calibration, hex,
                    left, top, image_scale);
                draw_greenbelt_improvements(graphics, greenbelt_draft.calibration, hex,
                    left, top, image_scale);
            }
            for (const auto& edge : pkkm::greenbelt_claimed_perimeter(greenbelt_draft.hexes)) {
                const auto vertices = pkkm::greenbelt_hex_vertices(
                    greenbelt_draft.calibration, edge.q, edge.r);
                const auto first = to_view(vertices[static_cast<size_t>(edge.side)]);
                const auto second = to_view(vertices[static_cast<size_t>((edge.side + 1) % 6)]);
                graphics.DrawLine(&perimeter_pen, first, second);
            }
        } else {
            Gdiplus::Font font(L"Segoe UI", 10.0F);
            Gdiplus::SolidBrush brush(Gdiplus::Color(40, 40, 40));
            graphics.DrawString(L"Greenbelt image could not be loaded.", -1, &font,
                Gdiplus::PointF(viewport.X + 8.0F, viewport.Y + 8.0F), &brush);
        }
    } catch (...) {
        Gdiplus::Font font(L"Segoe UI", 10.0F);
        Gdiplus::SolidBrush brush(Gdiplus::Color(40, 40, 40));
        graphics.DrawString(L"Greenbelt image path is invalid.", -1, &font,
            Gdiplus::PointF(viewport.X + 8.0F, viewport.Y + 8.0F), &brush);
    }
    EndPaint(window, &paint);
}

void create_greenbelt_controls(HWND window) {
    seed_greenbelt_calibration_if_empty();
    CreateWindowExW(0, L"STATIC",
        L"Greenbelt Map. Claims and improvements are recorded as manual state; Apply never spends BP or changes kingdom totals.",
        WS_CHILD | WS_VISIBLE | SS_LEFT, 18, 10, 1100, 28, window, nullptr, app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"Map asset", WS_CHILD | WS_VISIBLE | SS_LEFT,
        20, 45, 100, 22, window, nullptr, app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", widen(greenbelt_draft.asset_path).c_str(),
        WS_CHILD | WS_VISIBLE | SS_LEFT, 125, 45, 650, 22, window, nullptr, app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"Hexes", WS_CHILD | WS_VISIBLE | SS_LEFT,
        20, 78, 280, 22, window, nullptr, app_instance, nullptr);
    CreateWindowExW(0, L"BUTTON", L"Map Editor", WS_CHILD | WS_VISIBLE,
        360, 74, 125, 26, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kGreenbeltOpenMap)), app_instance, nullptr);
    CreateWindowExW(WS_EX_CLIENTEDGE, L"LISTBOX", L"",
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY, 20, 390, 700, 145, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kGreenbeltHexList)), app_instance, nullptr);
    manager_label(window, L"Q", 780, 82); manager_value(window, kGreenbeltQ, L"", 910, 82);
    manager_label(window, L"R", 780, 114); manager_value(window, kGreenbeltR, L"", 910, 114);
    refresh_greenbelt_hex_list(window);
}

void apply_greenbelt(HWND window) {
    save_greenbelt_form(window);
    pkkm::Kingdom candidate = kingdom;
    candidate.greenbelt_map = greenbelt_draft;
    pkkm::validate(candidate);
    kingdom.greenbelt_map = greenbelt_draft;
    dirty = true;
    greenbelt_draft = kingdom.greenbelt_map;
    refresh_greenbelt_hex_list(window);
    if (greenbelt_window && window != greenbelt_window && IsWindow(greenbelt_window))
        refresh_greenbelt_hex_list(greenbelt_window);
    refresh(GetDlgItem(GetWindow(window, GW_OWNER), kDisplay));
}

void add_greenbelt_improvement(HWND window) {
    save_greenbelt_form(window);
    if (greenbelt_active_index < 0)
        throw std::invalid_argument("Select a Greenbelt hex first.");
    const int selected = static_cast<int>(SendMessageW(
        GetDlgItem(window, kGreenbeltImprovement), CB_GETCURSEL, 0, 0));
    const int catalog_count = static_cast<int>(greenbelt_catalog.size());
    const int custom_count = static_cast<int>(greenbelt_draft.custom_improvements.size());
    if (selected <= 0 || selected > catalog_count + custom_count)
        throw std::invalid_argument("Select an improvement first.");
    const std::string improvement_id = selected <= catalog_count
        ? greenbelt_catalog[static_cast<size_t>(selected - 1)].id
        : greenbelt_draft.custom_improvements[static_cast<size_t>(selected - catalog_count - 1)].id;
    auto& improvements = greenbelt_draft.hexes[static_cast<size_t>(greenbelt_active_index)].improvements;
    if (std::find(improvements.begin(), improvements.end(), improvement_id) == improvements.end())
        improvements.push_back(improvement_id);
    refresh_greenbelt_hex_list(window);
    if (greenbelt_window && window != greenbelt_window && IsWindow(greenbelt_window))
        refresh_greenbelt_hex_list(greenbelt_window);
}

void cancel_greenbelt(HWND window) {
    greenbelt_draft = kingdom.greenbelt_map;
    greenbelt_active_index = greenbelt_draft.hexes.empty() ? -1 : 0;
    greenbelt_special_active_index = greenbelt_draft.special_terrain.empty() ? -1 : 0;
    greenbelt_custom_active_index = greenbelt_draft.custom_improvements.empty() ? -1 : 0;
    refresh_greenbelt_hex_list(window);
    refresh_greenbelt_special_list(window);
    refresh_greenbelt_custom_list(window);
}

void handle_greenbelt_map_click(HWND window, LPARAM lparam, bool toggle) {
    const double view_x = static_cast<double>(static_cast<short>(LOWORD(lparam)));
    const double view_y = static_cast<double>(static_cast<short>(HIWORD(lparam)) + greenbelt_scroll_y);
    const double viewport_x = kGreenbeltViewportX;
    const double viewport_y = kGreenbeltViewportY;
    const double viewport_width = kGreenbeltViewportWidth;
    const double viewport_height = kGreenbeltViewportHeight;
    if (view_x < viewport_x || view_x > viewport_x + viewport_width
        || view_y < viewport_y || view_y > viewport_y + viewport_height)
        return;
    const auto calibration = greenbelt_draft.calibration;
    if (calibration.hex_width <= 0.0 || calibration.hex_height <= 0.0 || calibration.scale <= 0.0) {
        MessageBoxW(window,
            L"Set Origin X, Origin Y, Hex width, Hex height, and Scale before selecting the map.",
            L"Greenbelt Map Calibration", MB_OK | MB_ICONINFORMATION);
        return;
    }
    Gdiplus::Image image(greenbelt_image_path().wstring().c_str());
    if (image.GetLastStatus() != Gdiplus::Ok || image.GetWidth() == 0 || image.GetHeight() == 0)
        return;
    const double image_scale = std::min(
        viewport_width / static_cast<double>(image.GetWidth()),
        viewport_height / static_cast<double>(image.GetHeight()));
    const double image_width = image.GetWidth() * image_scale;
    const double image_height = image.GetHeight() * image_scale;
    const double left = viewport_x + (viewport_width - image_width) * 0.5;
    const double top = viewport_y + (viewport_height - image_height) * 0.5;
    if (view_x < left || view_x > left + image_width || view_y < top || view_y > top + image_height)
        return;
    const double image_x = (view_x - left) / image_scale;
    const double image_y = (view_y - top) / image_scale;
    const auto coordinate = pkkm::greenbelt_hex_at_point(calibration, image_x, image_y);
    if (!pkkm::greenbelt_hex_inside_image(calibration, coordinate.q, coordinate.r,
            static_cast<int>(image.GetWidth()), static_cast<int>(image.GetHeight())))
        return;
    if (toggle) toggle_greenbelt_hex_at(window, coordinate.q, coordinate.r, image_x, image_y);
    else select_greenbelt_hex_at(window, coordinate.q, coordinate.r);
}

void paint_greenbelt_popout(HWND window) {
    PAINTSTRUCT paint{};
    HDC device = BeginPaint(window, &paint);
    Gdiplus::Graphics graphics(device);
    RECT client{};
    GetClientRect(window, &client);
    const Gdiplus::RectF viewport(10.0F, 112.0F,
        static_cast<float>(std::max(1L, client.right - 390L)),
        static_cast<float>(std::max(1L, client.bottom - 122L)));
    Gdiplus::SolidBrush background(Gdiplus::Color(245, 245, 245));
    graphics.FillRectangle(&background, viewport);
    try {
        Gdiplus::Image image(greenbelt_image_path().wstring().c_str());
        if (image.GetLastStatus() != Gdiplus::Ok || image.GetWidth() == 0 || image.GetHeight() == 0) {
            EndPaint(window, &paint);
            return;
        }
        const double fit_scale = std::min(
            static_cast<double>(viewport.Width) / image.GetWidth(),
            static_cast<double>(viewport.Height) / image.GetHeight());
        const double image_scale = fit_scale * greenbelt_popout_zoom;
        const float width = static_cast<float>(image.GetWidth() * image_scale);
        const float height = static_cast<float>(image.GetHeight() * image_scale);
        const float left = viewport.X + (viewport.Width - width) * 0.5F
            + static_cast<float>(greenbelt_popout_pan_x);
        const float top = viewport.Y + (viewport.Height - height) * 0.5F
            + static_cast<float>(greenbelt_popout_pan_y);
        graphics.DrawImage(&image, Gdiplus::RectF(left, top, width, height));
        const auto to_view = [&](const pkkm::GreenbeltPoint& point) {
            return Gdiplus::PointF(left + static_cast<float>(point.x * image_scale),
                top + static_cast<float>(point.y * image_scale));
        };
        Gdiplus::Pen grid_pen(Gdiplus::Color(255, 255, 255, 150), 1.0F);
        Gdiplus::Pen claimed_pen(Gdiplus::Color(150, 90, 100, 150), 1.0F);
        Gdiplus::Pen settlement_pen(Gdiplus::Color(220, 70, 180, 230), 2.0F);
        Gdiplus::Pen selected_pen(Gdiplus::Color(255, 220, 20), 3.0F);
        Gdiplus::Pen perimeter_pen(Gdiplus::Color(20, 90, 220), 4.0F);
        if (greenbelt_draft.calibration.hex_width > 0.0
            && greenbelt_draft.calibration.hex_height > 0.0) {
            for (int r = -20; r <= 30; ++r) {
            for (int q = -20; q <= 40; ++q) {
                if (!pkkm::greenbelt_hex_inside_image(greenbelt_draft.calibration,
                        q, r, static_cast<int>(image.GetWidth()), static_cast<int>(image.GetHeight())))
                    continue;
                const auto vertices = pkkm::greenbelt_hex_vertices(
                        greenbelt_draft.calibration, q, r);
                    Gdiplus::PointF polygon[6]{};
                    for (size_t vertex = 0; vertex < vertices.size(); ++vertex)
                        polygon[vertex] = to_view(vertices[vertex]);
                    graphics.DrawPolygon(&grid_pen, polygon, 6);
                }
            }
            const auto anchor = to_view(pkkm::greenbelt_hex_center(
                greenbelt_draft.calibration, 0, 0));
            Gdiplus::Font font(L"Segoe UI", 12.0F, Gdiplus::FontStyleBold);
            Gdiplus::SolidBrush brush(Gdiplus::Color(255, 255, 0));
            graphics.DrawString(L"(0, 0)", -1, &font,
                Gdiplus::PointF(anchor.X + 5.0F, anchor.Y + 5.0F), &brush);
        }
        for (size_t index = 0; index < greenbelt_draft.hexes.size(); ++index) {
            const auto& hex = greenbelt_draft.hexes[index];
            if (!pkkm::greenbelt_hex_inside_image(greenbelt_draft.calibration, hex.q, hex.r,
                    static_cast<int>(image.GetWidth()), static_cast<int>(image.GetHeight())))
                continue;
            const auto vertices = pkkm::greenbelt_hex_vertices(
                greenbelt_draft.calibration, hex.q, hex.r);
            Gdiplus::PointF polygon[6]{};
            for (size_t vertex = 0; vertex < vertices.size(); ++vertex)
                polygon[vertex] = to_view(vertices[vertex]);
            Gdiplus::Pen* hex_pen = nullptr;
            if (index == static_cast<size_t>(greenbelt_active_index))
                hex_pen = &selected_pen;
            else if (!hex.settlement_id.empty())
                hex_pen = &settlement_pen;
            else if (hex.claimed)
                hex_pen = &claimed_pen;
            if (hex_pen) graphics.DrawPolygon(hex_pen, polygon, 6);
            draw_greenbelt_settlement_label(graphics, greenbelt_draft.calibration, hex,
                left, top, image_scale);
            draw_greenbelt_improvements(graphics, greenbelt_draft.calibration, hex,
                left, top, image_scale);
        }
        for (const auto& edge : pkkm::greenbelt_claimed_perimeter(greenbelt_draft.hexes)) {
            const auto vertices = pkkm::greenbelt_hex_vertices(
                greenbelt_draft.calibration, edge.q, edge.r);
            const auto first = to_view(vertices[static_cast<size_t>(edge.side)]);
            const auto second = to_view(vertices[static_cast<size_t>((edge.side + 1) % 6)]);
            graphics.DrawLine(&perimeter_pen, first, second);
        }
    } catch (...) {
        Gdiplus::Font font(L"Segoe UI", 11.0F);
        Gdiplus::SolidBrush brush(Gdiplus::Color(40, 40, 40));
        graphics.DrawString(L"Greenbelt image could not be loaded.", -1, &font,
            Gdiplus::PointF(viewport.X + 8.0F, viewport.Y + 8.0F), &brush);
    }
    EndPaint(window, &paint);
}

void handle_greenbelt_popout_click(HWND window, LPARAM lparam, bool toggle) {
    RECT client{};
    GetClientRect(window, &client);
    const double view_x = static_cast<double>(static_cast<short>(LOWORD(lparam)));
    const double view_y = static_cast<double>(static_cast<short>(HIWORD(lparam)));
    const double viewport_x = 10.0;
    const double viewport_y = 112.0;
    const double viewport_width = std::max(1L, client.right - 390L);
    const double viewport_height = std::max(1L, client.bottom - 122L);
    if (view_x < viewport_x || view_y < viewport_y
        || view_x > viewport_x + viewport_width || view_y > viewport_y + viewport_height)
        return;
    const auto calibration = greenbelt_draft.calibration;
    if (calibration.hex_width <= 0.0 || calibration.hex_height <= 0.0 || calibration.scale <= 0.0)
        return;
    Gdiplus::Image image(greenbelt_image_path().wstring().c_str());
    if (image.GetLastStatus() != Gdiplus::Ok || image.GetWidth() == 0 || image.GetHeight() == 0)
        return;
    const double fit_scale = std::min(viewport_width / image.GetWidth(), viewport_height / image.GetHeight());
    const double image_scale = fit_scale * greenbelt_popout_zoom;
    const double image_width = image.GetWidth() * image_scale;
    const double image_height = image.GetHeight() * image_scale;
    const double left = viewport_x + (viewport_width - image_width) * 0.5 + greenbelt_popout_pan_x;
    const double top = viewport_y + (viewport_height - image_height) * 0.5 + greenbelt_popout_pan_y;
    if (view_x < left || view_y < top || view_x > left + image_width || view_y > top + image_height)
        return;
    const double image_x = (view_x - left) / image_scale;
    const double image_y = (view_y - top) / image_scale;
    const auto coordinate = pkkm::greenbelt_hex_at_point(calibration, image_x, image_y);
    if (!pkkm::greenbelt_hex_inside_image(calibration, coordinate.q, coordinate.r,
            static_cast<int>(image.GetWidth()), static_cast<int>(image.GetHeight())))
        return;
    if (toggle) toggle_greenbelt_hex_at(greenbelt_popout_window, coordinate.q, coordinate.r, image_x, image_y);
    else select_greenbelt_hex_at(greenbelt_popout_window, coordinate.q, coordinate.r);
    InvalidateRect(window, nullptr, FALSE);
}

void open_greenbelt_popout(HWND owner) {
    if (greenbelt_popout_window && IsWindow(greenbelt_popout_window)) {
        SetForegroundWindow(greenbelt_popout_window);
        return;
    }
    if (greenbelt_window && IsWindow(greenbelt_window)
        && greenbelt_active_index >= 0
        && greenbelt_active_index < static_cast<int>(greenbelt_draft.hexes.size()))
        save_greenbelt_form(greenbelt_window);
    greenbelt_alignment_mode = false;
    greenbelt_popout_zoom = 1.0;
    greenbelt_popout_pan_x = 0.0;
    greenbelt_popout_pan_y = 0.0;
    greenbelt_popout_scroll_y = 0;
    greenbelt_popout_window = CreateWindowExW(WS_EX_DLGMODALFRAME, L"PKKMGreenbeltMapPopout",
        L"Greenbelt Map - Large View", WS_CAPTION | WS_SYSMENU | WS_POPUP | WS_THICKFRAME
        | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_VSCROLL,
        CW_USEDEFAULT, CW_USEDEFAULT, 1250, 850, owner, nullptr, app_instance, nullptr);
    if (!greenbelt_popout_window)
        throw std::runtime_error("Could not open the large Greenbelt Map view");
    ShowWindow(greenbelt_popout_window, SW_SHOW);
    UpdateWindow(greenbelt_popout_window);
}

void update_greenbelt_alignment_status(HWND window) {
    const auto& calibration = greenbelt_draft.calibration;
    const std::wstring text = L"Alignment: origin (" + std::to_wstring(calibration.origin_x)
        + L", " + std::to_wstring(calibration.origin_y) + L") | size ("
        + std::to_wstring(calibration.hex_width) + L" x "
        + std::to_wstring(calibration.hex_height) + L") | scale "
        + std::to_wstring(calibration.scale)
        + L" | Partial cells: out of bounds";
    set_manager_text(window, kGreenbeltAlignmentStatus, text);
}

void set_greenbelt_alignment_controls_visible(HWND window, bool visible) {
    const int ids[] = {
        kGreenbeltPanLeft, kGreenbeltPanRight, kGreenbeltPanUp, kGreenbeltPanDown,
        kGreenbeltGrowWidth, kGreenbeltShrinkWidth, kGreenbeltGrowHeight,
        kGreenbeltShrinkHeight, kGreenbeltResetPan, kGreenbeltAlignmentStatus
    };
    for (const int id : ids)
        ShowWindow(GetDlgItem(window, id), visible ? SW_SHOW : SW_HIDE);
}

void adjust_greenbelt_alignment(HWND window, int dx, int dy, double dw, double dh) {
    if (!greenbelt_alignment_mode) return;
    auto& calibration = greenbelt_draft.calibration;
    calibration.origin_x += dx;
    calibration.origin_y += dy;
    calibration.hex_width = std::max(1.0, calibration.hex_width + dw);
    calibration.hex_height = std::max(1.0, calibration.hex_height + dh);
    update_greenbelt_alignment_status(window);
    InvalidateRect(window, nullptr, FALSE);
    if (greenbelt_window && IsWindow(greenbelt_window))
        InvalidateRect(greenbelt_window, nullptr, FALSE);
}

std::string greenbelt_custom_id_for_name(const std::string& name) {
    std::string slug;
    bool separator = false;
    for (const unsigned char character : name) {
        if (std::isalnum(character)) {
            slug.push_back(static_cast<char>(std::tolower(character)));
            separator = false;
        } else if (!slug.empty() && !separator) {
            slug.push_back('-');
            separator = true;
        }
    }
    while (!slug.empty() && slug.back() == '-') slug.pop_back();
    if (slug.empty()) slug = "improvement";
    std::string base = "custom." + slug;
    std::string candidate = base;
    int suffix = 2;
    const auto exists = [&candidate] {
        return std::any_of(greenbelt_draft.custom_improvements.begin(),
            greenbelt_draft.custom_improvements.end(),
            [&candidate](const auto& item) { return item.id == candidate; });
    };
    while (exists()) candidate = base + "." + std::to_string(suffix++);
    return candidate;
}

void populate_greenbelt_custom_popup_combo(HWND window, int control_id, bool effects) {
    const HWND combo = GetDlgItem(window, control_id);
    std::vector<std::string> values;
    for (const auto& item : greenbelt_catalog) {
        const std::string& value = effects ? item.effect : item.cost_description;
        if (!value.empty() && std::find(values.begin(), values.end(), value) == values.end())
            values.push_back(value);
    }
    for (const auto& value : values)
        SendMessageW(combo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(widen(value).c_str()));
}

std::vector<std::string> greenbelt_custom_popup_values(HWND window, int list_id) {
    std::vector<std::string> values;
    const HWND list = GetDlgItem(window, list_id);
    const int count = static_cast<int>(SendMessageW(list, LB_GETCOUNT, 0, 0));
    for (int index = 0; index < count; ++index) {
        const int length = static_cast<int>(SendMessageW(list, LB_GETTEXTLEN, index, 0));
        std::wstring text(static_cast<size_t>(length) + 1, L'\0');
        SendMessageW(list, LB_GETTEXT, index, reinterpret_cast<LPARAM>(text.data()));
        text.resize(static_cast<size_t>(length));
        values.push_back(narrow(text));
    }
    return values;
}

std::wstring greenbelt_custom_popup_selected_value(HWND window, int list_id) {
    const HWND list = GetDlgItem(window, list_id);
    const int selected = static_cast<int>(SendMessageW(list, LB_GETCURSEL, 0, 0));
    if (selected < 0) return {};
    const int length = static_cast<int>(SendMessageW(list, LB_GETTEXTLEN, selected, 0));
    std::wstring text(static_cast<size_t>(length) + 1, L'\0');
    SendMessageW(list, LB_GETTEXT, selected, reinterpret_cast<LPARAM>(text.data()));
    text.resize(static_cast<size_t>(length));
    return text;
}

void load_greenbelt_custom_popup_item(HWND window, int index) {
    if (index < 0 || index >= static_cast<int>(greenbelt_draft.custom_improvements.size())) return;
    const auto& item = greenbelt_draft.custom_improvements[static_cast<size_t>(index)];
    set_manager_text(window, kGreenbeltCustomPopupId, widen(item.id));
    set_manager_text(window, kGreenbeltCustomPopupName, widen(item.name));
    SendMessageW(GetDlgItem(window, kGreenbeltCustomPopupEffectList), LB_RESETCONTENT, 0, 0);
    SendMessageW(GetDlgItem(window, kGreenbeltCustomPopupCostList), LB_RESETCONTENT, 0, 0);
    const auto effects = item.effects.empty() && !item.effect_preview.empty()
        ? std::vector<std::string>{item.effect_preview} : item.effects;
    const auto costs = item.costs.empty() && !item.cost_description.empty()
        ? std::vector<std::string>{item.cost_description} : item.costs;
    for (const auto& value : effects)
        SendMessageW(GetDlgItem(window, kGreenbeltCustomPopupEffectList), LB_ADDSTRING, 0,
            reinterpret_cast<LPARAM>(widen(value).c_str()));
    for (const auto& value : costs)
        SendMessageW(GetDlgItem(window, kGreenbeltCustomPopupCostList), LB_ADDSTRING, 0,
            reinterpret_cast<LPARAM>(widen(value).c_str()));
    SendMessageW(GetDlgItem(window, kGreenbeltCustomPopupManual), BM_SETCHECK,
        item.manual_only ? BST_CHECKED : BST_UNCHECKED, 0);
    SetWindowTextW(window, L"Edit Custom Improvement");
    SetWindowTextW(GetDlgItem(window, kGreenbeltCustomPopupAdd), L"Save/Close");
}

void update_greenbelt_custom_popup_id(HWND window) {
    const std::string name = narrow(manager_text(window, kGreenbeltCustomPopupName));
    set_manager_text(window, kGreenbeltCustomPopupId,
        widen(name.empty() ? "custom.improvement" : greenbelt_custom_id_for_name(name)));
}

void add_greenbelt_custom_popup_value(HWND window, int combo_id, int list_id) {
    const std::wstring value = manager_text(window, combo_id);
    if (value.empty()) return;
    const HWND list = GetDlgItem(window, list_id);
    const int added = static_cast<int>(SendMessageW(list, LB_ADDSTRING, 0,
        reinterpret_cast<LPARAM>(value.c_str())));
    if (added >= 0) SendMessageW(list, LB_SETCURSEL, added, 0);
}

void update_greenbelt_custom_popup_value(HWND window, int combo_id, int list_id) {
    const std::wstring value = manager_text(window, combo_id);
    if (value.empty()) return;
    const HWND list = GetDlgItem(window, list_id);
    const int selected = static_cast<int>(SendMessageW(list, LB_GETCURSEL, 0, 0));
    if (selected < 0) return;
    SendMessageW(list, LB_DELETESTRING, selected, 0);
    SendMessageW(list, LB_INSERTSTRING, selected, reinterpret_cast<LPARAM>(value.c_str()));
    SendMessageW(list, LB_SETCURSEL, selected, 0);
}

void remove_greenbelt_custom_popup_value(HWND window, int list_id) {
    const HWND list = GetDlgItem(window, list_id);
    const int selected = static_cast<int>(SendMessageW(list, LB_GETCURSEL, 0, 0));
    if (selected >= 0) SendMessageW(list, LB_DELETESTRING, selected, 0);
}

void commit_greenbelt_custom_popup(HWND window) {
    if (greenbelt_active_index < 0
        || greenbelt_active_index >= static_cast<int>(greenbelt_draft.hexes.size()))
        throw std::invalid_argument("Select a Greenbelt hex first.");
    pkkm::GreenbeltCustomImprovement item;
    item.name = narrow(manager_text(window, kGreenbeltCustomPopupName));
    const bool editing = greenbelt_custom_popup_edit_index >= 0
        && greenbelt_custom_popup_edit_index < static_cast<int>(greenbelt_draft.custom_improvements.size());
    item.id = editing
        ? greenbelt_draft.custom_improvements[static_cast<size_t>(greenbelt_custom_popup_edit_index)].id
        : greenbelt_custom_id_for_name(item.name);
    item.effects = greenbelt_custom_popup_values(window, kGreenbeltCustomPopupEffectList);
    item.costs = greenbelt_custom_popup_values(window, kGreenbeltCustomPopupCostList);
    item.effect_preview = item.effects.empty() ? std::string{} : item.effects.front();
    item.cost_description = item.costs.empty() ? std::string{} : item.costs.front();
    item.manual_only = SendMessageW(GetDlgItem(window, kGreenbeltCustomPopupManual), BM_GETCHECK, 0, 0) == BST_CHECKED;
    if (item.name.empty()) throw std::invalid_argument("Custom improvements need a name.");
    if (editing) {
        greenbelt_draft.custom_improvements[static_cast<size_t>(greenbelt_custom_popup_edit_index)] = item;
    } else {
        const auto existing = std::find_if(greenbelt_draft.custom_improvements.begin(),
            greenbelt_draft.custom_improvements.end(),
            [&item](const auto& candidate) { return candidate.id == item.id; });
        if (existing == greenbelt_draft.custom_improvements.end()) {
            greenbelt_draft.custom_improvements.push_back(item);
        } else {
            *existing = item;
        }
    }
    auto& improvements = greenbelt_draft.hexes[static_cast<size_t>(greenbelt_active_index)].improvements;
    if (!editing && std::find(improvements.begin(), improvements.end(), item.id) == improvements.end())
        improvements.push_back(item.id);
    if (greenbelt_window && IsWindow(greenbelt_window))
        refresh_greenbelt_hex_list(greenbelt_window);
    if (greenbelt_popout_window && IsWindow(greenbelt_popout_window)) {
        const HWND improvement_combo = GetDlgItem(greenbelt_popout_window, kGreenbeltImprovement);
        SendMessageW(improvement_combo, CB_RESETCONTENT, 0, 0);
        SendMessageW(improvement_combo, CB_ADDSTRING, 0,
            reinterpret_cast<LPARAM>(L"(choose an improvement)"));
        for (const auto& catalog_item : greenbelt_catalog) {
            const std::wstring label = widen(catalog_item.name)
                + (catalog_item.third_party ? L" [3pp/source-specific]" : L"");
            SendMessageW(improvement_combo, CB_ADDSTRING, 0,
                reinterpret_cast<LPARAM>(label.c_str()));
        }
        for (const auto& custom_item : greenbelt_draft.custom_improvements) {
            const std::wstring label = widen(custom_item.name) + L" [custom/manual]";
            SendMessageW(improvement_combo, CB_ADDSTRING, 0,
                reinterpret_cast<LPARAM>(label.c_str()));
        }
        SendMessageW(improvement_combo, CB_ADDSTRING, 0,
            reinterpret_cast<LPARAM>(L"Custom Improvement..."));
        SendMessageW(improvement_combo, CB_SETCURSEL, 0, 0);
        refresh_greenbelt_hex_list(greenbelt_popout_window);
        refresh_greenbelt_preview(greenbelt_popout_window);
        InvalidateRect(greenbelt_popout_window, nullptr, FALSE);
    }
    DestroyWindow(window);
}

void open_greenbelt_custom_popup(HWND owner, int edit_index = -1) {
    if (greenbelt_custom_popup_window && IsWindow(greenbelt_custom_popup_window)) {
        SetForegroundWindow(greenbelt_custom_popup_window);
        return;
    }
    WNDCLASSW popup_class{};
    if (!GetClassInfoW(app_instance, L"PKKMGreenbeltCustomImprovement", &popup_class)) {
        popup_class.lpfnWndProc = greenbelt_custom_popup_proc;
        popup_class.hInstance = app_instance;
        popup_class.lpszClassName = L"PKKMGreenbeltCustomImprovement";
        popup_class.hCursor = LoadCursor(nullptr, IDC_ARROW);
        popup_class.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
        if (!RegisterClassW(&popup_class) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
            throw std::runtime_error("Could not register the custom improvement dialog class");
    }
    greenbelt_custom_popup_edit_index = edit_index;
    greenbelt_custom_popup_window = CreateWindowExW(WS_EX_DLGMODALFRAME,
        L"PKKMGreenbeltCustomImprovement", L"Custom Improvement",
        WS_CAPTION | WS_SYSMENU | WS_POPUP | WS_THICKFRAME,
        CW_USEDEFAULT, CW_USEDEFAULT, 640, 500, owner, nullptr, app_instance, nullptr);
    if (!greenbelt_custom_popup_window) {
        const DWORD error = GetLastError();
        throw std::runtime_error("Could not open the custom improvement dialog (Win32 error "
            + std::to_string(error) + ").");
    }
    ShowWindow(greenbelt_custom_popup_window, SW_SHOW);
    UpdateWindow(greenbelt_custom_popup_window);
}

LRESULT CALLBACK greenbelt_custom_popup_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
    switch (message) {
        case WM_CREATE:
            manager_label(window, L"ID (automatic)", 20, 20, 110);
            manager_edit(window, kGreenbeltCustomPopupId, L"custom.improvement", 140, 20, 430);
            SendMessageW(GetDlgItem(window, kGreenbeltCustomPopupId), EM_SETREADONLY, TRUE, 0);
            manager_label(window, L"Name", 20, 54, 110); manager_edit(window, kGreenbeltCustomPopupName, L"", 140, 54, 430);
            manager_label(window, L"Effects", 20, 88, 110);
            CreateWindowExW(WS_EX_CLIENTEDGE, L"COMBOBOX", L"",
                WS_CHILD | WS_VISIBLE | CBS_DROPDOWN | CBS_AUTOHSCROLL | WS_VSCROLL,
                140, 88, 360, 180, window,
                reinterpret_cast<HMENU>(static_cast<INT_PTR>(kGreenbeltCustomPopupEffectCombo)), app_instance, nullptr);
            CreateWindowExW(0, L"BUTTON", L"Add", WS_CHILD | WS_VISIBLE,
                510, 88, 90, 24, window,
                reinterpret_cast<HMENU>(static_cast<INT_PTR>(kGreenbeltCustomPopupEffectAdd)), app_instance, nullptr);
            CreateWindowExW(WS_EX_CLIENTEDGE, L"LISTBOX", L"",
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY,
                140, 118, 360, 100, window,
                reinterpret_cast<HMENU>(static_cast<INT_PTR>(kGreenbeltCustomPopupEffectList)), app_instance, nullptr);
            CreateWindowExW(0, L"BUTTON", L"Update", WS_CHILD | WS_VISIBLE,
                510, 118, 90, 24, window,
                reinterpret_cast<HMENU>(static_cast<INT_PTR>(kGreenbeltCustomPopupEffectUpdate)), app_instance, nullptr);
            CreateWindowExW(0, L"BUTTON", L"Remove", WS_CHILD | WS_VISIBLE,
                510, 148, 90, 24, window,
                reinterpret_cast<HMENU>(static_cast<INT_PTR>(kGreenbeltCustomPopupEffectRemove)), app_instance, nullptr);
            manager_label(window, L"Costs", 20, 236, 110);
            CreateWindowExW(WS_EX_CLIENTEDGE, L"COMBOBOX", L"",
                WS_CHILD | WS_VISIBLE | CBS_DROPDOWN | CBS_AUTOHSCROLL | WS_VSCROLL,
                140, 236, 360, 180, window,
                reinterpret_cast<HMENU>(static_cast<INT_PTR>(kGreenbeltCustomPopupCostCombo)), app_instance, nullptr);
            CreateWindowExW(0, L"BUTTON", L"Add", WS_CHILD | WS_VISIBLE,
                510, 236, 90, 24, window,
                reinterpret_cast<HMENU>(static_cast<INT_PTR>(kGreenbeltCustomPopupCostAdd)), app_instance, nullptr);
            CreateWindowExW(WS_EX_CLIENTEDGE, L"LISTBOX", L"",
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY,
                140, 266, 360, 100, window,
                reinterpret_cast<HMENU>(static_cast<INT_PTR>(kGreenbeltCustomPopupCostList)), app_instance, nullptr);
            CreateWindowExW(0, L"BUTTON", L"Update", WS_CHILD | WS_VISIBLE,
                510, 266, 90, 24, window,
                reinterpret_cast<HMENU>(static_cast<INT_PTR>(kGreenbeltCustomPopupCostUpdate)), app_instance, nullptr);
            CreateWindowExW(0, L"BUTTON", L"Remove", WS_CHILD | WS_VISIBLE,
                510, 296, 90, 24, window,
                reinterpret_cast<HMENU>(static_cast<INT_PTR>(kGreenbeltCustomPopupCostRemove)), app_instance, nullptr);
            CreateWindowExW(0, L"BUTTON", L"Manual only", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
                140, 380, 120, 24, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kGreenbeltCustomPopupManual)), app_instance, nullptr);
            CheckDlgButton(window, kGreenbeltCustomPopupManual, BST_CHECKED);
            CreateWindowExW(0, L"BUTTON", L"Add/Close", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
                390, 420, 105, 30, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kGreenbeltCustomPopupAdd)), app_instance, nullptr);
            CreateWindowExW(0, L"BUTTON", L"Cancel", WS_CHILD | WS_VISIBLE,
                505, 420, 90, 30, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kGreenbeltCustomPopupCancel)), app_instance, nullptr);
            populate_greenbelt_custom_popup_combo(window, kGreenbeltCustomPopupEffectCombo, true);
            populate_greenbelt_custom_popup_combo(window, kGreenbeltCustomPopupCostCombo, false);
            load_greenbelt_custom_popup_item(window, greenbelt_custom_popup_edit_index);
            return 0;
        case WM_COMMAND:
            if (LOWORD(wparam) == kGreenbeltCustomPopupName && HIWORD(wparam) == EN_CHANGE) {
                if (greenbelt_custom_popup_edit_index < 0)
                    update_greenbelt_custom_popup_id(window);
                return 0;
            }
            if (LOWORD(wparam) == kGreenbeltCustomPopupEffectList && HIWORD(wparam) == LBN_SELCHANGE) {
                set_manager_text(window, kGreenbeltCustomPopupEffectCombo,
                    greenbelt_custom_popup_selected_value(window, kGreenbeltCustomPopupEffectList));
                return 0;
            }
            if (LOWORD(wparam) == kGreenbeltCustomPopupCostList && HIWORD(wparam) == LBN_SELCHANGE) {
                set_manager_text(window, kGreenbeltCustomPopupCostCombo,
                    greenbelt_custom_popup_selected_value(window, kGreenbeltCustomPopupCostList));
                return 0;
            }
            if (LOWORD(wparam) == kGreenbeltCustomPopupEffectAdd) {
                add_greenbelt_custom_popup_value(window, kGreenbeltCustomPopupEffectCombo, kGreenbeltCustomPopupEffectList);
                return 0;
            }
            if (LOWORD(wparam) == kGreenbeltCustomPopupEffectUpdate) {
                update_greenbelt_custom_popup_value(window, kGreenbeltCustomPopupEffectCombo, kGreenbeltCustomPopupEffectList);
                return 0;
            }
            if (LOWORD(wparam) == kGreenbeltCustomPopupEffectRemove) {
                remove_greenbelt_custom_popup_value(window, kGreenbeltCustomPopupEffectList);
                return 0;
            }
            if (LOWORD(wparam) == kGreenbeltCustomPopupCostAdd) {
                add_greenbelt_custom_popup_value(window, kGreenbeltCustomPopupCostCombo, kGreenbeltCustomPopupCostList);
                return 0;
            }
            if (LOWORD(wparam) == kGreenbeltCustomPopupCostUpdate) {
                update_greenbelt_custom_popup_value(window, kGreenbeltCustomPopupCostCombo, kGreenbeltCustomPopupCostList);
                return 0;
            }
            if (LOWORD(wparam) == kGreenbeltCustomPopupCostRemove) {
                remove_greenbelt_custom_popup_value(window, kGreenbeltCustomPopupCostList);
                return 0;
            }
            if (LOWORD(wparam) == kGreenbeltCustomPopupAdd) {
                try {
                    commit_greenbelt_custom_popup(window);
                } catch (const std::exception& error) {
                    MessageBoxW(window, widen(error.what()).c_str(), L"Custom Improvement", MB_OK | MB_ICONWARNING);
                }
                return 0;
            }
            if (LOWORD(wparam) == kGreenbeltCustomPopupCancel) {
                DestroyWindow(window);
                return 0;
            }
            break;
        case WM_CLOSE:
            DestroyWindow(window);
            return 0;
        case WM_DESTROY:
            greenbelt_custom_popup_window = nullptr;
            greenbelt_custom_popup_edit_index = -1;
            return 0;
    }
    return DefWindowProcW(window, message, wparam, lparam);
}

void update_greenbelt_popout_scrollbar(HWND window) {
    RECT client{};
    GetClientRect(window, &client);
    SCROLLINFO info{sizeof(info), SIF_RANGE | SIF_PAGE | SIF_POS};
    info.nMin = 0;
    info.nMax = 1040;
    info.nPage = static_cast<UINT>(std::max(0L, client.bottom - client.top));
    const int max_position = std::max(0, info.nMax - static_cast<int>(info.nPage) + 1);
    greenbelt_popout_scroll_y = std::clamp(greenbelt_popout_scroll_y, 0, max_position);
    info.nPos = greenbelt_popout_scroll_y;
    SetScrollInfo(window, SB_VERT, &info, TRUE);
}

void scroll_greenbelt_popout_to(HWND window, int requested_position) {
    SCROLLINFO info{sizeof(info), SIF_RANGE | SIF_PAGE};
    GetScrollInfo(window, SB_VERT, &info);
    const int max_position = std::max(0, info.nMax - static_cast<int>(info.nPage) + 1);
    const int position = std::clamp(requested_position, 0, max_position);
    const int delta = position - greenbelt_popout_scroll_y;
    if (delta == 0) return;
    greenbelt_popout_scroll_y = position;
    SetScrollPos(window, SB_VERT, position, TRUE);
    ScrollWindowEx(window, 0, -delta, nullptr, nullptr, nullptr, nullptr,
        SW_SCROLLCHILDREN | SW_INVALIDATE | SW_ERASE);
}

LRESULT CALLBACK greenbelt_popout_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
    if (message == WM_GETMINMAXINFO) {
        set_minimum_track_size(lparam, 700, 900);
        return 0;
    }
    switch (message) {
        case WM_CREATE: {
            CreateWindowExW(0, L"BUTTON", L"Zoom In", WS_CHILD | WS_VISIBLE,
                10, 10, 90, 28, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kGreenbeltZoomIn)), app_instance, nullptr);
            CreateWindowExW(0, L"BUTTON", L"Zoom Out", WS_CHILD | WS_VISIBLE,
                110, 10, 90, 28, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kGreenbeltZoomOut)), app_instance, nullptr);
            CreateWindowExW(0, L"BUTTON", L"Fit", WS_CHILD | WS_VISIBLE,
                210, 10, 70, 28, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kGreenbeltZoomFit)), app_instance, nullptr);
            CreateWindowExW(0, L"BUTTON", L"Alignment Mode", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
                300, 10, 125, 28, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kGreenbeltAlignMode)), app_instance, nullptr);
            CreateWindowExW(0, L"BUTTON", L"Left", WS_CHILD | WS_VISIBLE,
                10, 44, 55, 26, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kGreenbeltPanLeft)), app_instance, nullptr);
            CreateWindowExW(0, L"BUTTON", L"Right", WS_CHILD | WS_VISIBLE,
                70, 44, 55, 26, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kGreenbeltPanRight)), app_instance, nullptr);
            CreateWindowExW(0, L"BUTTON", L"Up", WS_CHILD | WS_VISIBLE,
                130, 44, 45, 26, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kGreenbeltPanUp)), app_instance, nullptr);
            CreateWindowExW(0, L"BUTTON", L"Down", WS_CHILD | WS_VISIBLE,
                180, 44, 55, 26, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kGreenbeltPanDown)), app_instance, nullptr);
            CreateWindowExW(0, L"BUTTON", L"Grow W", WS_CHILD | WS_VISIBLE,
                245, 44, 65, 26, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kGreenbeltGrowWidth)), app_instance, nullptr);
            CreateWindowExW(0, L"BUTTON", L"Shrink W", WS_CHILD | WS_VISIBLE,
                315, 44, 70, 26, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kGreenbeltShrinkWidth)), app_instance, nullptr);
            CreateWindowExW(0, L"BUTTON", L"Grow H", WS_CHILD | WS_VISIBLE,
                390, 44, 65, 26, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kGreenbeltGrowHeight)), app_instance, nullptr);
            CreateWindowExW(0, L"BUTTON", L"Shrink H", WS_CHILD | WS_VISIBLE,
                460, 44, 70, 26, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kGreenbeltShrinkHeight)), app_instance, nullptr);
            CreateWindowExW(0, L"BUTTON", L"Reset Pan", WS_CHILD | WS_VISIBLE,
                535, 44, 80, 26, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kGreenbeltResetPan)), app_instance, nullptr);
            CreateWindowExW(0, L"STATIC", L"Click a map hex to select or create it. Drag the map while zoomed to pan.",
                WS_CHILD | WS_VISIBLE | SS_LEFT, 435, 14, 450, 22, window, nullptr, app_instance, nullptr);
            CreateWindowExW(0, L"BUTTON", L"Apply", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
                930, 10, 90, 28, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kGreenbeltApply)), app_instance, nullptr);
            CreateWindowExW(0, L"BUTTON", L"Close", WS_CHILD | WS_VISIBLE,
                1030, 10, 90, 28, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kGreenbeltPopoutClose)), app_instance, nullptr);
            CreateWindowExW(0, L"STATIC", L"", WS_CHILD | WS_VISIBLE | SS_LEFT,
                10, 78, 830, 24, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kGreenbeltAlignmentStatus)), app_instance, nullptr);
            CreateWindowExW(0, L"STATIC", L"Selected hex", WS_CHILD | WS_VISIBLE | SS_LEFT,
                860, 112, 330, 22, window, nullptr, app_instance, nullptr);
            manager_label(window, L"Q", 860, 140, 100); manager_value(window, kGreenbeltQ, L"", 970, 140, 220);
            manager_label(window, L"R", 860, 168, 100); manager_value(window, kGreenbeltR, L"", 970, 168, 220);
            manager_label(window, L"Terrain", 860, 196, 100); manager_edit(window, kGreenbeltTerrain, L"unresolved", 970, 196, 220);
            CreateWindowExW(0, L"STATIC",
                L"Auto-detection is advisory and editable.", WS_CHILD | WS_VISIBLE | SS_LEFT,
                860, 228, 330, 32, window,
                reinterpret_cast<HMENU>(static_cast<INT_PTR>(kGreenbeltAutodetected)), app_instance, nullptr);
            manager_label(window, L"Terrain notes", 860, 260, 100);
            manager_edit(window, kGreenbeltTerrainNotes, L"", 970, 260, 220);
            CreateWindowExW(0, L"BUTTON", L"Claimed", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
                860, 292, 120, 24, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kGreenbeltClaimed)), app_instance, nullptr);
            manager_label(window, L"Claim turn", 860, 322, 100); manager_edit(window, kGreenbeltClaimTurn, L"0", 970, 322, 220);
            manager_label(window, L"Settlement", 860, 354, 100);
            CreateWindowExW(0, L"COMBOBOX", L"", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
                970, 354, 220, 160, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kGreenbeltSettlement)), app_instance, nullptr);
            manager_label(window, L"Improvement", 860, 386, 100);
            CreateWindowExW(0, L"COMBOBOX", L"", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_VSCROLL,
                970, 386, 220, 180, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kGreenbeltImprovement)), app_instance, nullptr);
            CreateWindowExW(0, L"BUTTON", L"Add Improvement", WS_CHILD | WS_VISIBLE,
                970, 418, 140, 26, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kGreenbeltAddImprovement)), app_instance, nullptr);
            CreateWindowExW(0, L"STATIC", L"Rule preview", WS_CHILD | WS_VISIBLE | SS_LEFT,
                860, 462, 180, 22, window, nullptr, app_instance, nullptr);
            CreateWindowExW(0, L"BUTTON", L"Refresh Preview", WS_CHILD | WS_VISIBLE,
                1035, 458, 155, 28, window,
                reinterpret_cast<HMENU>(static_cast<INT_PTR>(kGreenbeltRefreshPreview)), app_instance, nullptr);
            CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"Select an improvement.",
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_READONLY,
                860, 488, 330, 120, window,
                reinterpret_cast<HMENU>(static_cast<INT_PTR>(kGreenbeltPreview)), app_instance, nullptr);
            CreateWindowExW(0, L"STATIC", L"Claimed / mapped hexes", WS_CHILD | WS_VISIBLE | SS_LEFT,
                860, 620, 330, 22, window, nullptr, app_instance, nullptr);
            CreateWindowExW(WS_EX_CLIENTEDGE, L"LISTBOX", L"",
                WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY, 860, 646, 330, 46, window,
                reinterpret_cast<HMENU>(static_cast<INT_PTR>(kGreenbeltHexList)), app_instance, nullptr);
            CreateWindowExW(0, L"STATIC", L"Supplemental preview facts (caller supplied; non-mutating)",
                WS_CHILD | WS_VISIBLE | SS_LEFT, 860, 700, 330, 22, window, nullptr, app_instance, nullptr);
            CreateWindowExW(0, L"BUTTON", L"Water adjacent", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
                860, 728, 150, 24, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kGreenbeltFactWater)), app_instance, nullptr);
            CreateWindowExW(0, L"BUTTON", L"River adjacent", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
                1015, 728, 150, 24, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kGreenbeltFactRiver)), app_instance, nullptr);
            CreateWindowExW(0, L"BUTTON", L"Canal adjacent", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
                860, 756, 150, 24, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kGreenbeltFactCanal)), app_instance, nullptr);
            CreateWindowExW(0, L"BUTTON", L"Settlement endpoint", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
                1015, 756, 170, 24, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kGreenbeltFactEndpoint)), app_instance, nullptr);
            CreateWindowExW(0, L"BUTTON", L"Connected Road", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
                860, 784, 150, 24, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kGreenbeltFactRoad)), app_instance, nullptr);
            CreateWindowExW(0, L"BUTTON", L"River crossing", WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
                1015, 784, 150, 24, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kGreenbeltFactCrossing)), app_instance, nullptr);
            manager_label(window, L"Farms", 860, 814, 45);
            manager_edit(window, kGreenbeltFactFarms, L"0", 905, 812, 55);
            manager_label(window, L"Size", 975, 814, 35);
            manager_edit(window, kGreenbeltFactSize, L"0", 1010, 812, 55);
            manager_label(window, L"Route", 1080, 814, 45);
            manager_edit(window, kGreenbeltFactRoute, L"0", 1125, 812, 65);
            const HWND settlement_combo = GetDlgItem(window, kGreenbeltSettlement);
            const int none_index = static_cast<int>(SendMessageW(settlement_combo, CB_ADDSTRING, 0,
                reinterpret_cast<LPARAM>(L"(none)")));
            SendMessageW(settlement_combo, CB_SETITEMDATA, none_index, 0);
            for (size_t index = 0; index < kingdom.settlements.size(); ++index) {
                const auto& settlement = kingdom.settlements[index];
                const auto label = widen(settlement.name) + L" [" + widen(settlement.id) + L"]";
                const int item_index = static_cast<int>(SendMessageW(settlement_combo, CB_ADDSTRING, 0,
                    reinterpret_cast<LPARAM>(label.c_str())));
                SendMessageW(settlement_combo, CB_SETITEMDATA, item_index, static_cast<LPARAM>(index + 1));
            }
            SendMessageW(settlement_combo, CB_SETCURSEL, none_index, 0);
            const HWND improvement_combo = GetDlgItem(window, kGreenbeltImprovement);
            SendMessageW(improvement_combo, CB_ADDSTRING, 0,
                reinterpret_cast<LPARAM>(L"(choose an improvement)"));
            for (const auto& item : greenbelt_catalog) {
                std::wstring label = widen(item.name) + (item.third_party ? L" [3pp/source-specific]" : L"");
                SendMessageW(improvement_combo, CB_ADDSTRING, 0,
                    reinterpret_cast<LPARAM>(label.c_str()));
            }
            for (const auto& item : greenbelt_draft.custom_improvements) {
                const std::wstring label = widen(item.name) + L" [custom/manual]";
                SendMessageW(improvement_combo, CB_ADDSTRING, 0,
                    reinterpret_cast<LPARAM>(label.c_str()));
            }
            SendMessageW(improvement_combo, CB_ADDSTRING, 0,
                reinterpret_cast<LPARAM>(L"Custom Improvement..."));
            SendMessageW(improvement_combo, CB_SETCURSEL, 0, 0);
            update_greenbelt_alignment_status(window);
            load_greenbelt_form(window);
            set_greenbelt_alignment_controls_visible(window, false);
            update_greenbelt_popout_scrollbar(window);
            return 0;
        }
        case WM_PAINT:
            paint_greenbelt_popout(window);
            return 0;
        case WM_SIZE:
            update_greenbelt_popout_scrollbar(window);
            InvalidateRect(window, nullptr, FALSE);
            return 0;
        case WM_VSCROLL: {
            SCROLLINFO info{sizeof(info), SIF_RANGE | SIF_PAGE | SIF_POS | SIF_TRACKPOS};
            GetScrollInfo(window, SB_VERT, &info);
            int position = info.nPos;
            switch (LOWORD(wparam)) {
                case SB_LINEUP: position -= 40; break;
                case SB_LINEDOWN: position += 40; break;
                case SB_PAGEUP: position -= static_cast<int>(info.nPage); break;
                case SB_PAGEDOWN: position += static_cast<int>(info.nPage); break;
                case SB_THUMBTRACK: position = info.nTrackPos; break;
                case SB_TOP: position = info.nMin; break;
                case SB_BOTTOM: position = info.nMax; break;
                default: break;
            }
            scroll_greenbelt_popout_to(window, position);
            return 0;
        }
        case WM_LBUTTONDOWN:
            greenbelt_popout_dragging = true;
            greenbelt_popout_drag_start = {static_cast<LONG>(static_cast<short>(LOWORD(lparam))),
                static_cast<LONG>(static_cast<short>(HIWORD(lparam)))};
            greenbelt_popout_drag_origin_x = greenbelt_popout_pan_x;
            greenbelt_popout_drag_origin_y = greenbelt_popout_pan_y;
            SetCapture(window);
            return 0;
        case WM_MOUSEMOVE:
            if (greenbelt_popout_dragging && (wparam & MK_LBUTTON)) {
                const LONG x = static_cast<LONG>(static_cast<short>(LOWORD(lparam)));
                const LONG y = static_cast<LONG>(static_cast<short>(HIWORD(lparam)));
                greenbelt_popout_pan_x = greenbelt_popout_drag_origin_x
                    + static_cast<double>(x - greenbelt_popout_drag_start.x);
                greenbelt_popout_pan_y = greenbelt_popout_drag_origin_y
                    + static_cast<double>(y - greenbelt_popout_drag_start.y);
                InvalidateRect(window, nullptr, FALSE);
            }
            return 0;
        case WM_LBUTTONUP:
            if (greenbelt_popout_dragging) {
                const LONG x = static_cast<LONG>(static_cast<short>(LOWORD(lparam)));
                const LONG y = static_cast<LONG>(static_cast<short>(HIWORD(lparam)));
                const bool click = std::abs(x - greenbelt_popout_drag_start.x) <= 3
                    && std::abs(y - greenbelt_popout_drag_start.y) <= 3;
                greenbelt_popout_dragging = false;
                ReleaseCapture();
                if (click) {
                    try {
                        handle_greenbelt_popout_click(window, lparam, false);
                    } catch (const std::exception& error) {
                        MessageBoxW(window, widen(error.what()).c_str(), L"Greenbelt Map", MB_OK | MB_ICONWARNING);
                    }
                }
            }
            return 0;
        case WM_LBUTTONDBLCLK:
            try {
                handle_greenbelt_popout_click(window, lparam, true);
            } catch (const std::exception& error) {
                MessageBoxW(window, widen(error.what()).c_str(), L"Greenbelt Map", MB_OK | MB_ICONWARNING);
            }
            return 0;
        case WM_COMMAND:
            if (LOWORD(wparam) == kGreenbeltZoomIn) {
                greenbelt_popout_zoom = std::min(4.0, greenbelt_popout_zoom * 1.25);
                InvalidateRect(window, nullptr, FALSE);
            } else if (LOWORD(wparam) == kGreenbeltZoomOut) {
                greenbelt_popout_zoom = std::max(0.35, greenbelt_popout_zoom / 1.25);
                InvalidateRect(window, nullptr, FALSE);
            } else if (LOWORD(wparam) == kGreenbeltZoomFit) {
                greenbelt_popout_zoom = 1.0;
                greenbelt_popout_pan_x = 0.0;
                greenbelt_popout_pan_y = 0.0;
                InvalidateRect(window, nullptr, FALSE);
            } else if (LOWORD(wparam) == kGreenbeltAlignMode) {
                greenbelt_alignment_mode = IsDlgButtonChecked(window, kGreenbeltAlignMode) == BST_CHECKED;
                set_greenbelt_alignment_controls_visible(window, greenbelt_alignment_mode);
                if (greenbelt_alignment_mode)
                    update_greenbelt_alignment_status(window);
            } else if (LOWORD(wparam) == kGreenbeltPanLeft) {
                adjust_greenbelt_alignment(window, -1, 0, 0.0, 0.0);
            } else if (LOWORD(wparam) == kGreenbeltPanRight) {
                adjust_greenbelt_alignment(window, 1, 0, 0.0, 0.0);
            } else if (LOWORD(wparam) == kGreenbeltPanUp) {
                adjust_greenbelt_alignment(window, 0, -1, 0.0, 0.0);
            } else if (LOWORD(wparam) == kGreenbeltPanDown) {
                adjust_greenbelt_alignment(window, 0, 1, 0.0, 0.0);
            } else if (LOWORD(wparam) == kGreenbeltGrowWidth) {
                adjust_greenbelt_alignment(window, 0, 0, 1.0, 0.0);
            } else if (LOWORD(wparam) == kGreenbeltShrinkWidth) {
                adjust_greenbelt_alignment(window, 0, 0, -1.0, 0.0);
            } else if (LOWORD(wparam) == kGreenbeltGrowHeight) {
                adjust_greenbelt_alignment(window, 0, 0, 0.0, 1.0);
            } else if (LOWORD(wparam) == kGreenbeltShrinkHeight) {
                adjust_greenbelt_alignment(window, 0, 0, 0.0, -1.0);
            } else if (LOWORD(wparam) == kGreenbeltResetPan) {
                greenbelt_popout_pan_x = 0.0;
                greenbelt_popout_pan_y = 0.0;
                InvalidateRect(window, nullptr, FALSE);
            } else if (LOWORD(wparam) == kGreenbeltApply) {
                try {
                    apply_greenbelt(window);
                } catch (const std::exception& error) {
                    MessageBoxW(window, widen(error.what()).c_str(), L"Greenbelt Map", MB_OK | MB_ICONWARNING);
                }
            } else if (LOWORD(wparam) == kGreenbeltHexList && HIWORD(wparam) == LBN_SELCHANGE) {
                try {
                    save_greenbelt_form(window);
                    greenbelt_active_index = static_cast<int>(SendMessageW(
                        GetDlgItem(window, kGreenbeltHexList), LB_GETCURSEL, 0, 0));
                    load_greenbelt_form(window);
                    InvalidateRect(window, nullptr, FALSE);
                } catch (const std::exception& error) {
                    MessageBoxW(window, widen(error.what()).c_str(), L"Greenbelt Map", MB_OK | MB_ICONWARNING);
                }
            } else if (LOWORD(wparam) == kGreenbeltAddImprovement) {
                try {
                    add_greenbelt_improvement(window);
                } catch (const std::exception& error) {
                    MessageBoxW(window, widen(error.what()).c_str(), L"Greenbelt Map", MB_OK | MB_ICONWARNING);
                }
            } else if (LOWORD(wparam) == kGreenbeltRefreshPreview) {
                try {
                    save_greenbelt_form(window);
                    refresh_greenbelt_preview(window);
                } catch (const std::exception& error) {
                    MessageBoxW(window, widen(error.what()).c_str(), L"Greenbelt Map", MB_OK | MB_ICONWARNING);
                }
            } else if (LOWORD(wparam) == kGreenbeltImprovement && HIWORD(wparam) == CBN_SELCHANGE) {
                try {
                    const int selected = static_cast<int>(SendMessageW(
                        GetDlgItem(window, kGreenbeltImprovement), CB_GETCURSEL, 0, 0));
                    const int custom_start = static_cast<int>(greenbelt_catalog.size()) + 1;
                    const int custom_end = custom_start
                        + static_cast<int>(greenbelt_draft.custom_improvements.size());
                    if (selected >= custom_start && selected < custom_end) {
                        save_greenbelt_form(window);
                        SendMessageW(GetDlgItem(window, kGreenbeltImprovement), CB_SETCURSEL, 0, 0);
                        open_greenbelt_custom_popup(window, selected - custom_start);
                    } else if (selected == custom_end) {
                        save_greenbelt_form(window);
                        SendMessageW(GetDlgItem(window, kGreenbeltImprovement), CB_SETCURSEL, 0, 0);
                        open_greenbelt_custom_popup(window);
                    } else {
                        save_greenbelt_form(window);
                        refresh_greenbelt_preview(window);
                    }
                } catch (const std::exception& error) {
                    MessageBoxW(window, widen(error.what()).c_str(), L"Greenbelt Map", MB_OK | MB_ICONWARNING);
                }
            } else if (LOWORD(wparam) == kGreenbeltSettlement && HIWORD(wparam) == CBN_SELCHANGE) {
                try {
                    save_greenbelt_form(window);
                    refresh_greenbelt_preview(window);
                    InvalidateRect(window, nullptr, FALSE);
                } catch (const std::exception& error) {
                    MessageBoxW(window, widen(error.what()).c_str(), L"Greenbelt Map", MB_OK | MB_ICONWARNING);
                }
            } else if (LOWORD(wparam) == kGreenbeltPopoutClose) {
                DestroyWindow(window);
            }
            return 0;
        case WM_CLOSE:
            DestroyWindow(window);
            return 0;
        case WM_DESTROY:
            if (greenbelt_custom_popup_window && IsWindow(greenbelt_custom_popup_window))
                DestroyWindow(greenbelt_custom_popup_window);
            greenbelt_popout_window = nullptr;
            return 0;
    }
    return DefWindowProcW(window, message, wparam, lparam);
}

LRESULT CALLBACK greenbelt_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
    if (message == WM_GETMINMAXINFO) {
        set_minimum_track_size(lparam, 1400, 850);
        return 0;
    }
    switch (message) {
        case WM_PAINT:
            paint_greenbelt_map(window);
            return 0;
        case WM_CREATE:
            create_greenbelt_controls(window);
            update_greenbelt_scrollbar(window);
            return 0;
        case WM_SIZE:
            update_greenbelt_scrollbar(window);
            return 0;
        case WM_VSCROLL: {
            SCROLLINFO info{sizeof(info), SIF_RANGE | SIF_PAGE | SIF_POS | SIF_TRACKPOS};
            GetScrollInfo(window, SB_VERT, &info);
            int position = info.nPos;
            switch (LOWORD(wparam)) {
                case SB_LINEUP: position -= 40; break;
                case SB_LINEDOWN: position += 40; break;
                case SB_PAGEUP: position -= static_cast<int>(info.nPage); break;
                case SB_PAGEDOWN: position += static_cast<int>(info.nPage); break;
                case SB_THUMBTRACK: position = info.nTrackPos; break;
                case SB_TOP: position = info.nMin; break;
                case SB_BOTTOM: position = info.nMax; break;
                default: return 0;
            }
            scroll_greenbelt_to(window, position);
            return 0;
        }
        case WM_MOUSEWHEEL: {
            const int wheel_delta = GET_WHEEL_DELTA_WPARAM(wparam);
            scroll_greenbelt_to(window, greenbelt_scroll_y
                - (wheel_delta / WHEEL_DELTA) * 80);
            return 0;
        }
        case WM_LBUTTONDOWN:
            try {
                handle_greenbelt_map_click(window, lparam, false);
            } catch (const std::exception& error) {
                MessageBoxW(window, widen(error.what()).c_str(), L"Greenbelt Map", MB_OK | MB_ICONWARNING);
            }
            return 0;
        case WM_LBUTTONDBLCLK:
            try {
                handle_greenbelt_map_click(window, lparam, true);
            } catch (const std::exception& error) {
                MessageBoxW(window, widen(error.what()).c_str(), L"Greenbelt Map", MB_OK | MB_ICONWARNING);
            }
            return 0;
        case WM_COMMAND: {
            const int id = LOWORD(wparam);
            try {
                if (id == kGreenbeltHexList && HIWORD(wparam) == LBN_SELCHANGE) {
                    save_greenbelt_form(window);
                    greenbelt_active_index = static_cast<int>(SendMessageW(
                        GetDlgItem(window, kGreenbeltHexList), LB_GETCURSEL, 0, 0));
                    load_greenbelt_form(window);
                } else if (id == kGreenbeltOpenMap) {
                    open_greenbelt_popout(window);
                } else if (id == kGreenbeltSpecialList && HIWORD(wparam) == LBN_SELCHANGE) {
                    greenbelt_special_active_index = static_cast<int>(SendMessageW(
                        GetDlgItem(window, kGreenbeltSpecialList), LB_GETCURSEL, 0, 0));
                    load_greenbelt_special_form(window);
                } else if (id == kGreenbeltSpecialSave) {
                    save_greenbelt_special_form(window);
                } else if (id == kGreenbeltSpecialRemove) {
                    remove_greenbelt_special(window);
                } else if (id == kGreenbeltCustomList && HIWORD(wparam) == LBN_SELCHANGE) {
                    greenbelt_custom_active_index = static_cast<int>(SendMessageW(
                        GetDlgItem(window, kGreenbeltCustomList), LB_GETCURSEL, 0, 0));
                    load_greenbelt_custom_form(window);
                } else if (id == kGreenbeltCustomSave) {
                    save_greenbelt_custom_form(window);
                } else if (id == kGreenbeltCustomRemove) {
                    remove_greenbelt_custom(window);
                } else if (id == kGreenbeltAddImprovement) {
                    add_greenbelt_improvement(window);
                } else if (id == kGreenbeltImprovement && HIWORD(wparam) == CBN_SELCHANGE) {
                    save_greenbelt_form(window);
                    refresh_greenbelt_preview(window);
                } else if (id == kGreenbeltSettlement && HIWORD(wparam) == CBN_SELCHANGE) {
                    save_greenbelt_form(window);
                    refresh_greenbelt_preview(window);
                    InvalidateRect(window, nullptr, FALSE);
                } else if (id == kGreenbeltApply) {
                    apply_greenbelt(window);
                } else if (id == kGreenbeltCancel) {
                    cancel_greenbelt(window);
                } else if (id == kGreenbeltClose) {
                    DestroyWindow(window);
                }
            } catch (const std::exception& error) {
                MessageBoxW(window, widen(error.what()).c_str(), L"Greenbelt Map", MB_OK | MB_ICONWARNING);
            }
            return 0;
        }
        case WM_CLOSE:
            DestroyWindow(window);
            return 0;
        case WM_DESTROY: {
            if (greenbelt_popout_window && IsWindow(greenbelt_popout_window))
                DestroyWindow(greenbelt_popout_window);
            greenbelt_window = nullptr;
            greenbelt_scroll_y = 0;
            greenbelt_draft = {};
            greenbelt_catalog.clear();
            greenbelt_active_index = -1;
            greenbelt_special_active_index = -1;
            greenbelt_custom_active_index = -1;
            const HWND owner = GetWindow(window, GW_OWNER);
            if (owner && IsWindow(owner)) {
                EnableWindow(owner, TRUE);
                SetForegroundWindow(owner);
            }
            return 0;
        }
    }
    return DefWindowProcW(window, message, wparam, lparam);
}

bool calendar_note_is_empty(const pkkm::CalendarNote& note) {
    return note.holiday.empty() && note.kingdom_upgrades.empty()
        && note.events.empty() && note.other.empty();
}

const pkkm::CalendarNote* find_calendar_note(const std::vector<pkkm::CalendarNote>& notes,
                                             const std::string& entry_id) {
    const auto found = std::find_if(notes.begin(), notes.end(), [&entry_id](const auto& note) {
        return note.entry_id == entry_id;
    });
    return found == notes.end() ? nullptr : &*found;
}

void load_calendar_entry(HWND window, int index) {
    const auto& entries = pkkm::calendar_template_entries();
    if (index < 0 || index >= static_cast<int>(entries.size())) return;
    const auto& entry = entries[static_cast<size_t>(index)];
    std::wstring title = L"Source row " + std::to_wstring(entry.source_row) + L" - ";
    if (!entry.year_marker.empty()) title += L"[" + widen(entry.year_marker) + L"] ";
    title += widen(entry.month);
    SetWindowTextW(GetDlgItem(window, kCalendarEntryTitle), title.c_str());
    const auto* note = find_calendar_note(calendar_note_draft, entry.id);
    set_manager_text(window, kCalendarHoliday, note ? widen(note->holiday) : L"");
    set_manager_text(window, kCalendarUpgrades, note ? widen(note->kingdom_upgrades) : L"");
    set_manager_text(window, kCalendarEvents, note ? widen(note->events) : L"");
    set_manager_text(window, kCalendarOther, note ? widen(note->other) : L"");
}

void store_calendar_form(HWND window) {
    const auto& entries = pkkm::calendar_template_entries();
    if (calendar_active_index < 0 || calendar_active_index >= static_cast<int>(entries.size())) return;
    const auto& entry = entries[static_cast<size_t>(calendar_active_index)];
    pkkm::CalendarNote note{entry.id, narrow(manager_text(window, kCalendarHoliday)),
        narrow(manager_text(window, kCalendarUpgrades)), narrow(manager_text(window, kCalendarEvents)),
        narrow(manager_text(window, kCalendarOther))};
    auto found = std::find_if(calendar_note_draft.begin(), calendar_note_draft.end(), [&entry](const auto& item) {
        return item.entry_id == entry.id;
    });
    if (calendar_note_is_empty(note)) {
        if (found != calendar_note_draft.end()) calendar_note_draft.erase(found);
    } else if (found == calendar_note_draft.end()) {
        calendar_note_draft.push_back(std::move(note));
    } else {
        *found = std::move(note);
    }
}

void create_calendar_controls(HWND window) {
    CreateWindowExW(0, L"STATIC",
        L"Partial source template: preserve listed order and literal year markers. Notes do not infer dates or advance turns.",
        WS_CHILD | WS_VISIBLE | SS_LEFT, 18, 10, 995, 28, window, nullptr, app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"Source month entries", WS_CHILD | WS_VISIBLE | SS_LEFT,
        20, 44, 290, 22, window, nullptr, app_instance, nullptr);
    CreateWindowExW(WS_EX_CLIENTEDGE, L"LISTBOX", L"",
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | LBS_NOTIFY, 20, 68, 300, 570, window,
        reinterpret_cast<HMENU>(static_cast<INT_PTR>(kCalendarEntryList)), app_instance, nullptr);
    CreateWindowExW(0, L"STATIC", L"", WS_CHILD | WS_VISIBLE | SS_LEFT,
        350, 44, 650, 28, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kCalendarEntryTitle)), app_instance, nullptr);
    const auto add_note = [window](const wchar_t* label, int id, int y) {
        CreateWindowExW(0, L"STATIC", label, WS_CHILD | WS_VISIBLE | SS_LEFT,
            350, y, 650, 20, window, nullptr, app_instance, nullptr);
        CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN,
            350, y + 22, 650, 96, window,
            reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), app_instance, nullptr);
    };
    add_note(L"Holiday", kCalendarHoliday, 82);
    add_note(L"Kingdom Upgrades", kCalendarUpgrades, 207);
    add_note(L"Events", kCalendarEvents, 332);
    add_note(L"Other", kCalendarOther, 457);
    CreateWindowExW(0, L"BUTTON", L"Apply", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        712, 635, 88, 30, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kCalendarApply)), app_instance, nullptr);
    CreateWindowExW(0, L"BUTTON", L"Cancel", WS_CHILD | WS_VISIBLE,
        810, 635, 88, 30, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kCalendarCancel)), app_instance, nullptr);
    CreateWindowExW(0, L"BUTTON", L"Close", WS_CHILD | WS_VISIBLE,
        908, 635, 88, 30, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kCalendarClose)), app_instance, nullptr);
    const HWND list = GetDlgItem(window, kCalendarEntryList);
    const auto& entries = pkkm::calendar_template_entries();
    for (const auto& entry : entries) {
        std::wstring label = L"Row " + std::to_wstring(entry.source_row) + L" - ";
        if (!entry.year_marker.empty()) label += L"[" + widen(entry.year_marker) + L"] ";
        label += widen(entry.month);
        SendMessageW(list, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(label.c_str()));
    }
    if (!entries.empty()) {
        calendar_active_index = 0;
        SendMessageW(list, LB_SETCURSEL, 0, 0);
        load_calendar_entry(window, calendar_active_index);
    }
}

void apply_calendar_notes(HWND window) {
    store_calendar_form(window);
    std::vector<pkkm::CalendarNote> updated;
    for (const auto& entry : pkkm::calendar_template_entries()) {
        if (const auto* note = find_calendar_note(calendar_note_draft, entry.id))
            if (!calendar_note_is_empty(*note)) updated.push_back(*note);
    }
    const auto equal_note = [](const auto& left, const auto& right) {
        return left.entry_id == right.entry_id && left.holiday == right.holiday
            && left.kingdom_upgrades == right.kingdom_upgrades && left.events == right.events
            && left.other == right.other;
    };
    const bool unchanged = updated.size() == kingdom.calendar_notes.size()
        && std::equal(updated.begin(), updated.end(), kingdom.calendar_notes.begin(), equal_note);
    if (!unchanged) {
        pkkm::Kingdom candidate = kingdom;
        candidate.calendar_notes = updated;
        pkkm::validate(candidate);
        kingdom.calendar_notes = std::move(updated);
        dirty = true;
        calendar_note_draft = kingdom.calendar_notes;
        const HWND owner = GetWindow(window, GW_OWNER);
        if (owner) refresh(GetDlgItem(owner, kDisplay));
    }
}

void cancel_calendar_notes(HWND window) {
    calendar_note_draft = kingdom.calendar_notes;
    if (calendar_active_index >= 0) load_calendar_entry(window, calendar_active_index);
}

LRESULT CALLBACK calendar_tracker_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
    if (message == WM_GETMINMAXINFO) {
        set_minimum_track_size(lparam, 760, 560);
        return 0;
    }
    switch (message) {
        case WM_CREATE:
            create_calendar_controls(window);
            return 0;
        case WM_COMMAND: {
            const int id = LOWORD(wparam);
            if (id == kCalendarEntryList && HIWORD(wparam) == LBN_SELCHANGE) {
                const int selected = static_cast<int>(SendMessageW(
                    GetDlgItem(window, kCalendarEntryList), LB_GETCURSEL, 0, 0));
                if (selected >= 0 && selected != calendar_active_index) {
                    store_calendar_form(window);
                    calendar_active_index = selected;
                    load_calendar_entry(window, selected);
                }
            } else if (id == kCalendarApply) {
                try { apply_calendar_notes(window); }
                catch (const std::exception& error) {
                    MessageBoxW(window, widen(error.what()).c_str(), L"Calendar notes", MB_OK | MB_ICONWARNING);
                }
            } else if (id == kCalendarCancel) {
                cancel_calendar_notes(window);
            } else if (id == kCalendarClose) {
                DestroyWindow(window);
            }
            return 0;
        }
        case WM_CLOSE:
            DestroyWindow(window);
            return 0;
        case WM_DESTROY: {
            calendar_window = nullptr;
            calendar_note_draft.clear();
            calendar_active_index = -1;
            const HWND owner = GetWindow(window, GW_OWNER);
            if (owner && IsWindow(owner)) {
                EnableWindow(owner, TRUE);
                SetForegroundWindow(owner);
            }
            return 0;
        }
    }
    return DefWindowProcW(window, message, wparam, lparam);
}

LRESULT CALLBACK turn_checklist_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
    if (message == WM_GETMINMAXINFO) {
        set_minimum_track_size(lparam, 760, 460);
        return 0;
    }
    switch (message) {
        case WM_CREATE:
            create_turn_checklist_controls(window);
            refresh_turn_transition_preview(window);
            SetWindowTextW(window, (L"Turn Checklist - Turn " + std::to_wstring(kingdom.turn)).c_str());
            return 0;
        case WM_COMMAND:
            if (LOWORD(wparam) == kChecklistEventLastTurn && HIWORD(wparam) == BN_CLICKED)
                refresh_turn_transition_preview(window);
            else if (LOWORD(wparam) == kChecklistApply) apply_turn_checklist(window);
            else if (LOWORD(wparam) == kChecklistCancel) {
                refresh_turn_checklist_controls();
                CheckDlgButton(window, kChecklistEventLastTurn, BST_UNCHECKED);
                refresh_turn_transition_preview(window);
            }
            else if (LOWORD(wparam) == kChecklistClose) DestroyWindow(window);
            return 0;
        case WM_CLOSE:
            DestroyWindow(window);
            return 0;
        case WM_DESTROY: {
            checklist_window = nullptr;
            checklist_controls.clear();
            checklist_step_ids.clear();
            const HWND owner = GetWindow(window, GW_OWNER);
            if (owner && IsWindow(owner)) {
                EnableWindow(owner, TRUE);
                SetForegroundWindow(owner);
            }
            return 0;
        }
    }
    return DefWindowProcW(window, message, wparam, lparam);
}

LRESULT CALLBACK editor_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
    if (message == WM_GETMINMAXINFO) {
        set_minimum_track_size(lparam, 760, 520);
        return 0;
    }
    switch (message) {
        case WM_CREATE:
            create_editor_controls(window);
            return 0;
        case WM_COMMAND:
            if (LOWORD(wparam) == kEditorApply) apply_editor(window);
            else if (LOWORD(wparam) == kEditorCancel) cancel_editor();
            else if (LOWORD(wparam) == kEditorClose) DestroyWindow(window);
            return 0;
        case WM_CLOSE:
            DestroyWindow(window);
            return 0;
        case WM_DESTROY: {
            editor_window = nullptr;
            const HWND owner = GetWindow(window, GW_OWNER);
            if (owner && IsWindow(owner)) {
                EnableWindow(owner, TRUE);
                SetForegroundWindow(owner);
            }
            return 0;
        }
    }
    return DefWindowProcW(window, message, wparam, lparam);
}

LRESULT CALLBACK main_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
    if (message == WM_GETMINMAXINFO) {
        set_minimum_track_size(lparam, 700, 500);
        return 0;
    }
    switch (message) {
        case WM_CREATE: {
            HMENU menu = CreateMenu();
            HMENU file = CreatePopupMenu();
            AppendMenuW(file, MF_STRING, kNew, L"New");
            AppendMenuW(file, MF_STRING, kOpen, L"Open...");
            AppendMenuW(file, MF_STRING, kSave, L"Save");
            AppendMenuW(menu, MF_POPUP, reinterpret_cast<UINT_PTR>(file), L"File");
            HMENU manager = CreatePopupMenu();
            AppendMenuW(manager, MF_STRING, kEditInputs, L"Edit Inputs...");
            AppendMenuW(manager, MF_STRING, kManageSettlements, L"Manage Settlements...");
            AppendMenuW(manager, MF_STRING, kTurnChecklist, L"Turn Checklist...");
            AppendMenuW(manager, MF_STRING, kTurnActions, L"Turn Actions...");
            AppendMenuW(manager, MF_STRING, kCalendarTracker, L"Calendar Notes...");
            AppendMenuW(manager, MF_STRING, kGreenbeltMap, L"Greenbelt Map Editor...");
            AppendMenuW(menu, MF_POPUP, reinterpret_cast<UINT_PTR>(manager), L"Kingdom Manager");
            SetMenu(window, menu);
            CreateWindowExW(0, L"STATIC", L"", WS_CHILD | WS_VISIBLE | SS_LEFT,
                24, 24, 720, 520, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kDisplay)), app_instance, nullptr);
            CreateWindowExW(0, L"BUTTON", L"Advance Turn...", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
                24, 555, 150, 30, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kAdvanceTurn)), app_instance, nullptr);
            refresh(GetDlgItem(window, kDisplay));
            return 0;
        }
        case WM_COMMAND:
            command(window, LOWORD(wparam));
            return 0;
        case WM_DESTROY:
            if (editor_window && IsWindow(editor_window)) DestroyWindow(editor_window);
            if (settlement_window && IsWindow(settlement_window)) DestroyWindow(settlement_window);
            if (checklist_window && IsWindow(checklist_window)) DestroyWindow(checklist_window);
            if (action_window && IsWindow(action_window)) DestroyWindow(action_window);
            if (calendar_window && IsWindow(calendar_window)) DestroyWindow(calendar_window);
            if (greenbelt_window && IsWindow(greenbelt_window)) DestroyWindow(greenbelt_window);
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(window, message, wparam, lparam);
}
}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int) {
    app_instance = instance;
    Gdiplus::GdiplusStartupInput gdiplus_input;
    if (Gdiplus::GdiplusStartup(&gdiplus_token, &gdiplus_input, nullptr) != Gdiplus::Ok)
        return 1;
    try {
        building_catalog = pkkm::load_building_catalog(bundled_catalog_path());
    } catch (const std::exception& error) {
        MessageBoxW(nullptr, widen(error.what()).c_str(), L"PKKM catalog error", MB_OK | MB_ICONERROR);
        Gdiplus::GdiplusShutdown(gdiplus_token);
        return 1;
    }
    WNDCLASSW main_class{};
    main_class.lpfnWndProc = main_proc;
    main_class.hInstance = instance;
    main_class.lpszClassName = L"PKKMWindow";
    main_class.hCursor = LoadCursor(nullptr, IDC_ARROW);
    main_class.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    if (!RegisterClassW(&main_class)) {
        Gdiplus::GdiplusShutdown(gdiplus_token);
        return 1;
    }

    WNDCLASSW editor_class{};
    editor_class.lpfnWndProc = editor_proc;
    editor_class.hInstance = instance;
    editor_class.lpszClassName = L"PKKMEditor";
    editor_class.hCursor = LoadCursor(nullptr, IDC_ARROW);
    editor_class.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    if (!RegisterClassW(&editor_class)) {
        Gdiplus::GdiplusShutdown(gdiplus_token);
        return 1;
    }

    WNDCLASSW settlement_class{};
    settlement_class.lpfnWndProc = settlement_manager_proc;
    settlement_class.hInstance = instance;
    settlement_class.lpszClassName = L"PKKMSettlements";
    settlement_class.hCursor = LoadCursor(nullptr, IDC_ARROW);
    settlement_class.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    if (!RegisterClassW(&settlement_class)) {
        Gdiplus::GdiplusShutdown(gdiplus_token);
        return 1;
    }

    WNDCLASSW checklist_class{};
    checklist_class.lpfnWndProc = turn_checklist_proc;
    checklist_class.hInstance = instance;
    checklist_class.lpszClassName = L"PKKMTurnChecklist";
    checklist_class.hCursor = LoadCursor(nullptr, IDC_ARROW);
    checklist_class.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    if (!RegisterClassW(&checklist_class)) {
        Gdiplus::GdiplusShutdown(gdiplus_token);
        return 1;
    }

    WNDCLASSW action_class{};
    action_class.lpfnWndProc = turn_actions_proc;
    action_class.hInstance = instance;
    action_class.lpszClassName = L"PKKMTurnActions";
    action_class.hCursor = LoadCursor(nullptr, IDC_ARROW);
    action_class.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    if (!RegisterClassW(&action_class)) {
        Gdiplus::GdiplusShutdown(gdiplus_token);
        return 1;
    }

    WNDCLASSW calendar_class{};
    calendar_class.lpfnWndProc = calendar_tracker_proc;
    calendar_class.hInstance = instance;
    calendar_class.lpszClassName = L"PKKMCalendarTracker";
    calendar_class.hCursor = LoadCursor(nullptr, IDC_ARROW);
    calendar_class.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    if (!RegisterClassW(&calendar_class)) {
        Gdiplus::GdiplusShutdown(gdiplus_token);
        return 1;
    }

    WNDCLASSW greenbelt_class{};
    greenbelt_class.style = CS_DBLCLKS;
    greenbelt_class.lpfnWndProc = greenbelt_proc;
    greenbelt_class.hInstance = instance;
    greenbelt_class.lpszClassName = L"PKKMGreenbeltMap";
    greenbelt_class.hCursor = LoadCursor(nullptr, IDC_ARROW);
    greenbelt_class.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    if (!RegisterClassW(&greenbelt_class)) {
        Gdiplus::GdiplusShutdown(gdiplus_token);
        return 1;
    }

    WNDCLASSW greenbelt_popout_class{};
    greenbelt_popout_class.style = CS_DBLCLKS;
    greenbelt_popout_class.lpfnWndProc = greenbelt_popout_proc;
    greenbelt_popout_class.hInstance = instance;
    greenbelt_popout_class.lpszClassName = L"PKKMGreenbeltMapPopout";
    greenbelt_popout_class.hCursor = LoadCursor(nullptr, IDC_ARROW);
    greenbelt_popout_class.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    if (!RegisterClassW(&greenbelt_popout_class)) {
        Gdiplus::GdiplusShutdown(gdiplus_token);
        return 1;
    }

    WNDCLASSW greenbelt_custom_popup_class{};
    greenbelt_custom_popup_class.lpfnWndProc = greenbelt_custom_popup_proc;
    greenbelt_custom_popup_class.hInstance = instance;
    greenbelt_custom_popup_class.lpszClassName = L"PKKMGreenbeltCustomImprovement";
    greenbelt_custom_popup_class.hCursor = LoadCursor(nullptr, IDC_ARROW);
    greenbelt_custom_popup_class.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    if (!RegisterClassW(&greenbelt_custom_popup_class)) {
        Gdiplus::GdiplusShutdown(gdiplus_token);
        return 1;
    }

    HWND window = CreateWindowW(main_class.lpszClassName, L"PKKM - Kingdom Manager",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 800, 620,
        nullptr, nullptr, instance, nullptr);
    if (!window) {
        Gdiplus::GdiplusShutdown(gdiplus_token);
        return 1;
    }
    ShowWindow(window, SW_SHOW);
    MSG message{};
    while (GetMessageW(&message, nullptr, 0, 0) > 0) {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }
    const int result = static_cast<int>(message.wParam);
    Gdiplus::GdiplusShutdown(gdiplus_token);
    return result;
}
