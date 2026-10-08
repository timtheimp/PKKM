#define UNICODE
#define _UNICODE
#define NOMINMAX
#include <windows.h>
#include <commdlg.h>
#include "pkkm/kingdom.hpp"
#include "pkkm/buildings.hpp"
#include "pkkm/map_editor.hpp"
#include <algorithm>
#include <filesystem>
#include <functional>
#include <limits>
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
constexpr int kDisplay = 10;
constexpr int kEditorSave = 3001;
constexpr int kEditorCancel = 3002;
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
constexpr int kEditMap = 5119;
constexpr int kMapSummary = 5120;
constexpr int kChecklistApply = 6201;
constexpr int kChecklistCancel = 6202;
constexpr int kChecklistControlBase = 6300;
constexpr int kCalendarEntryList = 7100;
constexpr int kCalendarEntryTitle = 7101;
constexpr int kCalendarHoliday = 7102;
constexpr int kCalendarUpgrades = 7103;
constexpr int kCalendarEvents = 7104;
constexpr int kCalendarOther = 7105;
constexpr int kCalendarApply = 7201;
constexpr int kCalendarCancel = 7202;

pkkm::Kingdom kingdom;
pkkm::Kingdom edit_draft;
std::wstring current_path;
HWND editor_window = nullptr;
HWND settlement_window = nullptr;
HWND checklist_window = nullptr;
HWND calendar_window = nullptr;
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

int parse_integer(const std::wstring& text) {
    size_t consumed = 0;
    const long long parsed = std::stoll(text, &consumed);
    if (consumed != text.size() || parsed < INT_MIN || parsed > INT_MAX) {
        throw std::invalid_argument("Enter a whole number within the supported range.");
    }
    return static_cast<int>(parsed);
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
    std::wstring text = widen(kingdom.name) + L" — Turn " + std::to_wstring(kingdom.turn)
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
        text += L"• " + widen(settlement.name) + L" — population "
            + std::to_wstring(size.population) + L", districts " + std::to_wstring(size.districts)
            + L", lots " + std::to_wstring(size.lots)
            + L", Economy " + signed_component(settlement.economy)
            + L", Loyalty " + signed_component(settlement.loyalty)
            + L", Stability " + signed_component(settlement.stability)
            + L", Defense " + signed_component(settlement.defense) + L"\r\n";
        if (!settlement.building_inventory.empty()) {
            text += L"  Building inventory: ";
            for (size_t index = 0; index < settlement.building_inventory.size(); ++index) {
                if (index > 0) text += L", ";
                const auto& building = settlement.building_inventory[index];
                text += widen(building.name) + L" x " + std::to_wstring(building.count);
            }
            text += L"\r\n";
            const auto effects = pkkm::calculate_settlement_building_totals(settlement, building_catalog);
            if (settlement.apply_catalog_stat_effects) {
                text += L"  Catalog stat bonuses applied to kingdom totals: Economy " + signed_component(effects.economy)
                    + L" | Loyalty " + signed_component(effects.loyalty)
                    + L" | Stability " + signed_component(effects.stability)
                    + L" | Defense/Unrest preview only: Defense " + signed_component(effects.defense)
                    + L" | Unrest " + signed_component(effects.unrest) + L"\r\n";
            } else {
                text += L"  Catalog effects preview (not applied): Economy " + signed_component(effects.economy)
                    + L" | Loyalty " + signed_component(effects.loyalty)
                    + L" | Stability " + signed_component(effects.stability)
                    + L" | Defense " + signed_component(effects.defense)
                    + L" | Unrest " + signed_component(effects.unrest) + L"\r\n";
            }
        }
    }
    SetWindowTextW(display, text.c_str());
    const HWND main = GetParent(display);
    SetWindowTextW(main, dirty ? L"PKKM — Kingdom Manager *" : L"PKKM — Kingdom Manager");
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
        810, 615, 88, 30, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kEditorSave)), app_instance, nullptr);
    CreateWindowExW(0, L"BUTTON", L"Cancel", WS_CHILD | WS_VISIBLE,
        908, 615, 88, 30, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kEditorCancel)), app_instance, nullptr);
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
        refresh(GetDlgItem(GetParent(window), kDisplay));
        DestroyWindow(window);
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

void manager_label(HWND window, const wchar_t* text, int x, int y, int width = 130) {
    CreateWindowExW(0, L"STATIC", text, WS_CHILD | WS_VISIBLE | SS_LEFT,
        x, y + 4, width, 22, window, nullptr, app_instance, nullptr);
}

