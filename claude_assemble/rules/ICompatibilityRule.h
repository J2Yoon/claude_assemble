#pragma once

#include <optional>
#include <string>

#include "../domain/Car.h"

class ICompatibilityRule
{
public:
    virtual ~ICompatibilityRule() = default;

    // 위반 시 실패 사유를 반환, 위반이 없으면 std::nullopt
    virtual std::optional<std::string> check(const Car& car) const = 0;
};
