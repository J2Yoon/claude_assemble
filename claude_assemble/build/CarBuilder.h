#pragma once

#include <optional>

#include "../domain/Car.h"

// Builder: 차량 조립 절차(타입 선택 -> 부품 선택)를 타입 안전하게 캡슐화한다.
// stack[10] 전역 배열의 원시 인덱싱 대신 std::optional 멤버로 "미조립 상태"를 표현한다.
class CarBuilder
{
public:
    CarBuilder& setCarType(CarType type)
    {
        type_ = type;
        return *this;
    }

    CarBuilder& setEngine(Engine engine)
    {
        engine_ = engine;
        return *this;
    }

    CarBuilder& setBrakeSystem(BrakeSystem brake)
    {
        brake_ = brake;
        return *this;
    }

    CarBuilder& setSteeringSystem(SteeringSystem steering)
    {
        steering_ = steering;
        return *this;
    }

    // 4개 부품이 모두 채워지지 않았으면 std::nullopt를 반환한다.
    std::optional<Car> build() const
    {
        if (!type_ || !engine_ || !brake_ || !steering_)
        {
            return std::nullopt;
        }
        return Car{ *type_, *engine_, *brake_, *steering_ };
    }

private:
    std::optional<CarType> type_;
    std::optional<Engine> engine_;
    std::optional<BrakeSystem> brake_;
    std::optional<SteeringSystem> steering_;
};
