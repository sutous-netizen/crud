# 데이터 영속성(DataPersistence) PoC 설계

## 배경

PDF `[CRA_AI] Day3_개인과제_반도체시료관리.pdf` 미션1 "PoC 개발"의 필요 항목 중 "데이터 영속성 처리"는 다음을 요구한다.

> 팀별로 선택한 방식(파일, JSON, DB 등)으로 데이터를 저장·불러오는 구조 구현, CRUD 포함

이 저장소(`crud`)는 이미 `json::Value` 기반 JSON 파일 저장소(`JsonRecordStore`)와 이를 사용하는 CRUD 콘솔 앱(`ConsoleApp`)을 갖추고 있어 요건을 구조적으로 충족한다. 다만 영속성(재시작해도 데이터가 유지되는 성질)을 직접 확인할 수 있는 수단이 대화형 메뉴 조작뿐이라, 이를 스크립트로 자동 시연하는 데모와 이를 설명하는 문서가 필요하다.

## 범위

- 기존 `IRecordStore` / `JsonRecordStore` / `ConsoleApp` 구조는 변경하지 않는다.
- `crud.exe --demo` 실행 모드를 추가하여 영속성 동작을 비대화형으로 시연한다.
- `docs/DataPersistence.md`에 기능 요약과 제약사항을 정리한다.

## 데모 모드 (`crud.exe --demo`)

`main.cpp`에서 `--demo` 인자를 인식해 `RunDataPersistenceDemo()`(가칭)를 호출한다. 이 함수는 전용 데이터 파일(`demo_data.json`, 실행마다 초기화)을 사용해 다음 순서로 콘솔에 단계 설명과 함께 출력한다.

1. `JsonRecordStore` 인스턴스 생성 (파일 없음 → 빈 저장소로 시작함을 보여줌)
2. 레코드 2~3건 Create → `All()`로 즉시 조회해 메모리 상태 확인
3. **재시작 시뮬레이션**: 같은 파일 경로로 새로운 `JsonRecordStore` 인스턴스를 생성 → `All()` 결과가 앞서 생성한 레코드와 동일함을 출력하여 "프로세스가 종료되어도 파일을 통해 데이터가 유지된다"는 영속성을 증명
4. 새 인스턴스에서 Update 1건, Delete 1건 수행 → `All()`로 반영 결과 확인
5. 저장된 JSON 파일의 raw 텍스트를 그대로 출력하여 실제 디스크 상태를 노출

인자 없이 실행 시 기존 대화형 앱, `--test` 는 기존 테스트 실행을 그대로 유지한다.

## 문서 (`docs/DataPersistence.md`)

다음 섹션으로 구성한다.

- **개요**: 영속성 계층의 목적과 위치(`IRecordStore`, `JsonRecordStore`, `ConsoleApp`)
- **CRUD 매핑**: Create/Read/Update/Delete가 각각 어떤 파일 함수로 이어지는지
- **저장 방식**: 매 변경(Create/Update/Delete) 직후 전체 컬렉션을 파일에 재직렬화하는 write-through 방식 설명
- **데모 실행 방법**: `crud.exe --demo` 사용법과 예상 출력 설명
- **제약사항**:
  - 트랜잭션/동시성 미지원 (단일 프로세스, 단일 파일 가정)
  - 매 변경마다 전체 컬렉션을 재직렬화하므로 레코드 수가 많아지면 비효율적
  - 스키마 검증 없음 (모든 필드는 문자열 `key=value`로 저장)
  - 손상된 항목/파일은 방어적으로 스킵하거나 빈 저장소로 시작 (예외로 앱이 죽지 않음, 대신 손상 사실을 사용자에게 알리지 않음)
  - 백업/버전 관리 없음 (덮어쓰기 방식)

## 커밋 및 제출

문서와 데모 코드를 하나의 커밋으로 정리하고 `origin/main`에 push한다.
