#ifdef _WIN32
#include "pkkm/map_editor.hpp"

#include <algorithm>
#include <string>
#include <stdexcept>
#include <vector>

namespace pkkm::ui {
namespace {
constexpr wchar_t kClassName[] = L"PKKMMapEditor";
constexpr int kMaxGrid = 20;
constexpr int kCellBase = 7000;
constexpr int kRows = 7300;
constexpr int kColumns = 7301;
constexpr int kResize = 7302;
constexpr int kBuilding = 7303;
constexpr int kWidth = 7304;
constexpr int kHeight = 7305;
constexpr int kPlace = 7306;
constexpr int kPlacements = 7307;
constexpr int kRemove = 7308;
constexpr int kApply = 7309;
constexpr int kCancel = 7310;
constexpr int kStatus = 7311;
constexpr int kLabelText = 7312;
constexpr int kAddLabel = 7313;
constexpr int kTemplate = 7314;
constexpr int kLoadTemplate = 7315;

HWND editor_window = nullptr;
HWND editor_owner = nullptr;
HINSTANCE editor_instance = nullptr;
SettlementMap map_draft;
std::vector<Building> map_inventory;
std::function<void(SettlementMap)> apply_callback;
std::vector<HWND> cells;
int selected_row = -1;
int selected_column = -1;
bool class_registered = false;

std::wstring widen(const std::string& value) {
    if (value.empty()) return {};
    const int count = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, value.data(),
        static_cast<int>(value.size()), nullptr, 0);
    if (count <= 0) return L"?";
    std::wstring result(static_cast<size_t>(count), L'\0');
    MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, value.data(),
        static_cast<int>(value.size()), result.data(), count);
    return result;
}

std::string narrow(const std::wstring& value) {
    if (value.empty()) return {};
    const int count = WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, value.data(),
        static_cast<int>(value.size()), nullptr, 0, nullptr, nullptr);
    if (count <= 0) return {};
    std::string result(static_cast<size_t>(count), '\0');
    if (WideCharToMultiByte(CP_UTF8, WC_ERR_INVALID_CHARS, value.data(),
            static_cast<int>(value.size()), result.data(), count, nullptr, nullptr) <= 0)
        return {};
    return result;
}

std::wstring control_text(int id) {
    HWND field = GetDlgItem(editor_window, id);
    const int length = GetWindowTextLengthW(field);
    std::wstring value(static_cast<size_t>(length) + 1, L'\0');
    GetWindowTextW(field, value.data(), length + 1);
    value.resize(static_cast<size_t>(length));
    return value;
}

void control(HWND parent, const wchar_t* type, const wchar_t* text, DWORD style,
    int x, int y, int width, int height, int id) {
    CreateWindowExW(0, type, text, WS_CHILD | WS_VISIBLE | style,
        x, y, width, height, parent, reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)),
        editor_instance, nullptr);
}

void set_status(const std::wstring& text) {
    if (editor_window) SetWindowTextW(GetDlgItem(editor_window, kStatus), text.c_str());
}

bool read_integer(int id, int& value) {
    wchar_t text[32]{};
    GetWindowTextW(GetDlgItem(editor_window, id), text, static_cast<int>(std::size(text)));
    try {
        size_t consumed = 0;
        const int parsed = std::stoi(text, &consumed);
        if (consumed != std::wstring(text).size()) return false;
        value = parsed;
        return true;
    } catch (...) {
        return false;
    }
}

void validate_map(const SettlementMap& map) {
    Kingdom candidate;
    Settlement settlement;
    settlement.name = "Map validation";
    settlement.map = map;
    candidate.settlements.push_back(std::move(settlement));
    pkkm::validate(candidate);
}

