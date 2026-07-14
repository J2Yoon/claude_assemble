#ifdef _DEBUG

#include "gmock/gmock.h"

#include "build/CarBuilder.h"
#include "domain/Car.h"
#include "domain/CarTypeCatalog.h"
#include "domain/PartCatalog.h"
#include "rules/CompatibilityChecker.h"
#include "rules/CompatibilityRules.h"
#include "rules/ICompatibilityRule.h"

// docs/spec.md 기준 자동차 조립 규칙 테스트용 (release 빌드의 isValidCheck 로직을 복제)
enum CarType_Test
{
    SEDAN = 1,
    SUV,
    TRUCK
};

enum Engine_Test
{
    GM = 1,
    TOYOTA,
    WIA
};

enum brakeSystem_Test
{
    MANDO = 1,
    CONTINENTAL,
    BOSCH_B
};

enum SteeringSystem_Test
{
    BOSCH_S = 1,
    MOBIS
};

// stack index: 0=CarType, 1=Engine, 2=brakeSystem, 3=SteeringSystem
bool isValidCombination(int carType, int engine, int brakeSystem, int steeringSystem)
{
    if (carType == SEDAN && brakeSystem == CONTINENTAL)
    {
        return false;
    }
    else if (carType == SUV && engine == TOYOTA)
    {
        return false;
    }
    else if (carType == TRUCK && engine == WIA)
    {
        return false;
    }
    else if (carType == TRUCK && brakeSystem == MANDO)
    {
        return false;
    }
    else if (brakeSystem == BOSCH_B && steeringSystem != BOSCH_S)
    {
        return false;
    }
    return true;
}

// 제한조건 1: 제동장치에 Bosch를 사용했다면, 조향장치도 Bosch를 사용해야 한다.
TEST(AssembleSpecTest, BoschBrakeRequiresBoschSteering)
{
    EXPECT_FALSE(isValidCombination(SEDAN, GM, BOSCH_B, MOBIS));
    EXPECT_TRUE(isValidCombination(SEDAN, GM, BOSCH_B, BOSCH_S));
}

// 제한조건 2-1: Continental은 Sedan용 제동장치를 만들지 않는다.
TEST(AssembleSpecTest, SedanCannotUseContinentalBrake)
{
    EXPECT_FALSE(isValidCombination(SEDAN, GM, CONTINENTAL, BOSCH_S));
    EXPECT_TRUE(isValidCombination(SUV, GM, CONTINENTAL, BOSCH_S));
    EXPECT_TRUE(isValidCombination(TRUCK, GM, CONTINENTAL, BOSCH_S));
}

// 제한조건 2-2: TOYOTA는 SUV용 엔진을 만들지 않는다.
TEST(AssembleSpecTest, SuvCannotUseToyotaEngine)
{
    EXPECT_FALSE(isValidCombination(SUV, TOYOTA, MANDO, BOSCH_S));
    EXPECT_TRUE(isValidCombination(SEDAN, TOYOTA, MANDO, BOSCH_S));
    EXPECT_TRUE(isValidCombination(TRUCK, TOYOTA, CONTINENTAL, BOSCH_S));
}

// 제한조건 2-3: WIA는 Truck용 엔진을 만들지 않는다.
TEST(AssembleSpecTest, TruckCannotUseWiaEngine)
{
    EXPECT_FALSE(isValidCombination(TRUCK, WIA, MANDO, BOSCH_S));
    EXPECT_TRUE(isValidCombination(SEDAN, WIA, MANDO, BOSCH_S));
    EXPECT_TRUE(isValidCombination(SUV, WIA, MANDO, BOSCH_S));
}

// 제한조건 2-4: Mando는 Truck용 제동장치를 만들지 않는다.
TEST(AssembleSpecTest, TruckCannotUseMandoBrake)
{
    EXPECT_FALSE(isValidCombination(TRUCK, GM, MANDO, BOSCH_S));
    EXPECT_TRUE(isValidCombination(SEDAN, GM, MANDO, BOSCH_S));
    EXPECT_TRUE(isValidCombination(SUV, GM, MANDO, BOSCH_S));
}

