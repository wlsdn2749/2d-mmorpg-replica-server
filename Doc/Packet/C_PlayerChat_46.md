# C_PlayerChat (46)

### **플레이어 채팅 메시지 전송**

- 설명: 클라이언트가 채팅 메시지를 서버에 전송한다. 룸 채팅과 전체 채팅 지원.
- 입력: `C_PlayerChat`
  ```protobuf
  message C_PlayerChat {
      PlayerChatInfo playerChatInfo = 1;
  }

  message PlayerChatInfo {
      oneof playerIncluded {
          bool NonePlayer = 1;     // 클라 → 서버 전송 시 사용
          int32 playerId = 2;      // 서버 → 클라 전송 시 사용
      }
      string playerName = 3;
      string message = 4;
      EChatType chatType = 5;
  }

  enum EChatType {
      CHAT_ROOM = 0;   // 현재 룸의 플레이어들에게만
      CHAT_ALL = 1;    // 모든 룸의 플레이어들에게
  }
  ```
- 처리 로직:
  - 세션 및 플레이어 검증
  - 채팅 타입 확인 (ROOM / ALL)
  - Room의 채팅 큐에 추가 (`Room::_chatQueue`)
  - Room Tick마다 모아둔 채팅을 배치 전송 (S_BroadcastPlayerChat)
- 출력: `S_BroadcastPlayerChat` (47번)
- 데이터 구조:
  - `Room::_chatQueue`: deque<Protocol::PlayerChatInfo>

### 채팅 타입
- **CHAT_ROOM**: 같은 룸에 있는 플레이어들에게만 전송
- **CHAT_ALL**: RoomManager::DoAsyncForAllRooms()로 전체 룸 브로드캐스트

### 배치 전송
- Room Tick마다 큐에 쌓인 채팅을 한 번에 전송
- 네트워크 효율성 향상

### 관련 파일
- `GameServer/ClientPacketHandler.cpp`
- `GameServer/Room.cpp`

### 클라이언트 처리
채팅 UI에 메시지 표시
