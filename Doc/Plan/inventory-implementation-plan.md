# 인벤토리 시스템 구현 계획

## 개요

2D MMORPG용 인벤토리 시스템을 구현하기 위한 상세 계획입니다.

## 구현 완료 항목 ✅

### 1. 데이터베이스 설계 ✅
- **ItemData 테이블**: 아이템 메타데이터 관리
  - item_id, name, description, is_stackable, max_stack, item_type
- **CharacterInventory 테이블**: 플레이어별 인벤토리 슬롯
  - character_id, slot_index, item_id, count, is_quickslot
- **저장 프로시저 3개**: 
  - `spGetCharacterInventory`: 인벤토리 로드
  - `spSaveInventorySlot`: 슬롯 저장/업데이트
  - `spDeleteInventorySlot`: 슬롯 삭제

### 2. 프로토콜 정의 ✅
- **새로운 메시지 5개** 추가:
  - `C_InventoryRequest` (26): 인벤토리 조회 요청
  - `S_InventoryReply` (27): 인벤토리 응답
  - `C_ItemUseRequest` (28): 아이템 사용 요청
  - `S_ItemUseReply` (29): 아이템 사용 응답
  - `S_InventoryUpdate` (30): 인벤토리 변경 브로드캐스트
- **새로운 구조체**: `InventorySlotInfo`, `EItemType` enum

### 3. 서버 메모리 구조 ✅
- **InventoryCore.h**: 공통 구조체 및 상수 정의
  - `ItemSlot`, `ItemData` 구조체
  - `EAddItemResult`, `ERemoveItemResult`, `EUseItemResult` enum
  - 인벤토리 상수 (총 40슬롯: 30개 일반 + 10개 퀵슬롯)

- **InventorySystem 클래스**: 40슬롯 관리 시스템
  - 아이템 추가/제거/사용 로직
  - 스택 처리 및 퀵슬롯 관리
  - 프로토콜 메시지 변환 기능

- **ItemManager 클래스**: 아이템 메타데이터 관리 (싱글톤)
  - 아이템 정보 캐싱
  - 아이템 효과 적용 (HP 회복 등)
  - 전역 접근 함수 제공

- **InventoryRepository 클래스**: DB 연동 레이어
  - 비동기 인벤토리 로드/저장
  - 배치 저장 기능

### 4. Player 클래스 통합 ✅
- `InventorySystem _inventory` 멤버 변수 추가
- 편의 메서드 추가: `AddItem()`, `RemoveItem()`, `UseItem()`
- DB 연동 메서드: `LoadInventoryFromDB()`, `SaveInventoryToDB()`

### 5. 패킷 핸들러 구현 ✅
- **Handle_C_InventoryRequest**: 인벤토리 조회 처리
- **Handle_C_ItemUseRequest**: 아이템 사용 처리
  - 사용 결과에 따른 응답 메시지 분기
  - 성공 시 인벤토리 업데이트 브로드캐스트

### 6. 코드 자동 생성 ✅
- `GenProcs.bat` 실행: DB 저장 프로시저 클래스 생성
- `GenPackets.bat` 실행: 패킷 핸들러 및 매니저 생성

### 7. 테스트 데이터 ✅
- `inventory-test-data.sql`: 샘플 아이템 및 인벤토리 데이터
- 4가지 아이템 타입별 샘플 (소비형, 장비, 퀘스트, 기타)

## 구현 방식 검증

### 1. 아키텍처 타당성 ✅
- **계층 분리**: Repository → Service → Controller 패턴 준수
- **비동기 처리**: DB 작업은 모두 비동기로 처리 (JobQueue 활용)
- **메모리 효율성**: 필요한 경우에만 DB에서 로드, 메모리에서 빠른 접근

### 2. 확장성 ✅
- **슬롯 확장**: 상수 변경으로 슬롯 수 조정 가능
- **새 아이템 타입**: enum 추가로 확장 가능
- **새 아이템 효과**: ItemManager에서 추가 구현 가능

### 3. 성능 고려사항 ✅
- **캐싱**: ItemManager에서 아이템 메타데이터 캐싱
- **배치 처리**: 인벤토리 전체 저장 시 배치 처리
- **스택 최적화**: 동일 아이템 자동 스택 처리

### 4. 동시성 처리 ✅
- **Room 기반**: 모든 인벤토리 작업은 Room의 JobQueue에서 실행
- **Thread-Safe**: DB 접근은 DbDispatcher를 통해 안전하게 처리

### 5. 오류 처리 ✅
- **결과 타입**: 각 작업별 명시적인 결과 enum 반환
- **클라이언트 알림**: 오류 발생 시 사용자에게 명확한 메시지 전송
- **로깅**: 모든 주요 작업에 대한 로그 출력

## 사용 예시

### 아이템 추가
```cpp
EAddItemResult result = player->AddItem(1, 10); // 체력 포션 10개 추가
```

### 아이템 사용
```cpp
EUseItemResult result = player->UseItem(0); // 0번 슬롯 아이템 사용
```

### 인벤토리 DB 저장
```cpp
auto future = player->SaveInventoryToDB(); // 비동기 저장
```

## 테스트 시나리오

1. **기본 기능 테스트**
   - 아이템 추가/제거/사용
   - 스택 기능 테스트
   - 퀵슬롯 등록/사용

2. **DB 연동 테스트**
   - 인벤토리 저장/로드
   - 서버 재시작 후 데이터 복원

3. **네트워크 테스트**
   - 인벤토리 조회 패킷
   - 아이템 사용 패킷
   - 브로드캐스트 테스트

4. **성능 테스트**
   - 대량 아이템 처리
   - 동시 접속자 인벤토리 처리

## 다음 단계 개발 고려사항

### 추가 기능 후보
1. **아이템 이동**: 슬롯 간 아이템 이동
2. **아이템 분할**: 스택 아이템 분할
3. **인벤토리 확장**: 캐시 아이템으로 슬롯 확장
4. **아이템 거래**: 플레이어 간 아이템 교환
5. **아이템 드롭**: 필드에 아이템 드롭/획득
6. **장비 시스템**: 장비 착용/해제 기능

### 최적화 방안
1. **캐싱 개선**: 자주 사용되는 인벤토리 데이터 캐싱
2. **배치 최적화**: 여러 슬롯 동시 업데이트 최적화
3. **압축**: 네트워크 패킷 압축
4. **인덱싱**: DB 쿼리 성능 최적화

## 결론

인벤토리 시스템이 성공적으로 구현되었습니다. 모든 핵심 기능이 완료되었고, 확장성과 성능을 고려한 설계로 향후 추가 기능 개발이 용이합니다.