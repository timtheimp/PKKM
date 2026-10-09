#include "pkkm/kingdom.hpp"
#include "pkkm/buildings.hpp"
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
 k.settlements[0].apply_catalog_stat_effects=true;
 k.settlements[0].building_inventory.push_back({"Academy", 2});
 k.settlements[0].map = {8, 8, {{"Castle", 5, 5, 2, 2}}};
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
 must(r.schema_version==9 && r.settlements[0].apply_catalog_stat_effects
      && r.settlements[0].building_inventory.size()==1
      && r.settlements[0].building_inventory[0].name=="Academy"
      && r.settlements[0].building_inventory[0].count==2,
      "schema 9 roundtrips existing settlement state");
 must(r.calendar_notes.size()==2 && r.calendar_notes[0].entry_id=="calendar.row.2"
      && r.calendar_notes[0].other=="Opening month"
      && r.calendar_notes[1].entry_id=="calendar.row.14"
      && r.calendar_notes[1].holiday=="Founding festival"
      && r.calendar_notes[1].kingdom_upgrades=="Build the bridge"
      && r.calendar_notes[1].events=="Dragon sighting" && r.calendar_notes[1].other.empty(),
      "schema 9 keeps notes for repeated month names on separate source occurrences");
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
 const auto magic_items=pkkm::magic_item_workflow();
 must(magic_items.buy_item_moves_to_pc && magic_items.success_recycles_item
      && !magic_items.failure_recycles_item && magic_items.excess_checks_economy_delta==-1
      && magic_items.gold_per_buy_for_kingdom_bp==2000 && magic_items.buy_for_kingdom_requires_use,
      "magic-item Turn workflow preserves explicit optional branches");
 const auto event_cadence=pkkm::event_cadence();
 must(event_cadence.no_event_last_month_percent==75
      && event_cadence.event_last_month_percent==25,
      "event Turn cadence preserves explicit source percentages");
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
 must(labeled_map.schema_version==9 && labeled_map.turn_progress.empty() && labeled_map.calendar_notes.empty()
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
 must(migrated_v8.schema_version==9 && migrated_v8.turn_progress.size()==2
      && migrated_v8.calendar_notes.empty(),
      "schema 8 migrates turn checklist history and defaults calendar notes to empty");
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
      && building_totals.stability==0 && building_totals.defense==2 && building_totals.unrest==-1,
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
 must(migrated_v3.schema_version==9 && migrated_v3.turn_progress.empty() && migrated_v3.calendar_notes.empty() && migrated_v3.settlements.size()==1
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
 must(migrated_v4.schema_version==9 && migrated_v4.turn_progress.empty() && migrated_v4.calendar_notes.empty() && !migrated_v4.settlements[0].apply_catalog_stat_effects
      && migrated_v4.settlements[0].economy==3 && migrated_v4.settlements[0].loyalty==-1
      && migrated_v4.settlements[0].building_inventory.size()==1,
      "schema 4 migration preserves saved totals and defaults catalog opt-in off");
 pkkm::save(k,path);
 auto legacy_v5=nlohmann::json::parse(contents(path));
 legacy_v5["schema_version"]=5;
 legacy_v5["settlements"][0].erase("map");
 put(path,legacy_v5.dump(2));
 const auto migrated_v5=pkkm::load(path);
 must(migrated_v5.schema_version==9 && migrated_v5.turn_progress.empty() && migrated_v5.calendar_notes.empty() && migrated_v5.settlements[0].map.rows==0
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
 must(migrated_v6.schema_version==9 && migrated_v6.turn_progress.empty() && migrated_v6.calendar_notes.empty()
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
 auto legacy_v2=original;
 const auto version_position=legacy_v2.find("\"schema_version\": 9");
 must(version_position!=std::string::npos,"saved schema version can be converted to a legacy fixture");
 legacy_v2.replace(version_position,std::string("\"schema_version\": 9").size(),"\"schema_version\": 2");
 const auto settlements_position=legacy_v2.find("  \"settlements\": [");
 const auto settlements_close=legacy_v2.find("\n  ],",settlements_position);
 must(settlements_position!=std::string::npos && settlements_close!=std::string::npos,
      "settlement array can be converted to a schema-v2 fixture");
 legacy_v2.replace(settlements_position,settlements_close+5-settlements_position,
     "  \"settlements\": [\n    {\n      \"name\": \"Oak\",\n      \"population\": 1200\n    }\n  ],");
 put(path,legacy_v2);
 const auto migrated_v2=pkkm::load(path);
 must(migrated_v2.schema_version==9 && migrated_v2.turn_progress.empty() && migrated_v2.calendar_notes.empty() && migrated_v2.settlements.size()==1
      && migrated_v2.settlements[0].districts==0 && migrated_v2.settlements[0].economy==0,
      "schema 2 settlements migrate with zero defaults for new aggregate inputs");
 put(path,R"({"schema_version":1,"name":"right","turn":1,"treasury_bp":0,"unrest":0,"settlements":[],"buildings":[],"extra":{"name":"wrong"}})");
 must(pkkm::load(path).name=="right","nested key does not shadow root field");
 auto migrated=pkkm::load(path);
 must(migrated.schema_version==9 && migrated.turn_progress.empty() && migrated.calendar_notes.empty() && migrated.rule_inputs.alignment=="N","schema 1 saves migrate to defaults");
 must_reject([&]{put(path,R"({"schema_version":1,"name":"x","turn":1,"treasury_bp":0,"unrest":0,"buildings":[]})");(void)pkkm::load(path);});
 must_reject([&]{put(path,R"({"schema_version":1,"name":"x","turn":1,"treasury_bp":0,"unrest":0,"settlements":[],"buildings":[] garbage})");(void)pkkm::load(path);});
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
 std::remove(map_v7_path.c_str());
 std::remove(map_v7_saved_path.c_str());
 std::cout<<"PASS: JSON round-trip and validation\\n";
 return 0;
 } catch (const std::exception& e) { std::cerr << "TEST ERROR: " << e.what() << "\\n"; return 1; }
}
