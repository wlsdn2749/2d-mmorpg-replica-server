# S_PartyList (63)

### **전체 공개 파티 목록 응답**

- 설명: 서버가 클라이언트에게 현재 생성된 모든 공개 파티의 목록을 전송한다.
- 입력: `C_PartyList` 처리 완료 후
- 처리 로직:
  - PartyManager에서 수집한 파티 정보를 패킷에 담아 전송
- 출력: `S_PartyList`
  ```protobuf
  message S_PartyList {
      repeated PartyInfo partyInfos = 1;
  }

  message PartyInfo {
      int32 partyId = 1;
      string partyName = 2;
      repeated PartyMemberStatusInfo members = 3;
      int32 curMemberCount = 4;
      int32 maxMemberCount = 5;
      int32 partyLeaderId = 6;
  }

  message PartyMemberStatusInfo {
      int32 playerId = 1;
      string playerName = 2;
      int32 hp = 3;
      int32 maxHp = 4;
      int32 level = 5;
      bool isLeader = 6;
  }
  ```
- 데이터 구조: `PartyInfo` 배열

### PartyInfo 필드 설명
- `partyId`: 파티 고유 ID (가입 요청 시 사용)
- `partyName`: 파티 이름
- `members`: 현재 파티원 전체 상태 (PlayerId, 이름, HP, Level, 리더 여부)
- `curMemberCount`: 현재 인원
- `maxMemberCount`: 최대 인원 (4명)
- `partyLeaderId`: 파티 리더의 PlayerId

### 관련 파일
- `GameServer/ClientPacketHandler.cpp` (Line 861-874)

### 클라이언트 UI 예시
```
[파티 목록]
┌─────────────────────────────────┐
│ [1] 레이드 파티 (1/4)           │ [가입 요청]
│     리더: PlayerA (Lv.10)       │
├─────────────────────────────────┤
│ [2] 던전 파티 (3/4)             │ [가입 요청]
│     리더: PlayerB (Lv.15)       │
└─────────────────────────────────┘
```
