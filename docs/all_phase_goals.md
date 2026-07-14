# 리팩토링 Phase별 목표

기준 문서: `docs/refactoring_structure.md`(구조 설계), `docs/spec.md`(요구사항)

전체 방식은 Strangler Fig(점진적 교체)다. 각 Phase는 **기존 동작을 깨지 않고**, Phase 종료 시점에 gmock 테스트가 100% 통과하는 것을 완료 조건으로 삼는다. 다음 Phase로 넘어가기 전 반드시 이전 Phase의 테스트가 통과해야 한다.

---

## Phase 0 — 안전망 구축 (베이스라인 테스트)

**목표**: 리팩토링 도중 동작이 깨지는 것을 감지할 수 있도록, 지금의 절차지향 로직(`isValidCheck`, `runProducedCar`, `testProducedCar`)을 기준으로 한 회귀 테스트를 확보한다.

**작업**
- 현재 `assemble.cpp`에 있는 `AssembleSpecTest` 6개 케이스 유지 및 spec.md 제한조건 1·2에 대한 누락 케이스 보강 (예: 여러 CarType × 여러 부품 조합의 경계값 케이스 추가).
- `CarType_Q`, `Engine_Q`, `brakeSystem_Q`, `SteeringSystem_Q`, `Run_Test` 입력 범위 검증(`1~3`, `0~4` 등) 관련 케이스도 필요 시 텍스트로 정리 (아직 로직 분리 전이므로 자동화 테스트는 Phase 4 이후).

**완료 조건**
- `assemble.cpp` Debug 빌드 실행 시 모든 테스트 PASS.
- 이후 Phase에서 회귀를 판단할 "기준선" 테스트 세트 확정.

---

## Phase 1 — 검증 로직 분리 (Chain of Responsibility)

**목표**: `isValidCheck()`와 `testProducedCar()`에 중복된 5개 제한조건을 규칙 객체로 추출해, 검증 로직을 단일화하고 확장 가능하게 만든다.

**작업**
- `ICompatibilityRule` 인터페이스 정의.
- 제한조건 1·2를 각각의 Rule 클래스로 구현:
  - `BoschBrakeRequiresBoschSteeringRule`
  - `SedanCannotUseContinentalBrakeRule`
  - `SuvCannotUseToyotaEngineRule`
  - `TruckCannotUseWiaEngineRule`
  - `TruckCannotUseMandoBrakeRule`
- `CompatibilityChecker`(규칙 체인) 구현: 여러 규칙을 등록하고, 위반 사유 목록을 반환.
- 기존 `isValidCheck()` / `testProducedCar()` 내부 조건문은 그대로 두되, 신규 `CompatibilityChecker`를 병행 추가 (아직 교체하지 않음 — 다음 Phase에서 교체).
- Phase 0의 테스트를 `CompatibilityChecker` 기준으로 다시 작성 (규칙 클래스 1개당 테스트 1개 이상 매핑).

**완료 조건**
- 신규 `CompatibilityChecker` 기준 테스트 전부 PASS.
- 기존 레거시 함수(`isValidCheck` 등)와 신규 `CompatibilityChecker`가 동일 입력에 대해 동일 결과를 내는지 교차 검증 테스트 통과.

---

## Phase 2 — 도메인 모델 도입 (Builder)

**목표**: 전역 배열 `stack[10]` 기반 조립 방식을 타입 안전한 `Car`/`CarBuilder`로 대체한다.

**작업**
- `Car`, `CarType`, `Engine`, `BrakeSystem`, `SteeringSystem` 값 타입(enum class 등) 정의.
- `CarBuilder` 구현 (`setCarType`/`setEngine`/`setBrakeSystem`/`setSteeringSystem`/`build()`).
- `build()`는 값이 모두 채워지지 않으면 `std::nullopt` 반환하도록 구현 (미완성 조합 방지).
- Phase 1의 `CompatibilityChecker`가 `Car`를 입력으로 받도록 시그니처 정리 (`int` 나열 대신 `Car` 하나).

**완료 조건**
- `CarBuilder` 단위 테스트: 정상 조립, 일부 부품 누락 시 `build()` 실패 케이스 PASS.
- Phase 1에서 만든 규칙 테스트를 `Car` 객체 기준으로 리팩토링해도 결과 동일하게 PASS.

---

## Phase 3 — 부품 카탈로그 도입 (Factory / Catalog)

**목표**: 제조사·차량 타입이 코드 곳곳(`if(answer==N)`)에 흩어져 있는 구조를, 한 곳에서 관리하는 카탈로그로 대체해 확장성을 확보한다.

