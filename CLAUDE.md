# CLAUDE.md

이 파일은 `claude_assemble` 저장소에서 작업할 때 참고해야 할 개발 목표, 구조적 설계, 문서 위치, 개발 규칙을 정의한다.

## 개발 목표

`claude_assemble/assemble.cpp`에 절차지향식으로 구현된 자동차 조립(가상 CLI) 레거시 코드를, GoF 디자인 패턴 기반의 구조적 설계로 리팩토링한다. 목표는 다음 4가지 문제를 해소하는 것이다.

1. 절차지향식 코드로 유지보수가 어려운 구조 → 계층 분리 및 State/Builder 패턴 적용
2. 안전하지 않은 문법 사용 (원시 배열 인덱싱, 수동 문자열 파싱 등) → 값 타입/`std::optional` 등 안전한 구조로 대체
3. 확장성이 고려되지 않음 (제조사·차량 타입 추가 시 기존 코드 직접 수정 필요) → Factory/Catalog로 개방-폐쇄 원칙 적용
4. 유닛테스트가 없음 (`printf`/`fgets`가 로직에 직결) → 입출력 추상화로 gmock 기반 테스트 가능하게 함

## 구조적 설계 개요

리팩토링 후 목표 아키텍처는 다음과 같은 계층으로 구성된다 (상세는 `docs/refactoring_structure.md` 참고).

- **Presentation (CLI)**: `CarWizard` — **State 패턴**으로 화면 흐름(타입 선택 → 부품 선택 → 결과) 관리
- **Application (Build)**: `CarBuilder` — **Builder 패턴**으로 차량 조립 절차 수행
- **Domain / Parts**: `CarType`, `Engine`, `BrakeSystem`, `SteeringSystem`, `PartCatalog` — **Factory Method**로 제조사/타입 확장 대응
- **Rule Engine**: `ICompatibilityRule` 구현체 5종 + `CompatibilityChecker` — **Chain of Responsibility**로 완성 가능 조합(제한조건 1·2) 검증
- **Command**: `RunCarCommand`, `TestCarCommand` — **Command 패턴**으로 RUN/Test 동작 캡슐화 (내부에서 `CompatibilityChecker` 재사용)
- **UI 추상화**: `IUserInterface` / `ConsoleUserInterface` / `MockUserInterface` — **Dependency Inversion**으로 입출력을 인터페이스화해 gmock 테스트 가능하게 함

## 참고 문서 위치

- **요구사항/spec**: `docs/spec.md` — 원본 슬라이드(`docs/[CRA_AI] Day2_1_Agentic Engineering.pdf` 20~24페이지)에서 도출한 자동차 조립 규칙(차량 타입, 부품-제조사 매핑, 제한조건 1·2, 레거시 시스템 문제점).
- **구조 설계 상세**: `docs/refactoring_structure.md` — 위 아키텍처의 클래스 설계, 디렉터리 구조안, 5단계 마이그레이션 로드맵.
- **Phase별 목표**: `docs/all_phase_goals.md` — Phase 0~5 각 단계의 목표, 작업 항목, 완료 조건 정의.
- **PR 템플릿**: `docs/pull_request_form.md` — PR 브랜치/제목 규칙과 PR 본문 템플릿.

작업 전 반드시 위 문서를 먼저 확인하고, 요구사항이나 설계와 어긋나는 구현을 하지 않는다.

## 개발 규칙

1. **Phase 순서를 지킨다.** `docs/all_phase_goals.md`에 정의된 Phase 0 → 5 순서대로 진행하며, 이전 Phase의 완료 조건(테스트 PASS)을 만족하기 전에는 다음 Phase로 넘어가지 않는다.
2. **구현 전에 반드시 계획 문서를 먼저 작성한다.** 각 Phase를 구현하기 시작하기 전, `docs_temp/phase_{단계}.md` 파일(예: `docs_temp/phase_1.md`)에 해당 Phase에서 무엇을 어떻게 할지(작업 목록, 변경/신규 파일, 테스트 계획)를 먼저 작성한 뒤에 코드를 작성한다. 계획 문서 없이 바로 구현을 시작하지 않는다.
3. **`docs_temp/phase_{단계}.md`는 해당 Phase 전용 작업 계획 문서**이며, `docs/all_phase_goals.md`의 해당 Phase 목표를 더 구체화한 실행 계획을 담는다. Phase가 끝나면 그 내용을 갱신해 실제로 무엇을 했는지 반영한다.
4. **Phase가 끝날 때마다 반드시 유닛 테스트를 빌드·실행해서 확인한다.** 1개 Phase의 구현이 끝나면 코드 작성을 마쳤다고 끝내지 말고, Debug 빌드를 실행해 gmock 유닛 테스트 결과를 직접 확인한다. 테스트가 전부 PASS된 것을 확인한 뒤에야 해당 Phase를 완료로 간주하고 다음 Phase로 넘어간다. 실패가 있으면 원인을 고치고 재실행해 PASS를 확인할 때까지 다음 Phase를 시작하지 않는다.
5. **회귀 테스트를 유지한다.** 리팩토링 도중 기존 gmock 테스트(`AssembleSpecTest` 및 이후 Phase에서 추가되는 테스트)가 깨지면, 해당 Phase 작업을 완료로 간주하지 않는다.
6. **spec.md의 규칙을 임의로 변경하지 않는다.** 제한조건 1·2, 부품-제조사 매핑, 차량 타입 목록 등은 `docs/spec.md`를 기준으로 하며, 구현 중 spec과 다른 동작이 필요하다고 판단되면 코드를 먼저 바꾸지 말고 사용자에게 확인한다.
7. **PR은 반드시 `docs/pull_request_form.md`의 템플릿을 따른다.** Phase 작업이 끝나 PR을 올릴 때는 해당 문서의 브랜치명 규칙(`feature/phase{단계}`), 1 PR = 1 Phase 원칙, PR 제목 형식, PR 본문 템플릿(목표/변경 사항/참고 문서/테스트 결과/체크리스트)을 그대로 사용한다.
