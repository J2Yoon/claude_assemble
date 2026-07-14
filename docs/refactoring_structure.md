# 자동차 조립 시스템 리팩토링 구조 설계

기준 문서: `docs/spec.md`, 현재 구현: `claude_assemble/assemble.cpp`

## 1. 목표

`docs/spec.md` 5번 항목 "기존 시스템의 아쉬운 점"에서 지적된 4가지 문제를 GoF 디자인 패턴 기반의 구조적 설계로 해결한다.

| spec.md 상 문제점                          | 원인 (현재 코드)                                                                 | 해결 방향 |
|---------------------------------------------|-----------------------------------------------------------------------------------|-----------|
| 절차지향식 코드로 유지보수가 어려운 구조     | `stack[10]` 전역 배열 + 거대한 `if/else` 상태 분기(`step`) + `switch` 없이 나열된 조건 | **State**, **Builder** 패턴으로 상태·조립 절차를 객체화 |
| 안전하지 않은 문법들이 사용                  | 원시 배열 인덱싱(`stack[Engine_Q]`), `strtol`+수동 파싱, 매직넘버 enum 캐스팅       | 값 객체(Value Object)/강타입 enum class, `std::optional`, 입력 파싱 책임 분리 |
| 확장성이 고려되지 않음                       | CarType/제조사 추가 시 `if(answer==1)` 형태 코드를 직접 수정해야 함 (OCP 위반)      | **Factory Method / Abstract Factory**, **Chain of Responsibility(Specification)**로 개방-폐쇄 원칙 적용 |
| 유닛테스트가 없음                            | `printf`/`fgets`가 로직에 직접 섞여 있어 입출력 없이는 테스트 불가                  | **Dependency Inversion**으로 `IUserInterface` 추상화, gmock으로 Mocking 가능한 구조 |

## 2. 전체 아키텍처 개요

```
+-----------------------------------------------------------+
|                     Presentation (CLI)                    |
|  CarWizard (State pattern context)                        |
|  - CarTypeSelectionState / PartSelectionState / ...        |
|  - IUserInterface (Console 구현체 + Mock 구현체)            |
+-----------------------------------------------------------+
                            |
                            v
+-----------------------------------------------------------+
|                     Application (Build)                   |
|  CarBuilder (Builder pattern)                              |
|  - setCarType(), setEngine(), setBrakeSystem(), ...        |
|  - build() -> Car                                          |
+-----------------------------------------------------------+
                 |                        |
                 v                        v
+---------------------------+  +----------------------------+
|      Domain / Parts       |  |     Rule Engine (검증)      |
|  CarType, Engine,          |  |  ICompatibilityRule (링크)  |
|  BrakeSystem,              |  |  - SedanNoContinental...    |
|  SteeringSystem            |  |  CompatibilityChecker       |
|  PartFactory (Factory)     |  |  (Chain of Responsibility)  |
+---------------------------+  +----------------------------+
                            |
                            v
+-----------------------------------------------------------+
|                      Command (동작)                       |
|  ICarCommand : RunCarCommand / TestCarCommand              |
+-----------------------------------------------------------+
```

레이어를 나눈 이유는 각 계층이 **독립적으로 단위 테스트 가능**하도록 만들기 위함이다 (spec.md의 "유닛테스트가 없음" 문제를 정면으로 해결).

## 3. 패턴별 상세 설계

### 3.1 Builder 패턴 — 차량 조립 절차

spec.md의 "차량 제조 순서"(타입 선택 → 부품 선택 → 테스트) 자체가 Builder 패턴의 전형적인 사용처다. 현재는 전역 배열 `stack[10]`에 순서대로 값을 채워 넣는 방식이라, 순서를 지키지 않거나 일부 값이 비어있는 상태(Truck 타입에 Untitled Engine 등)를 컴파일 타임에 막을 수 없다.

```cpp
class Car {
public:
    CarType type;
    Engine engine;
    BrakeSystem brake;
    SteeringSystem steering;
};

class CarBuilder {
public:
    CarBuilder& setCarType(CarType type);
    CarBuilder& setEngine(Engine engine);
    CarBuilder& setBrakeSystem(BrakeSystem brake);
    CarBuilder& setSteeringSystem(SteeringSystem steering);

    // 값이 모두 채워지지 않았으면 std::nullopt
    std::optional<Car> build() const;

private:
    std::optional<CarType> type_;
    std::optional<Engine> engine_;
    std::optional<BrakeSystem> brake_;
    std::optional<SteeringSystem> steering_;
};
```

