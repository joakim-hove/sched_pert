#ifndef WELL_PERT_HPP
#define WELL_PERT_HPP

#include <string>
#include <unordered_map>

class Perturbation {
public:
    Perturbation() = default;
private:
};

enum class RateType {
    ORAT,
    WRAT,
    GRAT
};


class WellPert {
public:
    WellPert();
    bool has_well(const std::string& well) const;
    double update_rate(const std::string& well, RateType rate_type, double sim_time, double org_rate) const;


private:
    std::unordered_map<std::string, std::unordered_map<RateType, Perturbation>> perturbations;
};





#endif