void manager_edit(HWND window, int id, const wchar_t* initial, int x, int y, int width = 130) {
    CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", initial,
        WS_CHILD | WS_VISIBLE | ES_AUTOHSCROLL, x, y, width, 24, window,
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
        830, 575, 88, 30, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kApplySettlements)), app_instance, nullptr);
    CreateWindowExW(0, L"BUTTON", L"Cancel", WS_CHILD | WS_VISIBLE,
        928, 575, 88, 30, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kCancelSettlements)), app_instance, nullptr);
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
        ? L"Map: " + std::to_wstring(settlement.map.rows) + L"×" + std::to_wstring(settlement.map.columns)
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
        pkkm::Kingdom candidate = kingdom;
        candidate.settlements = settlement_draft;
        pkkm::validate(candidate);
        kingdom.settlements = std::move(settlement_draft);
        dirty = true;
        const HWND owner = GetWindow(window, GW_OWNER);
        if (owner) refresh(GetDlgItem(owner, kDisplay));
        DestroyWindow(window);
    } catch (const std::exception& error) {
        MessageBoxW(window, widen(error.what()).c_str(), L"Invalid settlement data", MB_OK | MB_ICONWARNING);
    }
}

LRESULT CALLBACK settlement_manager_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
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
                else if (id == kCancelSettlements) DestroyWindow(window);
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
        } else if (id == kEditInputs) {
            if (editor_window && IsWindow(editor_window)) {
                SetForegroundWindow(editor_window);
                return;
            }
            prepare_editor_fields();
            editor_window = CreateWindowExW(WS_EX_DLGMODALFRAME, L"PKKMEditor", L"Edit Kingdom Inputs",
                WS_CAPTION | WS_SYSMENU | WS_POPUP, CW_USEDEFAULT, CW_USEDEFAULT, 1025, 700,
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
                WS_CAPTION | WS_SYSMENU | WS_POPUP, CW_USEDEFAULT, CW_USEDEFAULT, 1075, 660,
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
                L"Turn Checklist", WS_CAPTION | WS_SYSMENU | WS_POPUP,
                CW_USEDEFAULT, CW_USEDEFAULT, 1010, 620, window, nullptr, app_instance, nullptr);
            if (!checklist_window) throw std::runtime_error("Could not open the turn checklist");
            EnableWindow(window, FALSE);
            ShowWindow(checklist_window, SW_SHOW);
            UpdateWindow(checklist_window);
        } else if (id == kCalendarTracker) {
            if (calendar_window && IsWindow(calendar_window)) {
                SetForegroundWindow(calendar_window);
                return;
            }
            calendar_note_draft = kingdom.calendar_notes;
            calendar_active_index = -1;
            calendar_window = CreateWindowExW(WS_EX_DLGMODALFRAME, L"PKKMCalendarTracker",
                L"Calendar Notes", WS_CAPTION | WS_SYSMENU | WS_POPUP,
                CW_USEDEFAULT, CW_USEDEFAULT, 1050, 760, window, nullptr, app_instance, nullptr);
            if (!calendar_window) throw std::runtime_error("Could not open the calendar notes tracker");
            EnableWindow(window, FALSE);
            ShowWindow(calendar_window, SW_SHOW);
            UpdateWindow(calendar_window);
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
        L"Manual checklist only; PKKM does not resolve checks or turn outcomes.\r\nApply updates this file in memory; use File > Save to persist. Stability failure by exactly 4 is unspecified in the source.",
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
    CreateWindowExW(0, L"BUTTON", L"Apply", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        790, 535, 88, 30, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kChecklistApply)), app_instance, nullptr);
    CreateWindowExW(0, L"BUTTON", L"Cancel", WS_CHILD | WS_VISIBLE,
        888, 535, 88, 30, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kChecklistCancel)), app_instance, nullptr);
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
    DestroyWindow(window);
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
        810, 635, 88, 30, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kCalendarApply)), app_instance, nullptr);
    CreateWindowExW(0, L"BUTTON", L"Cancel", WS_CHILD | WS_VISIBLE,
        910, 635, 88, 30, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kCalendarCancel)), app_instance, nullptr);
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
        const HWND owner = GetWindow(window, GW_OWNER);
        if (owner) refresh(GetDlgItem(owner, kDisplay));
    }
    DestroyWindow(window);
}

