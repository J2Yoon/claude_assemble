# Pull Request 템플릿

`claude_assemble` 리팩토링(Phase 0~5)은 Phase 단위로 PR을 나눈다. PR을 올릴 때 아래 형식을 사용한다.

## 브랜치 / PR 규칙

- 브랜치명: `feature/phase{단계}` (예: `feature/phase0`, `feature/phase1`)
- PR 대상 브랜치: `main`
- **1 PR = 1 Phase**를 원칙으로 한다. 여러 Phase를 한 PR에 묶지 않는다.
- PR을 올리기 전에 반드시 다음을 확인한다 (`CLAUDE.md` 개발 규칙 참고).
  1. `docs_temp/phase_{단계}.md`에 해당 Phase 계획이 작성되어 있는가
  2. Debug 빌드로 gmock 유닛 테스트를 실행해 전부 PASS했는가
  3. `docs/spec.md`의 규칙(제한조건 1·2, 부품-제조사 매핑 등)을 임의로 바꾸지 않았는가

---

## PR 제목

```
[Phase {단계}] {한 줄 요약}
```

예: `[Phase 1] Chain of Responsibility로 완성 가능 조합 검증 로직 분리`

## PR 본문 템플릿

```markdown
## Phase

Phase {단계} — {docs/all_phase_goals.md 상의 Phase 이름}

## 목표

<!-- docs/all_phase_goals.md에 정의된 이번 Phase 목표를 1~2문장으로 -->

## 변경 사항

- <!-- 신규/변경 파일과 핵심 변경 내용을 항목별로 나열 -->
-

## 참고 문서

- 작업 계획: `docs_temp/phase_{단계}.md`
- Phase 목표: `docs/all_phase_goals.md` (Phase {단계} 항목)
- 구조 설계: `docs/refactoring_structure.md` ({해당 패턴 섹션})
- 요구사항: `docs/spec.md`

## 테스트 결과

<!-- Debug|x64 빌드 후 실행한 gmock 결과를 그대로 붙여넣기 -->

```
[==========] N tests from 1 test suite ran.
[  PASSED  ] N tests.
```

- [ ] 신규/변경된 테스트가 이번 Phase 목표를 검증하는지 확인함
- [ ] 이전 Phase까지의 테스트가 회귀 없이 모두 PASS함

## 체크리스트

- [ ] `docs_temp/phase_{단계}.md` 계획 문서 작성 및 실제 작업 반영 갱신 완료
- [ ] `docs/all_phase_goals.md`의 해당 Phase 완료 조건 충족
- [ ] spec.md 규칙과 동작이 일치함 (임의 변경 없음)
- [ ] 다음 Phase 진행 전 리뷰/머지 필요
```

---

## 예시 (Phase 0)

```markdown
## Phase

Phase 0 — 안전망 구축 (베이스라인 테스트)

## 목표

리팩토링 도중 회귀를 감지할 수 있도록 spec.md 제한조건 1·2 기준 베이스라인 gmock 테스트를 확보한다.

## 변경 사항

- claude_assemble/assemble.cpp: AssembleSpecTest에 경계값 케이스 보강

## 참고 문서

- 작업 계획: docs_temp/phase_0.md
- Phase 목표: docs/all_phase_goals.md (Phase 0)
- 요구사항: docs/spec.md

## 테스트 결과

[==========] 6 tests from 1 test suite ran.
[  PASSED  ] 6 tests.

- [x] 신규/변경된 테스트가 이번 Phase 목표를 검증하는지 확인함
- [x] 이전 Phase까지의 테스트가 회귀 없이 모두 PASS함

## 체크리스트

- [x] docs_temp/phase_0.md 계획 문서 작성 및 실제 작업 반영 갱신 완료
- [x] docs/all_phase_goals.md의 해당 Phase 완료 조건 충족
- [x] spec.md 규칙과 동작이 일치함 (임의 변경 없음)
- [x] 다음 Phase 진행 전 리뷰/머지 필요
```
