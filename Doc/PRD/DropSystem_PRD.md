# 드랍 시스템 PRD (Drop System - PRD)

## 1. 개요 (Overview)

**목적**  
일반 몬스터 처치 시, 별도의 드랍 오브젝트 없이 확률 기반으로 아이템을 즉시 처치한 플레이어에게 지급하는 드랍 시스템을 구현한다.

**적용 대상**  
- 일반 몬스터  
- 인스턴스, 파티, 레이드는 포함하지 않음

**특징 요약**  
- 드랍 아이템은 필드에 생성되지 않음  
- 처치한 플레이어에게 직접 인벤토리로 지급  
- 조건 없는 확률 기반 드랍  
- 단순하고 빠른 처리 구조

## 2. 기능 설명 (Feature Description)

### 2.1. 드랍 테이블 정의

- 몬스터 ID 별로 드랍 아이템 목록이 존재  
- 각 아이템 항목에는 다음 속성 포함:  
  - 아이템 ID  
  - 드랍 확률 (0.0 ~ 100.0 %)  
  - 수량 범위 (최소~최대)

#### 예시 테이블

| 몬스터 ID | 아이템 ID | 드랍 확률 (%) | 수량 범위 |
|-----------|-----------|----------------|-----------|
| 1001      | 2001      | 50.0           | 1~3       |
| 1001      | 2002      | 10.0           | 1         |
| 1002      | 2003      | 100.0          | 1         |

### 2.2. 드랍 처리 로직

1. 몬스터 사망 시, 서버는 해당 몬스터의 드랍 테이블을 조회  
2. 각 아이템에 대해 확률 계산 수행  
3. 확률 성공 시 수량 범위를 기반으로 실제 수량 결정  
4. 즉시 인벤토리에 아이템 지급  
5. 실패 또는 오류 발생 시, 로그 및 사용자 피드백 처리  

## 3. 데이터 구조 (Data Structure)

### 3.1. DropItemInfo

```csharp
class DropItemInfo
{
    public int ItemId;
    public float DropRate;     // 0.0 ~ 100.0
    public int MinCount;
    public int MaxCount;
}
```

### 3.2. MonsterDropTable

```csharp
class MonsterDropTable
{
    public int MonsterId;
    public List<DropItemInfo> DropItems;
}
```

### 3.3 DB MonsterDropItem Schema
```sql
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

## 4. 서버 처리 흐름 (Server Logic Flow)

```plaintext
[몬스터 사망]
   ↓
[DropTable 조회]
   ↓
[각 드랍 아이템별 확률 판단]
   ↓
[성공 시 수량 랜덤 결정]
   ↓
[처치한 플레이어 인벤토리에 지급 시도]
   ↓
[성공 → 완료 / 실패 → 예외 처리 및 로그 기록]
```

## 5. 예외 처리 (Exception Handling)

| 예외 상황              | 처리 내용                           |
|------------------------|------------------------------------|
| 드랍 테이블 없음       | 드랍 없음, 경고 로그 기록           |
| 인벤토리 공간 부족     | 지급 실패, 사용자 알림 + 로그 기록  |
| 아이템 ID 미등록       | 지급 스킵, 로그 기록                |
| 서버 지급 처리 실패    | 시스템 경고 로그, 필요 시 보정 처리 |

## 6. 로깅 및 분석 항목

- 몬스터 처치 시 드랍 시도 로그  
- 아이템별 드랍 성공/실패 횟수  
- 인벤토리 부족 및 지급 실패 비율  
- 드랍 확률과 실제 지급 통계 비교 가능 (운영 데이터 수집 목적)