# S_NpcShopBuyReply (39)

### **NPC 상점 구매 응답**

- 설명: 서버가 아이템 구매 요청의 결과를 전송한다. (WIP)
- 입력: `C_NpcShopBuyRequest` 처리 후
- 처리 로직: 미구현
- 출력: `S_NpcShopBuyReply`
  ```protobuf
  message S_NpcShopBuyReply {
      bool success = 1;
      string detail = 2;
  }
  ```
- 데이터 구조: 없음

### 관련 파일
- `GameServer/ClientPacketHandler.cpp`
