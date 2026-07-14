#pragma once

#include <memory>
#include <vector>

#include "CompatibilityRules.h"
#include "ICompatibilityRule.h"

// Chain of Responsibility: 등록된 규칙을 모두 순회하여 위반 사유를 전부 수집한다.
class CompatibilityChecker
{
public:
    void addRule(std::unique_ptr<ICompatibilityRule> rule)
    {
        rules_.push_back(std::move(rule));
    }

    std::vector<std::string> validate(const Car& car) const
    {
        std::vector<std::string> reasons;
        for (const auto& rule : rules_)
        {
            if (auto reason = rule->check(car))
            {
                reasons.push_back(*reason);
            }
        }
        return reasons;
    }

private:
    std::vector<std::unique_ptr<ICompatibilityRule>> rules_;
};

// spec.md 제한조건 1·2를 모두 등록한 기본 CompatibilityChecker를 생성한다.
inline CompatibilityChecker makeDefaultCompatibilityChecker()
{
    CompatibilityChecker checker;
    checker.addRule(std::make_unique<BoschBrakeRequiresBoschSteeringRule>());
    checker.addRule(std::make_unique<SedanCannotUseContinentalBrakeRule>());
    checker.addRule(std::make_unique<SuvCannotUseToyotaEngineRule>());
    checker.addRule(std::make_unique<TruckCannotUseWiaEngineRule>());
    checker.addRule(std::make_unique<TruckCannotUseMandoBrakeRule>());
    return checker;
}