LRESULT CALLBACK calendar_tracker_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
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
    switch (message) {
        case WM_CREATE:
            create_turn_checklist_controls(window);
            SetWindowTextW(window, (L"Turn Checklist — Turn " + std::to_wstring(kingdom.turn)).c_str());
            return 0;
        case WM_COMMAND:
            if (LOWORD(wparam) == kChecklistApply) apply_turn_checklist(window);
            else if (LOWORD(wparam) == kChecklistCancel) DestroyWindow(window);
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
    switch (message) {
        case WM_CREATE:
            create_editor_controls(window);
            return 0;
        case WM_COMMAND:
            if (LOWORD(wparam) == kEditorSave) apply_editor(window);
            else if (LOWORD(wparam) == kEditorCancel) DestroyWindow(window);
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
    switch (message) {
        case WM_CREATE: {
            HMENU menu = CreateMenu();
            HMENU file = CreatePopupMenu();
            AppendMenuW(file, MF_STRING, kNew, L"New");
            AppendMenuW(file, MF_STRING, kOpen, L"Open...");
            AppendMenuW(file, MF_STRING, kSave, L"Save");
            AppendMenuW(file, MF_SEPARATOR, 0, nullptr);
            AppendMenuW(file, MF_STRING, kEditInputs, L"Edit Inputs...");
            AppendMenuW(file, MF_STRING, kManageSettlements, L"Manage Settlements...");
            AppendMenuW(file, MF_STRING, kTurnChecklist, L"Turn Checklist...");
            AppendMenuW(file, MF_STRING, kCalendarTracker, L"Calendar Notes...");
            AppendMenuW(menu, MF_POPUP, reinterpret_cast<UINT_PTR>(file), L"File");
            SetMenu(window, menu);
            CreateWindowExW(0, L"STATIC", L"", WS_CHILD | WS_VISIBLE | SS_LEFT,
                24, 24, 720, 520, window, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kDisplay)), app_instance, nullptr);
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
            if (calendar_window && IsWindow(calendar_window)) DestroyWindow(calendar_window);
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcW(window, message, wparam, lparam);
}
}

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int) {
    app_instance = instance;
    try {
        building_catalog = pkkm::load_building_catalog(bundled_catalog_path());
    } catch (const std::exception& error) {
        MessageBoxW(nullptr, widen(error.what()).c_str(), L"PKKM catalog error", MB_OK | MB_ICONERROR);
        return 1;
    }
    WNDCLASSW main_class{};
    main_class.lpfnWndProc = main_proc;
    main_class.hInstance = instance;
    main_class.lpszClassName = L"PKKMWindow";
    main_class.hCursor = LoadCursor(nullptr, IDC_ARROW);
    main_class.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    if (!RegisterClassW(&main_class)) return 1;

    WNDCLASSW editor_class{};
    editor_class.lpfnWndProc = editor_proc;
    editor_class.hInstance = instance;
    editor_class.lpszClassName = L"PKKMEditor";
    editor_class.hCursor = LoadCursor(nullptr, IDC_ARROW);
    editor_class.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    if (!RegisterClassW(&editor_class)) return 1;

    WNDCLASSW settlement_class{};
    settlement_class.lpfnWndProc = settlement_manager_proc;
    settlement_class.hInstance = instance;
    settlement_class.lpszClassName = L"PKKMSettlements";
    settlement_class.hCursor = LoadCursor(nullptr, IDC_ARROW);
    settlement_class.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    if (!RegisterClassW(&settlement_class)) return 1;

    WNDCLASSW checklist_class{};
    checklist_class.lpfnWndProc = turn_checklist_proc;
    checklist_class.hInstance = instance;
    checklist_class.lpszClassName = L"PKKMTurnChecklist";
    checklist_class.hCursor = LoadCursor(nullptr, IDC_ARROW);
    checklist_class.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    if (!RegisterClassW(&checklist_class)) return 1;

    WNDCLASSW calendar_class{};
    calendar_class.lpfnWndProc = calendar_tracker_proc;
    calendar_class.hInstance = instance;
    calendar_class.lpszClassName = L"PKKMCalendarTracker";
    calendar_class.hCursor = LoadCursor(nullptr, IDC_ARROW);
    calendar_class.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    if (!RegisterClassW(&calendar_class)) return 1;

    HWND window = CreateWindowW(main_class.lpszClassName, L"PKKM — Kingdom Manager",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 800, 620,
        nullptr, nullptr, instance, nullptr);
    if (!window) return 1;
    ShowWindow(window, SW_SHOW);
    MSG message{};
    while (GetMessageW(&message, nullptr, 0, 0) > 0) {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }
    return static_cast<int>(message.wParam);
}
