#include "pch.h"
#include "MonsterContainer.h"

const Monster* MonsterContainer::Find(EntityId id) const {
	auto it = _monsters.find(id);
	return (it == _monsters.end()) ? nullptr : &it->second;
}

Monster* MonsterContainer::Find(EntityId id) {
	auto it = _monsters.find(id);
	return (it == _monsters.end()) ? nullptr : &it->second;
}


Monster& MonsterContainer::CreateFromSpawn(const SpawnPointCfg& sp, const MonsterStats& stats,
	EntityId newId, int64_t nowMs) {
	Monster m;
	m.core.id = newId;
	m.core.kind = EntityKind::Monster;
	m.core.pos = { sp.x, sp.y };
	m.core.dir = Protocol::EDirection::DIR_DOWN;
	m.typeId = sp.monsterTypeId;
	m.curHp = stats.maxHp;
	m.state = MState::Idle;
	m.spawnX = sp.x; m.spawnY = sp.y;
	m.fromSpawnId = sp.id;
	m.nextMoveAtMs = nowMs; // 즉시 이동/행동 가능
	m.nextAttackAtMs = nowMs;


	auto [it, ok] = _monsters.emplace(newId, std::move(m));
	IncAlive(sp.id);
	return it->second;
}


void MonsterContainer::Kill(EntityId id) {
	auto it = _monsters.find(id);
	if (it == _monsters.end()) return;
	it->second.state = MState::Dead;
}


bool MonsterContainer::Remove(EntityId id) {
	auto it = _monsters.find(id);
	if (it == _monsters.end()) return false;
	DecAlive(it->second.fromSpawnId);
	_monsters.erase(it);
	return true;
}


void MonsterContainer::EnqueueRespawn(int64_t dueMs, const SpawnPointCfg& sp) {
	_respawns.push(RespawnTask{ dueMs, sp });
}


bool MonsterContainer::PeekRespawn(int64_t nowMs, RespawnTask& out) const {
	if (_respawns.empty()) return false;
	const auto& top = _respawns.top();
	if (top.dueMs > nowMs) return false;
	out = top; return true;
}


bool MonsterContainer::PopRespawn(int64_t nowMs, RespawnTask& out) {
	if (!PeekRespawn(nowMs, out)) return false;
	_respawns.pop();
	return true;
}


int MonsterContainer::AliveCountBySpawn(int spId) const {
	auto it = _aliveBySpawn.find(spId);
	return (it == _aliveBySpawn.end()) ? 0 : it->second;
}
void MonsterContainer::IncAlive(int spId) { _aliveBySpawn[spId]++; }
void MonsterContainer::DecAlive(int spId) {
	auto it = _aliveBySpawn.find(spId);
	if (it == _aliveBySpawn.end()) return;
	if (--it->second <= 0) _aliveBySpawn.erase(it);
}