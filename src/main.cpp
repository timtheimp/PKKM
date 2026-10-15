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
        if (argc == 2 && (std::string(argv[1]) == "--version" || std::string(argv[1]) == "version")) {
            std::cout << "PKKM " << PKKM_VERSION << "\n";
            return 0;
        }
        if (argc == 2 && (std::string(argv[1]) == "--help" || std::string(argv[1]) == "help")) {
            std::cout << "Usage: pkkm --help | --version | new <kingdom.json> | show <kingdom.json> | catalog <catalog.json> [query]\n";
            return 0;
        }
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
            std::cout << kingdom.name << " | turn " << kingdom.turn
                      << " | rule profile " << pkkm::rule_profile_name(kingdom.rule_profile)
                      << " | treasury " << kingdom.treasury_bp
                      << " BP | unrest " << kingdom.unrest << "\n";
            std::cout << "Audit records | Turn actions " << kingdom.turn_actions.size()
                      << " | Taxes " << kingdom.tax_actions.size()
                      << " | Stability checks " << kingdom.stability_checks.size()
                      << " | Consumption " << kingdom.consumption_actions.size()
                      << " | Unrest modifications " << kingdom.unrest_modifications.size()
                      << " | Gold withdrawals " << kingdom.gold_withdrawals.size()
                      << " | Gold deposits " << kingdom.gold_deposits.size()
                      << " | Item deposits " << kingdom.item_deposits.size()
                      << " | Loyalty checks " << kingdom.loyalty_checks.size()
                      << " | Commissions " << kingdom.commissions.size()
                      << " | Magic-item outcomes " << kingdom.magic_item_outcomes.size()
                      << " | Kingdom events " << kingdom.kingdom_events.size()
                      << " | Construction transactions " << kingdom.construction_transactions.size() << "\n";
            for (const auto& event : kingdom.kingdom_events) {
                std::cout << " Event | turn " << event.turn
                          << " | d100 " << event.d100_roll
                          << " | " << pkkm::kingdom_event_kind_name(event.kind)
                          << " | " << (event.resolved ? "resolved" : "unresolved")
                          << " | continuous " << (event.continuous_active ? "active" : "inactive")
                          << " | leader " << single_line(event.affected_leader)
                          << " | city " << single_line(event.affected_city)
                          << " | block " << single_line(event.affected_block)
                          << " | hex " << single_line(event.affected_hex)
                          << " | treasury " << event.treasury_delta
                          << " | unrest " << event.unrest_delta
                          << " | size " << event.kingdom_size_delta
                          << " | hex loss " << (event.hex_loss_confirmed ? "yes" : "no")
                          << " | anarchy " << (event.anarchy_confirmed ? "yes" : "no")
                          << " | applied " << (event.consequences_applied ? "yes" : "no")
                          << " | notes " << single_line(event.consequence_notes) << "\n";
            }
            for (std::size_t index = 0; index < kingdom.construction_transactions.size(); ++index) {
                const auto& transaction = kingdom.construction_transactions[index];
                std::cout << " Construction | index " << index
                          << " | turn " << transaction.turn
                          << " | settlement " << single_line(transaction.settlement_id)
                          << " | bp " << transaction.bp_cost
                          << " | " << (transaction.successful ? "successful" : "failed")
                          << " | " << (transaction.reversal ? "reversal" : "forward")
                          << " | override limit " << (transaction.override_building_limit ? "yes" : "no")
                          << " | override checks " << (transaction.override_failed_checks ? "yes" : "no")
                          << " | reverses " << transaction.reverses_index
                          << " | failures " << transaction.failures.size()
                          << " | terrain facts " << transaction.terrain_facts.size() << "\n";
                for (const auto& line : transaction.lines)
                    std::cout << "  Construction line | " << single_line(line.building_name)
                              << " x " << line.count
                              << (line.upgrade_from.empty() ? "" : " | from " + single_line(line.upgrade_from))
                              << " | placement " << line.row << "," << line.column
                              << " " << line.width << "x" << line.height << "\n";
                for (const auto& failure : transaction.failures)
                    std::cout << "  Construction failure | " << single_line(failure) << "\n";
                for (const auto& fact : transaction.terrain_facts)
                    std::cout << "  Construction terrain | " << single_line(fact) << "\n";
                for (const auto& source : transaction.discount_sources)
                    std::cout << "  Construction discount source | " << single_line(source) << "\n";
            }
            std::cout << "Control DC " << summary.control_dc << " | population " << summary.population << "\n";
            if (summary.settlement_totals.count > 0) {
                const auto& totals = summary.settlement_totals;
                std::cout << "Settlements " << totals.count << " | population " << totals.population
                          << " | districts " << totals.districts << " | defense " << totals.defense
                          << " | lots " << totals.lots << "\n";
            }
            for (const auto& settlement : kingdom.settlements) {
                const auto effects = pkkm::calculate_settlement_building_totals(settlement, catalog);
                const auto size = pkkm::calculate_settlement_size(settlement, catalog);
                const auto market = pkkm::calculate_settlement_market_reference(settlement, catalog);
                const auto catalog_cost = pkkm::calculate_settlement_catalog_cost(settlement, catalog);
                const auto magic_items = pkkm::preview_settlement_magic_items(settlement, catalog);
                std::cout << "  " << single_line(settlement.name)
                          << " | population " << size.population
                          << " | districts " << size.districts
                          << " | lots " << size.lots
                          << " | market base " << market.base_value
                          << " | buy limit " << market.buy_limit
                          << " | sell limit " << market.sell_limit
                          << " | spellcasting level " << market.spellcasting_level
                          << " | catalog cost " << catalog_cost << " BP"
                          << " | magic slots minor " << magic_items.minor_slots
                          << " medium " << magic_items.medium_slots
                          << " major " << magic_items.major_slots << "\n";
                for (const auto& source : magic_items.sources)
                    std::cout << "   magic source | " << single_line(source) << "\n";
                if (settlement.apply_catalog_stat_effects) {
                    std::cout << " catalog stat bonuses applied to kingdom totals: Economy "
                              << signed_component(effects.economy) << " | Loyalty " << signed_component(effects.loyalty)
                              << " | Stability " << signed_component(effects.stability)
                              << " | Defense/Unrest preview only: Defense " << signed_component(effects.defense)
                              << " | Unrest " << signed_component(effects.unrest)
                              << " | Other previews: Corruption " << signed_component(effects.corruption)
                              << " | Crime " << signed_component(effects.crime)
                              << " | Law " << signed_component(effects.law)
                              << " | Lore " << signed_component(effects.lore)
                              << " | Society " << signed_component(effects.society)
                              << " | Productivity " << signed_component(effects.productivity)
                              << " | Fame " << signed_component(effects.fame) << "\n";
                } else {
                    std::cout << " catalog effects preview (not applied): Economy " << signed_component(effects.economy)
                              << " | Loyalty " << signed_component(effects.loyalty)
                              << " | Stability " << signed_component(effects.stability)
                              << " | Defense " << signed_component(effects.defense)
                              << " | Unrest " << signed_component(effects.unrest)
                              << " | Other previews: Corruption " << signed_component(effects.corruption)
                              << " | Crime " << signed_component(effects.crime)
                              << " | Law " << signed_component(effects.law)
                              << " | Lore " << signed_component(effects.lore)
                              << " | Society " << signed_component(effects.society)
                              << " | Productivity " << signed_component(effects.productivity)
                              << " | Fame " << signed_component(effects.fame) << "\n";
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
                          << " | Unrest " << signed_component(building.unrest)
                          << " | Corruption " << signed_component(building.corruption)
                          << " | Crime " << signed_component(building.crime)
                          << " | Law " << signed_component(building.law)
                          << " | Lore " << signed_component(building.lore)
                          << " | Society " << signed_component(building.society)
                          << " | Productivity " << signed_component(building.productivity)
                          << " | Fame " << signed_component(building.fame);
                if (!building.discounts.empty()) std::cout << " | Discounts: " << single_line(building.discounts);
                if (!building.magic_item.empty()) std::cout << " | Magic items: " << single_line(building.magic_item);
                if (!building.upgrade_from.empty()) std::cout << " | Upgrade from: " << single_line(building.upgrade_from);
                if (!building.upgrade_to.empty()) std::cout << " | Upgrade to: " << single_line(building.upgrade_to);
                std::cout << "\n";
            }
            return 0;
        }
        std::cerr << "Usage: pkkm --help | --version | new <kingdom.json> | show <kingdom.json> | catalog <catalog.json> [query]\n";
        return 2;
    } catch (const std::exception& error) {
        std::cerr << error.what() << "\n";
        return 1;
    }
}

