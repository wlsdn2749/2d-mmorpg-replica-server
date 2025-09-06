# 드랍 시스템 구현 계획

## 1. 분석 결과

### 현재 코드베이스 구조 파악
- **몬스터 사망 처리**: `MonsterService::KillMonster()` (GameServer/MonsterService.cpp:87)
- **플레이어 공격 플로우**: `PlayerCombatSystem::HandleAttack()` → `MonsterService::ApplyDamageToMonster()` → `MonsterService::KillMonster()`  
- **인벤토리 시스템**: 완전히 구현됨 (`InventorySystem`, `ItemManager`, DB 연동 포함)
- **프로토콜**: `S_InventoryUpdate` 패킷으로 인벤토리 변경 브로드캐스트 가능

## 2. 구현 계획

### Phase 1: DB 스키마 확장
1. **MonsterDropItem 테이블 추가** (GameDB.xml)
- 몬스터 ID별 드랍 아이템 정보 (아이템 ID, 확률, 수량 범위)
- 저장 프로시저: `spGetMonsterDropItems`


### Phase 2: 드랍 시스템 코어 구현
2. **DropSystem 클래스 구현**
   - 드랍 테이블 로딩 및 캐싱
   - 확률 기반 드랍 계산 (`RollDrop`)
   - 아이템 지급 로직 (인벤토리 연동)

3. **MonsterService 확장**
   - `KillMonster()` 함수에 드랍 처리 통합
   - `ApplyDamageToMonster()` 함수에서 처치한 플레이어 ID 전달

### Phase 3: 통합 및 테스트
4. **시스템 통합**
   - ItemManager와 InventorySystem 연동
   - 실패 처리 (인벤토리 부족 등)
   - 로깅 시스템 추가

5. **테스트 데이터 및 검증**
   - 샘플 드랍 테이블 데이터
   - 확률 검증 및 예외 상황 테스트

## 3. 핵심 통합 지점

### 몬스터 사망 → 드랍 처리
```cpp
// MonsterService::KillMonster() 내부에 추가
void MonsterService::KillMonster(EntityId id, Protocol::EDespawnReason reason)
{
    // 기존 코드...
    
    // 드랍 처리 (몬스터가 플레이어에 의해 처치된 경우)
    if (reason == Protocol::EDespawnReason::DESPAWN_KILLED && srcPlayerId > 0) {
        _dropSystem.ProcessMonsterDrop(m->typeId, srcPlayerId);
    }
    
    // 기존 코드 계속...
}
```

### 인벤토리 직접 지급
- 기존 `InventorySystem::AddItem()` 활용
- 성공/실패에 따른 `S_InventoryUpdate` 브로드캐스트

## 4. 예상 구현 범위

### 새로운 파일들
- `GameServer/DropSystem.h/.cpp`
- `GameServer/DropCore.h` (드랍 관련 구조체/enum)

### 수정 파일들  
- `GameServer/GameDB.xml` (DB 스키마)
- `GameServer/MonsterService.h/.cpp` (드랍 통합)
- `Protocol/GenProcs.bat` 실행 필요

### 테스트 파일
- `inventory-test-data.sql` 확장 (드랍 테이블 데이터)

## 5. 상세 구현 사항

### 5.1. 데이터베이스 스키마

```sql
-- GameDB.xml에 추가할 테이블
CREATE TABLE MonsterDropItem (
    Id INT PRIMARY KEY AUTO_INCREMENT,
    MonsterId INT NOT NULL,
    ItemId INT NOT NULL,
    DropRate FLOAT NOT NULL,
    MinCount INT NOT NULL,
    MaxCount INT NOT NULL,
    IsEnabled BOOLEAN DEFAULT TRUE
);
```

### 5.2. DropSystem 클래스 설계

```cpp
// DropCore.h
struct DropItemInfo {
    int itemId;
    float dropRate;     // 0.0 ~ 100.0
    int minCount;
    int maxCount;
};

struct MonsterDropTable {
    int monsterId;
    std::vector<DropItemInfo> dropItems;
};

enum class EDropResult {
    SUCCESS,
    NO_DROP_TABLE,
    INVENTORY_FULL,
    ITEM_NOT_FOUND,
    SYSTEM_ERROR
};

// DropSystem.h
class DropSystem {
public:
    bool Initialize();
    std::future<bool> LoadDropTablesAsync();
    
    EDropResult ProcessMonsterDrop(int monsterId, int playerId);
    
private:
    bool RollDrop(const DropItemInfo& dropInfo) const;
    int GetRandomCount(int minCount, int maxCount) const;
    
    std::unordered_map<int, MonsterDropTable> _dropTables;
};
```

### 5.3. 기존 시스템 연동점

#### MonsterService 수정
- `ApplyDamageToMonster()`: 처치한 플레이어 ID 추가 매개변수
- `KillMonster()`: 드랍 시스템 호출 추가

#### PlayerCombatSystem 수정
- `HandleAttack()`: 플레이어 ID 전달 체인 구성

## 6. 테스트 계획

### 6.1. 단위 테스트
- 드랍 확률 로직 검증 (통계적 검증)
- 수량 범위 정상 동작 확인
- 인벤토리 연동 테스트

### 6.2. 통합 테스트
- 몬스터 처치 → 아이템 지급 플로우
- 인벤토리 부족 시 예외 처리
- 네트워크 패킷 브로드캐스트 검증

### 6.3. 성능 테스트
- 대량 몬스터 처치 시 성능 측정
- 드랍 테이블 캐싱 효율성 검증

## 7. 운영 고려사항

### 7.1. 로깅
- 드랍 시도/성공/실패 로그
- 인벤토리 부족 등 예외 상황 로깅
- 드랍 확률 vs 실제 지급 통계

### 7.2. 밸런싱
- 드랍 확률 조정 시스템
- 실시간 드랍 테이블 리로드 (선택사항)
- 드랍 통계 모니터링

이 계획은 기존 아키텍처를 최대한 활용하면서 PRD 요구사항에 맞는 간단하고 효율적인 드랍 시스템을 구현하는 것을 목표로 합니다.