void refresh_map() {
    if (!editor_window) return;
    for (int row = 0; row < kMaxGrid; ++row) {
        for (int column = 0; column < kMaxGrid; ++column) {
            const size_t index = static_cast<size_t>(row * kMaxGrid + column);
            const bool visible = row < map_draft.rows && column < map_draft.columns;
            ShowWindow(cells[index], visible ? SW_SHOW : SW_HIDE);
            SetWindowTextW(cells[index], L"");
        }
    }
    for (const auto& placement : map_draft.placements) {
        for (int row = placement.row; row < placement.row + placement.height; ++row) {
            for (int column = placement.column; column < placement.column + placement.width; ++column) {
                const size_t index = static_cast<size_t>(row * kMaxGrid + column);
                SetWindowTextW(cells[index], row == placement.row && column == placement.column
                    ? widen(placement.building_name).substr(0, 1).c_str() : L"#");
            }
        }
    }
    HWND list = GetDlgItem(editor_window, kPlacements);
    SendMessageW(list, LB_RESETCONTENT, 0, 0);
    for (const auto& placement : map_draft.placements) {
        const std::wstring label = L"Building: " + widen(placement.building_name) + L" @ (" +
            std::to_wstring(placement.row) + L"," + std::to_wstring(placement.column) +
            L")  " + std::to_wstring(placement.width) + L"×" +
            std::to_wstring(placement.height);
        SendMessageW(list, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(label.c_str()));
    }
    for (const auto& map_label : map_draft.labels) {
        for (int row = map_label.row; row < map_label.row + map_label.height; ++row) {
            for (int column = map_label.column; column < map_label.column + map_label.width; ++column) {
                const size_t index = static_cast<size_t>(row * kMaxGrid + column);
                SetWindowTextW(cells[index], row == map_label.row && column == map_label.column
                    ? widen(map_label.text).substr(0, 1).c_str() : L"#");
            }
        }
        const std::wstring item = L"Label: " + widen(map_label.text) + L" @ (" +
            std::to_wstring(map_label.row) + L"," + std::to_wstring(map_label.column) +
            L")  " + std::to_wstring(map_label.width) + L"×" +
            std::to_wstring(map_label.height);
        SendMessageW(list, LB_ADDSTRING, 0, reinterpret_cast<LPARAM>(item.c_str()));
    }
    if (selected_row >= 0) {
        set_status(L"Selected cell: row " + std::to_wstring(selected_row) +
            L", column " + std::to_wstring(selected_column) + L" (zero-based)");
    }
}