// 유효한 조합 예시: 각 CarType 별로 spec 제한조건을 모두 만족하는 조합
TEST(AssembleSpecTest, ValidCombinationsPerCarType)
{
    EXPECT_TRUE(isValidCombination(SEDAN, GM, MANDO, BOSCH_S));
    EXPECT_TRUE(isValidCombination(SUV, GM, CONTINENTAL, MOBIS));
    EXPECT_TRUE(isValidCombination(TRUCK, GM, BOSCH_B, BOSCH_S));
}

// [Phase 0] 하나의 조합이 여러 제한조건을 동시에 위반해도 무효로 판정되는지 확인
TEST(AssembleSpecTest, MultipleRuleViolationsStillInvalid)
{
    // 제한조건 2-3(Truck+WIA) & 2-4(Truck+Mando) 동시 위반
    EXPECT_FALSE(isValidCombination(TRUCK, WIA, MANDO, BOSCH_S));
    // 제한조건 2-4(Truck+Mando) & 1(Bosch 브레이크 아님이므로 미해당) -> 2-4만 해당하지만
    // 제한조건 1(Bosch 브레이크) & 2-1(Sedan+Continental)은 브레이크가 다르므로 동시 발생 불가능함을
    // 문서화하는 차원에서, Bosch 브레이크 + Sedan 조합이 제한조건 1만으로 무효 처리되는지 확인
    EXPECT_FALSE(isValidCombination(SEDAN, GM, BOSCH_B, MOBIS));
    // 제한조건 2-2(SUV+TOYOTA) & 1(Bosch 브레이크 + Bosch 아닌 조향) 동시 위반
    EXPECT_FALSE(isValidCombination(SUV, TOYOTA, BOSCH_B, MOBIS));
}

// [Phase 0] CarType(3) x Engine(3) x BrakeSystem(3) x SteeringSystem(2) = 54가지 전체 조합에 대해
// spec.md 제한조건 1·2를 그대로 적용한 기대값과 isValidCombination() 결과를 전수 비교한다.
// Phase 1 이후 검증 로직을 Chain of Responsibility로 옮길 때 동작이 1건이라도 달라지면 즉시 실패한다.
TEST(AssembleSpecTest, ExhaustiveCombinationMatrix)
{
    const int carTypes[] = { SEDAN, SUV, TRUCK };
    const int engines[] = { GM, TOYOTA, WIA };
    const int brakes[] = { MANDO, CONTINENTAL, BOSCH_B };
    const int steerings[] = { BOSCH_S, MOBIS };

    for (int carType : carTypes)
    {
        for (int engine : engines)
        {
            for (int brake : brakes)
            {
                for (int steering : steerings)
                {
                    bool expected = true;
                    if (carType == SEDAN && brake == CONTINENTAL)
                        expected = false;
                    else if (carType == SUV && engine == TOYOTA)
                        expected = false;
                    else if (carType == TRUCK && engine == WIA)
                        expected = false;
                    else if (carType == TRUCK && brake == MANDO)
                        expected = false;
                    else if (brake == BOSCH_B && steering != BOSCH_S)
                        expected = false;

                    EXPECT_EQ(isValidCombination(carType, engine, brake, steering), expected)
                        << "carType=" << carType << " engine=" << engine
                        << " brake=" << brake << " steering=" << steering;
                }
            }
        }
    }
}

// [Phase 1] Chain of Responsibility로 추출한 CompatibilityChecker 기준 테스트.
// 레거시 isValidCombination() 기준선(위 AssembleSpecTest)은 회귀 확인용으로 그대로 유지한다.
// [Phase 2] CompatibilityChecker/ICompatibilityRule이 PartSelection(int 4개) 대신
// Car(enum class 기반 값 타입)를 입력받도록 시그니처가 바뀌어, 아래 테스트도 Car 리터럴을 사용한다.

// 제한조건 1: 제동장치에 Bosch를 사용했다면, 조향장치도 Bosch를 사용해야 한다.
TEST(CompatibilityCheckerTest, BoschBrakeRequiresBoschSteeringRule)
{
    BoschBrakeRequiresBoschSteeringRule rule;
    EXPECT_TRUE(rule.check(Car{ CarType::Sedan, Engine::Gm, BrakeSystem::Bosch, SteeringSystem::Mobis }).has_value());
    EXPECT_FALSE(rule.check(Car{ CarType::Sedan, Engine::Gm, BrakeSystem::Bosch, SteeringSystem::Bosch }).has_value());
}

