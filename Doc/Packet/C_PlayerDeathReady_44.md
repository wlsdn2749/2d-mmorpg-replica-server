# C_PlayerDeathReady (44)

### **플레이어 리스폰 준비 완료**

- 설명: 사망한 플레이어가 리스폰 준비가 완료되었음을 서버에 알린다.
- 입력: `C_PlayerDeathReady`
  ```protobuf
  message C_PlayerDeathReady {
      // 빈 메시지
  }
  ```
- 처리 로직:
  - 플레이어의 사망 상태 확인
  - 리스폰 처리:
    - HP 초기화 (maxHp로 설정)
    - 사망 상태 해제
    - 스폰 지역 결정 (region에 따라)
  - Room 전환 시작 (`S_ChangeRoomBegin` → `S_PlayerDeathCommit` → `S_ChangeRoomCommit`)
- 출력:
  - `S_PlayerDeathCommit` (45번): 리스폰 승인
  - `S_ChangeRoomCommit` (20번): 스폰 지역으로 이동
- 데이터 구조:
  - `Player::_isDead`: 사망 상태
  - `Player::region`: 스폰 지역 (고구려/백제)

### 리스폰 로직
- 고구려 캐릭터 → 고구려 스폰 지역
- 백제 캐릭터 → 백제 스폰 지역

### 관련 파일
- `GameServer/ClientPacketHandler.cpp`
- `GameServer/Room.cpp`

### 클라이언트 처리
리스폰 애니메이션 재생, 캐릭터 위치 업데이트