- `stack[10]` 배열 인덱싱(안전하지 않은 문법) → `std::optional` 멤버 변수로 대체.
- 조립이 끝났는지(모든 부품 선택 완료) 여부를 `build()`가 명시적으로 검증.

### 3.2 Factory Method / Abstract Factory — 부품/제조사 확장

spec.md 2·3번 슬라이드는 "향후에 타입이 더 추가될 수 있다"고 명시한다. 현재 코드는 제조사가 추가될 때마다 `selectEngine()`의 `if(answer==N)` 라인을 직접 늘려야 해서 OCP(개방-폐쇄 원칙)를 위반한다.

```cpp
enum class Engine { GM, TOYOTA, WIA /* 추가 시 이 나열에만 추가 */ };
enum class BrakeSystem { MANDO, CONTINENTAL, BOSCH };
enum class SteeringSystem { BOSCH, MOBIS };

// 제조사 메타데이터(이름 출력 등)를 개별 조건문 대신 테이블/팩토리로 관리
class PartCatalog {
public:
    static const std::vector<Engine>& availableEngines();
    static const std::vector<BrakeSystem>& availableBrakeSystems();
    static const std::vector<SteeringSystem>& availableSteeringSystems();
    static std::string nameOf(Engine e);
    static std::string nameOf(BrakeSystem b);
    static std::string nameOf(SteeringSystem s);
};
```

- 신규 제조사/타입 추가는 `PartCatalog`의 테이블 한 곳만 수정하면 되도록 만든다 (Open for extension, Closed for modification).
- 차량 타입(Sedan/SUV/Truck) 자체도 동일하게 `CarTypeCatalog`로 관리해 "향후 타입 추가"에 대비한다.

### 3.3 Chain of Responsibility (Specification 패턴 병행) — 완성 가능 조합 검증

spec.md 4번 항목의 제한조건 1·2는 규칙이 독립적으로 나열되어 있다. 현재 `isValidCheck()`는 하나의 함수 안에 `if/else if`로 모든 규칙을 하드코딩해서, 규칙이 추가될 때마다 기존 함수를 계속 수정해야 한다 (OCP 위반, 테스트 시 규칙별 격리 불가).

각 제한조건을 별도의 Rule 객체로 분리하고, 체인으로 연결한다.

```cpp
class ICompatibilityRule {
public:
    virtual ~ICompatibilityRule() = default;
    // 위반 시 실패 사유를 반환, 위반 없으면 std::nullopt
    virtual std::optional<std::string> check(const Car& car) const = 0;
};

// 제한조건 1
class BoschBrakeRequiresBoschSteeringRule : public ICompatibilityRule { ... };
// 제한조건 2-1
class SedanCannotUseContinentalBrakeRule : public ICompatibilityRule { ... };
// 제한조건 2-2
class SuvCannotUseToyotaEngineRule : public ICompatibilityRule { ... };
// 제한조건 2-3
class TruckCannotUseWiaEngineRule : public ICompatibilityRule { ... };
// 제한조건 2-4
class TruckCannotUseMandoBrakeRule : public ICompatibilityRule { ... };

class CompatibilityChecker {
public:
    void addRule(std::unique_ptr<ICompatibilityRule> rule);
    // 위반한 규칙들의 사유 목록을 모두 반환 (PASS면 빈 벡터)
    std::vector<std::string> validate(const Car& car) const;

private:
    std::vector<std::unique_ptr<ICompatibilityRule>> rules_;
};
```

- 신규 제한조건 추가 = 새로운 `ICompatibilityRule` 구현 클래스 하나 추가 + `addRule()` 한 줄. 기존 규칙 클래스는 전혀 건드리지 않는다.
- 규칙 하나하나가 독립 클래스이므로, 지금 `assemble.cpp`에 있는 `AssembleSpecTest`의 각 `TEST()`가 규칙 단위로 자연스럽게 1:1 대응된다 (예: `BoschBrakeRequiresBoschSteeringRule` ↔ `BoschBrakeRequiresBoschSteering` 테스트).
- `runProducedCar()`/`testProducedCar()`에 중복되어 있던 동일 조건 5개(현재 두 함수에 똑같은 if/else 블록이 복붙되어 있음)가 `CompatibilityChecker` 하나로 단일화된다.

