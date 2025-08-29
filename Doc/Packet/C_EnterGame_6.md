# C_EnterGame (6)

## 게임 입장 요청

- 설명: 선택한 캐릭터로 게임에 입장을 요청한다.
- 입력: `C_EnterGame` (playerIndex)
- 처리 로직:
  - playerIndex로 세션에 저장된 캐릭터 중 선택된 캐릭터를 currentPlayer로 설정한다.
  - 캐릭터 목록이 비어있는 경우 실패 응답을 보낸다.
  - RoomManager에서 기본 룸(roomId=0)을 찾아 플레이어를 입장시킨다.
  - 세션 상태를 InRoom으로 변경한다.
  - Room::Enter를 비동기로 호출하여 룸에 플레이어를 추가한다.
- 출력: `S_EnterGame`
- 데이터 구조: 
  - Room 시스템 (플레이어 입장 관리)
  - Player 객체 (현재 플레이어 정보)