# S_BroadcastMonsterAttack (22)

## 몬스터 공격 브로드캐스트

- 설명: 몬스터가 플레이어를 공격했을 때 룸 내 모든 플레이어에게 브로드캐스트한다.
- 입력: 몬스터 공격 정보 (monsterId, targetPid)
- 처리 로직:
  - MonsterCombatSystem에서 몬스터가 공격을 수행할 때 호출된다.
  - 공격한 몬스터의 ID와 공격 대상 플레이어 ID를 전송한다.
  - 룸 내 모든 플레이어에게 브로드캐스트되어 공격 애니메이션을 재생한다.
  - 실제 데미지 처리는 서버에서 수행하고 결과만 클라이언트에 전송한다.
- 출력: `int32 monsterId, int32 targetPid`
- 데이터 구조: 
  - MonsterCombatSystem
  - 전투 시스템