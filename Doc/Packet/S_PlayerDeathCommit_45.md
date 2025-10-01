# S_PlayerDeathCommit (45)

### **플레이어 리스폰 승인**

- 설명: 서버가 플레이어의 리스폰 요청을 승인하고 이동할 맵 정보를 전송한다.
- 입력: `C_PlayerDeathReady` 처리 완료 후
- 처리 로직:
  - 리스폰 처리 완료
  - 이동할 맵 ID 전송
- 출력: `S_PlayerDeathCommit`
  ```protobuf
  message S_PlayerDeathCommit {
      int32 mapId = 1;  // 리스폰될 맵 ID
  }
  ```
- 데이터 구조: 없음

### 패킷 순서
1. `S_BroadcastPlayerDeath` (43번): 사망 알림
2. `C_PlayerDeathReady` (44번): 클라이언트 준비 완료
3. `S_PlayerDeathCommit` (45번): 서버 승인
4. `S_ChangeRoomCommit` (20번): 실제 룸 전환

### 관련 파일
- `GameServer/ClientPacketHandler.cpp`
