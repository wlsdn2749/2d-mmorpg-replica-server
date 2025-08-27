#include "pch.h"
#include "PlayerCombatSystem.h"

#include "ProtocolHelper.h"

#include "Player.h"

void PlayerCombatSystem::HandleAttack(const PlayerRef& player, int64 nowMs)
{
	if(!player) return;

	auto pid = player->core.id;
	// 1) 쿨다운 체크
	if (!AttackCooldownReady(pid, nowMs)) return; // 쿨 안됬으면 공격 무효 처리

	// 2) 1타일 전방 판정
	MonsterView mv{};
	if (!InFrontOneTile(player, mv)) return;

	// 3) 타겟 조회
	auto targetMonsterId = mv.id;
	if(!_pLinker.TryGetMonster(targetMonsterId, mv)) return; // 몬스터 없으면 공격 무효


	// 4) 데미지 판정 = monsterHp = monsterHp - player.Atk
	const int damage = player->Atk();

	int hpAfter = mv.hp;
	_pLinker.ApplyDamageToMonster(mv.id, damage, pid); // 여기서 만약 죽으면 Death broadcast
	_lastAttackAtMs[pid] = nowMs;

	_pCaster.BroadcastPlayerAttack(pid, mv.id, damage, hpAfter); // 공격 브로드캐스팅


}


bool PlayerCombatSystem::InFrontOneTile(const PlayerRef& player, const MonsterView& m) const
{
	int fx, fy;
	ForwardTile(player->PosX(), player->PosY(), player->Dir(), fx, fy);
	
	// “바라보는 방향 앞 1타일” 엄격 체크
	return (m.x == fx && m.y == fy);
}

bool PlayerCombatSystem::AttackCooldownReady(int pid, int64 nowMs) const
{
	auto it = _lastAttackAtMs.find(pid);
	if(it == _lastAttackAtMs.end()) return true;
	return _cfg.attackCooldownMs <= (nowMs - it->second); // 마지막으로 공격한시간이 딜레이보다 크면 공격 성공
}
