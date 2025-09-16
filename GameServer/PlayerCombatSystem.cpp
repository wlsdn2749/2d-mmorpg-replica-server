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

	// 2) 1타일 전방 몬스터 여부 판정.
	MonsterView mv{};
	if (!FindMonsterInFrontOneTile(player, mv)) return;

	// 3) 타겟 조회
	auto targetMonsterId = mv.id;
	if(!_pLinker.TryGetMonster(targetMonsterId, mv)) return; // 몬스터 없으면 공격 무효

	// 4) 데미지 판정 = monsterHp = monsterHp - player.Atk
	const int damage = player->Atk();

	GConsoleLogger->WriteStdOut(Color::GREEN, L"[Player] Player:%d attacks Monster:%d (dmg:%d hp:%d->%d)", 
		pid, mv.id, damage, mv.hp, mv.hp - damage);

	_pLinker.ApplyDamageToMonster(mv.id, damage, pid); // 여기서 만약 죽으면 Death broadcast
	_lastAttackAtMs[pid] = nowMs;

	int hpAfter = mv.hp;
	_pCaster.BroadcastPlayerAttack(pid, mv.id, damage, hpAfter); // 공격 브로드캐스팅


}

bool PlayerCombatSystem::FindMonsterInFrontOneTile(const PlayerRef& player, MonsterView& outMonster) const
{
	int fx, fy;
	ForwardTile(player->PosX(), player->PosY(), player->Dir(), fx, fy);

	return _pLinker.TryGetMonsterAt(fx, fy, outMonster);
}

bool PlayerCombatSystem::AttackCooldownReady(int pid, int64 nowMs) const
{
	auto it = _lastAttackAtMs.find(pid);
	if(it == _lastAttackAtMs.end()) return true;
	return _cfg.attackCooldownMs <= (nowMs - it->second); // 마지막으로 공격한시간이 딜레이보다 크면 공격 성공
}
