# Phase 1 — 검증 로직 분리 (Chain of Responsibility)

참고: `docs/all_phase_goals.md` Phase 1, `docs/refactoring_structure.md` 3.3절, `docs/spec.md`

## 목표

`isValidCheck()`/`testProducedCar()`에 중복된 제한조건 1·2(총 5개 규칙)를 `ICompatibilityRule` 구현체 5종 + `CompatibilityChecker`(Chain of Responsibility)로 추출한다. 레거시 `#else` 분기(절차지향 구현)는 이번 Phase에서 건드리지 않고, 신규 클래스를 병행 추가해 Phase 0 기준선과 결과가 동일한지 교차 검증한다. Car/CarBuilder(Phase 2)는 아직 도입하지 않으므로, 규칙은 4개의 정수 필드로 구성된 최소 스냅샷 타입을 입력으로 받는다.

## 작업 목록

1. `claude_assemble/rules/ICompatibilityRule.h` 신규 작성
   - `struct PartSelection { int carType; int engine; int brakeSystem; int steeringSystem; };` (Phase 0의 `isValidCombination` 파라미터를 그대로 값 타입으로 옮긴 것. Phase 2에서 `Car`로 대체 예정)
   - `class ICompatibilityRule` — `virtual std::optional<std::string> check(const PartSelection&) const = 0;`
2. `claude_assemble/rules/CompatibilityRules.h` 신규 작성 — 5개 규칙 클래스 (모두 헤더 온리, `ICompatibilityRule` 상속)
   - `BoschBrakeRequiresBoschSteeringRule`
   - `SedanCannotUseContinentalBrakeRule`
   - `SuvCannotUseToyotaEngineRule`
   - `TruckCannotUseWiaEngineRule`
   - `TruckCannotUseMandoBrakeRule`
   - 각 규칙은 spec.md 제한조건 문구를 위반 사유 문자열로 반환한다.
3. `claude_assemble/rules/CompatibilityChecker.h` 신규 작성
   - `addRule(std::unique_ptr<ICompatibilityRule>)`, `validate(const PartSelection&) const -> std::vector<std::string>` (위반 사유 전체 수집, 순서 무관)
   - `defaultChecker()` 헬퍼 함수: 5개 규칙을 모두 등록한 `CompatibilityChecker`를 생성해 반환 (테스트/향후 Command에서 재사용)
4. `claude_assemble/assemble.cpp`의 `#ifdef _DEBUG` 테스트 섹션 수정
   - 상단에 `#include "rules/ICompatibilityRule.h"`, `#include "rules/CompatibilityRules.h"`, `#include "rules/CompatibilityChecker.h"` 추가
   - 기존 6개 `AssembleSpecTest` + Phase 0에서 추가한 2개 테스트는 **그대로 유지** (레거시 `isValidCombination` 기준 회귀 기준선이므로 수정하지 않음)
   - 신규 테스트 스위트 `CompatibilityCheckerTest` 추가:
     - 규칙 5종 각각에 대해 1개 이상 테스트 (`BoschBrakeRequiresBoschSteeringRule`, `SedanCannotUseContinentalBrakeRule`, `SuvCannotUseToyotaEngineRule`, `TruckCannotUseWiaEngineRule`, `TruckCannotUseMandoBrakeRule`) — `docs/refactoring_structure.md`가 명시한 "테스트가 규칙 단위로 1:1 대응" 요구 충족
     - `MultipleRuleViolationsReturnsAllReasons` — 여러 규칙 동시 위반 시 `validate()`가 위반 사유를 모두 반환하는지 확인
     - `ExhaustiveCrossCheckWithLegacy` — 54개 전체 조합에 대해 레거시 `isValidCombination()`과 `CompatibilityChecker::validate().empty()`가 동일한 결과를 내는지 교차 검증 (Phase 0 `ExhaustiveCombinationMatrix`와 동일한 조합 루프 재사용)
5. `claude_assemble.vcxproj`/`claude_assemble.vcxproj.filters`에 신규 헤더 3종을 `ClInclude`로 등록 (컴파일 단위는 아니므로 빌드에 필수는 아니지만 솔루션 탐색기 가시성을 위해 추가).
6. 레거시 `#else` 분기(`isValidCheck`, `runProducedCar`, `testProducedCar`, `stack[10]`)는 **수정하지 않음** — Phase 2 이후 점진적으로 교체.

## 테스트 계획

- 기존 8개 `AssembleSpecTest` (Phase 0 기준선) 무수정 유지 — 회귀 확인용.
- 신규 `CompatibilityCheckerTest` 스위트 추가 (규칙 5개 + 다중 위반 1개 + 전수 교차검증 1개 = 7개 케이스).
- Debug|x64 빌드 후 실행하여 전체 테스트(8 + 7 = 15개) PASS 확인 (완료 조건).

## 완료 조건 (docs/all_phase_goals.md 기준)

- 신규 `CompatibilityChecker` 기준 테스트 전부 PASS.
- 기존 레거시 함수(`isValidCombination`, 레거시 `isValidCheck`와 동일 로직)와 신규 `CompatibilityChecker`가 동일 입력에 대해 동일 결과를 내는지 교차 검증 테스트 통과.

---

## 실행 결과 (작업 완료 후 갱신)

- 신규 파일: `claude_assemble/rules/ICompatibilityRule.h`, `claude_assemble/rules/CompatibilityRules.h`, `claude_assemble/rules/CompatibilityChecker.h`
- 변경 파일: `claude_assemble/assemble.cpp` (`_ifdef _DEBUG` 섹션에 include 추가 + `CompatibilityCheckerTest` 7개 케이스 추가, 기존 `AssembleSpecTest` 8개는 무수정), `claude_assemble.vcxproj`/`.filters` (신규 헤더 3종 `ClInclude` 등록)
- 레거시 `#else` 분기(`isValidCheck`, `runProducedCar`, `testProducedCar`, `stack[10]`)는 무수정.
- 빌드: `MSBuild claude_assemble.slnx -p:Configuration=Debug -p:Platform=x64 -t:Rebuild` → 성공 (경고 4개는 기존에도 있던 EUC-KR 인코딩 관련 C4819, 오류 0개)
- 실행 결과 (`x64/Debug/claude_assemble.exe`):

```
[==========] Running 15 tests from 2 test suites.
[----------] Global test environment set-up.
[----------] 8 tests from AssembleSpecTest
...
[  PASSED  ] 8 tests. (AssembleSpecTest, Phase 0 기준선 회귀 없음)
[----------] 7 tests from CompatibilityCheckerTest
...
[  PASSED  ] 7 tests. (CompatibilityCheckerTest: 규칙 5종 + 다중 위반 + 54개 조합 교차검증)

[==========] 15 tests from 2 test suites ran. (1 ms total)
[  PASSED  ] 15 tests.
```

- 완료 조건 충족: `CompatibilityChecker` 기준 테스트 전부 PASS, `ExhaustiveCrossCheckWithLegacy`로 레거시 `isValidCombination()`과 54개 조합 전체 결과 일치 확인.