### 3.4 State 패턴 — CLI 마법사(Wizard) 흐름

현재 `main()`의 `while(1)` 루프는 `step`(int)이라는 하나의 변수로 5가지 화면(CarType_Q, Engine_Q, brakeSystem_Q, SteeringSystem_Q, Run_Test)을 모두 분기하며, "뒤로가기"/"처음으로" 로직까지 한 함수에 뒤섞여 있다.

```cpp
class WizardState {
public:
    virtual ~WizardState() = default;
    virtual void render(IUserInterface& ui) const = 0;
    // 입력을 처리하고 다음 State를 반환 (뒤로가기 포함)
    virtual std::unique_ptr<WizardState> handleInput(int answer, CarBuilder& builder, IUserInterface& ui) = 0;
};

class CarTypeSelectionState : public WizardState { ... };
class EngineSelectionState : public WizardState { ... };
class BrakeSystemSelectionState : public WizardState { ... };
class SteeringSystemSelectionState : public WizardState { ... };
class ResultState : public WizardState { ... }; // Run_Test 화면

class CarWizard {
public:
    void run(IUserInterface& ui);
private:
    std::unique_ptr<WizardState> currentState_ = std::make_unique<CarTypeSelectionState>();
    CarBuilder builder_;
};
```

- "뒤로가기"는 각 State가 이전 State를 반환하도록 구현 (`answer == 0`).
- 신규 화면(예: 향후 옵션 부품 선택 단계)이 추가돼도 `WizardState` 하나만 추가하면 되고, 기존 State 클래스는 수정할 필요가 없다.

### 3.5 Command 패턴 — RUN / Test 동작

`Run_Test` 화면에서의 두 동작(RUN, Test)도 각각 `runProducedCar()`, `testProducedCar()`라는 별개 함수로 구현되어 있고, 내부에 동일한 검증 로직이 중복되어 있다. Command 패턴으로 캡슐화한다.

```cpp
class ICarCommand {
public:
    virtual ~ICarCommand() = default;
    virtual void execute(const Car& car, const CompatibilityChecker& checker, IUserInterface& ui) = 0;
};

class RunCarCommand : public ICarCommand { ... };   // "자동차가 동작됩니다" 출력
class TestCarCommand : public ICarCommand { ... };  // "PASS"/"FAIL" + 사유 출력
```

- `RunCarCommand`, `TestCarCommand` 모두 내부적으로 `CompatibilityChecker::validate()`를 호출하므로, 3.3에서 통합한 검증 로직이 여기서도 재사용된다 (현재 코드의 중복 제거).

### 3.6 Dependency Inversion — 입출력 추상화 (테스트 가능성 확보)

spec.md가 지적한 "유닛테스트가 없음" 문제의 근본 원인은 로직 함수 내부에서 `printf`/`fgets`를 직접 호출하기 때문이다. 입출력을 인터페이스로 추상화하면, gmock으로 Mock을 만들어 표준입출력 없이도 Wizard 전체 흐름을 테스트할 수 있다.

```cpp
class IUserInterface {
public:
    virtual ~IUserInterface() = default;
    virtual void print(const std::string& message) = 0;
    virtual int readAnswer() = 0; // 숫자 파싱 + "exit" 처리까지 책임짐
};

class ConsoleUserInterface : public IUserInterface { ... }; // 실제 printf/fgets 구현

// 테스트용 (gmock)
class MockUserInterface : public IUserInterface {
public:
    MOCK_METHOD(void, print, (const std::string&), (override));
    MOCK_METHOD(int, readAnswer, (), (override));
};
```

- 이미 프로젝트에 `gmock` 패키지가 포함되어 있으므로(`packages/gmock.1.11.0`), 이 인터페이스만 도입하면 `CarWizard`, `WizardState` 전체를 실제 콘솔 입력 없이 시나리오 테스트(예: "1 → 2 → 1 → 1 → 1 입력 시 Sedan/TOYOTA/MANDO/BOSCH 차량이 만들어지고 PASS") 할 수 있다.

## 4. 디렉터리 구조 제안

