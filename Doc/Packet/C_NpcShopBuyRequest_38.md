# C_NpcShopBuyRequest (38)

### **NPC 상점 아이템 구매 요청**

- 설명: 클라이언트가 NPC 상점에서 아이템을 구매하는 요청을 전송한다. (WIP)
- 입력: `C_NpcShopBuyRequest`
  ```protobuf
  message C_NpcShopBuyRequest {
      int32 npcId = 1;
      int32 itemId = 2;
      int32 quantity = 3;
  }
  ```
- 처리 로직: 미구현
- 출력: `S_NpcShopBuyReply` (39번)
- 데이터 구조: NPC Shop, Player Money (미구현)

### 관련 파일
- `GameServer/ClientPacketHandler.cpp`
