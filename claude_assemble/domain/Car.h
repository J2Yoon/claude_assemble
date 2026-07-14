#pragma once

// docs/spec.md의 부품-제조사 매핑을 강타입 enum class로 표현한다.
// 값은 레거시 코드/Phase 1 정수 매핑(SEDAN=1 등)과 동일하게 맞춰,
// 기존 int 기반 코드와의 교차검증(static_cast)을 안전하게 한다.

enum class CarType : int
{
    Sedan = 1,
    Suv = 2,
    Truck = 3,
};

enum class Engine : int
{
    Gm = 1,
    Toyota = 2,
    Wia = 3,
};

enum class BrakeSystem : int
{
    Mando = 1,
    Continental = 2,
    Bosch = 3,
};

enum class SteeringSystem : int
{
    Bosch = 1,
    Mobis = 2,
};

struct Car
{
    CarType type;
    Engine engine;
    BrakeSystem brake;
    SteeringSystem steering;
};
