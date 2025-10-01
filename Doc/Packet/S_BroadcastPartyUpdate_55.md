# S_BroadcastPartyUpdate (55)

### **파티 상태 업데이트 브로드캐스트**

- 설명: 서버가 파티 상태 변경사항을 파티원 전체에게 브로드캐스트한다.
- 입력: `PartyService::SendPartyStatusUpdate()` 호출
- 처리 로직:
  - 파티 ID로 파티 조회 (`PartyManager::FindParty()`)
  - 파티의 온라인 멤버 목록 조회 (`Party::GetOnlineMembers()`)
  - 각 멤버의 상태 정보 수집 (playerId, playerName, hp, maxHp, level, isLeader)
  - S_BroadcastPartyUpdate 패킷 생성
  - 모든 파티원의 세션에게 브로드캐스트 (`BroadcastToPartyMembers()`)
- 출력: `S_BroadcastPartyUpdate`
  ```protobuf
  message S_BroadcastPartyUpdate {
      EPartyUpdateType updateType = 1;
      repeated PartyMemberStatusInfo members = 2;
  }

  message PartyMemberStatusInfo {
      int32 playerId = 1;
      string playerName = 2;
      int32 hp = 3;
      int32 maxHp = 4;
      int32 level = 5;
      bool isLeader = 6;
  }

  enum EPartyUpdateType {
      PARTY_UPDATE_MEMBER_JOIN = 0;    // 멤버 가입
      PARTY_UPDATE_MEMBER_LEAVE = 1;   // 멤버 탈퇴/강퇴
      PARTY_UPDATE_STATUS = 2;         // 상태 업데이트 (HP, Level 변경)
      PARTY_UPDATE_DISBANDED = 3;      // 파티 해산
  }
  ```
- 데이터 구조:
  - `Party::_members`: 파티 멤버 목록
  - `Player`: 각 멤버의 상태 정보 (hp, maxHp, level)

### 전송 시점
1. 파티 생성 시 (`MEMBER_JOIN`)
2. 파티원 가입 시 (`MEMBER_JOIN`)
3. 파티원 탈퇴/강퇴 시 (`MEMBER_LEAVE`)
4. 파티 해산 시 (`DISBANDED`)
5. 주기적 상태 업데이트 (`STATUS`) - Room Tick마다 호출

### 관련 파일
- `GameServer/PartyService.cpp` (Line 49-80)

### 클라이언트 처리
- `MEMBER_JOIN`: 파티 UI에 새 멤버 추가
- `MEMBER_LEAVE`: 파티 UI에서 멤버 제거
- `STATUS`: 멤버의 HP/Level 정보 갱신
- `DISBANDED`: 파티 UI 닫기
