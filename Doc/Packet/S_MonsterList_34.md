# S_MonsterList (34)

### **몬스터 리스트 스냅샷**

- 설명: 서버가 클라이언트에게 현재 맵의 모든 몬스터 정보를 한 번에 전송한다. 주로 룸 입장 시 초기 스냅샷으로 사용.
- 입력: 플레이어 룸 입장 시 (`S_EnterGame` 이후)
- 처리 로직:
  - Room의 모든 몬스터 조회
  - 각 몬스터의 정보 수집 (monsterId, monsterTypeId, 위치, 방향)
  - S_MonsterList 패킷 생성 및 전송
- 출력: `S_MonsterList`
  ```protobuf
  message S_MonsterList {
      int32 mapId = 1;
      repeated MonsterInfo monsters = 2;
  }

  message MonsterInfo {
      int32 monsterId = 1;        // Entity 고유 ID
      int32 monsterTypeId = 2;    // 몬스터 타입 (Template ID)
      Vector2Info pos = 3;         // 위치
      EDirection direction = 4;    // 방향
  }
  ```
- 데이터 구조:
  - `Room::_monsters`: 룸의 몬스터 컨테이너
  - `Monster`: 몬스터 Entity

### 사용 시점
1. `C_EnterGame` 처리 후 룸 입장 시
2. `S_ChangeRoomCommit` 처리 후 룸 전환 시

### 관련 파일
- `GameServer/Room.cpp`

### 클라이언트 처리
받은 몬스터 정보를 기반으로 게임 월드에 몬스터 스폰
