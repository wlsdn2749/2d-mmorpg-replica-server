# S_PlayerStat (40)

### **플레이어 스탯 정보 전송**

- 설명: 서버가 플레이어의 현재 스탯 정보를 클라이언트에게 전송한다.
- 입력: 플레이어 게임 입장 시, 스탯 변경 시
- 처리 로직:
  - 플레이어의 스탯 정보 수집
  - PlayerStatInfo 패킷 생성 및 전송
- 출력: `S_PlayerStat`
  ```protobuf
  message S_PlayerStat {
      PlayerStatInfo statInfo = 1;
  }

  message PlayerStatInfo {
      int32 maxHp = 1;
      int32 hp = 2;
      int32 curExp = 3;    // 현재 경험치
      int32 maxExp = 4;    // 레벨업 필요 경험치
      int32 level = 5;
      int32 money = 6;
  }
  ```
- 데이터 구조:
  - `Player::_statInfo`: 플레이어 스탯

### 전송 시점
- 게임 입장 시
- 레벨업 시
- HP 변경 시
- 경험치 획득 시
- 돈 변경 시

### 관련 파일
- `GameServer/Player.cpp`
- `GameServer/Room.cpp`

### 클라이언트 처리
UI에 플레이어 스탯 표시 (HP바, 경험치바, 레벨, 돈)
