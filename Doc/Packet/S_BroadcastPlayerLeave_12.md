# S_BroadcastPlayerLeave (12)

## 플레이어 퇴장 브로드캐스트

- 설명: 플레이어가 룸에서 퇴장했을 때 다른 플레이어들에게 알림을 브로드캐스트한다.
- 입력: 퇴장한 플레이어 정보 (playerId, reason)
- 처리 로직:
  - 플레이어가 룸에서 퇴장할 때 Room 시스템에서 자동으로 호출된다.
  - 퇴장한 플레이어를 제외한 룸 내 모든 플레이어에게 브로드캐스트된다.
  - 퇴장 이유(LOGOUT, CHANGE_ROOM, DISCONNECT)를 함께 전송한다.
  - 클라이언트는 이 패킷을 받아 해당 플레이어를 화면에서 제거한다.
- 출력: `int32 playerId, ELeaveReason reason`
- 데이터 구조: 
  - Room 브로드캐스트 시스템
  - ELeaveReason 열거형