void create_controls(HWND window) {
    control(window, L"STATIC", L"Select a cell, then add a building or label.",
        0, 20, 20, 490, 24, -1);
    control(window, L"STATIC", L"Workbook layout", 0, 700, 20, 105, 22, -1);
    control(window, L"COMBOBOX", L"", CBS_DROPDOWNLIST | WS_VSCROLL | WS_TABSTOP,
        805, 16, 255, 180, kTemplate);
    control(window, L"BUTTON", L"Load Template", BS_PUSHBUTTON | WS_TABSTOP,
        1070, 15, 115, 29, kLoadTemplate);
    control(window, L"STATIC", L"Rows", 0, 700, 55, 48, 22, -1);
    control(window, L"EDIT", L"", WS_BORDER | ES_NUMBER | WS_TABSTOP, 750, 52, 60, 25, kRows);
    control(window, L"STATIC", L"Columns", 0, 830, 55, 60, 22, -1);
    control(window, L"EDIT", L"", WS_BORDER | ES_NUMBER | WS_TABSTOP, 895, 52, 60, 25, kColumns);
    control(window, L"BUTTON", L"Resize Grid", BS_PUSHBUTTON | WS_TABSTOP, 975, 50, 110, 29, kResize);
    control(window, L"STATIC", L"Building name (or choose inventory item)", 0, 700, 100, 300, 22, -1);
    control(window, L"COMBOBOX", L"", CBS_DROPDOWN | WS_VSCROLL | WS_TABSTOP,
        700, 124, 300, 220, kBuilding);
    control(window, L"STATIC", L"Width", 0, 1015, 100, 55, 22, -1);
    control(window, L"EDIT", L"1", WS_BORDER | ES_NUMBER | WS_TABSTOP, 1015, 124, 55, 25, kWidth);
    control(window, L"STATIC", L"Height", 0, 1090, 100, 55, 22, -1);
    control(window, L"EDIT", L"1", WS_BORDER | ES_NUMBER | WS_TABSTOP, 1090, 124, 55, 25, kHeight);
    control(window, L"BUTTON", L"Place at Selected Cell", BS_PUSHBUTTON | WS_TABSTOP,
        700, 162, 190, 30, kPlace);
    control(window, L"STATIC", L"Terrain / corridor label", 0, 700, 205, 200, 22, -1);
    control(window, L"EDIT", L"", WS_BORDER | WS_TABSTOP, 860, 202, 245, 25, kLabelText);
    control(window, L"BUTTON", L"Add Label", BS_PUSHBUTTON | WS_TABSTOP, 1115, 201, 95, 28, kAddLabel);
    control(window, L"STATIC", L"Map items (labels and buildings cannot overlap)", 0, 700, 235, 420, 22, -1);
    control(window, L"LISTBOX", L"", WS_BORDER | LBS_NOTIFY | WS_VSCROLL | WS_TABSTOP,
        700, 260, 475, 220, kPlacements);
    control(window, L"BUTTON", L"Remove Selected", BS_PUSHBUTTON | WS_TABSTOP,
        700, 490, 140, 30, kRemove);
    control(window, L"STATIC", L"Footprints are explicit; catalog lot counts do not set map size.",
        0, 700, 535, 475, 30, -1);
    control(window, L"BUTTON", L"Apply", BS_DEFPUSHBUTTON | WS_TABSTOP, 975, 575, 90, 32, kApply);
    control(window, L"BUTTON", L"Cancel", BS_PUSHBUTTON | WS_TABSTOP, 1080, 575, 90, 32, kCancel);
    control(window, L"STATIC", L"", 0, 20, 535, 480, 28, kStatus);

    for (int row = 0; row < kMaxGrid; ++row) {
        for (int column = 0; column < kMaxGrid; ++column) {
            const int id = kCellBase + row * kMaxGrid + column;
            const int x = 20 + column * 32;
            const int y = 62 + row * 30;
            HWND cell = CreateWindowExW(0, L"BUTTON", L"",
                WS_CHILD | BS_PUSHBUTTON | WS_TABSTOP,
                x, y, 30, 28, window,
                reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), editor_instance, nullptr);
            cells.push_back(cell);
        }
    }
    for (size_t i = 0; i < map_inventory.size(); ++i) {
        const std::wstring label = widen(map_inventory[i].name) + L" (" +
            std::to_wstring(map_inventory[i].count) + L")";
        SendMessageW(GetDlgItem(window, kBuilding), CB_ADDSTRING, 0,
            reinterpret_cast<LPARAM>(label.c_str()));
    }
    const HWND template_combo = GetDlgItem(window, kTemplate);
    SendMessageW(template_combo, CB_ADDSTRING, 0,
        reinterpret_cast<LPARAM>(L"Choose a workbook layout..."));
    for (const auto preset : {WorkbookMapTemplate::Capital, WorkbookMapTemplate::City,
            WorkbookMapTemplate::Example, WorkbookMapTemplate::MultiDistrict}) {
        const std::wstring label = widen(workbook_map_template_name(preset));
        SendMessageW(template_combo, CB_ADDSTRING, 0,
            reinterpret_cast<LPARAM>(label.c_str()));
    }
    SendMessageW(template_combo, CB_SETCURSEL, 0, 0);
    if (!map_inventory.empty()) SendMessageW(GetDlgItem(window, kBuilding), CB_SETCURSEL, 0, 0);
    SetWindowTextW(GetDlgItem(window, kRows), std::to_wstring(map_draft.rows).c_str());
    SetWindowTextW(GetDlgItem(window, kColumns), std::to_wstring(map_draft.columns).c_str());
    refresh_map();
}

