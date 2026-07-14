#pragma once

#include "ICompatibilityRule.h"

// 제한조건 1: 제동장치에 Bosch를 사용했다면, 조향장치도 Bosch를 사용해야 한다.
class BoschBrakeRequiresBoschSteeringRule : public ICompatibilityRule
{
public:
    std::optional<std::string> check(const Car& car) const override
    {
        if (car.brake == BrakeSystem::Bosch && car.steering != SteeringSystem::Bosch)
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
    std::optional<std::string> check(const Car& car) const override
    {
        if (car.type == CarType::Sedan && car.brake == BrakeSystem::Continental)
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
    std::optional<std::string> check(const Car& car) const override
    {
        if (car.type == CarType::Suv && car.engine == Engine::Toyota)
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
    std::optional<std::string> check(const Car& car) const override
    {
        if (car.type == CarType::Truck && car.engine == Engine::Wia)
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
    std::optional<std::string> check(const Car& car) const override
    {
        if (car.type == CarType::Truck && car.brake == BrakeSystem::Mando)
        {
            return "Truck에는 Mando제동장치 사용 불가";
        }
        return std::nullopt;
    }
};
