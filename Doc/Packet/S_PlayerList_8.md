# S_PlayerList (8)

## 플레이어 목록

- 설명: 현재 룸에 있는 모든 플레이어의 정보를 클라이언트에 전송한다.
- 입력: 룸 내 플레이어 목록 (myPlayerId, PlayerInfo 배열)
- 처리 로직:
  - 플레이어가 룸에 입장할 때 현재 룸에 있는 모든 플레이어의 정보를 전송한다.
  - 자신의 playerId를 myPlayerId 필드에 포함하여 클라이언트가 자신을 구별할 수 있게 한다.
  - 각 플레이어의 ID, 이름, 위치, 방향 정보를 PlayerInfo로 전송한다.
- 출력: `int32 myPlayerId, repeated PlayerInfo players`
- 데이터 구조: 
  - Room 내 플레이어 목록
  - PlayerInfo (플레이어 기본 정보)