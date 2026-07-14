#pragma once

#include <optional>
#include <string>

// Phase 1: 아직 Car/CarBuilder(Phase 2)가 없으므로, 레거시 isValidCombination()과 동일한
// 4개 정수 필드를 값 타입으로 옮긴 최소 스냅샷. Phase 2에서 Car로 대체될 예정.
struct PartSelection
{
    int carType;
    int engine;
    int brakeSystem;
    int steeringSystem;
};

class ICompatibilityRule
{
public:
    virtual ~ICompatibilityRule() = default;

    // 위반 시 실패 사유를 반환, 위반이 없으면 std::nullopt
    virtual std::optional<std::string> check(const PartSelection& selection) const = 0;
};