// 제한조건 2-1: Continental은 Sedan용 제동장치를 만들지 않는다.
TEST(CompatibilityCheckerTest, SedanCannotUseContinentalBrakeRule)
{
    SedanCannotUseContinentalBrakeRule rule;
    EXPECT_TRUE(rule.check(Car{ CarType::Sedan, Engine::Gm, BrakeSystem::Continental, SteeringSystem::Bosch }).has_value());
    EXPECT_FALSE(rule.check(Car{ CarType::Suv, Engine::Gm, BrakeSystem::Continental, SteeringSystem::Bosch }).has_value());
}

// 제한조건 2-2: TOYOTA는 SUV용 엔진을 만들지 않는다.
TEST(CompatibilityCheckerTest, SuvCannotUseToyotaEngineRule)
{
    SuvCannotUseToyotaEngineRule rule;
    EXPECT_TRUE(rule.check(Car{ CarType::Suv, Engine::Toyota, BrakeSystem::Mando, SteeringSystem::Bosch }).has_value());
    EXPECT_FALSE(rule.check(Car{ CarType::Sedan, Engine::Toyota, BrakeSystem::Mando, SteeringSystem::Bosch }).has_value());
}

// 제한조건 2-3: WIA는 Truck용 엔진을 만들지 않는다.
TEST(CompatibilityCheckerTest, TruckCannotUseWiaEngineRule)
{
    TruckCannotUseWiaEngineRule rule;
    EXPECT_TRUE(rule.check(Car{ CarType::Truck, Engine::Wia, BrakeSystem::Mando, SteeringSystem::Bosch }).has_value());
    EXPECT_FALSE(rule.check(Car{ CarType::Sedan, Engine::Wia, BrakeSystem::Mando, SteeringSystem::Bosch }).has_value());
}

// 제한조건 2-4: Mando는 Truck용 제동장치를 만들지 않는다.
TEST(CompatibilityCheckerTest, TruckCannotUseMandoBrakeRule)
{
    TruckCannotUseMandoBrakeRule rule;
    EXPECT_TRUE(rule.check(Car{ CarType::Truck, Engine::Gm, BrakeSystem::Mando, SteeringSystem::Bosch }).has_value());
    EXPECT_FALSE(rule.check(Car{ CarType::Suv, Engine::Gm, BrakeSystem::Mando, SteeringSystem::Bosch }).has_value());
}

// 하나의 조합이 여러 규칙을 동시에 위반하면 위반 사유가 모두 수집되는지 확인
TEST(CompatibilityCheckerTest, MultipleRuleViolationsReturnsAllReasons)
{
    CompatibilityChecker checker = makeDefaultCompatibilityChecker();

    // TRUCK + WIA + MANDO -> 제한조건 2-3, 2-4 동시 위반
    std::vector<std::string> reasons = checker.validate(Car{ CarType::Truck, Engine::Wia, BrakeSystem::Mando, SteeringSystem::Bosch });
    EXPECT_EQ(reasons.size(), 2u);

    // 위반 없는 조합은 빈 벡터
    EXPECT_TRUE(checker.validate(Car{ CarType::Sedan, Engine::Gm, BrakeSystem::Mando, SteeringSystem::Bosch }).empty());
}