LRESULT CALLBACK map_editor_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
    switch (message) {
    case WM_CREATE:
        editor_window = window;
        create_controls(window);
        return 0;
    case WM_COMMAND: {
        const int id = LOWORD(wparam);
        if (id >= kCellBase && id < kCellBase + kMaxGrid * kMaxGrid && HIWORD(wparam) == BN_CLICKED) {
            const int index = id - kCellBase;
            selected_row = index / kMaxGrid;
            selected_column = index % kMaxGrid;
            refresh_map();
            return 0;
        }
        if (id == kLoadTemplate && HIWORD(wparam) == BN_CLICKED) {
            const LRESULT selection = SendMessageW(GetDlgItem(window, kTemplate), CB_GETCURSEL, 0, 0);
            WorkbookMapTemplate preset;
            switch (selection) {
                case 1: preset = WorkbookMapTemplate::Capital; break;
                case 2: preset = WorkbookMapTemplate::City; break;
                case 3: preset = WorkbookMapTemplate::Example; break;
                case 4: preset = WorkbookMapTemplate::MultiDistrict; break;
                default:
                    set_status(L"Choose a workbook layout first.");
                    return 0;
            }
            if ((!map_draft.placements.empty() || !map_draft.labels.empty())
                && MessageBoxW(window,
                    L"Loading this layout replaces the current map draft. The settlement changes only if you Apply.",
                    L"Replace map draft?", MB_YESNO | MB_ICONWARNING | MB_DEFBUTTON2) != IDYES) {
                return 0;
            }
            try {
                SettlementMap candidate = workbook_map_template(preset);
                validate_map(candidate);
                map_draft = std::move(candidate);
                selected_row = -1;
                selected_column = -1;
                SetWindowTextW(GetDlgItem(window, kRows), std::to_wstring(map_draft.rows).c_str());
                SetWindowTextW(GetDlgItem(window, kColumns), std::to_wstring(map_draft.columns).c_str());
                SetWindowTextW(GetDlgItem(window, kLabelText), L"");
                refresh_map();
                set_status(L"Loaded " + widen(workbook_map_template_name(preset))
                    + L". Labels are visual only; Apply to use this layout.");
            } catch (const std::exception& error) {
                set_status(L"Template rejected: " + widen(error.what()));
            }
            return 0;
        }
        if (id == kResize && HIWORD(wparam) == BN_CLICKED) {
            int rows = 0, columns = 0;
            if (!read_integer(kRows, rows) || !read_integer(kColumns, columns)
                || rows < 1 || rows > kMaxGrid || columns < 1 || columns > kMaxGrid) {
                set_status(L"Grid dimensions must each be between 1 and 20.");
                return 0;
            }
            SettlementMap candidate = map_draft;
            candidate.rows = rows;
            candidate.columns = columns;
            try {
                validate_map(candidate);
                map_draft = std::move(candidate);
                refresh_map();
            } catch (const std::exception& error) {
                set_status(L"Resize rejected: " + widen(error.what()));
                SetWindowTextW(GetDlgItem(window, kRows), std::to_wstring(map_draft.rows).c_str());
                SetWindowTextW(GetDlgItem(window, kColumns), std::to_wstring(map_draft.columns).c_str());
            }
            return 0;
        }
        if (id == kPlace && HIWORD(wparam) == BN_CLICKED) {
            const LRESULT building_index = SendMessageW(GetDlgItem(window, kBuilding), CB_GETCURSEL, 0, 0);
            int width = 0, height = 0;
            std::string building_name;
            const std::wstring text = control_text(kBuilding);
            const bool inventory_item_selected = building_index >= 0
                && building_index < static_cast<LRESULT>(map_inventory.size())
                && text == widen(map_inventory[static_cast<size_t>(building_index)].name) + L" ("
                    + std::to_wstring(map_inventory[static_cast<size_t>(building_index)].count) + L")";
            if (inventory_item_selected) {
                building_name = map_inventory[static_cast<size_t>(building_index)].name;
            } else {
                const size_t first = text.find_first_not_of(L" \t\r\n");
                if (first != std::wstring::npos) {
                    const size_t last = text.find_last_not_of(L" \t\r\n");
                    building_name = narrow(text.substr(first, last - first + 1));
                }
            }
            if (selected_row < 0 || building_name.empty() || !read_integer(kWidth, width)
                || !read_integer(kHeight, height) || width < 1 || height < 1) {
                set_status(L"Select a cell, enter a building name, and provide positive width and height.");
                return 0;
            }
            SettlementMap candidate = map_draft;
            candidate.placements.push_back({std::move(building_name),
                selected_row, selected_column, width, height});
            try {
                validate_map(candidate);
                map_draft = std::move(candidate);
                refresh_map();
            } catch (const std::exception& error) {
                set_status(L"Placement rejected: " + widen(error.what()));
            }
            return 0;
        }
        if (id == kAddLabel && HIWORD(wparam) == BN_CLICKED) {
            const std::wstring text = control_text(kLabelText);
            const size_t first = text.find_first_not_of(L" \t\r\n");
            int width = 0, height = 0;
            if (first == std::wstring::npos || selected_row < 0
                || !read_integer(kWidth, width) || !read_integer(kHeight, height)
                || width < 1 || height < 1) {
                set_status(L"Select a cell, enter a label, and provide positive width and height.");
                return 0;
            }
            const size_t last = text.find_last_not_of(L" \t\r\n");
            const std::string label_text = narrow(text.substr(first, last - first + 1));
            if (label_text.empty()) {
                set_status(L"The label could not be converted to UTF-8.");
                return 0;
            }
            SettlementMap candidate = map_draft;
            candidate.labels.push_back({label_text, selected_row, selected_column, width, height});
            try {
                validate_map(candidate);
                map_draft = std::move(candidate);
                SetWindowTextW(GetDlgItem(window, kLabelText), L"");
                refresh_map();
            } catch (const std::exception& error) {
                set_status(L"Label rejected: " + widen(error.what()));
            }
            return 0;
        }
        if (id == kRemove && HIWORD(wparam) == BN_CLICKED) {
            const LRESULT selected = SendMessageW(GetDlgItem(window, kPlacements), LB_GETCURSEL, 0, 0);
            if (selected == LB_ERR) {
                set_status(L"Select a placement to remove.");
                return 0;
            }
            if (static_cast<size_t>(selected) < map_draft.placements.size()) {
                map_draft.placements.erase(map_draft.placements.begin() + selected);
            } else {
                const size_t label_index = static_cast<size_t>(selected) - map_draft.placements.size();
                if (label_index >= map_draft.labels.size()) return 0;
                map_draft.labels.erase(map_draft.labels.begin() + label_index);
            }
            refresh_map();
            return 0;
        }
        if (id == kApply && HIWORD(wparam) == BN_CLICKED) {
            try {
                validate_map(map_draft);
                if (apply_callback) apply_callback(map_draft);
                DestroyWindow(window);
            } catch (const std::exception& error) {
                set_status(L"Map rejected: " + widen(error.what()));
            }
            return 0;
        }
        if (id == kCancel && HIWORD(wparam) == BN_CLICKED) {
            DestroyWindow(window);
            return 0;
        }
        break;
    }
    case WM_CLOSE:
        DestroyWindow(window);
        return 0;
    case WM_DESTROY:
        if (editor_owner && IsWindow(editor_owner)) {
            EnableWindow(editor_owner, TRUE);
            SetForegroundWindow(editor_owner);
        }
        editor_window = nullptr;
        editor_owner = nullptr;
        cells.clear();
        map_inventory.clear();
        apply_callback = {};
        return 0;
    }
    return DefWindowProcW(window, message, wparam, lparam);
}
} // namespace