**작업**
- `PartCatalog`(Engine/BrakeSystem/SteeringSystem 목록·이름 매핑) 구현.
- `CarTypeCatalog`(Sedan/SUV/Truck 목록·이름 매핑) 구현.
- CLI 출력 문자열(`"1. GM"`, `"2. TOYOTA"` 등)을 하드코딩 대신 카탈로그에서 생성하도록 변경.
- **확장성 검증**: 가상의 신규 제조사 1개(예: 테스트용 더미 Engine)를 추가해, 카탈로그 테이블 수정만으로 반영되는지 확인 (실제 제품에 추가하지 않고 테스트 코드 상에서만 검증 후 되돌림).

**완료 조건**
- 카탈로그 기반 목록 조회 단위 테스트 PASS.
- 더미 제조사 추가/제거 실험에서 `PartCatalog` 외 다른 파일 수정이 필요 없었음을 확인.

---

## Phase 4 — 입출력 추상화 (Dependency Inversion)

**목표**: `printf`/`fgets`가 로직에 직접 섞여 있어 유닛테스트가 불가능한 구조를 개선해, gmock으로 전체 시나리오를 테스트할 수 있게 만든다.

**작업**
- `IUserInterface` 인터페이스 정의 (`print`, `readAnswer`).
- `ConsoleUserInterface`(실제 `printf`/`fgets` 구현) 구현.
- `MockUserInterface`(gmock 기반) 구현.
- 기존 입력 파싱 로직(개행 제거, `strtol`, `exit` 처리, 범위 검증 메시지)을 `ConsoleUserInterface`/`IUserInterface` 책임으로 이동.

**완료 조건**
- `MockUserInterface`로 입력 시퀀스를 주입해, 표준입출력 없이 최소 1개 시나리오(예: 부품 선택 → PASS 확인) 테스트 PASS.

---

## Phase 5 — 상태·커맨드 도입 및 엔트리포인트 정리 (State + Command)

**목표**: `main()`의 `while(1)` + 전역 `step` 분기와 `runProducedCar`/`testProducedCar` 중복 로직을 각각 State/Command 객체로 교체하고, 레거시 절차지향 코드를 제거한다.

**작업**
- `WizardState` 인터페이스 및 5개 구현체(`CarTypeSelectionState`, `EngineSelectionState`, `BrakeSystemSelectionState`, `SteeringSystemSelectionState`, `ResultState`) 구현.
- `CarWizard`(State 컨텍스트, `CarBuilder` 보유) 구현.
- `ICarCommand`, `RunCarCommand`, `TestCarCommand` 구현 (내부에서 Phase 1의 `CompatibilityChecker` 재사용).
- `main.cpp`를 `ConsoleUserInterface` + `CarWizard` 생성/실행만 남도록 축소.
- 기존 `assemble.cpp`의 레거시 `#else` 분기(절차지향 구현) 제거.
- `docs/refactoring_structure.md`의 디렉터리 구조안대로 파일 분리 완료 (`domain/`, `build/`, `rules/`, `commands/`, `ui/`, `wizard/`, `tests/`).

**완료 조건**
- `CarWizard` + `MockUserInterface` 기반 End-to-End 시나리오 테스트: "뒤로가기" 포함 최소 2개 이상 시나리오 PASS.
- Phase 0~4에서 작성된 모든 테스트가 신규 구조 기준으로 재작성되어 PASS.
- 레거시 절차지향 코드(원래 `assemble.cpp`의 전역 배열/거대 if-else)가 저장소에 더 이상 존재하지 않음.

---

## Phase 요약 표

| Phase | 적용 패턴 | 핵심 산출물 | 완료 기준 |
|-------|-----------|-------------|-----------|
| 0 | - | 베이스라인 회귀 테스트 | 기존 로직 테스트 전부 PASS |
| 1 | Chain of Responsibility | `ICompatibilityRule` 5종 + `CompatibilityChecker` | 신규 규칙 테스트 PASS, 레거시와 결과 일치 |
| 2 | Builder | `Car`, `CarBuilder` | 조립/미완성 케이스 테스트 PASS |
| 3 | Factory / Catalog | `PartCatalog`, `CarTypeCatalog` | 카탈로그 테스트 PASS, 확장성 실험 통과 |
| 4 | Dependency Inversion | `IUserInterface`, `ConsoleUserInterface`, `MockUserInterface` | Mock 기반 시나리오 테스트 PASS |
| 5 | State + Command | `WizardState`군, `CarWizard`, `ICarCommand`군 | E2E 시나리오 PASS, 레거시 코드 제거 완료 |