// CarType(3) x Engine(3) x BrakeSystem(3) x SteeringSystem(2) = 54가지 전체 조합에 대해
// 레거시 isValidCombination()과 신규 CompatibilityChecker::validate()가 동일한 결과를 내는지 교차 검증한다.
TEST(CompatibilityCheckerTest, ExhaustiveCrossCheckWithLegacy)
{
    CompatibilityChecker checker = makeDefaultCompatibilityChecker();

    const int carTypes[] = { SEDAN, SUV, TRUCK };
    const int engines[] = { GM, TOYOTA, WIA };
    const int brakes[] = { MANDO, CONTINENTAL, BOSCH_B };
    const int steerings[] = { BOSCH_S, MOBIS };

    for (int carType : carTypes)
    {
        for (int engine : engines)
        {
            for (int brake : brakes)
            {
                for (int steering : steerings)
                {
                    bool legacyValid = isValidCombination(carType, engine, brake, steering);

                    Car car{ static_cast<CarType>(carType), static_cast<Engine>(engine),
                             static_cast<BrakeSystem>(brake), static_cast<SteeringSystem>(steering) };
                    bool checkerValid = checker.validate(car).empty();

                    EXPECT_EQ(checkerValid, legacyValid)
                        << "carType=" << carType << " engine=" << engine
                        << " brake=" << brake << " steering=" << steering;
                }
            }
        }
    }
}

// [Phase 2] Builder 패턴: std::optional 기반으로 "미조립 상태"를 표현하는 CarBuilder 테스트.
TEST(CarBuilderTest, BuildSucceedsWhenAllPartsAreSet)
{
    CarBuilder builder;
    builder.setCarType(CarType::Sedan)
        .setEngine(Engine::Gm)
        .setBrakeSystem(BrakeSystem::Mando)
        .setSteeringSystem(SteeringSystem::Bosch);

    std::optional<Car> car = builder.build();
    ASSERT_TRUE(car.has_value());
    EXPECT_EQ(car->type, CarType::Sedan);
    EXPECT_EQ(car->engine, Engine::Gm);
    EXPECT_EQ(car->brake, BrakeSystem::Mando);
    EXPECT_EQ(car->steering, SteeringSystem::Bosch);
}

TEST(CarBuilderTest, BuildFailsWhenAnyPartIsMissing)
{
    CarBuilder builder;
    builder.setCarType(CarType::Suv)
        .setEngine(Engine::Gm)
        .setBrakeSystem(BrakeSystem::Continental);
    // SteeringSystem을 설정하지 않음

    EXPECT_FALSE(builder.build().has_value());
}

TEST(CarBuilderTest, BuildFailsWhenNothingIsSet)
{
    CarBuilder builder;
    EXPECT_FALSE(builder.build().has_value());
}

// [Phase 3] Factory/Catalog: 부품 목록·이름 매핑이 한 곳(PartCatalog)에서 관리되는지 확인.
TEST(PartCatalogTest, AvailableEnginesMatchSpec)
{
    const std::vector<Engine>& engines = PartCatalog::availableEngines();
    ASSERT_EQ(engines.size(), 3u);
    EXPECT_EQ(engines[0], Engine::Gm);
    EXPECT_EQ(engines[1], Engine::Toyota);
    EXPECT_EQ(engines[2], Engine::Wia);

    EXPECT_EQ(PartCatalog::nameOf(Engine::Gm), "GM");
    EXPECT_EQ(PartCatalog::nameOf(Engine::Toyota), "TOYOTA");
    EXPECT_EQ(PartCatalog::nameOf(Engine::Wia), "WIA");
}

TEST(PartCatalogTest, AvailableBrakeSystemsMatchSpec)
{
    const std::vector<BrakeSystem>& brakes = PartCatalog::availableBrakeSystems();
    ASSERT_EQ(brakes.size(), 3u);
    EXPECT_EQ(brakes[0], BrakeSystem::Mando);
    EXPECT_EQ(brakes[1], BrakeSystem::Continental);
    EXPECT_EQ(brakes[2], BrakeSystem::Bosch);

    EXPECT_EQ(PartCatalog::nameOf(BrakeSystem::Mando), "MANDO");
    EXPECT_EQ(PartCatalog::nameOf(BrakeSystem::Continental), "CONTINENTAL");
    EXPECT_EQ(PartCatalog::nameOf(BrakeSystem::Bosch), "BOSCH");
}

TEST(PartCatalogTest, AvailableSteeringSystemsMatchSpec)
{
    const std::vector<SteeringSystem>& steerings = PartCatalog::availableSteeringSystems();
    ASSERT_EQ(steerings.size(), 2u);
    EXPECT_EQ(steerings[0], SteeringSystem::Bosch);
    EXPECT_EQ(steerings[1], SteeringSystem::Mobis);

    EXPECT_EQ(PartCatalog::nameOf(SteeringSystem::Bosch), "BOSCH");
    EXPECT_EQ(PartCatalog::nameOf(SteeringSystem::Mobis), "MOBIS");
}

