# S_PlayerMoveReply (14)

## 플레이어 이동 응답

- 설명: 플레이어 이동 요청에 대한 처리 결과를 응답한다.
- 입력: 이동 처리 결과 (playerId, clientSeq, result, tick, newPos, direction)
- 처리 로직:
  - 룸에서 이동 요청을 처리한 후 결과를 클라이언트에 전송한다.
  - 이동 결과(EMoveResult), 새로운 위치, 방향, 틱 정보를 포함한다.
  - 클라이언트는 이 응답을 받아 플레이어의 위치를 업데이트한다.
  - 동시에 다른 플레이어들에게는 S_BroadcastPlayerMove로 브로드캐스트된다.
- 출력: `int32 playerId, int32 clientSeq, EMoveResult result, int32 tick, Vector2Info newPos, EDirection direction`
- 데이터 구조: 
  - EMoveResult 열거형
  - EDirection 열거형
  - Vector2Info (위치 정보)