````markdown
# Inventory System PRD

## 1. 요구사항

### 1.1 아이템 추가 / 제거
- **추가**
  - 몬스터 드롭, 상점 구매, 퀘스트 보상 등으로 인벤토리에 아이템 추가
- **제거**
  - 버리기, 판매, 사용 등으로 제거
- **스택 기능**
  - 동일 아이템은 하나의 슬롯에 수량으로 누적 가능
- **슬롯 기반 구조**
  - 제한된 슬롯 수를 가지며, 슬롯 단위로 아이템을 저장
- **퀵 슬롯**
  - 회복 아이템 등 자주 사용하는 아이템을 퀵 슬롯에 등록
  - (기본 슬롯 + 퀵 슬롯 구조)

---

## 2. 기능 상세

### 2.1 아이템 추가 / 제거

**설명**
- 몬스터 드롭으로 아이템을 인벤토리에 추가
- 아이템 사용, 드롭, 판매 시 제거

**처리 로직**
1. **추가**
   - 인벤토리에 동일한 아이템이 존재하고 **스택 가능**하다면 → 해당 슬롯의 수량 증가  
   - 아니라면 → 새로운 슬롯에 삽입  
   - 슬롯이 부족한 경우 → 실패 처리
2. **제거**
   - 수량 감소
   - 수량이 0이 되면 슬롯 제거

**출력**
- 인벤토리 갱신 결과

---

### 2.2 아이템 스택 기능

**설명**
- 동일한 스택형 아이템은 하나의 슬롯에 누적 가능
- 비스택형 아이템은 무조건 새로운 슬롯 차지

**조건**
- 아이템 메타데이터에 `isStackable: true` 여부 존재
- `maxStack` 존재 가능

---

### 2.3 슬롯 기반 인벤토리

**설명**
- 기본 슬롯: **40칸 고정 (30 Slot + 10 QuickSlot)**
- 슬롯이 꽉 차면 새 아이템 추가 불가

---

### 2.4 퀵 슬롯

**설명**
- 인벤토리와 같은 구조
- 총 40칸 중 **10칸은 퀵슬롯**, **30칸은 일반 인벤토리**
- 퀵 슬롯에서 사용 시 → 해당 슬롯의 수량 차감
- 해당 슬롯의 아이템 수량이 0이 되면 삭제

---

## 3. 데이터 모델링

### 3.1 아이템 메타 정보 테이블

```sql
CREATE TABLE ItemData (
    item_id INT PRIMARY KEY,                         
    name NVARCHAR(64) NOT NULL,                      
    description NVARCHAR(256) NULL,                  
    is_stackable BIT NOT NULL DEFAULT 0,            
    max_stack INT NOT NULL DEFAULT 99,           
    item_type INT NOT NULL DEFAULT 0
);
````

---

### 3.2 플레이어 인벤토리 슬롯 테이블

```sql
CREATE TABLE CharacterInventory (
    character_id INT NOT NULL,                       -- 캐릭터 ID (외래키)
    slot_index INT NOT NULL,                         -- 슬롯 인덱스 (0~39)
    item_id INT NOT NULL,                            -- 아이템 ID
    count INT NOT NULL DEFAULT 1,                    -- 해당 슬롯의 아이템 수량
    is_quickslot BIT NOT NULL DEFAULT 0,             -- 퀵슬롯 여부

    CONSTRAINT PK_CharacterInventory PRIMARY KEY (character_id, slot_index),
    CONSTRAINT FK_Inventory_ItemData FOREIGN KEY (item_id) REFERENCES ItemData(item_id),
    CONSTRAINT FK_Inventory_Character FOREIGN KEY (character_id) REFERENCES Character(character_id)
);
```

---

## 4. 서버 메모리 구조

### 4.1 플레이어 단위 객체 구조

* `Player` 객체 안에 인벤토리를 포함한 모든 게임 상태 보관
* `InventorySystem`을 `Player` 객체가 소유
* `InventorySystem` 내 `ItemSlot` 구조체(Struct)로 슬롯 관리

---

## 5. 인벤토리 시스템 구성

### 5.1 InventorySystem

* 슬롯 상태 관리
* 아이템 스택 처리
* 아이템 사용 시 효과 처리 (예: 회복)

### 5.2 ItemManager

* 아이템 정적 데이터 보관
* 메모리에 로딩된 메타 정보 조회

### 5.3 InventoryRepository

* DB로부터 인벤토리 데이터를 불러오고 저장
* 게임 접속 시 → 슬롯 데이터 로딩
* 로그아웃 시 또는 주기적으로 슬롯 저장

