# S_BroadcastPlayerTryAttack (41)

### **플레이어 공격 시도 브로드캐스트**

- 설명: 서버가 플레이어의 공격 시도를 룸의 모든 플레이어에게 브로드캐스트한다. 공격 애니메이션 재생용.
- 입력: `C_PlayerAttackRequest` 처리 시
- 처리 로직:
  - 플레이어의 공격 요청 검증
  - 공격 애니메이션 브로드캐스트
  - 실제 데미지 계산은 별도 로직
- 출력: `S_BroadcastPlayerTryAttack`
  ```protobuf
  message S_BroadcastPlayerTryAttack {
      int32 playerId = 1;  // 공격하는 플레이어 ID
  }
  ```
- 데이터 구조: 없음

### S_BroadcastPlayerAttack (27번)과의 차이
- **TryAttack (41번)**: 공격 시도 (애니메이션 재생)
- **PlayerAttack (27번)**: 공격 결과 (데미지, 대상)

### 관련 파일
- `GameServer/ClientPacketHandler.cpp`
- `GameServer/PlayerCombatSystem.cpp`

### 클라이언트 처리
공격 애니메이션 재생
