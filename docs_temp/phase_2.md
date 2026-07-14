# Phase 2 — 도메인 모델 도입 (Builder)

참고: `docs/all_phase_goals.md` Phase 2, `docs/refactoring_structure.md` 3.1절, `docs/spec.md`

## 목표

전역 배열 `stack[10]` 기반 조립 방식(현재는 레거시 `#else` 분기에만 존재)을 타입 안전한 `Car`/`CarBuilder`로 대체할 준비를 한다. Phase 1에서 만든 `CompatibilityChecker`가 4개의 `int` 나열(`PartSelection`) 대신 `Car` 객체 하나를 입력받도록 시그니처를 정리한다. 레거시 `#else` 분기는 이번 Phase에서도 건드리지 않는다(Phase 5에서 제거).

## 작업 목록

1. `claude_assemble/domain/Car.h` 신규 작성
   - `enum class CarType : int { Sedan = 1, Suv = 2, Truck = 3 };`
   - `enum class Engine : int { Gm = 1, Toyota = 2, Wia = 3 };`
   - `enum class BrakeSystem : int { Mando = 1, Continental = 2, Bosch = 3 };`
   - `enum class SteeringSystem : int { Bosch = 1, Mobis = 2 };`
   - 값을 spec.md/레거시 정수 매핑과 동일하게 명시해, 기존 int 기반 테스트와의 교차검증(`static_cast`)이 쉬워지도록 한다.
   - `struct Car { CarType type; Engine engine; BrakeSystem brake; SteeringSystem steering; };`
2. `claude_assemble/build/CarBuilder.h` 신규 작성
   - `setCarType`/`setEngine`/`setBrakeSystem`/`setSteeringSystem`이 각각 `CarBuilder&`를 반환(체이닝).
   - `build() const -> std::optional<Car>` — 4개 필드 중 하나라도 비어 있으면 `std::nullopt`.
3. `claude_assemble/rules/ICompatibilityRule.h` 수정
   - `PartSelection` 제거, `domain/Car.h`의 `Car`를 입력으로 받도록 `check(const Car&)`로 변경.
4. `claude_assemble/rules/CompatibilityRules.h` 수정
   - 매직넘버 비교(`selection.carType == 1` 등)를 `car.type == CarType::Sedan` 등 enum class 비교로 교체.
5. `claude_assemble/rules/CompatibilityChecker.h` 수정
   - `validate(const Car&)`로 시그니처 변경 (내부 로직은 동일).
6. `claude_assemble/assemble.cpp`의 `_DEBUG` 테스트 섹션 수정
   - `#include "domain/Car.h"`, `#include "build/CarBuilder.h"` 추가.
   - Phase 1 `CompatibilityCheckerTest`의 `PartSelection{...}` 리터럴을 `Car{CarType::..., Engine::..., ...}`로 교체 (기대 결과는 동일해야 함 — 회귀 없음 확인).
   - `ExhaustiveCrossCheckWithLegacy`는 레거시 int 루프 변수를 `static_cast<CarType>(carType)` 등으로 `Car`에 담아 비교하도록 수정 (enum 값이 레거시 정수와 동일하므로 안전).
   - 신규 `CarBuilderTest` 스위트 추가:
     - `BuildSucceedsWhenAllPartsAreSet` — 4개 필드를 모두 설정하면 `build()`가 값 있는 `Car`를 반환.
     - `BuildFailsWhenAnyPartIsMissing` — 4개 필드 중 하나라도 비어 있으면 `build()`가 `std::nullopt`.
     - `BuildFailsWhenNothingIsSet` — 아무것도 설정하지 않으면 `std::nullopt`.
   - 기존 `AssembleSpecTest` 8개는 레거시 회귀 기준선이므로 무수정 유지.
7. `claude_assemble.vcxproj`/`.vcxproj.filters`에 신규 헤더 2종(`domain/Car.h`, `build/CarBuilder.h`) `ClInclude` 등록.
8. 레거시 `#else` 분기(`stack[10]`, `isValidCheck`, `runProducedCar`, `testProducedCar`)는 수정하지 않음.

## 테스트 계획

- 기존 8개 `AssembleSpecTest` 무수정 유지 (레거시 회귀 기준선).
- `CompatibilityCheckerTest` 7개는 `Car` 기준으로 재작성하되 기대 결과는 Phase 1과 동일하게 유지.
- 신규 `CarBuilderTest` 3개 추가 (정상 조립 1 + 누락 케이스 2).
- Debug|x64 빌드 후 실행하여 전체 테스트(8 + 7 + 3 = 18개) PASS 확인.

## 완료 조건 (docs/all_phase_goals.md 기준)

- `CarBuilder` 단위 테스트: 정상 조립, 일부 부품 누락 시 `build()` 실패 케이스 PASS.
- Phase 1에서 만든 규칙 테스트를 `Car` 객체 기준으로 리팩토링해도 결과 동일하게 PASS.

---

## 실행 결과 (작업 완료 후 갱신)

- 신규 파일: `claude_assemble/domain/Car.h` (`CarType`/`Engine`/`BrakeSystem`/`SteeringSystem` enum class + `Car` 구조체), `claude_assemble/build/CarBuilder.h` (`std::optional` 기반 Builder)
- 변경 파일:
  - `claude_assemble/rules/ICompatibilityRule.h`, `CompatibilityRules.h`, `CompatibilityChecker.h` — `PartSelection`(int 4개) 제거, `Car` 입력으로 시그니처 변경
  - `claude_assemble/assemble.cpp` — `_DEBUG` 섹션에 include 추가, Phase 1 `CompatibilityCheckerTest` 7개를 `Car`/enum class 리터럴로 교체(기대 결과 동일), 신규 `CarBuilderTest` 3개 추가. `AssembleSpecTest` 8개와 레거시 `#else` 분기는 무수정.
  - `claude_assemble.vcxproj`/`.filters` — `domain/Car.h`, `build/CarBuilder.h` `ClInclude` 등록
- 빌드: `MSBuild claude_assemble.slnx -p:Configuration=Debug -p:Platform=x64 -t:Rebuild` → 성공 (경고 6개는 기존 EUC-KR 인코딩 C4819, 오류 0개)
- 실행 결과 (`x64/Debug/claude_assemble.exe`):

```
[==========] Running 18 tests from 3 test suites.
[----------] 8 tests from AssembleSpecTest ... [  PASSED  ] (전부 OK, Phase 0 기준선 회귀 없음)
[----------] 7 tests from CompatibilityCheckerTest ... [  PASSED  ] (Car 기준으로 재작성, 결과 동일)
[----------] 3 tests from CarBuilderTest
[ RUN      ] CarBuilderTest.BuildSucceedsWhenAllPartsAreSet
[       OK ] CarBuilderTest.BuildSucceedsWhenAllPartsAreSet (0 ms)
[ RUN      ] CarBuilderTest.BuildFailsWhenAnyPartIsMissing
[       OK ] CarBuilderTest.BuildFailsWhenAnyPartIsMissing (0 ms)
[ RUN      ] CarBuilderTest.BuildFailsWhenNothingIsSet
[       OK ] CarBuilderTest.BuildFailsWhenNothingIsSet (0 ms)

[==========] 18 tests from 3 test suites ran. (1 ms total)
[  PASSED  ] 18 tests.
```

- 완료 조건 충족: `CarBuilder` 정상 조립/누락 케이스 PASS, Phase 1 규칙 테스트를 `Car` 객체 기준으로 리팩토링해도 결과 동일하게 PASS.
