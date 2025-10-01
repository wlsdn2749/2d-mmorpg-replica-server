# S_GiveItemReply (49)

### **아이템 지급 결과 응답**

- 설명: 서버가 테스트용 아이템 지급 요청의 결과를 전송한다.
- 입력: `C_GiveItemRequest` 처리 완료 후
- 처리 로직:
  - 아이템 추가 결과 확인
  - 성공 시 추가된 슬롯 정보 포함
- 출력: `S_GiveItemReply`
  ```protobuf
  message S_GiveItemReply {
      bool success = 1;
      string errorMessage = 2;
      InventorySlotInfo addedSlot = 3;
  }

  message InventorySlotInfo {
      int32 slotIndex = 1;
      int32 itemId = 2;
      int32 count = 3;
      bool isQuickslot = 4;
  }
  ```
- 데이터 구조:
  - `InventorySlotInfo`: 추가된 슬롯 정보

### 에러 메시지 종류
- "Inventory is full": 인벤토리 가득 찬
- "Invalid itemId": 잘못된 아이템 ID
- "Failed to add item": 아이템 추가 실패

### 성공 시
- `success = true`
- `addedSlot`: 아이템이 추가된 슬롯의 상세 정보

### 관련 파일
- `GameServer/ClientPacketHandler.cpp`
- `GameServer/InventorySystem.cpp`

### 클라이언트 처리
- 성공: 인벤토리 UI에 아이템 추가 표시
- 실패: 에러 메시지 표시
