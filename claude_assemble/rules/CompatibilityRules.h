#pragma once

#include "ICompatibilityRule.h"

// 값은 docs/spec.md 매핑을 그대로 사용한다 (레거시 enum과 동일한 정수값).
// CarType: SEDAN=1, SUV=2, TRUCK=3
// Engine: GM=1, TOYOTA=2, WIA=3
// BrakeSystem: MANDO=1, CONTINENTAL=2, BOSCH=3
// SteeringSystem: BOSCH=1, MOBIS=2
//
// Phase 3에서 PartCatalog가 도입되면 이 매직넘버들은 카탈로그 상수로 대체될 예정이다.

// 제한조건 1: 제동장치에 Bosch를 사용했다면, 조향장치도 Bosch를 사용해야 한다.
class BoschBrakeRequiresBoschSteeringRule : public ICompatibilityRule
{
public:
    std::optional<std::string> check(const PartSelection& selection) const override
    {
        if (selection.brakeSystem == 3 && selection.steeringSystem != 1)
        {
            return "Bosch제동장치에는 Bosch조향장치 이외 사용 불가";
        }
        return std::nullopt;
    }
};

// 제한조건 2-1: Continental은 Sedan용 제동장치를 만들지 않는다.
class SedanCannotUseContinentalBrakeRule : public ICompatibilityRule
{
public:
    std::optional<std::string> check(const PartSelection& selection) const override
    {
        if (selection.carType == 1 && selection.brakeSystem == 2)
        {
            return "Sedan에는 Continental제동장치 사용 불가";
        }
        return std::nullopt;
    }
};

// 제한조건 2-2: TOYOTA는 SUV용 엔진을 만들지 않는다.
class SuvCannotUseToyotaEngineRule : public ICompatibilityRule
{
public:
    std::optional<std::string> check(const PartSelection& selection) const override
    {
        if (selection.carType == 2 && selection.engine == 2)
        {
            return "SUV에는 TOYOTA엔진 사용 불가";
        }
        return std::nullopt;
    }
};

// 제한조건 2-3: WIA는 Truck용 엔진을 만들지 않는다.
class TruckCannotUseWiaEngineRule : public ICompatibilityRule
{
public:
    std::optional<std::string> check(const PartSelection& selection) const override
    {
        if (selection.carType == 3 && selection.engine == 3)
        {
            return "Truck에는 WIA엔진 사용 불가";
        }
        return std::nullopt;
    }
};

// 제한조건 2-4: Mando는 Truck용 제동장치를 만들지 않는다.
class TruckCannotUseMandoBrakeRule : public ICompatibilityRule
{
public:
    std::optional<std::string> check(const PartSelection& selection) const override
    {
        if (selection.carType == 3 && selection.brakeSystem == 1)
        {
            return "Truck에는 Mando제동장치 사용 불가";
        }
        return std::nullopt;
    }
};
