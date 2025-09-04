# C_LeaveGame (10)

## 게임 퇴장 요청

- 설명: 현재 게임에서 나가기를 요청한다.
- 입력: `C_LeaveGame` (reason)
- 처리 로직:
  - 현재 플레이어가 속한 룸을 찾는다.
  - Room::Leave를 비동기로 호출하여 플레이어를 룸에서 제거한다.
  - 퇴장 이유(ELeaveReason)에 따라 적절한 처리를 수행한다 (LOGOUT, CHANGE_ROOM, DISCONNECT).
  - 퇴장 성공 응답을 클라이언트에 전송한다.
- 출력: `S_LeaveGame`
- 데이터 구조: 
  - Room 시스템 (플레이어 퇴장 관리)
  - ELeaveReason 열거형