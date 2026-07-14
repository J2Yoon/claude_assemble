# Phase 0 — 안전망 구축 (베이스라인 테스트)

참고: `docs/all_phase_goals.md` Phase 0, `docs/spec.md`

## 목표

리팩토링(Phase 1~5)을 진행하다가 동작이 깨지는 것을 즉시 감지할 수 있도록, 현재 절차지향 로직(`isValidCombination`, 원본은 `isValidCheck`/`testProducedCar`)을 기준으로 한 회귀 테스트 기준선을 확정한다. 이번 Phase에서는 코드 구조를 바꾸지 않고 테스트만 보강한다.

## 작업 목록

1. 기존 `assemble.cpp`의 `AssembleSpecTest` 6개 케이스는 그대로 유지 (수정하지 않음).
2. spec.md 제한조건 1·2에 대한 **경계값/조합 케이스**를 보강한다.
   - 제한조건 2의 4개 규칙이 서로 다른 CarType에는 영향을 주지 않는지 교차 검증 (이미 일부 테스트에 포함되어 있으나 전수 조사는 아님).
   - 하나의 조합이 **동시에 여러 규칙**을 위반하는 경우에도 정상적으로 무효 판정되는지 확인 (예: `TRUCK + WIA + MANDO` → 규칙 3, 4 동시 위반).
   - CarType(3) × Engine(3) × BrakeSystem(3) × SteeringSystem(2) = 54가지 전체 조합에 대해 spec.md 규칙 그대로 기대값을 산출해 **전수 검증(Exhaustive Matrix)** 테스트를 추가한다. 이렇게 하면 Phase 1에서 `isValidCombination` 로직을 `ICompatibilityRule` 체인으로 옮길 때, 동작이 1비트라도 달라지면 즉시 실패로 드러난다.
3. CLI 입력 범위 검증(`CarType_Q`: 1~3, `Engine_Q`: 0~4, `brakeSystem_Q`: 0~3, `SteeringSystem_Q`: 0~2, `Run_Test`: 0~2)은 현재 `printf`/`fgets`에 직결되어 자동화 테스트가 불가능하므로(Phase 4에서 `IUserInterface` 도입 후 자동화 예정), 이번 Phase에서는 이 문서에 기대 동작만 기록해 둔다.
   - `CarType_Q`: 1(Sedan), 2(SUV), 3(Truck) 이외 입력은 에러 메시지 후 재입력 요구.
   - `Engine_Q`: 0(뒤로가기), 1~3(정상 제조사), 4(고장난 엔진 — 정상 선택으로 취급되나 RUN 시 "엔진이 고장나있습니다" 출력)이 유효 범위. 5 이상/음수는 에러.
   - `brakeSystem_Q`: 0(뒤로가기), 1~3 정상. 4 이상/음수는 에러.
   - `SteeringSystem_Q`: 0(뒤로가기), 1~2 정상. 3 이상/음수는 에러.
   - `Run_Test`: 0(처음으로), 1(RUN), 2(Test). 3 이상/음수는 에러.
4. 변경 파일: `claude_assemble/assemble.cpp` (테스트 코드만 추가, `#else` 분기의 레거시 로직은 건드리지 않음).

## 테스트 계획

- 기존 6개 `TEST` 유지.
- 신규 `TEST(AssembleSpecTest, MultipleRuleViolationsStillInvalid)`: 여러 규칙을 동시에 위반하는 조합이 무효 처리되는지 확인.
- 신규 `TEST(AssembleSpecTest, ExhaustiveCombinationMatrix)`: 54개 전체 조합에 대해 spec.md 규칙 기반 기대값과 `isValidCombination()` 결과를 비교.
- Debug|x64 빌드 후 실행하여 전체 테스트 PASS 확인 (완료 조건).

## 완료 조건 (docs/all_phase_goals.md 기준)

- `assemble.cpp` Debug 빌드 실행 시 모든 테스트 PASS.
- 이 기준 테스트 세트가 이후 Phase 1부터의 회귀 판단 기준선이 됨.

---

## 실행 결과 (작업 완료 후 갱신)

- 변경 파일: `claude_assemble/assemble.cpp` — `MultipleRuleViolationsStillInvalid`, `ExhaustiveCombinationMatrix` 테스트 추가 (기존 6개 케이스는 무수정).
- 빌드: `MSBuild claude_assemble.slnx -p:Configuration=Debug -p:Platform=x64 -t:Rebuild`
- 실행 결과 (`x64/Debug/claude_assemble.exe`):

```
[==========] Running 8 tests from 1 test suite.
[----------] Global test environment set-up.
[----------] 8 tests from AssembleSpecTest
[ RUN      ] AssembleSpecTest.BoschBrakeRequiresBoschSteering
[       OK ] AssembleSpecTest.BoschBrakeRequiresBoschSteering (0 ms)
[ RUN      ] AssembleSpecTest.SedanCannotUseContinentalBrake
[       OK ] AssembleSpecTest.SedanCannotUseContinentalBrake (0 ms)
[ RUN      ] AssembleSpecTest.SuvCannotUseToyotaEngine
[       OK ] AssembleSpecTest.SuvCannotUseToyotaEngine (0 ms)
[ RUN      ] AssembleSpecTest.TruckCannotUseWiaEngine
[       OK ] AssembleSpecTest.TruckCannotUseWiaEngine (0 ms)
[ RUN      ] AssembleSpecTest.TruckCannotUseMandoBrake
[       OK ] AssembleSpecTest.TruckCannotUseMandoBrake (0 ms)
[ RUN      ] AssembleSpecTest.ValidCombinationsPerCarType
[       OK ] AssembleSpecTest.ValidCombinationsPerCarType (0 ms)
[ RUN      ] AssembleSpecTest.MultipleRuleViolationsStillInvalid
[       OK ] AssembleSpecTest.MultipleRuleViolationsStillInvalid (0 ms)
[ RUN      ] AssembleSpecTest.ExhaustiveCombinationMatrix
[       OK ] AssembleSpecTest.ExhaustiveCombinationMatrix (0 ms)
[----------] 8 tests from AssembleSpecTest (0 ms total)

[----------] Global test environment tear-down
[==========] 8 tests from 1 test suite ran. (1 ms total)
[  PASSED  ] 8 tests.
```

- 완료 조건 충족: 전체 8개 테스트 PASS. 이 세트를 Phase 1부터의 회귀 판단 기준선으로 확정한다.
