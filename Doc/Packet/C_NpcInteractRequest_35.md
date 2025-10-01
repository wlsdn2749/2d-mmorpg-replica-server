# C_NpcInteractRequest (35)

### **NPC 상호작용 요청**

- 설명: 클라이언트가 NPC와 상호작용을 시작하는 요청을 전송한다. (현재 기본 구조만 구현, 실제 동작 로직 미완성)
- 입력: `C_NpcInteractRequest`
  ```protobuf
  message C_NpcInteractRequest {
      // 빈 메시지 (향후 npcId 추가 예정)
  }
  ```
- 처리 로직:
  - NPC 시스템 기본 구조만 존재
  - Component-based NPC 시스템 설계 (Shop, Dialog, Quest)
  - 실제 동작 로직 미구현
- 출력: `S_NpcInteractReply` (36번)
- 데이터 구조:
  - `NPC`: NPC Entity (WIP)
  - `ShopComponent`, `DialogComponent`, `QuestComponent` 인터페이스 설계

### 현재 상태
- 기본 인터페이스만 구현
- 실제 NPC 인터랙션 로직 미완성
- 향후 구현 예정

### 관련 파일
- `GameServer/ClientPacketHandler.cpp` (구현 stub)
