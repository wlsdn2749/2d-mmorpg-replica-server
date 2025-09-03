# S_BroadcastPlayerAttack (25)

## 플레이어 공격 브로드캐스트

- 설명: 플레이어가 공격을 수행했을 때 룸 내 모든 플레이어에게 브로드캐스트한다.
- 입력: 플레이어 공격 정보 (playerId, targetId, damage, hpAfter)
- 처리 로직:
  - PlayerCombatSystem에서 플레이어 공격이 처리된 후 호출된다.
  - 공격한 플레이어 ID, 공격 대상 ID, 입힌 데미지, 공격 후 대상의 HP를 전송한다.
  - 룸 내 모든 플레이어에게 브로드캐스트되어 공격 효과와 데미지를 표시한다.
  - 클라이언트는 이 정보를 받아 공격 애니메이션과 데미지 UI를 표시한다.
- 출력: `int32 playerId, int32 targetId, int32 damage, int32 hpAfter`
- 데이터 구조: 
  - PlayerCombatSystem
  - 전투 결과 데이터
  - HP 관리 시스템