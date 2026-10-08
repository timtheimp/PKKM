#include "pkkm/kingdom.hpp"
#include "pkkm/buildings.hpp"
#include <iostream>
#include <filesystem>
#include <stdexcept>
#include <string>

namespace {
std::string signed_component(int value) {
    return value > 0 ? "+" + std::to_string(value) : std::to_string(value);
}

std::string single_line(std::string value) {
    std::string result;
    bool pending_space = false;
    for (std::size_t index = 0; index < value.size();) {
        std::size_t separator_size = 0;
        if (value.compare(index, 3, "\xE2\x80\xA8") == 0 || value.compare(index, 3, "\xE2\x80\xA9") == 0)
            separator_size = 3;
        else if (value.compare(index, 2, "\xC2\x85") == 0)
            separator_size = 2;
        const auto character = static_cast<unsigned char>(value[index]);
        if (separator_size != 0) {
            pending_space = !result.empty();
            index += separator_size;
        } else if (character <= 0x20 || character == 0x7F) {
            pending_space = !result.empty();
            ++index;
        } else {
            if (pending_space) result.push_back(' ');
            pending_space = false;
            result.push_back(value[index++]);
        }
    }
    return result;
}

void print_stat(const char* name, const pkkm::KingdomStatBreakdown& stat) {
    std::cout << name << " " << stat.total << " (" << stat.check_threshold_percent << "% check)\n"
              << "  Events " << signed_component(stat.events)
              << " | Alignment " << signed_component(stat.alignment)
              << " | Improvements " << signed_component(stat.improvements)
              << " | Leadership " << signed_component(stat.leadership)
              << " | Laws " << signed_component(stat.laws)
              << " | Unrest " << signed_component(stat.unrest_adjustment)
              << " | Vacancies " << signed_component(stat.vacancy_adjustment)
              << " | Other " << signed_component(stat.other) << "\n";
}
}

int main(int argc, char** argv) {
    try {
        if (argc == 3 && std::string(argv[1]) == "new") {
            pkkm::Kingdom kingdom;
            pkkm::save(kingdom, argv[2]);
            std::cout << "Created " << argv[2] << "\n";
            return 0;
        }
        if (argc == 3 && std::string(argv[1]) == "show") {
            const auto kingdom = pkkm::load(argv[2]);
            const auto executable = std::filesystem::absolute(std::filesystem::path(argv[0]));
            const auto catalog_path = (executable.parent_path() / "building_catalog.json").string();
            const auto catalog = pkkm::load_building_catalog(catalog_path);
            const auto summary = pkkm::calculate_kingdom_summary(
                kingdom.unrest, kingdom.rule_inputs, kingdom.settlements, &catalog);
            std::cout << kingdom.name << " | turn " << kingdom.turn << " | treasury " << kingdom.treasury_bp
                      << " BP | unrest " << kingdom.unrest << "\n";
            std::cout << "Control DC " << summary.control_dc << " | population " << summary.population << "\n";
            if (summary.settlement_totals.count > 0) {
                const auto& totals = summary.settlement_totals;
                std::cout << "Settlements " << totals.count << " | population " << totals.population
                          << " | districts " << totals.districts << " | defense " << totals.defense
                          << " | lots " << totals.lots << "\n";
            }
            for (const auto& settlement : kingdom.settlements) {
                if (settlement.building_inventory.empty()) continue;
                const auto effects = pkkm::calculate_settlement_building_totals(settlement, catalog);
                std::cout << "  " << single_line(settlement.name);
                if (settlement.apply_catalog_stat_effects) {
                    std::cout << " catalog stat bonuses applied to kingdom totals: Economy "
                              << signed_component(effects.economy) << " | Loyalty " << signed_component(effects.loyalty)
                              << " | Stability " << signed_component(effects.stability)
                              << " | Defense/Unrest preview only: Defense " << signed_component(effects.defense)
                              << " | Unrest " << signed_component(effects.unrest) << "\n";
                } else {
                    std::cout << " catalog effects preview (not applied): Economy " << signed_component(effects.economy)
                              << " | Loyalty " << signed_component(effects.loyalty)
                              << " | Stability " << signed_component(effects.stability)
                              << " | Defense " << signed_component(effects.defense)
                              << " | Unrest " << signed_component(effects.unrest) << "\n";
                }
            }
            print_stat("Economy", summary.economy);
            print_stat("Loyalty", summary.loyalty);
            print_stat("Stability", summary.stability);
            return 0;
        }
        if ((argc == 3 || argc == 4) && std::string(argv[1]) == "catalog") {
            const auto catalog = pkkm::load_building_catalog(argv[2]);
            const auto matches = pkkm::search_building_catalog(catalog, argc == 4 ? argv[3] : "");
            for (const auto& building : matches) {
                std::cout << single_line(building.name) << " | Data row " << building.source_row
                          << " | Cost " << building.cost << " BP | Lots " << building.lots
                          << " | Economy " << signed_component(building.economy)
                          << " | Loyalty " << signed_component(building.loyalty)
                          << " | Stability " << signed_component(building.stability)
                          << " | Defense " << signed_component(building.defense)
                          << " | Unrest " << signed_component(building.unrest);
                if (!building.discounts.empty()) std::cout << " | Discounts: " << single_line(building.discounts);
                if (!building.magic_item.empty()) std::cout << " | Magic items: " << single_line(building.magic_item);
                if (!building.upgrade_from.empty()) std::cout << " | Upgrade from: " << single_line(building.upgrade_from);
                if (!building.upgrade_to.empty()) std::cout << " | Upgrade to: " << single_line(building.upgrade_to);
                std::cout << "\n";
            }
            return 0;
        }
        std::cerr << "Usage: pkkm new|show <kingdom.json> | catalog <catalog.json> [query]\n";
        return 2;
    } catch (const std::exception& error) {
        std::cerr << error.what() << "\n";
        return 1;
    }
}

