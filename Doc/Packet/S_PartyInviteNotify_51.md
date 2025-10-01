# S_PartyInviteNotify (51)

### **파티 초대 알림**

- 설명: 서버가 파티 초대를 받은 플레이어에게 초대 알림을 전송한다.
- 입력: `Room::HandlePartyInvite()` 내부 처리
- 처리 로직:
  - 초대자(inviter)와 초대받는 사람(invitee) 정보 확인
  - 초대자의 파티 ID 조회
  - S_PartyInviteNotify 패킷 생성
  - 초대받는 사람의 세션에게만 전송
- 출력: `S_PartyInviteNotify`
  ```protobuf
  message S_PartyInviteNotify {
      int32 inviterPid = 1;      // 초대한 사람 플레이어 ID
      string inviterName = 2;     // 초대한 사람 이름
      int32 partyId = 3;          // 파티 ID
  }
  ```
- 데이터 구조:
  - `Player`: 초대자 및 초대받는 사람의 플레이어 정보
  - `GameSession`: 초대받는 사람의 네트워크 세션

### 관련 파일
- `GameServer/Room.cpp` (Line 383-395)

### 클라이언트 처리
초대받은 플레이어는 UI 팝업을 표시하고 수락/거절 버튼을 제공해야 한다.
- 수락: `C_PartyInviteResponse` (accept=true) 전송
- 거절: `C_PartyInviteResponse` (accept=false) 전송
