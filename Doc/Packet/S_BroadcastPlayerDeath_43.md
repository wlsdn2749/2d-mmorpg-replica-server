# S_BroadcastPlayerDeath (43)

### **플레이어 사망 브로드캐스트**

- 설명: 서버가 플레이어의 사망을 룸의 모든 플레이어에게 브로드캐스트한다.
- 입력: 플레이어 HP가 0 이하로 떨어졌을 때
- 처리 로직:
  - 플레이어 사망 상태 설정
  - 사망 브로드캐스트 전송
  - 사망한 플레이어는 C_PlayerDeathReady (44번)로 리스폰 요청 대기
- 출력: `S_BroadcastPlayerDeath`
  ```protobuf
  message S_BroadcastPlayerDeath {
      int32 mapId = 1;
      int32 playerId = 2;
      int32 killerMonsterId = 3;  // 죽인 몬스터 ID
  }
  ```
- 데이터 구조:
  - `Player::_isDead`: 사망 상태 플래그

### 사망 플로우
1. HP 0 → `S_BroadcastPlayerDeath` 전송
2. 클라이언트 사망 애니메이션 재생
3. 클라이언트 `C_PlayerDeathReady` (44번) 전송
4. 서버 `S_PlayerDeathCommit` (45번) + `S_ChangeRoomCommit` (20번) 전송
5. 스폰 지역으로 이동, HP 초기화

### 관련 파일
- `GameServer/PlayerCombatSystem.cpp`
- `GameServer/MonsterCombatSystem.cpp`

### 클라이언트 처리
사망 애니메이션 재생, 리스폰 UI 표시
