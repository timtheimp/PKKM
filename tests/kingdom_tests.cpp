#include "pkkm/kingdom.hpp"
#include "pkkm/buildings.hpp"
#include "pkkm/greenbelt.hpp"
#include <nlohmann/json.hpp>
#include <cstdio>
#include <iostream>
#include <stdexcept>
#include <fstream>
#include <iterator>
#include <limits>
#include <string>
#include <algorithm>
static std::string contents(const std::string& p){std::ifstream f(p,std::ios::binary);return {std::istreambuf_iterator<char>(f),{}};}
static void put(const std::string& p,const std::string& s){std::ofstream f(p,std::ios::binary|std::ios::trunc);f<<s;}
void must(bool condition,const char* message){if(!condition){std::cerr<<"FAIL: "<<message<<"\n";throw std::runtime_error(message);}}
template<class F> static void must_reject(F f){bool rejected=false;try{f();}catch(const std::exception&){rejected=true;}must(rejected,"expected rejection");}
int main(int argc, char** argv) {
 try {
 pkkm::Kingdom k; k.name="Test Realm"; k.turn=4; k.treasury_bp=120; k.unrest=2;
 k.turn_progress = {{4,{"upkeep.stability","income.deposit_gold"}},
                    {5,{"events.roll_d100"}}};
 k.calendar_notes = {{"calendar.row.2", "", "", "", "Opening month"},
                     {"calendar.row.14", "Founding festival", "Build the bridge", "Dragon sighting", ""}};
 k.settlements.push_back({"Oak", 1200, 2, 3, -1, 4, 7});
 k.settlements[0].id="settlement.oak";
 k.settlements[0].apply_catalog_stat_effects=true;
 k.settlements[0].building_inventory.push_back({"Academy", 2});
 k.settlements[0].map = {8, 8, {{"Castle", 5, 5, 2, 2}}};
 k.greenbelt_map.asset_path="Kingmaker-Greenbelt-Map.jpg";
 k.greenbelt_map.image_width=7865; k.greenbelt_map.image_height=2610;
 k.greenbelt_map.calibration={120.0, 40.0, 46.0, 40.0, 1.0};
 k.greenbelt_map.hexes.push_back({3, -2, "plains", true, 4, "settlement.oak", {"farm"}});
 k.greenbelt_map.special_terrain.push_back({4, -1, "river", "North Fork", "Manual waterway record"});
 k.greenbelt_map.custom_improvements.push_back({"custom.greenhouse", "Greenhouse",
     "Manual effect preview", "Manual cost", true});
 k.buildings.push_back({"Granary", 2});
 const std::string path="pkkm_test_roundtrip.json";
 k.rule_inputs.alignment = "CE";
 k.rule_inputs.laws = {pkkm::PromotionLaw::Aggressive, true, pkkm::TaxationLaw::Heavy, true, 12};
 k.rule_inputs.leadership.magister = {true, 2};
 k.rule_inputs.leadership.marshal = {true, 1};
 k.rule_inputs.leadership.treasurer = {true, 3};
 k.rule_inputs.leadership.spymaster = {true, 4};
 k.rule_inputs.leadership.spymaster_benefit = pkkm::KingdomStatFocus::Stability;
 k.rule_inputs.economy = {1, 2, 3};
 k.rule_inputs.kingdom_size = 24; k.rule_inputs.total_city_districts = 7;
 k.rule_inputs.city_count = 3; k.rule_inputs.total_city_population = 3250;
 pkkm::save(k,path); const auto r=pkkm::load(path);
 must(r.name==k.name && r.turn==4 && r.treasury_bp==120 && r.unrest==2,"roundtrip scalar fields");
 must(r.settlements.size()==1 && r.settlements[0].population==1200 && r.buildings[0].count==2,"roundtrip collections");
 must(r.settlements[0].districts==2 && r.settlements[0].economy==3 && r.settlements[0].loyalty==-1
      && r.settlements[0].stability==4 && r.settlements[0].defense==7,
      "roundtrip settlement workbook totals");
 must(r.schema_version==10 && r.settlements[0].id=="settlement.oak"
      && r.settlements[0].apply_catalog_stat_effects
      && r.settlements[0].building_inventory.size()==1
      && r.settlements[0].building_inventory[0].name=="Academy"
      && r.settlements[0].building_inventory[0].count==2,
      "schema 10 roundtrips existing settlement state");
 must(r.greenbelt_map.asset_path=="Kingmaker-Greenbelt-Map.jpg"
      && r.greenbelt_map.image_width==7865 && r.greenbelt_map.image_height==2610
      && r.greenbelt_map.calibration.origin_x==120.0
      && r.greenbelt_map.hexes.size()==1 && r.greenbelt_map.hexes[0].q==3
      && r.greenbelt_map.hexes[0].r==-2 && r.greenbelt_map.hexes[0].claimed
      && r.greenbelt_map.hexes[0].settlement_id=="settlement.oak"
      && r.greenbelt_map.hexes[0].improvements[0]=="farm",
      "schema 10 roundtrips Greenbelt calibration and linked hex state");
 must(r.greenbelt_map.special_terrain.size()==1
      && r.greenbelt_map.special_terrain[0].type=="river"
      && r.greenbelt_map.special_terrain[0].name=="North Fork"
      && r.greenbelt_map.custom_improvements.size()==1
      && r.greenbelt_map.custom_improvements[0].id=="custom.greenhouse"
      && r.greenbelt_map.custom_improvements[0].manual_only,
      "schema 10 roundtrips special terrain and custom Greenbelt improvements");
 const auto& greenbelt_catalog=pkkm::greenbelt_improvement_catalog();
 const auto* vineyard=pkkm::find_greenbelt_improvement(greenbelt_catalog,"vineyard");
 must(greenbelt_catalog.size()==14 && vineyard!=nullptr && vineyard->third_party
      && vineyard->terrain_requirement=="hills" && vineyard->cost_bp==3,
      "Greenbelt catalog includes the source-specific Vineyard entry");
 const auto farm_preview=pkkm::preview_greenbelt_improvement(
     greenbelt_catalog,"farm","plains",true,false);
 must(farm_preview.known && farm_preview.allowed && farm_preview.bp_delta==0
      && farm_preview.manual_effect && farm_preview.summary.find("Consumption")!=std::string::npos,
      "Greenbelt improvement preview records effects without spending BP");
 const auto highway_preview=pkkm::preview_greenbelt_improvement(
     greenbelt_catalog,"highway","plains",true,false);
 must(highway_preview.known && !highway_preview.allowed
      && !highway_preview.warnings.empty(),
      "Greenbelt preview preserves unmet Highway prerequisites as warnings");
 pkkm::GreenbeltImprovementPreviewFacts desert_farm_facts;
 desert_farm_facts.terrain="desert";
 desert_farm_facts.claimed=true;
 const auto blocked_desert_farm=pkkm::preview_greenbelt_improvement(
     greenbelt_catalog,"farm",desert_farm_facts,false);
 must(blocked_desert_farm.known && !blocked_desert_farm.allowed
      && !blocked_desert_farm.warnings.empty(),
      "Terrain preview rejects a desert Farm without water access");
 desert_farm_facts.river_adjacent=true;
 const auto allowed_desert_farm=pkkm::preview_greenbelt_improvement(
     greenbelt_catalog,"farm",desert_farm_facts,false);
 must(allowed_desert_farm.known && allowed_desert_farm.allowed,
      "Terrain preview accepts a desert Farm with river access");
 pkkm::GreenbeltImprovementPreviewFacts isolated_farm_facts;
 isolated_farm_facts.terrain="plains";
 isolated_farm_facts.claimed=true;
 const auto isolated_farm=pkkm::preview_greenbelt_improvement(
     greenbelt_catalog,"farm",isolated_farm_facts,false);
 must(std::any_of(isolated_farm.warnings.begin(), isolated_farm.warnings.end(),
      [](const auto& warning){ return warning.find("two existing Farms")!=std::string::npos; }),
      "Terrain preview warns when a Farm lacks water access and Farm adjacency");
 isolated_farm_facts.neighboring_farms=2;
 const auto adjacent_farm=pkkm::preview_greenbelt_improvement(
     greenbelt_catalog,"farm",isolated_farm_facts,false);
 must(!std::any_of(adjacent_farm.warnings.begin(), adjacent_farm.warnings.end(),
      [](const auto& warning){ return warning.find("two existing Farms")!=std::string::npos; }),
      "Terrain preview accepts Farm adjacency as an alternative prerequisite");
 pkkm::GreenbeltImprovementPreviewFacts highway_facts;
 highway_facts.terrain="plains";
 highway_facts.claimed=true;
 highway_facts.connected_road=true;
 highway_facts.kingdom_size=25;
 const auto undersized_highway=pkkm::preview_greenbelt_improvement(
     greenbelt_catalog,"highway",highway_facts,false);
 must(undersized_highway.known && !undersized_highway.allowed
      && !undersized_highway.warnings.empty(),
      "Terrain preview preserves the Highway kingdom-size prerequisite");
 highway_facts.kingdom_size=26;
 highway_facts.requested_upgrade="highway";
 const auto missing_road_upgrade=pkkm::preview_greenbelt_improvement(
     greenbelt_catalog,"highway",highway_facts,false);
 must(std::any_of(missing_road_upgrade.warnings.begin(), missing_road_upgrade.warnings.end(),
      [](const auto& warning){ return warning.find("existing Road")!=std::string::npos; }),
      "Terrain preview requires an existing Road for a Highway upgrade");
 highway_facts.existing_improvements.push_back("road");
 highway_facts.route_length=3;
 const auto road_upgrade=pkkm::preview_greenbelt_improvement(
     greenbelt_catalog,"highway",highway_facts,false);
 must(!std::any_of(road_upgrade.warnings.begin(), road_upgrade.warnings.end(),
      [](const auto& warning){ return warning.find("existing Road")!=std::string::npos
          || warning.find("Size 26")!=std::string::npos; }),
      "Terrain preview clears resolved Highway upgrade prerequisites");
 must(road_upgrade.summary.find("Route length 3")!=std::string::npos,
      "Terrain preview reports supplied route length without applying route effects");
 pkkm::GreenbeltImprovementPreviewFacts fort_upgrade_facts;
 fort_upgrade_facts.terrain="plains";
 fort_upgrade_facts.claimed=true;
 fort_upgrade_facts.requested_upgrade="fort";
 const auto missing_watchtower_upgrade=pkkm::preview_greenbelt_improvement(
     greenbelt_catalog,"fort",fort_upgrade_facts,false);
 must(std::any_of(missing_watchtower_upgrade.warnings.begin(), missing_watchtower_upgrade.warnings.end(),
      [](const auto& warning){ return warning.find("existing Watchtower")!=std::string::npos; }),
      "Terrain preview requires an existing Watchtower for a Fort upgrade");
 fort_upgrade_facts.existing_improvements.push_back("watchtower");
 const auto watchtower_upgrade=pkkm::preview_greenbelt_improvement(
     greenbelt_catalog,"fort",fort_upgrade_facts,false);
 must(!std::any_of(watchtower_upgrade.warnings.begin(), watchtower_upgrade.warnings.end(),
      [](const auto& warning){ return warning.find("existing Watchtower")!=std::string::npos; }),
      "Terrain preview clears resolved Fort upgrade prerequisites");
 pkkm::GreenbeltImprovementPreviewFacts canal_facts;
 canal_facts.terrain="plains";
 canal_facts.claimed=true;
 const auto plains_canal=pkkm::preview_greenbelt_improvement(
     greenbelt_catalog,"canal",canal_facts,false);
 must(!std::any_of(plains_canal.warnings.begin(), plains_canal.warnings.end(),
      [](const auto& warning){ return warning.find("Terrain does not meet")!=std::string::npos; }),
      "Terrain preview accepts supplemental Canal terrain");
 canal_facts.terrain="water";
 const auto water_canal=pkkm::preview_greenbelt_improvement(
     greenbelt_catalog,"canal",canal_facts,false);
 must(std::any_of(water_canal.warnings.begin(), water_canal.warnings.end(),
      [](const auto& warning){ return warning.find("Terrain does not meet")!=std::string::npos; }),
      "Terrain preview rejects a Canal on a water hex");
 pkkm::GreenbeltImprovementPreviewFacts duplicate_facts;
 duplicate_facts.terrain="plains";
 duplicate_facts.claimed=true;
 duplicate_facts.existing_improvements.push_back("aqueduct");
 const auto duplicate_aqueduct=pkkm::preview_greenbelt_improvement(
     greenbelt_catalog,"aqueduct",duplicate_facts,false);
 must(std::any_of(duplicate_aqueduct.warnings.begin(), duplicate_aqueduct.warnings.end(),
      [](const auto& warning){ return warning.find("not shareable")!=std::string::npos; }),
      "Terrain preview warns when a non-shareable improvement already occupies the hex");
 duplicate_facts.existing_improvements.push_back("farm");
 const auto shareable_farm=pkkm::preview_greenbelt_improvement(
     greenbelt_catalog,"farm",duplicate_facts,false);
 must(!std::any_of(shareable_farm.warnings.begin(), shareable_farm.warnings.end(),
      [](const auto& warning){ return warning.find("not shareable")!=std::string::npos; }),
      "Terrain preview permits shareable improvement occupancy");
 duplicate_facts.existing_improvements={"fort"};
 const auto fort_watchtower=pkkm::preview_greenbelt_improvement(
     greenbelt_catalog,"watchtower",duplicate_facts,false);
 must(std::any_of(fort_watchtower.warnings.begin(), fort_watchtower.warnings.end(),
      [](const auto& warning){ return warning.find("Fort or another Watchtower")!=std::string::npos; }),
      "Terrain preview preserves Watchtower and Fort exclusivity");
 duplicate_facts.existing_improvements={"watchtower"};
 const auto watchtower_duplicate=pkkm::preview_greenbelt_improvement(
     greenbelt_catalog,"watchtower",duplicate_facts,false);
 must(std::any_of(watchtower_duplicate.warnings.begin(), watchtower_duplicate.warnings.end(),
      [](const auto& warning){ return warning.find("Fort or another Watchtower")!=std::string::npos; }),
      "Terrain preview preserves duplicate Watchtower exclusivity");
 pkkm::GreenbeltImprovementPreviewFacts mine_facts;
 mine_facts.terrain="desert";
 mine_facts.claimed=true;
 const auto desert_mine=pkkm::preview_greenbelt_improvement(
     greenbelt_catalog,"mine",mine_facts,false);
 must(!std::any_of(desert_mine.warnings.begin(), desert_mine.warnings.end(),
      [](const auto& warning){ return warning.find("Terrain does not meet")!=std::string::npos; }),
      "Terrain preview accepts supplemental Mine terrain");
 mine_facts.terrain="cavern";
 const auto cavern_quarry=pkkm::preview_greenbelt_improvement(
     greenbelt_catalog,"quarry",mine_facts,false);
 must(!std::any_of(cavern_quarry.warnings.begin(), cavern_quarry.warnings.end(),
      [](const auto& warning){ return warning.find("Terrain does not meet")!=std::string::npos; }),
      "Terrain preview accepts supplemental Quarry terrain");
 mine_facts.terrain="jungle";
 const auto jungle_sawmill=pkkm::preview_greenbelt_improvement(
     greenbelt_catalog,"sawmill",mine_facts,false);
 must(!std::any_of(jungle_sawmill.warnings.begin(), jungle_sawmill.warnings.end(),
      [](const auto& warning){ return warning.find("Terrain does not meet")!=std::string::npos; }),
      "Terrain preview accepts supplemental Sawmill terrain");
 pkkm::GreenbeltImprovementPreviewFacts aqueduct_facts;
 aqueduct_facts.terrain="plains";
 aqueduct_facts.claimed=true;
 const auto incomplete_aqueduct=pkkm::preview_greenbelt_improvement(
     greenbelt_catalog,"aqueduct",aqueduct_facts,false);
 must(incomplete_aqueduct.summary.find("Rule profile: workbook")!=std::string::npos,
      "Terrain preview records the selected rule profile in its summary");
 aqueduct_facts.rule_profile=pkkm::RuleProfile::CityInfo;
 const auto city_info_aqueduct=pkkm::preview_greenbelt_improvement(
     greenbelt_catalog,"aqueduct",aqueduct_facts,false);
 must(city_info_aqueduct.summary.find("Rule profile: city_info")!=std::string::npos,
      "Terrain preview records the City Info rule profile in its summary");
 must(std::any_of(incomplete_aqueduct.warnings.begin(), incomplete_aqueduct.warnings.end(),
      [](const auto& warning){ return warning.find("connected water source")!=std::string::npos; }),
      "Terrain preview warns when an Aqueduct lacks a connected water source");
 must(std::any_of(incomplete_aqueduct.warnings.begin(), incomplete_aqueduct.warnings.end(),
      [](const auto& warning){ return warning.find("settlement endpoint")!=std::string::npos; }),
      "Terrain preview warns when an Aqueduct lacks a settlement endpoint");
 aqueduct_facts.water_adjacent=true;
 aqueduct_facts.settlement_endpoint=true;
 const auto valid_aqueduct=pkkm::preview_greenbelt_improvement(
     greenbelt_catalog,"aqueduct",aqueduct_facts,false);
 must(!std::any_of(valid_aqueduct.warnings.begin(), valid_aqueduct.warnings.end(),
      [](const auto& warning){ return warning.find("connected water source")!=std::string::npos
          || warning.find("settlement endpoint")!=std::string::npos; }),
      "Terrain preview accepts an Aqueduct with explicit water and settlement facts");
 pkkm::GreenbeltImprovementPreviewFacts crossing_facts;
 crossing_facts.terrain="plains";
 crossing_facts.claimed=true;
 crossing_facts.river_crossing=true;
 const auto crossing_road=pkkm::preview_greenbelt_improvement(
     greenbelt_catalog,"road",crossing_facts,false);
 must(crossing_road.summary.find("River crossing supplied")!=std::string::npos,
      "Terrain preview reports supplied Road river crossings without applying costs");
 crossing_facts.terrain="river";
 crossing_facts.river_crossing=false;
 const auto incomplete_bridge=pkkm::preview_greenbelt_improvement(
     greenbelt_catalog,"bridge",crossing_facts,false);
 must(std::any_of(incomplete_bridge.warnings.begin(), incomplete_bridge.warnings.end(),
      [](const auto& warning){ return warning.find("river crossing fact")!=std::string::npos; }),
      "Terrain preview warns when a Bridge lacks an explicit river crossing fact");
 crossing_facts.river_crossing=true;
 const auto valid_bridge=pkkm::preview_greenbelt_improvement(
     greenbelt_catalog,"bridge",crossing_facts,false);
 must(!std::any_of(valid_bridge.warnings.begin(), valid_bridge.warnings.end(),
      [](const auto& warning){ return warning.find("river crossing fact")!=std::string::npos; }),
      "Terrain preview accepts a Bridge with an explicit river crossing fact");
 const auto center=pkkm::greenbelt_hex_center({100.0,50.0,20.0,10.0,1.0},2,4);
 must(center.x==180.0 && center.y==90.0,"Greenbelt calibration projects axial coordinates");
 const auto single_perimeter=pkkm::greenbelt_claimed_perimeter({{0,0,"plains",true,1,"",{}}});
 const auto adjacent_perimeter=pkkm::greenbelt_claimed_perimeter({
     {0,0,"plains",true,1,"",{}},{1,0,"plains",true,1,"",{}}});
 must(single_perimeter.size()==6 && adjacent_perimeter.size()==10,
      "Greenbelt perimeter removes shared edges between adjacent claimed hexes");
 must(r.calendar_notes.size()==2 && r.calendar_notes[0].entry_id=="calendar.row.2"
      && r.calendar_notes[0].other=="Opening month"
      && r.calendar_notes[1].entry_id=="calendar.row.14"
      && r.calendar_notes[1].holiday=="Founding festival"
      && r.calendar_notes[1].kingdom_upgrades=="Build the bridge"
      && r.calendar_notes[1].events=="Dragon sighting" && r.calendar_notes[1].other.empty(),
      "schema 10 keeps notes for repeated month names on separate source occurrences");
 must(r.turn_progress.size()==2 && r.turn_progress[0].turn==4
      && r.turn_progress[0].completed_step_ids.size()==2
      && r.turn_progress[0].completed_step_ids[0]=="upkeep.stability"
      && r.turn_progress[1].turn==5
      && r.turn_progress[1].completed_step_ids[0]=="events.roll_d100",
      "schema 8 roundtrips manual checklist completion independently by turn");
 const auto& turn_steps=pkkm::turn_checklist_steps();
 must(turn_steps.size()==20 && turn_steps.front().id=="upkeep.stability"
      && turn_steps.front().phase=="Phase 1 Upkeep"
      && turn_steps.front().marker=="Step 1"
      && turn_steps.back().id=="events.roll_d100"
      && turn_steps.back().label=="Roll d% for event",
      "manual checklist exposes the 20 source procedure items in phase order");
 must(std::count_if(turn_steps.begin(),turn_steps.end(),[](const auto& step){return step.optional;})==4,
      "manual checklist preserves all four source optional tasks");
 must(pkkm::classify_stability_failure(3)==pkkm::StabilityFailureOutcome::FailByLessThanFour
      && pkkm::classify_stability_failure(4)==pkkm::StabilityFailureOutcome::UnresolvedExactlyFour
      && pkkm::classify_stability_failure(5)==pkkm::StabilityFailureOutcome::FailByAtLeastFive,
      "stability failure boundary remains explicit instead of inventing the missing exact-four outcome");
 const auto claim_hex=pkkm::resolve_turn_action(pkkm::TurnAction::ClaimHex);
 const auto abandon_hex=pkkm::resolve_turn_action(pkkm::TurnAction::AbandonHex);
 const auto abandon_city=pkkm::resolve_turn_action(pkkm::TurnAction::AbandonCity);
 must(claim_hex.treasury_delta==-1 && claim_hex.kingdom_size_delta==1 && claim_hex.unrest_delta==0
      && abandon_hex.treasury_delta==0 && abandon_hex.kingdom_size_delta==-1 && abandon_hex.unrest_delta==1
      && abandon_city.treasury_delta==0 && abandon_city.kingdom_size_delta==-1 && abandon_city.unrest_delta==4,
      "explicit Turn hex and city actions preserve workbook deltas");
 pkkm::Kingdom advanced=k;
 advanced.turn=4;
 advanced.turn_progress={{4,{"upkeep.stability"}}};
 pkkm::advance_turn(advanced);
 must(advanced.turn==5 && advanced.turn_progress.size()==1
      && advanced.turn_progress[0].turn==4
      && advanced.turn_progress[0].completed_step_ids[0]=="upkeep.stability",
      "advancing a turn preserves prior checklist history without inventing outcomes");
 pkkm::Kingdom action_k=k;
 pkkm::apply_turn_action(action_k, pkkm::TurnAction::ClaimHex, 2);
 must(action_k.treasury_bp==118 && action_k.rule_inputs.kingdom_size==26
      && action_k.unrest==2 && action_k.turn_actions.size()==1
      && action_k.turn_actions[0].turn==4 && action_k.turn_actions[0].count==2
      && action_k.turn_actions[0].applied.treasury_delta==-2,
      "Claim Hex applies only its explicit delta and records the action");
 const std::string action_path="pkkm_test_turn_actions.json";
 pkkm::save(action_k, action_path);
 const auto action_r=pkkm::load(action_path);
 must(action_r.turn_actions.size()==1
      && action_r.turn_actions[0].action==pkkm::TurnAction::ClaimHex
      && action_r.turn_actions[0].count==2
      && action_r.turn_actions[0].applied.kingdom_size_delta==2,
      "Turn action records roundtrip through JSON persistence");
 std::remove(action_path.c_str());
 pkkm::Kingdom reversal_k=k;
 pkkm::apply_turn_action(reversal_k, pkkm::TurnAction::ClaimHex);
 pkkm::apply_turn_action(reversal_k, pkkm::TurnAction::AbandonCity);
 pkkm::reverse_turn_action(reversal_k, 0);
 must(reversal_k.treasury_bp==120 && reversal_k.rule_inputs.kingdom_size==23
      && reversal_k.unrest==6 && reversal_k.turn_actions.size()==3
      && reversal_k.turn_actions[2].reversal
      && reversal_k.turn_actions[2].reverses_index==0
      && reversal_k.turn_actions[2].applied.treasury_delta==1
      && reversal_k.turn_actions[2].applied.kingdom_size_delta==-1,
      "Selected Turn Action reversal restores explicit state and appends an indexed audit record");
 const std::string reversal_path="pkkm_test_turn_action_reversal.json";
 pkkm::save(reversal_k, reversal_path);
 const auto reversal_r=pkkm::load(reversal_path);
 must(reversal_r.turn_actions.size()==3 && reversal_r.turn_actions[2].reversal
      && reversal_r.turn_actions[2].reverses_index==0,
      "Turn action reversal state roundtrips through JSON persistence");
 std::remove(reversal_path.c_str());
 must_reject([&]{ pkkm::reverse_turn_action(reversal_k, 0); });
 must_reject([&]{ pkkm::reverse_turn_action(reversal_k, 2); });
 const auto size_10=pkkm::kingdom_size_limits(10);
 const auto size_11=pkkm::kingdom_size_limits(11);
 const auto size_26=pkkm::kingdom_size_limits(26);
 const auto size_101=pkkm::kingdom_size_limits(101);
 const auto size_201=pkkm::kingdom_size_limits(201);
 must(size_10.new_settlements==1 && size_10.new_buildings==1 && !size_10.unlimited_new_buildings
      && size_10.improvements_per_hex==2 && size_10.hex_claims==1
      && size_11.new_buildings==2 && size_26.new_buildings==5
      && size_101.new_settlements==3 && size_101.improvements_per_hex==9 && size_101.hex_claims==8
      && size_201.new_settlements==4 && size_201.unlimited_new_buildings
      && size_201.improvements_per_hex==12 && size_201.hex_claims==12,
      "kingdom-size Turn reference boundaries preserve source table values");
 const auto economy_rates=pkkm::turn_economy_rates();
 must(economy_rates.gold_per_withdrawn_bp==2000 && economy_rates.gold_per_deposited_bp==4000
      && economy_rates.item_value_per_bp==8000 && economy_rates.unrest_per_withdrawn_bp==1
      && economy_rates.gold_per_buy_for_kingdom_bp==2000,
      "income Turn reference rates preserve source conversion values");
 must(pkkm::resolve_tax_bp_delta(true, 4, 2)==6
      && pkkm::resolve_tax_bp_delta(false, 99, 2)==2,
      "tax Turn branches preserve success and failure BP outcomes");
 must(pkkm::resolve_tax_bp_delta(pkkm::RuleProfile::CityInfo, true, 10, 2)==4,
      "City Info tax profile divides the economy result by five");
 pkkm::Kingdom tax_k=k;
 pkkm::apply_tax_action(tax_k, true, 4, 2);
 must(tax_k.treasury_bp==126 && tax_k.tax_actions.size()==1
      && tax_k.tax_actions[0].turn==4 && tax_k.tax_actions[0].treasury_delta==6,
      "explicit tax action applies only the source-defined BP result and records its branch inputs");
 const std::string tax_path="pkkm_test_tax_actions.json";
 pkkm::save(tax_k, tax_path);
 const auto tax_r=pkkm::load(tax_path);
 must(tax_r.tax_actions.size()==1 && tax_r.tax_actions[0].economy_check_success
      && tax_r.tax_actions[0].result_input==4
      && tax_r.tax_actions[0].other_bp_gained==2
      && tax_r.tax_actions[0].treasury_delta==6,
      "tax action records roundtrip through JSON persistence");
 auto legacy_tax_json=nlohmann::json::parse(contents(tax_path));
 legacy_tax_json["tax_actions"][0]["result_divided_by_three"]
     = legacy_tax_json["tax_actions"][0]["result_input"];
 legacy_tax_json["tax_actions"][0].erase("result_input");
 put(tax_path, legacy_tax_json.dump());
 const auto legacy_tax_r=pkkm::load(tax_path);
 must(legacy_tax_r.tax_actions.size()==1
      && legacy_tax_r.tax_actions[0].result_input==4
      && legacy_tax_r.tax_actions[0].rule_profile==pkkm::RuleProfile::Workbook,
      "legacy tax result_divided_by_three records migrate to profile-neutral input");
 std::remove(tax_path.c_str());
 pkkm::Kingdom withdrawal_k=k;
 pkkm::apply_gold_withdrawal(withdrawal_k, 3, true);
 must(withdrawal_k.treasury_bp==117 && withdrawal_k.unrest==5
      && withdrawal_k.gold_withdrawals.size()==1
      && withdrawal_k.gold_withdrawals[0].gold_received==6000,
      "gold withdrawal applies the explicit BP, gold, and unrest conversion");
 const std::string withdrawal_path="pkkm_test_gold_withdrawal.json";
 pkkm::save(withdrawal_k, withdrawal_path);
 const auto withdrawal_r=pkkm::load(withdrawal_path);
 must(withdrawal_r.gold_withdrawals.size()==1
      && withdrawal_r.gold_withdrawals[0].bp_withdrawn==3
      && withdrawal_r.gold_withdrawals[0].gold_received==6000
      && withdrawal_r.gold_withdrawals[0].treasury_delta==-3
      && withdrawal_r.gold_withdrawals[0].unrest_delta==3,
      "gold withdrawal records roundtrip through JSON persistence");
 std::remove(withdrawal_path.c_str());
 pkkm::Kingdom withdrawal_failure_k=k;
 pkkm::apply_gold_withdrawal(withdrawal_failure_k, 3, false);
 must(withdrawal_failure_k.treasury_bp==117 && withdrawal_failure_k.unrest==8
      && withdrawal_failure_k.gold_withdrawals[0].loyalty_check_success==false
      && withdrawal_failure_k.gold_withdrawals[0].loyalty_failure_unrest_delta==3
      && withdrawal_failure_k.gold_withdrawals[0].unrest_delta==6,
      "failed withdrawal Loyalty Check adds unrest equal to BP withdrawn");
 must(pkkm::resolve_gold_deposit_bp(8000)==2,
      "gold deposit converts exact 4,000 gp units to BP");
 pkkm::Kingdom deposit_k=k;
 pkkm::apply_gold_deposit(deposit_k, 8000);
 must(deposit_k.treasury_bp==122 && deposit_k.gold_deposits.size()==1
      && deposit_k.gold_deposits[0].gold_deposited==8000
      && deposit_k.gold_deposits[0].bp_received==2
      && deposit_k.gold_deposits[0].treasury_delta==2,
      "gold deposit applies only the explicit BP conversion and records it");
 const std::string deposit_path="pkkm_test_gold_deposit.json";
 pkkm::save(deposit_k, deposit_path);
 const auto deposit_r=pkkm::load(deposit_path);
 must(deposit_r.gold_deposits.size()==1 && deposit_r.gold_deposits[0].gold_deposited==8000
      && deposit_r.gold_deposits[0].bp_received==2,
      "gold deposit records roundtrip through JSON persistence");
 std::remove(deposit_path.c_str());
 must_reject([&]{ (void)pkkm::resolve_gold_deposit_bp(4001); });
 must(pkkm::resolve_item_deposit_bp(16000)==2,
      "item deposit converts exact 8,000 gp base-value units to BP");
 must(pkkm::resolve_item_deposit_bp(pkkm::RuleProfile::CityInfo, 16000)==4,
      "City Info item deposit converts exact 4,000 gp units to BP");
 pkkm::Kingdom item_deposit_k=k;
 pkkm::apply_item_deposit(item_deposit_k, 16000);
 must(item_deposit_k.treasury_bp==122 && item_deposit_k.item_deposits.size()==1
      && item_deposit_k.item_deposits[0].item_value==16000
      && item_deposit_k.item_deposits[0].bp_received==2
      && item_deposit_k.item_deposits[0].treasury_delta==2,
      "item deposit applies only the explicit BP conversion and records it");
 const std::string item_deposit_path="pkkm_test_item_deposit.json";
 pkkm::save(item_deposit_k, item_deposit_path);
 const auto item_deposit_r=pkkm::load(item_deposit_path);
 must(item_deposit_r.item_deposits.size()==1
      && item_deposit_r.item_deposits[0].item_value==16000
      && item_deposit_r.item_deposits[0].bp_received==2,
      "item deposit records roundtrip through JSON persistence");
 std::remove(item_deposit_path.c_str());
 pkkm::Kingdom city_info_item_deposit_k=k;
 city_info_item_deposit_k.rule_profile=pkkm::RuleProfile::CityInfo;
 pkkm::apply_item_deposit(city_info_item_deposit_k, 16000);
 must(city_info_item_deposit_k.treasury_bp==124
      && city_info_item_deposit_k.item_deposits[0].bp_received==4
      && city_info_item_deposit_k.item_deposits[0].rule_profile==pkkm::RuleProfile::CityInfo,
      "City Info item deposit applies its profile conversion");
 must_reject([&]{ (void)pkkm::resolve_item_deposit_bp(8001); });
 pkkm::Kingdom loyalty_success_k=k;
 loyalty_success_k.unrest=2;
 pkkm::apply_loyalty_check(loyalty_success_k, true);
 must(loyalty_success_k.unrest==1 && loyalty_success_k.loyalty_checks.size()==1
      && loyalty_success_k.loyalty_checks[0].unrest_delta==-1
      && loyalty_success_k.loyalty_checks[0].loyalty_delta==0,
      "successful Loyalty Check reduces unrest and records its branch");
 pkkm::Kingdom loyalty_failure_k=k;
 const int loyalty_other_before=loyalty_failure_k.rule_inputs.loyalty.other;
 pkkm::apply_loyalty_check(loyalty_failure_k, false);
 must(loyalty_failure_k.rule_inputs.loyalty.other==loyalty_other_before-1
      && loyalty_failure_k.loyalty_checks.size()==1
      && loyalty_failure_k.loyalty_checks[0].loyalty_delta==-1
      && loyalty_failure_k.loyalty_checks[0].unrest_delta==0,
      "failed Loyalty Check applies the permanent Loyalty penalty");
 const std::string loyalty_path="pkkm_test_loyalty_check.json";
 pkkm::save(loyalty_failure_k, loyalty_path);
 const auto loyalty_r=pkkm::load(loyalty_path);
 must(loyalty_r.loyalty_checks.size()==1 && !loyalty_r.loyalty_checks[0].success
      && loyalty_r.loyalty_checks[0].loyalty_delta==-1,
      "Loyalty Check records roundtrip through JSON persistence");
 std::remove(loyalty_path.c_str());
 pkkm::Kingdom commission_k=k;
 commission_k.treasury_bp=10;
 pkkm::apply_commission(commission_k, "Minor Commission", "Alyssa", 3, 2);
 pkkm::apply_commission(commission_k, "Medium Commission", "Borin", 2, 2);
 must(commission_k.treasury_bp==5 && commission_k.commissions.size()==2
      && commission_k.commissions[0].item_name=="Minor Commission"
      && commission_k.commissions[0].recipient=="Alyssa",
      "commission records manually entered item and commissioning player");
 must_reject([&]{ pkkm::apply_commission(commission_k, "Third Commission", "Cora", 1, 2); });
 const std::string commission_path="pkkm_test_commissions.json";
 pkkm::save(commission_k, commission_path);
 const auto commission_r=pkkm::load(commission_path);
 must(commission_r.commissions.size()==2 && commission_r.commissions[1].cost_bp==2
      && commission_r.commissions[1].exotic_crafters_available==2,
      "commission records roundtrip through JSON persistence");
 std::remove(commission_path.c_str());
 pkkm::Kingdom unavailable_crafter_k=k;
 must_reject([&]{ pkkm::apply_commission(unavailable_crafter_k, "Unavailable", "Dara", 1, 0); });
 pkkm::Kingdom city_info_profile_k=k;
 city_info_profile_k.rule_profile=pkkm::RuleProfile::CityInfo;
 const std::string profile_path="pkkm_test_rule_profile.json";
 pkkm::save(city_info_profile_k, profile_path);
 const auto profile_r=pkkm::load(profile_path);
 must(profile_r.rule_profile==pkkm::RuleProfile::CityInfo
      && std::string(pkkm::rule_profile_name(profile_r.rule_profile))=="city_info",
      "rule profile selection roundtrips through JSON persistence");
 std::remove(profile_path.c_str());
 must(pkkm::rule_profile_from_name("workbook")==pkkm::RuleProfile::Workbook,
      "workbook rule profile name resolves");
 must_reject([&]{ (void)pkkm::rule_profile_from_name("unknown"); });
 must(pkkm::resolve_stability_check(true, 0).treasury_delta==1,
      "successful stability check at zero unrest yields one BP");
 must(pkkm::resolve_stability_check(true, 3).unrest_delta==-1,
      "successful stability check reduces positive unrest");
 must(pkkm::resolve_stability_check(false, 0, 3).unrest_delta==1,
      "stability failure by less than four yields one unrest");
 must(pkkm::resolve_stability_check(false, 0, 5, 4).unrest_delta==4,
      "stability failure by five or more preserves caller-supplied d4 unrest");
 must(pkkm::resolve_stability_check(pkkm::RuleProfile::CityInfo, false, 0, 5, 4).unrest_delta==2,
      "City Info stability failure by five or more applies two unrest");
 must_reject([&]{ (void)pkkm::resolve_stability_check(false, 0, 4, 0); });
 must_reject([&]{ (void)pkkm::resolve_stability_check(false, 0, 5, 5); });
 pkkm::Kingdom item_outcome_k=k;
 pkkm::apply_magic_item_outcome(item_outcome_k, pkkm::MagicItemCategory::Minor, true);
 pkkm::apply_magic_item_outcome(item_outcome_k, pkkm::MagicItemCategory::Medium, false);
 pkkm::apply_magic_item_outcome(item_outcome_k, pkkm::MagicItemCategory::Major, false, true);
 must_reject([&]{ pkkm::apply_magic_item_outcome(item_outcome_k,
      pkkm::MagicItemCategory::Minor, true, true); });
  auto invalid_magic_record=item_outcome_k;
  invalid_magic_record.magic_item_outcomes.push_back({invalid_magic_record.turn,
      pkkm::MagicItemCategory::Minor, true, true, false, 0});
  must_reject([&]{ pkkm::validate(invalid_magic_record); });
 must(item_outcome_k.treasury_bp==122 && item_outcome_k.magic_item_outcomes.size()==3
      && item_outcome_k.magic_item_outcomes[0].bp_received==2
      && item_outcome_k.magic_item_outcomes[1].lost
      && item_outcome_k.magic_item_outcomes[2].commissioned
      && item_outcome_k.magic_item_outcomes[2].bp_received==0,
      "manual magic-item outcomes calculate sale BP and preserve commissioned exemptions");
 const std::string item_outcome_path="pkkm_test_magic_item_outcomes.json";
 pkkm::save(item_outcome_k, item_outcome_path);
 const auto item_outcome_r=pkkm::load(item_outcome_path);
 must(item_outcome_r.magic_item_outcomes.size()==3
      && item_outcome_r.magic_item_outcomes[1].lost
      && item_outcome_r.magic_item_outcomes[2].commissioned,
      "magic-item outcome records roundtrip through JSON persistence");
 std::remove(item_outcome_path.c_str());
 pkkm::Kingdom stability_k=k;
 stability_k.unrest=0;
 pkkm::apply_stability_check(stability_k, true);
 must(stability_k.treasury_bp==121 && stability_k.stability_checks.size()==1
      && stability_k.stability_checks[0].treasury_delta==1,
      "successful stability check applies the zero-unrest BP branch");
 stability_k.unrest=3;
 pkkm::apply_stability_check(stability_k, true);
 must(stability_k.unrest==2 && stability_k.stability_checks.size()==2,
      "successful stability check applies the positive-unrest branch");
 const std::string stability_path="pkkm_test_stability.json";
 pkkm::save(stability_k, stability_path);
 const auto stability_r=pkkm::load(stability_path);
 must(stability_r.stability_checks.size()==2 && stability_r.stability_checks[1].unrest_delta==-1,
      "stability check records roundtrip through JSON persistence");
 std::remove(stability_path.c_str());
 pkkm::Kingdom consumption_k=k;
 consumption_k.treasury_bp=3;
 consumption_k.unrest=0;
 pkkm::apply_consumption_action(consumption_k, 2);
 must(consumption_k.treasury_bp==1 && consumption_k.unrest==0
      && consumption_k.consumption_actions[0].paid,
      "consumption is paid when treasury covers the caller-supplied amount");
 pkkm::apply_consumption_action(consumption_k, 2);
 must(consumption_k.treasury_bp==1 && consumption_k.unrest==2
      && !consumption_k.consumption_actions[1].paid,
      "insufficient consumption payment adds the source-defined two unrest");
 const std::string consumption_path="pkkm_test_consumption.json";
 pkkm::save(consumption_k, consumption_path);
 const auto consumption_r=pkkm::load(consumption_path);
 must(consumption_r.consumption_actions.size()==2 && !consumption_r.consumption_actions[1].paid,
      "consumption records roundtrip through JSON persistence");
 std::remove(consumption_path.c_str());
 pkkm::Kingdom unrest_k=k;
 unrest_k.unrest=2;
 pkkm::apply_unrest_modification(unrest_k, 3);
 must(unrest_k.unrest==5 && unrest_k.unrest_modifications.size()==1
      && unrest_k.unrest_modifications[0].unrest_delta==3,
      "negative kingdom stat count adds one unrest per stat");
 const std::string unrest_path="pkkm_test_unrest_modification.json";
 pkkm::save(unrest_k, unrest_path);
 const auto unrest_r=pkkm::load(unrest_path);
 must(unrest_r.unrest_modifications.size()==1
      && unrest_r.unrest_modifications[0].negative_stat_count==3,
      "unrest modification records roundtrip through JSON persistence");
 std::remove(unrest_path.c_str());
 const auto magic_items=pkkm::magic_item_workflow();
 must(magic_items.buy_item_moves_to_pc && magic_items.success_recycles_item
      && !magic_items.failure_recycles_item && magic_items.excess_checks_economy_delta==-1
      && magic_items.gold_per_buy_for_kingdom_bp==2000 && magic_items.buy_for_kingdom_requires_use,
      "magic-item Turn workflow preserves explicit optional branches");
 const auto event_cadence=pkkm::event_cadence();
 must(event_cadence.no_event_last_month_percent==75
      && event_cadence.event_last_month_percent==25,
      "event Turn cadence preserves explicit source percentages");
 must(pkkm::classify_event_resolution(false)==pkkm::EventResolution::NoEvent
      && pkkm::classify_event_resolution(true)==pkkm::EventResolution::ManualResolutionRequired,
      "event Turn resolution remains an explicit manual boundary");
 const auto& calendar=pkkm::calendar_template_entries();
 must(calendar.size()==85 && calendar.front().id=="calendar.row.2"
      && calendar.front().source_row==2 && calendar.front().month=="Pharast (March)"
      && calendar[12].source_row==14 && calendar[12].year_marker=="1 yr!"
      && calendar[24].source_row==26 && calendar[24].year_marker=="2 years!"
      && calendar[36].source_row==38 && calendar[36].year_marker=="year 3"
      && calendar.back().source_row==86 && calendar.back().month=="Pharast (March)",
      "calendar template preserves all 85 source occurrences and literal markers in order");
 const std::vector<std::string> source_months={"Pharast (March)","Gozran (April)","Desnus (may)","Sarenith (June)",
     "Erastus (July)","Arodus (Aug)","Rova (Sept)","Lamashan (Oct)","Neth (Nov)","Kuthona (Dec)",
     "Abadus (Jan)","Kalestril (Feb)"};
 const auto expected_marker=[](size_t index)->std::string {
     if(index==12)return "1 yr!"; if(index==24)return "2 years!"; if(index==36)return "year 3"; return {};
 };
 bool calendar_matches_source=true;
 for(size_t index=0;index<calendar.size();++index)
     calendar_matches_source=calendar_matches_source && calendar[index].source_row==static_cast<int>(index)+2
         && calendar[index].month==source_months[index%source_months.size()]
         && calendar[index].year_marker==expected_marker(index);
 must(calendar_matches_source,"all 85 month labels and three year markers match the read-only workbook sequence");
 must(r.settlements[0].map.rows==8 && r.settlements[0].map.columns==8
      && r.settlements[0].map.placements.size()==1
      && r.settlements[0].map.placements[0].building_name=="Castle"
      && r.settlements[0].map.placements[0].row==5 && r.settlements[0].map.placements[0].column==5
      && r.settlements[0].map.placements[0].width==2 && r.settlements[0].map.placements[0].height==2,
      "settlement map grid and explicit building footprint roundtrip");
 auto map_v7_fixture=nlohmann::json::parse(contents(path));
 map_v7_fixture["schema_version"]=7;
 map_v7_fixture.erase("turn_progress");
 map_v7_fixture.erase("calendar_notes");
 map_v7_fixture["settlements"][0]["map"]["rows"]=15;
 map_v7_fixture["settlements"][0]["map"]["columns"]=15;
 map_v7_fixture["settlements"][0]["map"]["labels"]={
     {{"text","Hill"},{"row",0},{"column",1},{"width",6},{"height",1}},
     {{"text","Forest"},{"row",1},{"column",7},{"width",1},{"height",6}},
     {{"text","Gate"},{"row",0},{"column",10},{"width",2},{"height",1}}};
 const std::string map_v7_path="pkkm_map_v7_test.json";
 put(map_v7_path,map_v7_fixture.dump(2));
 const auto labeled_map=pkkm::load(map_v7_path);
 const std::string map_v7_saved_path="pkkm_map_v7_roundtrip_test.json";
 pkkm::save(labeled_map,map_v7_saved_path);
 const auto labeled_json=nlohmann::json::parse(contents(map_v7_saved_path));
 must(labeled_map.schema_version==10 && labeled_map.turn_progress.empty() && labeled_map.calendar_notes.empty()
      && labeled_json["settlements"][0]["map"]["labels"].size()==3
      && labeled_json["settlements"][0]["map"]["labels"][0]["text"]=="Hill"
      && labeled_json["settlements"][0]["map"]["labels"][1]["row"]==1
      && labeled_json["settlements"][0]["map"]["labels"][2]["width"]==2,
      "schema 7 roundtrips terrain and corridor labels as explicit map regions");
 auto legacy_v8=nlohmann::json::parse(contents(path));
 legacy_v8["schema_version"]=8;
 legacy_v8.erase("calendar_notes");
 put(path,legacy_v8.dump(2));
 const auto migrated_v8=pkkm::load(path);
 must(migrated_v8.schema_version==10 && migrated_v8.turn_progress.size()==2
      && migrated_v8.calendar_notes.empty(),
      "schema 8 migrates turn checklist history and defaults calendar notes to empty");
 must(migrated_v8.greenbelt_map.hexes.empty() && migrated_v8.greenbelt_map.asset_path.empty(),
      "schema 8 migrates to an empty Greenbelt map");
 auto out_of_bounds_label=map_v7_fixture;
 out_of_bounds_label["settlements"][0]["map"]["labels"][0]["column"]=14;
 put(map_v7_path,out_of_bounds_label.dump(2));
 must_reject([&]{(void)pkkm::load(map_v7_path);});
 auto overlapping_labels=map_v7_fixture;
 overlapping_labels["settlements"][0]["map"]["labels"].push_back(
     {{"text","Overlap"},{"row",0},{"column",3},{"width",1},{"height",1}});
 put(map_v7_path,overlapping_labels.dump(2));
 must_reject([&]{(void)pkkm::load(map_v7_path);});
 auto label_over_structure=map_v7_fixture;
 label_over_structure["settlements"][0]["map"]["labels"].push_back(
     {{"text","Hidden"},{"row",5},{"column",5},{"width",1},{"height",1}});
 put(map_v7_path,label_over_structure.dump(2));
 must_reject([&]{(void)pkkm::load(map_v7_path);});
 const auto capital_map=pkkm::workbook_map_template(pkkm::WorkbookMapTemplate::Capital);
 const auto city_map=pkkm::workbook_map_template(pkkm::WorkbookMapTemplate::City);
 const auto example_map=pkkm::workbook_map_template(pkkm::WorkbookMapTemplate::Example);
 const auto multi_district_map=pkkm::workbook_map_template(pkkm::WorkbookMapTemplate::MultiDistrict);
 const auto matches_label=[](const pkkm::MapLabel& label,const std::string& text,
     int row,int column,int width,int height){
     return label.text==text && label.row==row && label.column==column
         && label.width==width && label.height==height;
 };
 must(capital_map.rows==8 && capital_map.columns==8 && capital_map.placements.size()==1
      && capital_map.placements[0].building_name=="Castle" && capital_map.placements[0].row==5
      && capital_map.placements[0].column==5 && capital_map.placements[0].width==2
      && capital_map.placements[0].height==2 && capital_map.labels.size()==4,
      "Capital Map preset preserves its 8x8 border labels and merged Castle region");
 must(matches_label(capital_map.labels[0],"Hill",0,1,6,1)
      && matches_label(capital_map.labels[1],"Forest",1,0,1,6)
      && matches_label(capital_map.labels[2],"Water",1,7,1,6)
      && matches_label(capital_map.labels[3],"Water",7,1,6,1),
      "Capital Map preset copies all four merged border ranges");
 must(city_map.rows==8 && city_map.columns==8 && city_map.placements.empty()
      && city_map.labels.size()==4 && matches_label(city_map.labels[0],"Hill",0,1,6,1)
      && matches_label(city_map.labels[1],"Forest",1,0,1,6)
      && matches_label(city_map.labels[2],"Water",1,7,1,6)
      && matches_label(city_map.labels[3],"Water",7,1,6,1),
      "City Map preset contains visual borders but no building placement");
 must(example_map.rows==8 && example_map.columns==8 && example_map.placements.empty()
      && example_map.labels.size()==7 && matches_label(example_map.labels[4],"2x lots",5,1,1,2)
      && matches_label(example_map.labels[5],"To be built",5,2,1,1)
      && matches_label(example_map.labels[6],"4x lots eg. castle",5,5,2,2),
      "Example Map preset keeps lot captions as visual annotations with exact merged ranges");
 must(multi_district_map.rows==15 && multi_district_map.columns==15
      && multi_district_map.placements.empty() && multi_district_map.labels.size()==14
      && matches_label(multi_district_map.labels[0],"Hill",0,1,6,1)
      && matches_label(multi_district_map.labels[1],"Hill",0,8,2,1)
      && matches_label(multi_district_map.labels[2],"Gate",0,10,2,1)
      && matches_label(multi_district_map.labels[3],"Hill",0,12,2,1)
      && matches_label(multi_district_map.labels[4],"Forest",1,0,1,6)
      && matches_label(multi_district_map.labels[5],"Forest",1,7,1,6)
      && matches_label(multi_district_map.labels[6],"Water",1,14,1,6)
      && matches_label(multi_district_map.labels[7],"Forest",7,1,6,1)
      && matches_label(multi_district_map.labels[8],"Hill",7,8,6,1)
      && matches_label(multi_district_map.labels[9],"Forest",8,0,1,6)
      && matches_label(multi_district_map.labels[10],"Forest",8,7,1,6)
      && matches_label(multi_district_map.labels[11],"Water",8,14,1,6)
      && matches_label(multi_district_map.labels[12],"Water",14,1,6,1)
      && matches_label(multi_district_map.labels[13],"Water",14,8,6,1),
      "Multi-District Map preset preserves all 14 merged annotations without mechanics");
 must(r.rule_inputs.alignment=="CE" && r.rule_inputs.economy.events==1
      && r.rule_inputs.leadership.spymaster.bonus==4 && r.rule_inputs.kingdom_size==24,
      "roundtrip workbook-derived rule inputs");
 auto summary=pkkm::calculate_kingdom_summary(r.unrest,r.rule_inputs);
 must(summary.control_dc==51 && summary.population==8500 && summary.laws.consumption==6,
      "kingdom summary combines persisted rule inputs");
 must(summary.economy.total==15 && summary.loyalty.total==-7 && summary.stability.total==1,
      "kingdom summary calculates all three workbook statistics");
 must(summary.economy.events==1 && summary.economy.alignment==2 && summary.economy.improvements==2
      && summary.economy.leadership==6 && summary.economy.laws==3 && summary.economy.unrest_adjustment==-2
      && summary.economy.vacancy_adjustment==0 && summary.economy.other==3
      && summary.economy.events+summary.economy.alignment+summary.economy.improvements+summary.economy.leadership
         +summary.economy.laws+summary.economy.unrest_adjustment+summary.economy.vacancy_adjustment+summary.economy.other
         ==summary.economy.total,
      "kingdom summary exposes signed Economy modifier breakdown");
 must(summary.stability.unrest_adjustment==-2 && summary.stability.vacancy_adjustment==-4,
      "kingdom summary explains unrest and vacancy deductions as signed contributions");
 const auto settlement_totals=pkkm::aggregate_settlements({
     {"Oak",1200,2,3,-1,4,7}, {"Pine",800,1,2,3,-2,5}});
 must(settlement_totals.count==2 && settlement_totals.population==2000 && settlement_totals.districts==3
      && settlement_totals.economy==5 && settlement_totals.loyalty==2
      && settlement_totals.stability==2 && settlement_totals.defense==12,
      "settlement aggregation sums city-sheet totals and counts named settlements");
 must_reject([] { (void)pkkm::aggregate_settlements({{"Bad",0,-1}}); });
 auto map_15 = k;
 map_15.settlements[0].map = {15, 15, {{"Gate", 0, 10, 2, 1}}};
 pkkm::validate(map_15);
 auto overlapping_map = map_15;
 overlapping_map.settlements[0].map.placements.push_back({"Hall", 0, 11, 2, 1});
 must_reject([&] { pkkm::validate(overlapping_map); });
 auto out_of_bounds_map = map_15;
 out_of_bounds_map.settlements[0].map.placements[0] = {"Gate", 14, 14, 2, 1};
 must_reject([&] { pkkm::validate(out_of_bounds_map); });
 auto partial_map = map_15;
 partial_map.settlements[0].map.columns = 0;
 must_reject([&] { pkkm::validate(partial_map); });
 auto zero_footprint_map = map_15;
 zero_footprint_map.settlements[0].map.placements[0].width = 0;
 must_reject([&] { pkkm::validate(zero_footprint_map); });
 must_reject([] { (void)pkkm::aggregate_settlements({
     {"Oak",2147483647,0}, {"Pine",1,0}}); });
 const auto aggregated_summary=pkkm::calculate_kingdom_summary(0,r.rule_inputs,{
     {"Oak",1200,2,3,-1,4,7}, {"Pine",800,1,2,-2,1,5}});
 must(aggregated_summary.control_dc==47 && aggregated_summary.population==7500
      && aggregated_summary.economy.improvements==7 && aggregated_summary.loyalty.improvements==-3
      && aggregated_summary.stability.improvements==5,
      "settlement totals feed territory, population, and kingdom statistic summaries");
 const auto catalog=pkkm::load_building_catalog(argc>1 ? argv[1] : "assets/building_catalog.json");
 int improvement_count=0;
 const pkkm::BuildingCatalogEntry* academy=nullptr;
 for (const auto& entry : catalog) {
     if (entry.kind==pkkm::CatalogRowKind::Improvement) {
         ++improvement_count;
         if (entry.name=="Academy") academy=&entry;
     }
 }
 must(improvement_count==70 && academy && academy->source_row==2 && academy->cost==52
      && academy->lots==2 && academy->economy==2 && academy->loyalty==2,
      "catalog import preserves the named-range rows and maps Academy source columns");
 const auto building_totals=pkkm::calculate_settlement_building_totals(
     {"Effects",0,0,0,0,0,0,{{"Academy",2},{"Barracks",1}}},catalog);
 must(building_totals.lots==5 && building_totals.economy==4 && building_totals.loyalty==4
      && building_totals.stability==0 && building_totals.defense==2 && building_totals.unrest==-1
      && building_totals.corruption==academy->corruption*2
      && building_totals.crime==academy->crime*2
      && building_totals.law==academy->law*2
      && building_totals.lore==academy->lore*2
      && building_totals.society==academy->society*2
      && building_totals.productivity==academy->productivity*2
      && building_totals.fame==academy->fame*2,
      "building inventory totals preserve catalog stat bonuses as a separate preview");
 pkkm::Settlement preview_only{"Preview",0,0,3,-1,4,0,{{"Academy",2}}};
 const auto preview_totals=pkkm::aggregate_settlements({preview_only},catalog);
 must(preview_totals.economy==3 && preview_totals.loyalty==-1 && preview_totals.stability==4,
      "catalog stat bonuses remain unapplied by default");
 pkkm::Settlement opted_in{"Opted In",0,0,3,-1,4,0,{{"Academy",2}}};
 opted_in.apply_catalog_stat_effects=true;
 const auto applied_totals=pkkm::aggregate_settlements({opted_in},catalog);
 must(applied_totals.economy==7 && applied_totals.loyalty==3 && applied_totals.stability==4
      && opted_in.economy==3 && opted_in.loyalty==-1 && opted_in.stability==4,
      "opted-in catalog bonuses affect totals without changing stored legacy values");
 must_reject([&] { (void)pkkm::aggregate_settlements({opted_in}); });
 const auto derived_size=pkkm::calculate_settlement_size(
     {"New Town",0,0,0,0,0,0,{{"Academy",2}}},catalog);
 must(derived_size.lots==4 && derived_size.districts==1 && derived_size.population==1000,
      "settlement size derives lots, minimum districts, and population from building inventory");
 pkkm::KingdomRuleInputs size_rules;
 size_rules.kingdom_size=1;
 const auto building_summary=pkkm::calculate_kingdom_summary(0,size_rules,
     {{"New Town",0,0,0,0,0,0,{{"Academy",2}}}},&catalog);
 must(building_summary.settlement_totals.lots==4 && building_summary.settlement_totals.districts==1
      && building_summary.settlement_totals.population==1000 && building_summary.population==1000,
      "catalog-derived settlement size feeds kingdom totals and population");
 const auto market_reference=pkkm::calculate_settlement_market_reference(
     {"Market Town",0,0,0,0,0,0,{{"Academy",2}}},catalog);
 must(pkkm::calculate_settlement_catalog_cost(
     {"Market Town",0,0,0,0,0,0,{{"Academy",2}}},catalog)==academy->cost*2,
     "settlement catalog cost reference aggregates source costs without mutation");
 auto invalid_cost_catalog=catalog;
 for (auto& entry : invalid_cost_catalog) {
     if (entry.name=="Academy") entry.cost=-1;
 }
 must_reject([&] { (void)pkkm::calculate_settlement_catalog_cost(
     {"Invalid Cost",0,0,0,0,0,0,{{"Academy",1}}},invalid_cost_catalog); });
 auto overflowing_cost_catalog=catalog;
 for (auto& entry : overflowing_cost_catalog) {
     if (entry.name=="Academy") entry.cost=std::numeric_limits<int>::max();
 }
 must_reject([&] { (void)pkkm::calculate_settlement_catalog_cost(
     {"Overflow Cost",0,0,0,0,0,0,{{"Academy",2}}},overflowing_cost_catalog); });
 must(market_reference.base_value==academy->base_value*2
     && market_reference.buy_limit==market_reference.base_value+1000
     && market_reference.sell_limit==market_reference.base_value+5000
     && market_reference.spellcasting_level==4,
     "settlement market reference follows workbook population tiers and catalog base value");
 const auto market_boundary=pkkm::calculate_settlement_market_reference(
     {"Small Town",250,0,0,0,0,0,{}},catalog);
 must(market_boundary.buy_limit==1000 && market_boundary.sell_limit==2500
     && market_boundary.spellcasting_level==4,
     "settlement market reference preserves the 250-population boundary");
 auto overflowing_market_catalog=catalog;
 for (auto& entry : overflowing_market_catalog) {
     if (entry.name=="Academy") entry.base_value=std::numeric_limits<int>::max();
 }
 must_reject([&] { (void)pkkm::calculate_settlement_market_reference(
     {"Overflow Market",0,0,0,0,0,0,{{"Academy",1}}},overflowing_market_catalog); });
 const auto boundary_size=pkkm::calculate_settlement_size(
     {"Boundary Town",9000,1,0,0,0,0,{{"Academy",18}}},catalog);
 must(boundary_size.lots==36 && boundary_size.districts==1 && boundary_size.population==9000,
      "settlement size follows strict district-override and inclusive population-override boundaries");
 const auto raised_override_size=pkkm::calculate_settlement_size(
     {"Raised Override",9001,2,0,0,0,0,{{"Academy",18}}},catalog);
 must(raised_override_size.districts==2 && raised_override_size.population==9001,
      "settlement size honors overrides above workbook minimums");
 must_reject([&] { (void)pkkm::calculate_settlement_size(
     {"Overflow Town",0,0,0,0,0,0,{{"Academy",std::numeric_limits<int>::max()}}},catalog); });
 const auto town_matches=pkkm::search_building_catalog(catalog,"Town");
 must(town_matches.size()==1 && town_matches[0].name=="Town Hall"
      && town_matches[0].kind==pkkm::CatalogRowKind::Improvement,
      "catalog search excludes settlement-kind markers from selectable improvements");
 const std::string catalog_fixture="pkkm_catalog_filter_test.json";
 put(catalog_fixture,R"({"records":[
 {"source_row":1,"kind":"basic","name":"Basic","source_values":[]},
 {"source_row":2,"kind":"town","name":"Town","source_values":[]},
 {"source_row":3,"kind":"city","name":"City","source_values":[]},
 {"source_row":4,"kind":"district","name":"District","source_values":[]},
 {"source_row":5,"kind":"improvement","name":"District Hall","source_values":[],"building":{}}
]})");
 const auto mixed_catalog=pkkm::load_building_catalog(catalog_fixture);
 const auto district_matches=pkkm::search_building_catalog(mixed_catalog,"District");
 must(mixed_catalog.size()==5 && mixed_catalog[0].kind==pkkm::CatalogRowKind::Basic
      && mixed_catalog[1].kind==pkkm::CatalogRowKind::Town && mixed_catalog[2].kind==pkkm::CatalogRowKind::City
      && mixed_catalog[3].kind==pkkm::CatalogRowKind::District
      && district_matches.size()==1 && district_matches[0].name=="District Hall",
      "catalog retains explicit non-building kinds while exposing only improvements to search");
 std::remove(catalog_fixture.c_str());
 auto invalid_inventory=k;
 invalid_inventory.settlements[0].building_inventory[0].count=-1;
 must_reject([&]{pkkm::validate(invalid_inventory);});
 auto oversized_inventory=nlohmann::json::parse(contents(path));
 oversized_inventory["settlements"][0]["building_inventory"][0]["count"]=4294967296LL;
 put(path,oversized_inventory.dump(2));
 must_reject([&]{(void)pkkm::load(path);});
 auto underflow_inventory=nlohmann::json::parse(contents(path));
 underflow_inventory["settlements"][0]["building_inventory"][0]["count"]=-4294967296LL;
 put(path,underflow_inventory.dump(2));
 must_reject([&]{(void)pkkm::load(path);});
 auto unsigned_overflow_inventory=nlohmann::json::parse(contents(path));
 unsigned_overflow_inventory["settlements"][0]["building_inventory"][0]["count"]=18446744073709551615ULL;
 put(path,unsigned_overflow_inventory.dump(2));
 must_reject([&]{(void)pkkm::load(path);});
 auto integer_boundaries=k;
 integer_boundaries.settlements[0].building_inventory[0].count=std::numeric_limits<int>::max();
 integer_boundaries.settlements[0].economy=std::numeric_limits<int>::min();
 pkkm::save(integer_boundaries,path);
 const auto boundary_roundtrip=pkkm::load(path);
 must(boundary_roundtrip.settlements[0].building_inventory[0].count==std::numeric_limits<int>::max()
      && boundary_roundtrip.settlements[0].economy==std::numeric_limits<int>::min(),
      "native int minimum and maximum values roundtrip without truncation");
 pkkm::save(k,path);
 auto legacy_v3=nlohmann::json::parse(contents(path));
 legacy_v3["schema_version"]=3;
 legacy_v3["settlements"][0].erase("building_inventory");
 put(path,legacy_v3.dump(2));
 const auto migrated_v3=pkkm::load(path);
 must(migrated_v3.schema_version==10 && migrated_v3.turn_progress.empty() && migrated_v3.calendar_notes.empty() && migrated_v3.settlements.size()==1
      && migrated_v3.settlements[0].building_inventory.empty()
      && migrated_v3.settlements[0].population==1200
      && !migrated_v3.settlements[0].apply_catalog_stat_effects,
      "schema 3 settlement records migrate with empty building inventories");
 pkkm::save(k,path);
 auto legacy_v4=nlohmann::json::parse(contents(path));
 legacy_v4["schema_version"]=4;
 legacy_v4["settlements"][0].erase("apply_catalog_stat_effects");
 legacy_v4["settlements"][0].erase("map");
 put(path,legacy_v4.dump(2));
 const auto migrated_v4=pkkm::load(path);
 must(migrated_v4.schema_version==10 && migrated_v4.turn_progress.empty() && migrated_v4.calendar_notes.empty() && !migrated_v4.settlements[0].apply_catalog_stat_effects
      && migrated_v4.settlements[0].economy==3 && migrated_v4.settlements[0].loyalty==-1
      && migrated_v4.settlements[0].building_inventory.size()==1,
      "schema 4 migration preserves saved totals and defaults catalog opt-in off");
 pkkm::save(k,path);
 auto legacy_v5=nlohmann::json::parse(contents(path));
 legacy_v5["schema_version"]=5;
 legacy_v5["settlements"][0].erase("map");
 put(path,legacy_v5.dump(2));
 const auto migrated_v5=pkkm::load(path);
 must(migrated_v5.schema_version==10 && migrated_v5.turn_progress.empty() && migrated_v5.calendar_notes.empty() && migrated_v5.settlements[0].map.rows==0
      && migrated_v5.settlements[0].map.columns==0 && migrated_v5.settlements[0].map.placements.empty(),
      "schema 5 migration defaults map to empty");
 pkkm::save(k,path);
 auto legacy_v6=nlohmann::json::parse(contents(path));
 legacy_v6["schema_version"]=6;
 legacy_v6["settlements"][0]["map"].erase("labels");
 put(path,legacy_v6.dump(2));
 const auto migrated_v6=pkkm::load(path);
 pkkm::save(migrated_v6,path);
 const auto migrated_v6_json=nlohmann::json::parse(contents(path));
 must(migrated_v6.schema_version==10 && migrated_v6.turn_progress.empty() && migrated_v6.calendar_notes.empty()
      && migrated_v6_json["settlements"][0]["map"]["labels"].empty(),
      "schema 6 maps migrate with an empty annotation list");
 auto invalid=k; invalid.turn=0; bool rejected=false;
 try {pkkm::validate(invalid);} catch(const std::invalid_argument&) {rejected=true;}
 must(rejected,"validation rejects turn zero");
 auto unknown_turn_step=k; unknown_turn_step.turn_progress[0].completed_step_ids.push_back("unknown.step");
 must_reject([&]{pkkm::validate(unknown_turn_step);});
 auto duplicate_turn_step=k; duplicate_turn_step.turn_progress[0].completed_step_ids.push_back("upkeep.stability");
 must_reject([&]{pkkm::validate(duplicate_turn_step);});
 auto duplicate_turn=k; duplicate_turn.turn_progress.push_back({4,{}});
 must_reject([&]{pkkm::validate(duplicate_turn);});
 auto unknown_calendar_entry=k; unknown_calendar_entry.calendar_notes.push_back({"calendar.row.999"});
 must_reject([&]{pkkm::validate(unknown_calendar_entry);});
 auto duplicate_calendar_entry=k; duplicate_calendar_entry.calendar_notes.push_back(k.calendar_notes.front());
 must_reject([&]{pkkm::validate(duplicate_calendar_entry);});
 k.name=std::string("A\x01\x1f",3)+" Realm";
 pkkm::save(k,path);  must(pkkm::load(path).name==k.name,"control escape roundtrip");
  must(contents(path).find("\\u0001")!=std::string::npos,"control escape serialized");
 auto original=contents(path);
 must_reject([&]{pkkm::save(k,"missing-parent/save.json");});
 must(contents(path)==original,"failed save preserves existing file");
 const std::string source_guard_path="pkkm_source_guard.xlsx";
 put(source_guard_path,"source sentinel");
 must_reject([&]{pkkm::save(k,source_guard_path);});
 must(contents(source_guard_path)=="source sentinel","spreadsheet source is not mutated by save");
 auto legacy_v2=original;
 const auto version_position=legacy_v2.find("\"schema_version\": 10");
 must(version_position!=std::string::npos,"saved schema version can be converted to a legacy fixture");
 legacy_v2.replace(version_position,std::string("\"schema_version\": 10").size(),"\"schema_version\": 2");
 const auto settlements_position=legacy_v2.find("  \"settlements\": [");
 const auto settlements_close=legacy_v2.find("\n  ],",settlements_position);
 must(settlements_position!=std::string::npos && settlements_close!=std::string::npos,
      "settlement array can be converted to a schema-v2 fixture");
 legacy_v2.replace(settlements_position,settlements_close+5-settlements_position,
     "  \"settlements\": [\n    {\n      \"name\": \"Oak\",\n      \"population\": 1200\n    }\n  ],");
 put(path,legacy_v2);
 const auto migrated_v2=pkkm::load(path);
 must(migrated_v2.schema_version==10 && migrated_v2.turn_progress.empty() && migrated_v2.calendar_notes.empty() && migrated_v2.settlements.size()==1
      && migrated_v2.settlements[0].districts==0 && migrated_v2.settlements[0].economy==0,
      "schema 2 settlements migrate with zero defaults for new aggregate inputs");
 put(path,R"({"schema_version":1,"name":"right","turn":1,"treasury_bp":0,"unrest":0,"settlements":[],"buildings":[],"extra":{"name":"wrong"}})");
 must(pkkm::load(path).name=="right","nested key does not shadow root field");
 auto migrated=pkkm::load(path);
 must(migrated.schema_version==10 && migrated.turn_progress.empty() && migrated.calendar_notes.empty() && migrated.rule_inputs.alignment=="N","schema 1 saves migrate to defaults");
 must_reject([&]{put(path,R"({"schema_version":1,"name":"x","turn":1,"treasury_bp":0,"unrest":0,"buildings":[]})");(void)pkkm::load(path);});
 must_reject([&]{put(path,R"({"schema_version":1,"name":"x","turn":1,"treasury_bp":0,"unrest":0,"settlements":[],"buildings":[] garbage})");(void)pkkm::load(path);});
 must_reject([&]{put(path,"[]");(void)pkkm::load(path);});
 put(path,R"({"schema_version":10,"name":"x","turn":1,"treasury_bp":0,"unrest":0,"settlements":[],"buildings":[],"item_deposits":[{"turn":1,"item_value":8000,"bp_received":2,"treasury_delta":2}]})");
 must_reject([&]{(void)pkkm::load(path);});
 put(path,R"({"schema_version":10,"name":"x","turn":1,"treasury_bp":0,"unrest":0,"settlements":[],"buildings":[],"loyalty_checks":[{"turn":1,"success":true,"unrest_delta":0,"loyalty_delta":-1}]})");
 must_reject([&]{(void)pkkm::load(path);});
 auto neutral = pkkm::alignment_bonuses("N");
 must(neutral.economy == 0 && neutral.loyalty == 0 && neutral.stability == 4, "single-letter neutral alignment gets both stability bonuses");
 auto chaotic_evil = pkkm::alignment_bonuses("CE");
 must(chaotic_evil.economy == 2 && chaotic_evil.loyalty == 2 && chaotic_evil.stability == 0, "alignment endpoints award the workbook's distinct bonuses");
 auto lawful = pkkm::calculate_laws({pkkm::PromotionLaw::Aggressive, true, pkkm::TaxationLaw::Heavy, true, 12});
 must(lawful.economy == 3 && lawful.loyalty == 1 && lawful.stability == 3 && lawful.consumption == 6,
      "edict and law bonuses combine with workbook rounding and special buildings");
 auto half_tax = pkkm::calculate_laws({pkkm::PromotionLaw::Token, true, pkkm::TaxationLaw::Light, true, 0});
 must(half_tax.economy == 1 && half_tax.loyalty == -2 && half_tax.stability == 1 && half_tax.consumption == 1,
      "negative half taxation rounds away from zero and cathedral halves consumption upward");
 must_reject([] { (void)pkkm::alignment_bonuses("bad"); });
 pkkm::KingdomLeadershipInput vacant_leadership;
 auto vacancies = pkkm::calculate_leadership(vacant_leadership);
 must(vacancies.economy_vacancy == 12 && vacancies.loyalty_vacancy == 8 && vacancies.stability_vacancy == 4,
      "leadership vacancies use source-specific per-role penalties");
 pkkm::KingdomLeadershipInput staffed;
 staffed.baron_bonus = 3; staffed.baron_benefits = {true, true, false};
 staffed.councilor = {true, 2}; staffed.general = {true, 1}; staffed.grand_diplomat = {true, 2};
 staffed.high_priest = {true, 3}; staffed.magister = {true, 2}; staffed.marshal = {true, 1};
 staffed.royal_enforcer = {true, 1}; staffed.spymaster = {true, 4}; staffed.spymaster_benefit = pkkm::KingdomStatFocus::Stability;
 staffed.treasurer = {true, 3}; staffed.warden = {true, 3};
 auto leadership = pkkm::calculate_leadership(staffed);
 must(leadership.economy_bonus == 9 && leadership.loyalty_bonus == 9 && leadership.stability_bonus == 10,
      "leader role bonuses and Baron/Spymaster benefits aggregate by stat");
 must(leadership.economy_vacancy == 0 && leadership.loyalty_vacancy == 0 && leadership.stability_vacancy == 0,
      "filled leadership slots remove their vacancy deductions");
 must(pkkm::control_dc(24, 7, 3) == 54, "Control DC matches kingdom size, districts, and other input");
 must(pkkm::population_total(24, 3, 3250) == 8500, "population includes 250 residents per non-city hex");
 auto stat = pkkm::calculate_stat({3, 2, 1, 4, 0, 2, 3, 1}, 20);
 must(stat.total == 6, "kingdom stat sums categories and subtracts unrest/vacancies");
 must(stat.check_threshold_percent == 65, "kingdom check threshold follows workbook formula");
 must(pkkm::calculate_stat({}, 20).check_threshold_percent == 95, "empty kingdom stat uses control DC threshold formula");
 std::remove(path.c_str());
 std::remove(source_guard_path.c_str());
 std::remove(map_v7_path.c_str());
 std::remove(map_v7_saved_path.c_str());
 std::cout<<"PASS: JSON round-trip and validation\\n";
 return 0;
 } catch (const std::exception& e) { std::cerr << "TEST ERROR: " << e.what() << "\\n"; return 1; }
}
