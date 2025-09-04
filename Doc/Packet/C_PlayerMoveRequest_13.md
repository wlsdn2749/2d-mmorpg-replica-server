# C_PlayerMoveRequest (13)

## 플레이어 이동 요청

- 설명: 플레이어가 특정 좌표로 이동을 요청한다.
- 입력: `C_PlayerMoveRequest` (clickWorldPos)
- 처리 로직:
  - 세션이 InRoom 상태인지 확인한다.
  - 현재 플레이어 정보를 가져온다.
  - 플레이어가 속한 룸의 OnRecvMoveReq()를 비동기로 호출한다.
  - 룸에서 이동 유효성 검증, 충돌 검사, 쿨다운 체크를 수행한다.
  - 이동 결과를 EMoveResult로 반환한다 (OK, DIR, BLOCKED, COOLDOWN).
- 출력: `S_PlayerMoveReply`
- 데이터 구조: 
  - Room 이동 시스템
  - 맵 타일 데이터 (충돌 검사)
  - Vector2Info (좌표 정보)