# S_BroadcastPlayerChat (47)

### **채팅 메시지 브로드캐스트**

- 설명: 서버가 Room Tick마다 모아둔 채팅 메시지들을 배치로 브로드캐스트한다.
- 입력: Room Tick 처리 시
- 처리 로직:
  - Room의 `_chatQueue`에서 모든 채팅 수집
  - playerId 필드 설정 (서버에서 추가)
  - 채팅 타입에 따라 브로드캐스트 범위 결정
  - S_BroadcastPlayerChat 패킷 전송
  - 큐 비우기
- 출력: `S_BroadcastPlayerChat`
  ```protobuf
  message S_BroadcastPlayerChat {
      repeated PlayerChatInfo playerChatInfos = 1;
  }
  ```
- 데이터 구조:
  - `Room::_chatQueue`: deque<Protocol::PlayerChatInfo>

### 배치 전송의 장점
- 여러 채팅을 한 번에 전송하여 네트워크 오버헤드 감소
- Room Tick 주기에 맞춰 안정적으로 전송
- 채팅 순서 보장

### 브로드캐스트 범위
- **CHAT_ROOM**: 현재 룸의 플레이어들에게만
- **CHAT_ALL**: 전체 룸의 플레이어들에게 (`RoomManager::DoAsyncForAllRooms()`)

### 관련 파일
- `GameServer/Room.cpp`

### 클라이언트 처리
받은 채팅 목록을 순서대로 UI에 표시
