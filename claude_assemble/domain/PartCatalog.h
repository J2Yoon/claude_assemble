#pragma once

#include <string>
#include <vector>

#include "Car.h"

// 부품(Engine/BrakeSystem/SteeringSystem)의 목록·표시 이름을 한 곳에서 관리한다.
// 신규 제조사가 추가되어도 이 파일의 목록/이름 매핑만 수정하면 되도록 만든다 (OCP).
class PartCatalog
{
public:
    static const std::vector<Engine>& availableEngines()
    {
        static const std::vector<Engine> engines{ Engine::Gm, Engine::Toyota, Engine::Wia };
        return engines;
    }

    static const std::vector<BrakeSystem>& availableBrakeSystems()
    {
        static const std::vector<BrakeSystem> brakes{ BrakeSystem::Mando, BrakeSystem::Continental, BrakeSystem::Bosch };
        return brakes;
    }

    static const std::vector<SteeringSystem>& availableSteeringSystems()
    {
        static const std::vector<SteeringSystem> steerings{ SteeringSystem::Bosch, SteeringSystem::Mobis };
        return steerings;
    }

    static std::string nameOf(Engine engine)
    {
        switch (engine)
        {
        case Engine::Gm: return "GM";
        case Engine::Toyota: return "TOYOTA";
        case Engine::Wia: return "WIA";
        }
        return "UNKNOWN";
    }

    static std::string nameOf(BrakeSystem brake)
    {
        switch (brake)
        {
        case BrakeSystem::Mando: return "MANDO";
        case BrakeSystem::Continental: return "CONTINENTAL";
        case BrakeSystem::Bosch: return "BOSCH";
        }
        return "UNKNOWN";
    }

    static std::string nameOf(SteeringSystem steering)
    {
        switch (steering)
        {
        case SteeringSystem::Bosch: return "BOSCH";
        case SteeringSystem::Mobis: return "MOBIS";
        }
        return "UNKNOWN";
    }
};
