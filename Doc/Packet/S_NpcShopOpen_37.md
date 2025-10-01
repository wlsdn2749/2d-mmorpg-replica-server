# S_NpcShopOpen (37)

### **NPC 상점 오픈**

- 설명: 서버가 클라이언트에게 NPC 상점 정보를 전송한다. (WIP)
- 입력: NPC Shop 인터랙션 시
- 처리 로직: 미구현
- 출력: `S_NpcShopOpen`
  ```protobuf
  message S_NpcShopOpen {
      int32 npcId = 1;
      repeated ShopItemInfo items = 2;
  }

  message ShopItemInfo {
      int32 itemId = 1;
      int32 quantity = 2;   // 수량
      int32 price = 3;      // 1개당 가격
  }
  ```
- 데이터 구조: NPC Shop 데이터 (미구현)

### 관련 파일
- `GameServer/ClientPacketHandler.cpp`
