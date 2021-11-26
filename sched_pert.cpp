#include <optional>
#include <iostream>
#include <fmt/format.h>

#include <opm/parser/eclipse/Deck/DeckOutput.hpp>
#include <opm/parser/eclipse/Parser/Parser.hpp>
#include <opm/parser/eclipse/Parser/ParseContext.hpp>
#include <opm/parser/eclipse/Parser/ErrorGuard.hpp>
#include <opm/common/utility/TimeService.hpp>

#include <opm/parser/eclipse/Parser/ParserKeywords/S.hpp>
#include <opm/parser/eclipse/Parser/ParserKeywords/W.hpp>
#include <opm/parser/eclipse/EclipseState/Schedule/ScheduleDeck.hpp>
#include <opm/parser/eclipse/EclipseState/Runspec.hpp>

#include "WellPert.hpp"

void error_exit() {
    std::exit(1);
}


Opm::ScheduleDeck load(const std::string& input_file, std::optional<Opm::time_point> start_time_arg) {
    Opm::ParseContext parseContext(std::vector<std::pair<std::string , Opm::InputError::Action>>
                                   {{Opm::ParseContext::PARSE_RANDOM_SLASH, Opm::InputError::IGNORE},
                                    {Opm::ParseContext::PARSE_MISSING_DIMS_KEYWORD, Opm::InputError::WARN},
                                    {Opm::ParseContext::SUMMARY_UNKNOWN_WELL, Opm::InputError::WARN},
                                    {Opm::ParseContext::SUMMARY_UNKNOWN_GROUP, Opm::InputError::WARN}});
    Opm::ErrorGuard errors;
    Opm::Parser parser;

    Opm::time_point start_time;
    const auto deck = parser.parseFile(input_file, parseContext, errors);
    if (deck.hasKeyword<Opm::ParserKeywords::START>()) {
        Opm::Runspec runspec(deck);
        start_time = Opm::TimeService::from_time_t(runspec.start_time());
    } else if (start_time_arg.has_value())
        start_time = start_time_arg.value();
    else
        error_exit();

    return Opm::ScheduleDeck{start_time, deck, Opm::ScheduleRestartInfo{}};
}

void handleRATE(const Opm::DeckItem& rate_item, const std::string& wname, RateType rate_type, const WellPert& well_pert, double sim_time, Opm::DeckOutput& output) {
    if (rate_item.defaultApplied(0))
        output.write_string(" '*' ");
    else {
        auto org_value = rate_item.get<Opm::UDAValue>(0).get<double>();
        auto new_rate = well_pert.update_rate(wname, rate_type, sim_time, org_value);
        output.write_string(" " + fmt::format("{}", new_rate));
    }
}

void handleWELL(const Opm::DeckRecord& well_record, const std::string& wname, const WellPert& well_pert, double sim_time, Opm::DeckOutput& output) {
    output.start_record();
    output.write_string("  '" + wname + "' ");
    well_record.getItem(1).write(output);
    well_record.getItem(2).write(output);

    handleRATE(well_record.getItem(3), wname, RateType::ORAT, well_pert, sim_time, output);
    handleRATE(well_record.getItem(4), wname, RateType::WRAT, well_pert, sim_time, output);
    handleRATE(well_record.getItem(5), wname, RateType::GRAT, well_pert, sim_time, output);

    well_record.write(output, 6);
}

void handleWCONHIST(const Opm::DeckKeyword& kw, const WellPert& well_pert, double sim_time, Opm::DeckOutput& output) {
    output.write_string("WCONHIST\n");
    for (const auto& record : kw) {
        const auto& wname = record.getItem<Opm::ParserKeywords::WCONHIST::WELL>().get<std::string>(0);
        if (well_pert.has_well(wname))
            handleWELL(record, wname, well_pert, sim_time, output);
        else
            record.write(output);
    }
    output.write_string("/\n");
}

void output(const Opm::ScheduleDeck& sched_deck, const WellPert& well_pert) {
    std::stringstream ss;
    Opm::DeckOutput output(std::cout);

    output.write_string("SCHEDULE\n");
    const auto start_time = sched_deck[0].start_time();
    auto current_time = start_time;

    for (const auto& block : sched_deck) {
        block.dump_time(current_time, output);
        std::chrono::duration<double> sim_time = current_time - start_time;
        for (const auto& kw : block) {
            if (kw.is<Opm::ParserKeywords::WCONHIST>())
                handleWCONHIST(kw, well_pert, sim_time.count(), output);
            else
                kw.write(output);
            output.write_string("\n");
        }
        if (block.end_time().has_value())
            current_time = block.end_time().value();
    }
}


int main(int argc, char** argv){
    const auto& sched_deck = load(argv[1], {});
    WellPert well_pert;


    output(sched_deck, well_pert);
}
