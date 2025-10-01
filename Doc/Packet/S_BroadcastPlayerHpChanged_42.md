# S_BroadcastPlayerHpChanged (42)

### **플레이어 HP 변경 브로드캐스트**

- 설명: 서버가 플레이어의 HP 변경사항을 룸의 모든 플레이어에게 브로드캐스트한다.
- 입력: HP 변경 이벤트 발생 시 (공격받음, 포션 사용 등)
- 처리 로직:
  - HP 변경 감지
  - 변경된 HP 값 브로드캐스트
- 출력: `S_BroadcastPlayerHpChanged`
  ```protobuf
  message S_BroadcastPlayerHpChanged {
      int32 playerId = 1;
      int32 hp = 2;
      int32 maxHp = 3;
  }
  ```
- 데이터 구조:
  - `Player::_statInfo`: HP 정보

### 전송 시점
- 몬스터에게 공격받았을 때
- 포션 사용 시
- 기타 HP 변경 이벤트

### 관련 파일
- `GameServer/PlayerCombatSystem.cpp`
- `GameServer/InventorySystem.cpp`

### 클라이언트 처리
다른 플레이어의 HP바 UI 갱신
