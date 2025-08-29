# S_BroadcastMonsterDeath (23)

## 몬스터 사망 브로드캐스트

- 설명: 몬스터가 사망했을 때 룸 내 모든 플레이어에게 브로드캐스트한다.
- 입력: 사망한 몬스터 정보 (monsterId)
- 처리 로직:
  - MonsterCombatSystem에서 몬스터의 HP가 0이 되어 사망할 때 호출된다.
  - 사망한 몬스터의 ID를 전송한다.
  - 룸 내 모든 플레이어에게 브로드캐스트되어 사망 효과를 표시한다.
  - 이후 S_DespawnMonster 패킷으로 몬스터가 완전히 제거된다.
- 출력: `int32 monsterId`
- 데이터 구조: 
  - MonsterCombatSystem
  - 몬스터 생명 관리 시스템