bool open_map_editor(HWND owner, HINSTANCE instance, const SettlementMap& initial,
    const std::vector<Building>& inventory, std::function<void(SettlementMap)> on_apply) {
    if (editor_window && IsWindow(editor_window)) {
        SetForegroundWindow(editor_window);
        return false;
    }
    map_draft = initial;
    if (map_draft.rows == 0 && map_draft.columns == 0
        && map_draft.placements.empty() && map_draft.labels.empty()) {
        map_draft.rows = 8;
        map_draft.columns = 8;
    }
    if (map_draft.rows < 1 || map_draft.columns < 1 || map_draft.rows > kMaxGrid || map_draft.columns > kMaxGrid) {
        MessageBoxW(owner, L"The visual editor supports maps from 1×1 through 20×20. The saved map was not changed.",
            L"Map editor", MB_OK | MB_ICONWARNING);
        return false;
    }
    try {
        validate_map(map_draft);
    } catch (const std::exception& error) {
        MessageBoxW(owner, widen(error.what()).c_str(), L"Invalid saved map", MB_OK | MB_ICONERROR);
        return false;
    }
    editor_owner = owner;
    editor_instance = instance;
    map_inventory = inventory;
    apply_callback = std::move(on_apply);
    if (!class_registered) {
        WNDCLASSW wc{};
        wc.lpfnWndProc = map_editor_proc;
        wc.hInstance = instance;
        wc.lpszClassName = kClassName;
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
        if (!RegisterClassW(&wc) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS) return false;
        class_registered = true;
    }
    HWND window = CreateWindowExW(WS_EX_DLGMODALFRAME, kClassName, L"Settlement Map Editor",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU, CW_USEDEFAULT, CW_USEDEFAULT, 1240, 700,
        owner, nullptr, instance, nullptr);
    if (!window) {
        editor_owner = nullptr;
        map_inventory.clear();
        apply_callback = {};
        return false;
    }
    EnableWindow(owner, FALSE);
    ShowWindow(window, SW_SHOW);
    UpdateWindow(window);
    return true;
}

void close_map_editor(HWND owner) {
    if (editor_window && IsWindow(editor_window) && editor_owner == owner)
        DestroyWindow(editor_window);
}
} // namespace pkkm::ui
#endif
