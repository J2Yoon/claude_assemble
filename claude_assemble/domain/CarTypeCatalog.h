#pragma once

#include <string>
#include <vector>

#include "Car.h"

// 차량 타입(Sedan/SUV/Truck)의 목록·표시 이름을 한 곳에서 관리한다.
// spec.md가 명시한 "향후 타입 추가"에 대비해, PartCatalog와 동일한 방식으로 확장 가능하게 만든다.
class CarTypeCatalog
{
public:
    static const std::vector<CarType>& availableCarTypes()
    {
        static const std::vector<CarType> carTypes{ CarType::Sedan, CarType::Suv, CarType::Truck };
        return carTypes;
    }

    static std::string nameOf(CarType type)
    {
        switch (type)
        {
        case CarType::Sedan: return "Sedan";
        case CarType::Suv: return "SUV";
        case CarType::Truck: return "Truck";
        }
        return "UNKNOWN";
    }
};
