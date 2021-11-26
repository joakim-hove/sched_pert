#include "WellPert.hpp"

WellPert::WellPert() {

}

bool WellPert::has_well(const std::string& well) const {
    return (this->perturbations.count(well) != 0);
}

double WellPert::update_rate(const std::string& well, RateType rate_type, double sim_time, double org_rate) const {
    return org_rate;
}
