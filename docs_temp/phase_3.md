# Phase 3 — 부품 카탈로그 도입 (Factory / Catalog)

참고: `docs/all_phase_goals.md` Phase 3, `docs/refactoring_structure.md` 3.2절, `docs/spec.md`

## 브랜치 안내

origin에 `feature/phase3`가 아직 없어, PR #3(`feature/phase2`)이 아직 main에 머지되지 않은 상태이므로 `feature/phase2` 위에 `feature/phase3`를 새로 만들어 진행한다 (Phase 3는 Phase 2의 `Car`/`CarType`/`Engine`/`BrakeSystem`/`SteeringSystem` enum class에 의존).

## 목표

제조사·차량 타입이 코드 곳곳(`if(answer==N)` 나열)에 흩어져 있는 구조를, 한 곳에서 관리하는 카탈로그(`PartCatalog`, `CarTypeCatalog`)로 대체해 확장성(OCP)을 확보한다. 레거시 CLI(`#else` 분기)의 출력 문자열은 이번 Phase에서 하드코딩 대신 카탈로그 기반으로 생성하도록 변경한다(요구사항에 명시된 유일한 레거시 코드 변경 지점). 단, 출력 텍스트/동작은 기존과 완전히 동일해야 한다.

## 작업 목록

1. `claude_assemble/domain/PartCatalog.h` 신규 작성
   - `availableEngines()`, `availableBrakeSystems()`, `availableSteeringSystems()` — 각각 `const std::vector<Engine>&` 등을 반환.
   - `nameOf(Engine)`, `nameOf(BrakeSystem)`, `nameOf(SteeringSystem)` — 표시 이름 매핑 ("GM", "TOYOTA", "WIA" 등).
2. `claude_assemble/domain/CarTypeCatalog.h` 신규 작성
   - `availableCarTypes()` — `const std::vector<CarType>&`.
   - `nameOf(CarType)` — "Sedan"/"SUV"/"Truck".
3. `claude_assemble/assemble.cpp`의 `#else`(레거시 CLI) 분기 수정
   - 상단에 `#include "domain/CarTypeCatalog.h"`, `#include "domain/PartCatalog.h"` 추가.
   - `main()`의 각 메뉴 출력을 카탈로그 순회로 교체:
     - `CarType_Q`: `CarTypeCatalog::availableCarTypes()`를 순회해 `"N. 이름\n"` 출력 (기존 "1. Sedan/2. SUV/3. Truck"과 동일 텍스트).
     - `Engine_Q`: `PartCatalog::availableEngines()` 순회 + 마지막에 카탈로그와 무관한 "4. 고장난 엔진" 고정 출력 유지 (스펙상 제조사가 아닌 고장 상태이므로 카탈로그 대상이 아님).
     - `brakeSystem_Q`, `SteeringSystem_Q`: 각각 카탈로그 순회로 교체.
   - `selectCarType`/`selectEngine`/`selectbrakeSystem`/`selectSteeringSystem`의 선택 확인 메시지("N 엔진을 선택하셨습니다" 등)를 `if(answer==N) printf(...)` 나열 대신 카탈로그 `nameOf()` 기반 문자열로 교체 (기존 텍스트가 카탈로그 표기와 정확히 일치함을 확인 후 교체: CarType은 "Sedan/SUV/Truck", Engine/BrakeSystem/SteeringSystem은 모두 대문자 표기로 일치).
   - `runProducedCar()`의 "Car Type : Sedan"/"Engine : GM" 등 결과 요약 출력은 **변경하지 않음**. 원본이 "Brake System : Mando"/"SteeringSystem : Bosch"처럼 제동·조향장치명을 Title-case로 하드코딩해 두어(메뉴·선택 확인 메시지의 대문자 표기와 다름), 카탈로그(`nameOf()`가 항상 대문자 반환)로 교체하면 출력 텍스트가 바뀌어버린다. spec.md 규칙/출력 텍스트를 임의로 바꾸지 않는다는 원칙(CLAUDE.md 규칙 6)에 따라 이 부분은 이번 Phase 범위에서 제외한다.
   - `isValidCheck()`, `testProducedCar()`, `stack[10]` 구조, 입력 파싱/범위 검증 로직은 이번 Phase에서 변경하지 않음(Phase 4~5에서 다룸).
4. `claude_assemble.vcxproj`/`.vcxproj.filters`에 신규 헤더 2종(`domain/PartCatalog.h`, `domain/CarTypeCatalog.h`) `ClInclude` 등록.
5. `claude_assemble/assemble.cpp`의 `_DEBUG` 테스트 섹션에 신규 테스트 추가
   - `#include "domain/PartCatalog.h"`, `#include "domain/CarTypeCatalog.h"` 추가.
   - `PartCatalogTest`: `availableEngines()`/`availableBrakeSystems()`/`availableSteeringSystems()` 개수·순서, `nameOf()` 매핑 검증.
   - `CarTypeCatalogTest`: `availableCarTypes()` 개수·순서, `nameOf()` 매핑 검증.
