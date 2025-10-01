# S_NpcInteractReply (36)

### **NPC 상호작용 응답**

- 설명: 서버가 NPC 상호작용 요청에 대한 응답을 전송한다. (WIP)
- 입력: `C_NpcInteractRequest` 처리 후
- 처리 로직: 미구현
- 출력: `S_NpcInteractReply`
  ```protobuf
  message S_NpcInteractReply {
      repeated string dialogs = 1;       // 대화 텍스트 목록
      int32 interactionType = 2;          // 0=Talk, 1=Shop, 2=Quest
  }
  ```
- 데이터 구조: NPC 데이터 (미구현)

### 현재 상태
기본 구조만 정의, 실제 로직 미구현

### 관련 파일
- `GameServer/ClientPacketHandler.cpp`