TEST(CarTypeCatalogTest, AvailableCarTypesMatchSpec)
{
    const std::vector<CarType>& carTypes = CarTypeCatalog::availableCarTypes();
    ASSERT_EQ(carTypes.size(), 3u);
    EXPECT_EQ(carTypes[0], CarType::Sedan);
    EXPECT_EQ(carTypes[1], CarType::Suv);
    EXPECT_EQ(carTypes[2], CarType::Truck);

    EXPECT_EQ(CarTypeCatalog::nameOf(CarType::Sedan), "Sedan");
    EXPECT_EQ(CarTypeCatalog::nameOf(CarType::Suv), "SUV");
    EXPECT_EQ(CarTypeCatalog::nameOf(CarType::Truck), "Truck");
}

int main()
{
    testing::InitGoogleMock();
    return RUN_ALL_TESTS();
}

#else

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "domain/CarTypeCatalog.h"
#include "domain/PartCatalog.h"

#define CLEAR_SCREEN "\033[H\033[2J"

int stack[10];

void selectCarType(int answer);
void selectEngine(int answer);
void selectbrakeSystem(int answer);
void selectSteeringSystem(int answer);
void runProducedCar();
void testProducedCar();
void delay(int ms);

enum QuestionType
{
    CarType_Q,
    Engine_Q,
    brakeSystem_Q,
    SteeringSystem_Q,
    Run_Test,
};

enum CarType
{
    SEDAN = 1,
    SUV,
    TRUCK
};

enum Engine
{
    GM = 1,
    TOYOTA,
    WIA
};

enum brakeSystem
{
    MANDO = 1,
    CONTINENTAL,
    BOSCH_B
};

enum SteeringSystem
{
    BOSCH_S = 1,
    MOBIS
};

void delay(int ms)
{
    volatile int sum = 0;
    for (int i = 0; i < 1000; i++)
    {
        for (int j = 0; j < 1000; j++)
        {
            for (int t = 0; t < ms; t++)
            {
                sum++;
            }
        }
    }
}

