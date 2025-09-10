# NPC 시스템 개발 Task List

## 시스템 구성

- [ ] `NPC.h / NPC.cpp` 작성 (기본 NPC 클래스 정의)
- [ ] `NPCTypes.h` 작성 (역할 타입, 상태, 설정 enum/const 등 정의)
- [ ] `EntityKind::Npc` 관련 로직 확장 (`EntityManager` 등)

## Component 구현

- [ ] `NPCDialogComponent` 클래스 생성
  - [ ] 대화 데이터 조회 인터페이스 설계
  - [ ] `DialogManager` 연동 기능 추가
  - [ ] 클라이언트 응답 패킷(`S_NPC_DIALOG`) 정의 및 전송 처리

- [ ] `NPCShopComponent` 클래스 생성
  - [ ] 상점 아이템 목록 로딩 기능 구현
  - [ ] `ShopDataManager` 연동
  - [ ] 클라이언트 응답 패킷(`S_NPC_SHOP_OPEN`) 정의 및 전송 처리

- [ ] `NPCQuestComponent` 클래스 생성
  - [ ] 퀘스트 목록 전달 기능 구현
  - [ ] 클라이언트 응답 패킷(`S_NPC_QUEST_LIST`) 정의 및 전송 처리

## NPC 역할 처리

- [ ] 역할 정의 구조 설계 (`eNpcRole` 또는 `NpcRoleId` 등)
- [ ] 역할별 Component 자동 attach 로직 구현
- [ ] `NPCManager`에 NPC 등록 시 역할 처리 로직 포함

## NPC 초기화 및 매니저

- [ ] `NPCManager` 생성 및 전역 관리 구조 설계
  - [ ] 맵 단위로 NPC 로딩 및 초기화 처리
  - [ ] `EntityCore` 생성 및 등록 처리 포함

- [ ] 맵별 NPC 배치 데이터 정의 (코드/리소스 기반)
  - [ ] NPC의 위치, 방향, 역할 등 정의 포함

## 상호작용 처리

- [ ] 클라이언트 요청 패킷 `C_NPC_INTERACT` 구조 정의 (좌표 기반)
- [ ] 좌표 기반 NPC 탐색 로직 구현
- [ ] NPC와 상호작용 처리 루틴 구현
  - [ ] Component 별 실행 분기 처리
- [ ] 잘못된 좌표일 경우 에러 응답 처리

## 테스트 케이스 작성

- [ ] 정상적인 NPC 상호작용 테스트
- [ ] Component별 응답 처리 정상 확인
- [ ] 잘못된 위치 입력 시 예외 처리 검증
- [ ] 복합 Component NPC 동작 확인
- [ ] 맵 진입 시 NPC 자동 로딩 검증

## DB 관련 항목 (보류)

- [ ] `NPCRepository` 사용 여부 결정
- [ ] DB 기반 NPC 정의 구조 설계
- [ ] DB 쿼리 및 NPC 생성 연동 로직

## 향후 작업 후보 (Backlog)

- [ ] Lua 기반 NPC 스크립트 시스템 도입
- [ ] NPC별 외형/애니메이션 처리 구조 설계
- [ ] 이벤트 기반 상호작용 지원 (예: 퀘스트 자동 시작 등)