6. **확장성 검증 실험** (완료 조건의 핵심): `PartCatalog.h`의 `availableEngines()`에 임시로 더미 엔진 1개(`static_cast<Engine>(99)`, 이름 "DUMMY")를 추가해:
   - `PartCatalog.h` 한 파일만 수정하면 되는지 확인 (`Car.h`의 enum에 새 열거자를 추가하지 않아도, enum class는 명명된 열거자 외 임의의 int 값을 담을 수 있으므로 카탈로그 목록/이름 매핑만으로 신규 항목이 반영됨).
   - `CompatibilityChecker`/`CarBuilder`/CLI 메뉴 출력 로직 등 다른 파일은 전혀 수정하지 않고도 빌드·테스트가 정상 동작하는지 확인.
   - 확인 후 더미 엔진 추가분을 되돌려 실제 제품 코드에는 반영하지 않는다.
   - 실험 과정과 결과는 이 문서 "실행 결과" 절에 기록한다 (커밋에는 되돌린 상태만 반영).

## 테스트 계획

- 기존 8개 `AssembleSpecTest` + 7개 `CompatibilityCheckerTest` + 3개 `CarBuilderTest` = 18개 무수정 유지 (회귀 확인).
- 신규 `PartCatalogTest`(3개: engine/brake/steering) + `CarTypeCatalogTest`(1개) = 4개 추가.
- Debug|x64 빌드 후 실행하여 전체 테스트(18 + 4 = 22개) PASS 확인.
- 레거시 CLI는 자동화 테스트 대상이 아니므로(Phase 4 이후), 코드 리딩으로 출력 텍스트가 기존과 동일한지 확인.

## 완료 조건 (docs/all_phase_goals.md 기준)

- 카탈로그 기반 목록 조회 단위 테스트 PASS.
- 더미 제조사 추가/제거 실험에서 `PartCatalog` 외 다른 파일 수정이 필요 없었음을 확인.

---

## 실행 결과 (작업 완료 후 갱신)

- 신규 파일: `claude_assemble/domain/PartCatalog.h`(Engine/BrakeSystem/SteeringSystem 목록·이름), `claude_assemble/domain/CarTypeCatalog.h`(CarType 목록·이름)
- 변경 파일:
  - `claude_assemble/assemble.cpp` `#else`(레거시 CLI) 분기: 각 메뉴(`CarType_Q`/`Engine_Q`/`brakeSystem_Q`/`SteeringSystem_Q`) 출력과 `selectCarType`/`selectEngine`/`selectbrakeSystem`/`selectSteeringSystem`의 선택 확인 메시지를 카탈로그 순회/`nameOf()` 기반으로 교체. 출력 텍스트는 기존과 100% 동일함을 확인 후 교체(선택 확인 메시지 casing이 카탈로그 표기와 정확히 일치).
  - `runProducedCar()`/`testProducedCar()`/`isValidCheck()`/`stack[10]`은 **무수정**. `runProducedCar()`의 "Brake System : Mando"/"SteeringSystem : Bosch"는 원본이 Title-case로 하드코딩돼 있어(메뉴·선택 확인 메시지의 대문자 표기와 불일치), 카탈로그로 교체하면 출력 텍스트가 바뀌므로 이번 Phase 범위에서 제외함(spec.md/출력 텍스트 임의 변경 금지 원칙).
  - `claude_assemble/assemble.cpp` `_DEBUG` 섹션: `PartCatalogTest` 3개 + `CarTypeCatalogTest` 1개 추가. 기존 18개는 무수정.
  - `claude_assemble.vcxproj`/`.filters`: `domain/PartCatalog.h`, `domain/CarTypeCatalog.h` `ClInclude` 등록
- 빌드: `MSBuild claude_assemble.slnx -p:Configuration=Debug -p:Platform=x64 -t:Rebuild` → 성공 (경고 8개는 기존 EUC-KR 인코딩 C4819, 오류 0개)
- 실행 결과 (`x64/Debug/claude_assemble.exe`): `[==========] Running 22 tests from 5 test suites.` ... `[  PASSED  ] 22 tests.` (전체 PASS)

### 확장성 검증 실험 (완료 조건)

`PartCatalog.h`의 `availableEngines()`에 `static_cast<Engine>(99)`(이름 "DUMMY")를 임시로 추가하고 재빌드·재실행:

- **수정한 파일: `PartCatalog.h` 1개뿐.** `domain/Car.h`의 `Engine` enum에는 새 열거자를 추가하지 않았음(enum class가 명명되지 않은 int 값도 담을 수 있어 가능). `rules/`, `build/`, CLI 메뉴 출력 로직 등 다른 파일은 전혀 수정하지 않음.
- 빌드: 오류 0개로 정상 컴파일.
- 테스트: 22개 중 21개 PASS, 1개(`PartCatalogTest.AvailableEnginesMatchSpec`)만 실패 — 실패 이유는 "엔진 목록이 3개여야 한다"는 고정 스펙 검증용 assertion(`ASSERT_EQ(engines.size(), 3u)`)이 더미 추가로 4개가 되어 발생한 것으로, **카탈로그 메커니즘 자체는 정상 동작**함을 확인(`CompatibilityChecker`/`CarBuilder`/CLI 메뉴 출력 등 나머지는 모두 정상).
- 실험 완료 후 `PartCatalog.h`의 더미 엔진 추가분을 되돌리고 재빌드·재실행하여 22개 전체 PASS를 재확인함. 실제 제품 코드에는 더미가 반영되지 않았음.
- 완료 조건 충족: 카탈로그 기반 목록 조회 단위 테스트 PASS, 더미 제조사 추가/제거 실험에서 `PartCatalog` 외 다른 파일 수정이 필요 없었음을 확인.