int main()
{
    char buf[100];
    int step = CarType_Q;

    while (1)
    {

        if (step == CarType_Q)
        {
            printf(CLEAR_SCREEN);

            printf("        ______________\n");
            printf("       /|            | \n");
            printf("  ____/_|_____________|____\n");
            printf(" |                      O  |\n");
            printf(" '-(@)----------------(@)--'\n");
            printf("===============================\n");
            printf("어떤 차량 타입을 선택할까요?\n");
            {
                const auto& carTypes = CarTypeCatalog::availableCarTypes();
                for (size_t i = 0; i < carTypes.size(); i++)
                {
                    printf("%zu. %s\n", i + 1, CarTypeCatalog::nameOf(carTypes[i]).c_str());
                }
            }
        }
        else if (step == Engine_Q)
        {
            printf(CLEAR_SCREEN);
            printf("어떤 엔진을 탑재할까요?\n");
            printf("0. 뒤로가기\n");
            {
                const auto& engines = PartCatalog::availableEngines();
                for (size_t i = 0; i < engines.size(); i++)
                {
                    printf("%zu. %s\n", i + 1, PartCatalog::nameOf(engines[i]).c_str());
                }
            }
            printf("4. 고장난 엔진\n");
        }
        else if (step == brakeSystem_Q)
        {
            printf(CLEAR_SCREEN);
            printf("어떤 제동장치를 선택할까요?\n");
            printf("0. 뒤로가기\n");
            {
                const auto& brakes = PartCatalog::availableBrakeSystems();
                for (size_t i = 0; i < brakes.size(); i++)
                {
                    printf("%zu. %s\n", i + 1, PartCatalog::nameOf(brakes[i]).c_str());
                }
            }
        }
        else if (step == SteeringSystem_Q)
        {
            printf(CLEAR_SCREEN);
            printf("어떤 조향장치를 선택할까요?\n");
            printf("0. 뒤로가기\n");
            {
                const auto& steerings = PartCatalog::availableSteeringSystems();
                for (size_t i = 0; i < steerings.size(); i++)
                {
                    printf("%zu. %s\n", i + 1, PartCatalog::nameOf(steerings[i]).c_str());
                }
            }
        }
        else if (step == Run_Test)
        {
            printf(CLEAR_SCREEN);
            printf("멋진 차량이 완성되었습니다.\n");
            printf("어떤 동작을 할까요?\n");
            printf("0. 처음 화면으로 돌아가기\n");
            printf("1. RUN\n");
            printf("2. Test\n");
        }
        printf("===============================\n");

        printf("INPUT > ");
        fgets(buf, sizeof(buf), stdin);

        // 엔터 개행문자 제거
        char* context = nullptr;
        strtok_s(buf, "\r", &context);
        strtok_s(buf, "\n", &context);

        if (!strcmp(buf, "exit"))
        {
            printf("바이바이\n");
            break;
        }

        // 숫자로 된 대답인지 확인
        char* checkNumber;
        int answer = strtol(buf, &checkNumber, 10); // 문자열을 10진수로 변환

        // 입력받은 문자가 숫자가 아니라면
        if (*checkNumber != '\0')
        {
            printf("ERROR :: 숫자만 입력 가능\n");
            delay(800);
            continue;
        }

        if (step == CarType_Q && !(answer >= 1 && answer <= 3))
        {
            printf("ERROR :: 차량 타입은 1 ~ 3 범위만 선택 가능\n");
            delay(800);
            continue;
        }

        if (step == Engine_Q && !(answer >= 0 && answer <= 4))
        {
            printf("ERROR :: 엔진은 1 ~ 4 범위만 선택 가능\n");
            delay(800);
            continue;
        }

        if (step == brakeSystem_Q && !(answer >= 0 && answer <= 3))
        {
            printf("ERROR :: 제동장치는 1 ~ 3 범위만 선택 가능\n");
            delay(800);
            continue;
        }

        if (step == SteeringSystem_Q && !(answer >= 0 && answer <= 2))
        {
            printf("ERROR :: 조향장치는 1 ~ 2 범위만 선택 가능\n");
            delay(800);
            continue;
        }

        if (step == Run_Test && !(answer >= 0 && answer <= 2))
        {
            printf("ERROR :: Run 또는 Test 중 하나를 선택 필요\n");
            delay(800);
            continue;
        }

        // 처음으로 돌아가기
        if (answer == 0 && step == Run_Test)
        {
            step = CarType_Q;
            continue;
        }

        // 이전으로 돌아가기
        if (answer == 0 && step >= 1)
        {
            step -= 1;
            continue;
        }

        if (step == CarType_Q)
        {
            selectCarType(answer);
            delay(800);
            step = Engine_Q;
        }
        else if (step == Engine_Q)
        {
            selectEngine(answer);
            delay(800);
            step = brakeSystem_Q;
        }
        else if (step == brakeSystem_Q)
        {
            selectbrakeSystem(answer);
            delay(800);
            step = SteeringSystem_Q;
        }
        else if (step == SteeringSystem_Q)
        {
            selectSteeringSystem(answer);
            delay(800);
            step = Run_Test;
        }
        else if (step == Run_Test && answer == 1)
        {
            runProducedCar();
            delay(2000);
        }
        else if (step == Run_Test && answer == 2)
        {
            printf("Test...\n");
            delay(1500);
            testProducedCar();
            delay(2000);
        }
    }
}

void selectCarType(int answer)
{
    stack[CarType_Q] = answer;
    if (answer >= 1 && answer <= 3)
    {
        printf("차량 타입으로 %s을 선택하셨습니다.\n", CarTypeCatalog::nameOf(static_cast<CarType>(answer)).c_str());
    }
}

void selectEngine(int answer)
{
    stack[Engine_Q] = answer;
    if (answer >= 1 && answer <= 3)
    {
        printf("%s 엔진을 선택하셨습니다.\n", PartCatalog::nameOf(static_cast<Engine>(answer)).c_str());
    }
}