```
claude_assemble/
├── domain/
│   ├── Car.h                 # Car, CarType, Engine, BrakeSystem, SteeringSystem
│   └── PartCatalog.h/.cpp    # 3.2 Factory
├── build/
│   └── CarBuilder.h/.cpp     # 3.1 Builder
├── rules/
│   ├── ICompatibilityRule.h
│   ├── CompatibilityRules.h/.cpp   # 3.3 규칙 구현체 5종
│   └── CompatibilityChecker.h/.cpp # 3.3 Chain
├── commands/
│   ├── ICarCommand.h
│   ├── RunCarCommand.h/.cpp
│   └── TestCarCommand.h/.cpp       # 3.5 Command
├── ui/
│   ├── IUserInterface.h             # 3.6 DIP
│   └── ConsoleUserInterface.h/.cpp
├── wizard/
│   ├── WizardState.h
│   ├── CarWizardStates.h/.cpp       # 3.4 State
│   └── CarWizard.h/.cpp
├── main.cpp                          # ConsoleUserInterface + CarWizard 조립만 담당
└── tests/
    ├── CompatibilityRuleTests.cpp    # 규칙 단위 테스트 (현재 AssembleSpecTest 대체/확장)
    ├── CarBuilderTests.cpp
    └── CarWizardTests.cpp            # MockUserInterface로 시나리오 테스트
```

파일을 분리하는 이유는 하나의 `assemble.cpp`(현재 약 400줄, 게임 로직·UI·검증·엔트리포인트가 전부 뒤섞임)에 계속 코드를 쌓는 대신, 계층별 책임을 물리적으로도 분리해 spec.md가 지적한 "유지보수가 어려운 구조"를 근본적으로 해소하기 위함이다.

## 5. 단계별 마이그레이션 로드맵 (Strangler Fig 방식)

기존 `assemble.cpp`를 한 번에 갈아엎지 않고, 기능을 유지한 채 점진적으로 대체한다.

1. **1단계 — 검증 로직 분리 (Chain of Responsibility)**
   `isValidCheck()` / `testProducedCar()`의 중복 로직을 `ICompatibilityRule` 5종 + `CompatibilityChecker`로 추출. 기존 `assemble.cpp`에 이미 작성된 `AssembleSpecTest` 6개를 이 신규 클래스 기준으로 재작성하며 회귀 검증.
2. **2단계 — 도메인 모델 도입 (Builder)**
   전역 `stack[10]` 배열을 `Car`/`CarBuilder`로 대체.
3. **3단계 — 부품 카탈로그 도입 (Factory)**
   제조사별 `if(answer==N)` 나열을 `PartCatalog` 테이블로 대체. 이 시점부터 신규 제조사/타입 추가가 코드 한 곳(카탈로그)만 수정하면 되는지 검증.
4. **4단계 — 입출력 추상화 (DIP)**
   `printf`/`fgets` 호출부를 `IUserInterface`로 감싸고, `MockUserInterface`로 Wizard 시나리오 테스트 작성.
5. **5단계 — 상태/커맨드 도입 (State + Command)**
   `main()`의 `while(1)` + `step` 분기를 `CarWizard` + `WizardState`로, `runProducedCar`/`testProducedCar`를 `ICarCommand`로 교체. 최종적으로 `main.cpp`는 `ConsoleUserInterface`와 `CarWizard`를 생성해 실행하는 것 외에 로직을 갖지 않게 된다.

각 단계마다 기존 gmock 테스트가 통과하는 것을 확인하고 다음 단계로 넘어가, 리팩토링 중간에 동작이 깨지는 것을 방지한다.

## 6. 기대 효과 요약

| 패턴 | 해결하는 spec.md 문제 |
|------|------------------------|
| Builder | 전역 배열 기반 조립 절차 → 타입 안전한 단계별 조립 |
| Factory Method / Catalog | 제조사·타입 추가 시 기존 코드 수정(OCP 위반) → 카탈로그 테이블만 수정 |
| Chain of Responsibility | 검증 로직 중복(2곳에 동일 if/else) 및 규칙 추가 시 기존 함수 수정 → 규칙별 클래스 추가만으로 확장 |
| State | 전역 `step` 변수 + 거대 if/else → 화면별 클래스로 분리, 흐름 추가 용이 |
| Command | RUN/Test 동작 중복 → 공통 검증 재사용, 동작 추가 용이 |
| Dependency Inversion (IUserInterface) | `printf`/`fgets` 직결로 유닛테스트 불가 → Mock 주입으로 전체 시나리오 테스트 가능 |
