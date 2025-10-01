# C_GiveItemRequest (48)

### **아이템 지급 테스트 요청**

- 설명: 클라이언트가 테스트 목적으로 아이템 지급을 요청한다. 개발/디버깅 용도.
- 입력: `C_GiveItemRequest`
  ```protobuf
  message C_GiveItemRequest {
      int32 itemId = 1;
      int32 count = 2;
  }
  ```
- 처리 로직:
  - 세션 InRoom 상태 검증
  - 플레이어 인벤토리 조회
  - `InventorySystem::TryAddItem(itemId, count)` 호출
  - 인벤토리에 아이템 추가
  - DB 업데이트
  - 클라이언트에게 결과 전송
- 출력: `S_GiveItemReply` (49번)
  - `success`: 지급 성공 여부
  - `errorMessage`: 실패 시 에러 메시지
  - `addedSlot`: 추가된 슬롯 정보
- 데이터 구조:
  - `Inventory`: 플레이어 인벤토리
  - `InventorySlot`: 슬롯 정보
  - `GameDB.Inventory` 테이블

### 주의사항
- **테스트 전용**: 실제 게임에서는 비활성화 필요
- 인벤토리 가득 찬 경우 실패
- 잘못된 itemId인 경우 실패

### 관련 파일
- `GameServer/ClientPacketHandler.cpp`
- `GameServer/InventorySystem.cpp`

### 테스트 시나리오
DummyClient에서 아이템 ID와 수량을 입력하여 인벤토리에 아이템 추가 테스트
