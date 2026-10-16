#include "pkkm/city_layout_editor.hpp"
#ifdef _WIN32
#include <algorithm>
#include <sstream>
#include <stdexcept>

namespace pkkm::ui {
namespace {
constexpr int kNorth = 1001, kSouth = 1002, kWest = 1003, kEast = 1004;
constexpr int kBaseValue = 1005, kItems0 = 1010, kItems1 = 1011, kItems2 = 1012, kItems3 = 1013;
constexpr int kApply = 1020, kCancel = 1021;
HINSTANCE g_instance = nullptr;
HWND g_window = nullptr;
CityLayoutWorksheet g_initial;
std::function<void(CityLayoutWorksheet)> g_apply;

std::wstring widen(const std::string& text) { return std::wstring(text.begin(), text.end()); }
std::string narrow(const std::wstring& text) {
    std::string result;
    result.reserve(text.size());
    for (const wchar_t character : text) result.push_back(static_cast<char>(character));
    return result;
}
void label(HWND w, const wchar_t* text, int x, int y, int width = 120) {
    CreateWindowExW(0, L"STATIC", text, WS_CHILD | WS_VISIBLE, x, y + 4, width, 22, w, nullptr, g_instance, nullptr);
}
HWND edit(HWND w, int id, const std::wstring& text, int x, int y, int width, int height, DWORD style = ES_AUTOHSCROLL) {
    return CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", text.c_str(), WS_CHILD | WS_VISIBLE | style,
        x, y, width, height, w, reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), g_instance, nullptr);
}
std::wstring text(HWND w, int id) {
    HWND control = GetDlgItem(w, id);
    int length = GetWindowTextLengthW(control);
    std::wstring value(static_cast<size_t>(length) + 1, L'\0');
    GetWindowTextW(control, value.data(), length + 1);
    value.resize(static_cast<size_t>(length));
    return value;
}
void combo(HWND w, int id, CityBoundarySurface value, int x, int y) {
    HWND control = CreateWindowExW(0, L"COMBOBOX", L"", WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST,
        x, y, 130, 160, w, reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), g_instance, nullptr);
    SendMessageW(control, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Land"));
    SendMessageW(control, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(L"Water"));
    SendMessageW(control, CB_SETCURSEL, value == CityBoundarySurface::Water ? 1 : 0, 0);
}
CityBoundarySurface combo_value(HWND w, int id) {
    return SendMessageW(GetDlgItem(w, id), CB_GETCURSEL, 0, 0) == 1
        ? CityBoundarySurface::Water : CityBoundarySurface::Land;
}
std::vector<std::string> lines(const std::wstring& value) {
    std::vector<std::string> result;
    std::wstringstream stream(value);
    std::wstring line;
    while (std::getline(stream, line)) {
        while (!line.empty() && (line.back() == L' ' || line.back() == L'\r' || line.back() == L'\t')) line.pop_back();
        size_t first = 0;
        while (first < line.size() && (line[first] == L' ' || line[first] == L'\t')) ++first;
        line = line.substr(first);
        if (!line.empty()) result.push_back(narrow(line));
    }
    return result;
}
void fill(HWND w, int id, const std::vector<std::string>& values) {
    std::wstring joined;
    for (size_t i = 0; i < values.size(); ++i) {
        if (i) joined += L"\r\n";
        joined += widen(values[i]);
    }
    SetWindowTextW(GetDlgItem(w, id), joined.c_str());
}
void create_controls(HWND w) {
    CreateWindowExW(0, L"STATIC", L"Manual PDF city worksheet. These values are recorded only; they do not generate rules or items.",
        WS_CHILD | WS_VISIBLE, 18, 12, 820, 24, w, nullptr, g_instance, nullptr);
    label(w, L"North boundary", 18, 52); combo(w, kNorth, g_initial.north, 140, 50);
    label(w, L"South boundary", 300, 52); combo(w, kSouth, g_initial.south, 422, 50);
    label(w, L"West boundary", 18, 88); combo(w, kWest, g_initial.west, 140, 86);
    label(w, L"East boundary", 300, 88); combo(w, kEast, g_initial.east, 422, 86);
    label(w, L"Base value", 610, 52); edit(w, kBaseValue, std::to_wstring(g_initial.base_value), 700, 50, 120, 24);
    const wchar_t* headings[] = {L"Minor items (column 1)", L"Minor items (column 2)", L"Medium items", L"Major items"};
    const int ids[] = {kItems0, kItems1, kItems2, kItems3};
    for (int i = 0; i < 4; ++i) {
        label(w, headings[i], 18 + i * 210, 132, 195);
        edit(w, ids[i], L"", 18 + i * 210, 160, 195, 300, ES_MULTILINE | ES_AUTOVSCROLL | ES_WANTRETURN | WS_VSCROLL);
        fill(w, ids[i], g_initial.item_columns[i]);
    }
    CreateWindowExW(0, L"BUTTON", L"Apply", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
        650, 500, 85, 30, w, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kApply)), g_instance, nullptr);
    CreateWindowExW(0, L"BUTTON", L"Cancel", WS_CHILD | WS_VISIBLE,
        745, 500, 85, 30, w, reinterpret_cast<HMENU>(static_cast<INT_PTR>(kCancel)), g_instance, nullptr);
}
void apply(HWND w) {
    CityLayoutWorksheet value = g_initial;
    try {
        value.north = combo_value(w, kNorth); value.south = combo_value(w, kSouth);
        value.west = combo_value(w, kWest); value.east = combo_value(w, kEast);
        value.base_value = std::stoi(narrow(text(w, kBaseValue)));
        if (value.base_value < 0) throw std::invalid_argument("Base value cannot be negative.");
        const int ids[] = {kItems0, kItems1, kItems2, kItems3};
        for (int i = 0; i < 4; ++i) {
            value.item_columns[i] = lines(text(w, ids[i]));
            if (value.item_columns[i].size() > 7)
                throw std::invalid_argument("Each worksheet item column supports at most seven entries.");
        }
        g_apply(std::move(value));
        DestroyWindow(w);
    } catch (const std::exception& error) {
        MessageBoxW(w, widen(error.what()).c_str(), L"Invalid PDF worksheet", MB_OK | MB_ICONWARNING);
    }
}
LRESULT CALLBACK proc(HWND w, UINT message, WPARAM wp, LPARAM lp) {
    switch (message) {
    case WM_CREATE: create_controls(w); return 0;
    case WM_COMMAND:
        if (LOWORD(wp) == kApply) { apply(w); return 0; }
        if (LOWORD(wp) == kCancel) { DestroyWindow(w); return 0; }
        return 0;
    case WM_CLOSE: DestroyWindow(w); return 0;
    case WM_DESTROY: if (g_window == w) g_window = nullptr; return 0;
    default: return DefWindowProcW(w, message, wp, lp);
    }
}
}

bool open_city_layout_worksheet(HWND owner, HINSTANCE instance,
    const CityLayoutWorksheet& initial, std::function<void(CityLayoutWorksheet)> on_apply) {
    if (g_window) { SetForegroundWindow(g_window); return false; }
    g_instance = instance; g_initial = initial; g_apply = std::move(on_apply);
    static bool registered = false;
    if (!registered) {
        WNDCLASSW klass{}; klass.lpfnWndProc = proc; klass.hInstance = instance;
        klass.lpszClassName = L"PKKM_CityLayoutWorksheet"; klass.hCursor = LoadCursor(nullptr, IDC_ARROW);
        if (!RegisterClassW(&klass) && GetLastError() != ERROR_CLASS_ALREADY_EXISTS) return false;
        registered = true;
    }
    g_window = CreateWindowExW(WS_EX_DLGMODALFRAME, L"PKKM_CityLayoutWorksheet", L"PDF City Layout Worksheet",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 860, 580, owner, nullptr, instance, nullptr);
    if (!g_window) return false;
    ShowWindow(g_window, SW_SHOW); UpdateWindow(g_window); return true;
}
void close_city_layout_worksheet(HWND) { if (g_window) DestroyWindow(g_window); }
}
#endif
