# C_PlayerAttackRequest (24)

## 플레이어 공격 요청

- 설명: 플레이어가 공격 행동을 요청한다.
- 입력: `C_PlayerAttackRequest` (빈 메시지)
- 처리 로직:
  - 세션이 InRoom 상태인지 확인한다.
  - 현재 플레이어 정보를 가져온다.
  - 플레이어가 속한 룸의 OnRecvAttackReq()를 비동기로 호출한다.
  - 룸에서 공격 가능한 대상을 찾고 PlayerCombatSystem에서 공격을 처리한다.
  - 공격 결과는 S_BroadcastPlayerAttack으로 브로드캐스트된다.
- 출력: `S_BroadcastPlayerAttack`
- 데이터 구조: 
  - PlayerCombatSystem
  - Room 전투 관리 시스템