void selectbrakeSystem(int answer)
{
    stack[brakeSystem_Q] = answer;
    if (answer >= 1 && answer <= 3)
    {
        printf("%s 제동장치를 선택하셨습니다.\n", PartCatalog::nameOf(static_cast<BrakeSystem>(answer)).c_str());
    }
}

void selectSteeringSystem(int answer)
{
    stack[SteeringSystem_Q] = answer;
    if (answer >= 1 && answer <= 2)
    {
        printf("%s 조향장치를 선택하셨습니다.\n", PartCatalog::nameOf(static_cast<SteeringSystem>(answer)).c_str());
    }
}

int isValidCheck()
{
    if (stack[CarType_Q] == SEDAN && stack[brakeSystem_Q] == CONTINENTAL)
    {
        return false;
    }
    else if (stack[CarType_Q] == SUV && stack[Engine_Q] == TOYOTA)
    {
        return false;
    }
    else if (stack[CarType_Q] == TRUCK && stack[Engine_Q] == WIA)
    {
        return false;
    }
    else if (stack[CarType_Q] == TRUCK && stack[brakeSystem_Q] == MANDO)
    {
        return false;
    }
    else if (stack[brakeSystem_Q] == BOSCH_B && stack[SteeringSystem_Q] != BOSCH_S)
    {
        return false;
    }
    else
    {
        return true;
    }
    return true;
}

void runProducedCar()
{
    if (isValidCheck() == false)
    {
        printf("자동차가 동작되지 않습니다\n");
    }
    else
    {
        if (stack[Engine_Q] == 4)
        {
            printf("엔진이 고장나있습니다.\n");
            printf("자동차가 움직이지 않습니다.\n");
        }
        else
        {
            if (stack[CarType_Q] == 1)
                printf("Car Type : Sedan\n");
            if (stack[CarType_Q] == 2)
                printf("Car Type : SUV\n");
            if (stack[CarType_Q] == 3)
                printf("Car Type : Truck\n");
            if (stack[Engine_Q] == 1)
                printf("Engine : GM\n");
            if (stack[Engine_Q] == 2)
                printf("Engine : TOYOTA\n");
            if (stack[Engine_Q] == 3)
                printf("Engine : WIA\n");
            if (stack[brakeSystem_Q] == 1)
                printf("Brake System : Mando\n");
            if (stack[brakeSystem_Q] == 2)
                printf("Brake System : Continental\n");
            if (stack[brakeSystem_Q] == 3)
                printf("Brake System : Bosch\n");
            if (stack[SteeringSystem_Q] == 1)
                printf("SteeringSystem : Bosch\n");
            if (stack[SteeringSystem_Q] == 2)
                printf("SteeringSystem : Mobis\n");

            printf("자동차가 동작됩니다.\n");
        }
    }
}

void testProducedCar()
{
    if (stack[CarType_Q] == SEDAN && stack[brakeSystem_Q] == CONTINENTAL)
    {
        printf("자동차 부품 조합 테스트 결과 : FAIL\n");
        printf("Sedan에는 Continental제동장치 사용 불가\n");
    }
    else if (stack[CarType_Q] == SUV && stack[Engine_Q] == TOYOTA)
    {
        printf("자동차 부품 조합 테스트 결과 : FAIL\n");
        printf("SUV에는 TOYOTA엔진 사용 불가\n");
    }
    else if (stack[CarType_Q] == TRUCK && stack[Engine_Q] == WIA)
    {
        printf("자동차 부품 조합 테스트 결과 : FAIL\n");
        printf("Truck에는 WIA엔진 사용 불가\n");
    }
    else if (stack[CarType_Q] == TRUCK && stack[brakeSystem_Q] == MANDO)
    {
        printf("자동차 부품 조합 테스트 결과 : FAIL\n");
        printf("Truck에는 Mando제동장치 사용 불가\n");
    }
    else if (stack[brakeSystem_Q] == BOSCH_B && stack[SteeringSystem_Q] != BOSCH_S)
    {
        printf("자동차 부품 조합 테스트 결과 : FAIL\n");
        printf("Bosch제동장치에는 Bosch조향장치 이외 사용 불가\n");
    }
    else
    {
        printf("자동차 부품 조합 테스트 결과 : PASS\n");
    }
}

#endif