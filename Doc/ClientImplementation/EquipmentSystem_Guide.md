# 장비 시스템 구현 가이드 (Unity Client)

## 1. Protocol 정의 (PKT 66-72)

### 1.1 장비 슬롯 타입 Enum
```protobuf
enum EEquipmentSlotType
{
    EQUIPMENT_WEAPON = 0;  // 무기
    EQUIPMENT_HELMET = 1;  // 투구
    EQUIPMENT_ARMOR = 2;   // 갑옷
    EQUIPMENT_PANTS = 3;   // 바지
}
```

### 1.2 장비 슬롯 정보 구조체
```protobuf
message EquipmentSlotInfo
{
    EEquipmentSlotType slotType = 1;
    int32 itemId = 2;                  // 0이면 빈 슬롯
    int32 equipmentInstanceId = 3;     // 장비 고유 ID
    int32 enhancementLevel = 4;        // 강화 레벨
}
```

### 1.3 패킷 정의

#### (66) C_EquipItemRequest - 장비 장착 요청
```protobuf
message C_EquipItemRequest
{
    int32 inventorySlotIndex = 1;  // 인벤토리의 아이템 위치
}
```

#### (67) S_EquipItemReply - 장비 장착 응답
```protobuf
message S_EquipItemReply
{
    bool success = 1;
    string errorMessage = 2;
    EEquipmentSlotType slotType = 3;           // 어느 부위에 장착됐는지
    InventorySlotInfo changedSlotInfo = 4;     // 인벤토리 변경 사항 (장착한 아이템 슬롯)
    InventorySlotInfo returnedEquipmentSlot = 5; // 기존 장비가 인벤토리로 돌아온 경우
}
```

#### (68) C_UnequipItemRequest - 장비 해제 요청
```protobuf
message C_UnequipItemRequest
{
    EEquipmentSlotType slotType = 1;  // 해제할 장착 부위
}
```

#### (69) S_UnequipItemReply - 장비 해제 응답
```protobuf
message S_UnequipItemReply
{
    bool success = 1;
    string errorMessage = 2;
    InventorySlotInfo returnedEquipmentSlot = 3; // 해제된 장비가 인벤토리로
    EEquipmentSlotType slotType = 4;
}
```

#### (70) C_EquipmentInfoRequest - 장비 정보 요청
```protobuf
message C_EquipmentInfoRequest
{
    // 빈 메시지 (자신의 장비 정보 조회)
}
```

#### (71) S_EquipmentInfoReply - 장비 정보 응답
```protobuf
message S_EquipmentInfoReply
{
    repeated EquipmentSlotInfo equipments = 1;  // 4개 슬롯 정보 배열
}
```

#### (72) S_BroadcastPlayerEquipment - 장비 변경 브로드캐스트
```protobuf
message S_BroadcastPlayerEquipment
{
    int32 playerId = 1;
    EEquipmentSlotType slotType = 2;
    int32 itemId = 3;  // 0이면 해제, 아니면 장착
}
```
**용도**: 룸 내 다른 플레이어의 외형 변환에 사용

---

## 2. 장비 데이터 시트 (Equipment_data.json)

### 2.1 데이터 구조
```json
[
  {
    "itemId": "20001",      // 아이템 ID
    "slotType": "WEAPON",   // 장착 부위
    "minLevel": "1",        // 최소 레벨
    "atk": "10",           // 공격력
    "def": "0",            // 방어력
    "maxHp": "0"           // 체력
  },
  {
    "itemId": "21001",
    "slotType": "HELMET",
    "minLevel": "1",
    "atk": "0",
    "def": "5",
    "maxHp": "50"
  }
]
```

### 2.2 현재 등록된 장비
- **20001**: 무기 (공격+10, Lv.1)
- **21001**: 투구 (방어+5, HP+50, Lv.1)
- **22001**: 갑옷 (방어+10, HP+100, Lv.1)
- **23001**: 바지 (방어+7, HP+70, Lv.1)

### 2.3 파일 위치
```
GameServer/Resources/Data/Equipment_data.json
TestClientUnity/Assets/Resources/Data/Equipment_data.json
```

---

## 3. DB 스키마 구조

### 3.1 EquipmentInstance 테이블
장비의 메타데이터 저장 (강화 레벨, 획득 시간 등)

| 컬럼명 | 타입 | 설명 |
|--------|------|------|
| equipmentInstanceId | int | 장비 고유 ID (PK, Auto Increment) |
| itemId | int | 아이템 ID |
| enhancementLevel | int | 강화 레벨 |
| acquiredAt | datetime | 획득 시간 |

### 3.2 CharacterEquipment 테이블
캐릭터의 장비 장착 상태 저장

| 컬럼명 | 타입 | 설명 |
|--------|------|------|
| characterId | int | 캐릭터 ID (PK) |
| slotType | int | 장착 부위 (PK, 0-3) |
| equipmentInstanceId | int | 장착된 장비 인스턴스 ID (FK) |

### 3.3 관계
- `CharacterEquipment.characterId` → `Characters.characterId` (CASCADE DELETE)
- `CharacterEquipment.equipmentInstanceId` → `EquipmentInstance.equipmentInstanceId`

---

## 4. 동작 플로우

### 4.1 장비 장착 플로우

```
[Client] 인벤토리 슬롯 클릭 → C_EquipItemRequest(inventorySlotIndex)
    ↓
[Server] 검증 단계:
    1. 슬롯이 비어있는지 확인
    2. 아이템이 장비 타입인지 확인
    3. 레벨 요구사항 체크
    4. 기존 장비가 있으면 인벤토리에 반환 (equipmentInstanceId 보존)
    5. 인벤토리에서 아이템 제거 (equipmentInstanceId 보존)
    6. 장비 장착
    7. 스탯 재계산
    8. DB 저장
    ↓
[Server] S_EquipItemReply 전송:
    - success: true/false
    - slotType: 장착된 부위
    - changedSlotInfo: 인벤토리에서 제거된 슬롯 정보
    - returnedEquipmentSlot: 기존 장비가 돌아온 슬롯 정보 (있는 경우)
    ↓
[Server] S_BroadcastPlayerEquipment 브로드캐스트 (룸 전체):
    - playerId, slotType, itemId (외형 변환용)
    ↓
[Client] 처리:
    1. success 확인
    2. changedSlotInfo로 인벤토리 UI 업데이트
    3. returnedEquipmentSlot이 있으면 인벤토리에 기존 장비 표시
    4. 장비 창 UI 업데이트
    5. 브로드캐스트 받으면 해당 플레이어 외형 변경
```

### 4.2 장비 해제 플로우

```
[Client] 장비 슬롯 클릭 → C_UnequipItemRequest(slotType)
    ↓
[Server] 검증 단계:
    1. 슬롯이 비어있지 않은지 확인
    2. 장비 해제
    3. 인벤토리에 추가 (equipmentInstanceId 보존)
    4. 스탯 재계산
    5. DB 저장
    ↓
[Server] S_UnequipItemReply 전송:
    - success: true/false
    - slotType: 해제된 부위
    - returnedEquipmentSlot: 인벤토리로 돌아간 슬롯 정보
    ↓
[Server] S_BroadcastPlayerEquipment 브로드캐스트:
    - playerId, slotType, itemId=0 (해제)
    ↓
[Client] 처리:
    1. success 확인
    2. returnedEquipmentSlot으로 인벤토리 UI 업데이트
    3. 장비 창 UI에서 해당 슬롯 비우기
    4. 브로드캐스트 받으면 해당 플레이어 외형 변경
```

### 4.3 장비 정보 조회 플로우

```
[Client] 장비 창 열기 → C_EquipmentInfoRequest()
    ↓
[Server] S_EquipmentInfoReply 전송:
    - equipments: 4개 슬롯 정보 배열 (비어있으면 itemId=0)
    ↓
[Client] 처리:
    - 장비 창 UI에 4개 슬롯 표시
    - itemId != 0인 슬롯만 아이템 아이콘 표시
```

---

## 5. 에러 케이스

### 5.1 장착 실패 케이스
| errorMessage | 설명 | 처리 방법 |
|--------------|------|----------|
| "Empty inventory slot" | 인벤토리 슬롯이 비어있음 | 토스트 메시지 표시 |
| "Item is not equipment" | 장비 아이템이 아님 | 토스트 메시지 표시 |
| "Level requirement not met" | 레벨 요구사항 미달 | "레벨 부족" 메시지 표시 |
| "Inventory is full" | 인벤토리 가득 참 (장비 교체 시) | "인벤토리 공간 필요" 메시지 |
| "Failed to remove from inventory" | 인벤토리 제거 실패 | 일반 오류 메시지 |
| "Failed to equip item" | 장착 실패 | 일반 오류 메시지 |

### 5.2 해제 실패 케이스
| errorMessage | 설명 | 처리 방법 |
|--------------|------|----------|
| "Equipment slot is empty" | 이미 비어있는 슬롯 | 토스트 메시지 표시 |
| "Invalid equipped item" | 잘못된 장비 데이터 | 일반 오류 메시지 |
| "Inventory is full" | 인벤토리 가득 참 | "인벤토리 공간 필요" 메시지 |

---

## 6. 중요 구현 포인트

### 6.1 equipmentInstanceId의 중요성
- **각 장비는 고유한 equipmentInstanceId를 가짐**
- 같은 itemId라도 다른 equipmentInstanceId를 가질 수 있음 (강화 레벨이 다를 수 있음)
- 인벤토리 ↔ 장비 슬롯 이동 시 equipmentInstanceId를 반드시 추적해야 함
- **InventorySlotInfo에도 equipmentInstanceId 필드가 있음**

### 6.2 장비 교체 로직
서버는 자동으로 장비 교체를 처리:
1. 기존 장비를 먼저 인벤토리로 반환
2. 새 장비를 인벤토리에서 제거
3. 새 장비 장착
4. 실패 시 자동 롤백

**Client는 S_EquipItemReply의 returnedEquipmentSlot을 확인해야 함**

### 6.3 브로드캐스트 처리
- S_BroadcastPlayerEquipment는 룸 내 모든 플레이어에게 전송됨
- playerId를 확인하여 자신이면 무시 (이미 S_EquipItemReply로 처리됨)
- 다른 플레이어면 외형 변경 애니메이션 적용
- itemId=0이면 장비 해제, 아니면 장착

### 6.4 스탯 계산
서버는 장비 장착/해제 시 자동으로 스탯 재계산:
- atk, def, maxHp 값이 적용됨
- 레벨 당 + 10% 의 가중치가 붙음
- Client는 업데이트된 스탯을 별도로 받을 수 있음 (확인 필요)

### 6.5 게임 접속 시 초기화
- Enter game 시 서버가 자동으로 LoadEquipmentFromDB() 호출
- Client는 접속 후 C_EquipmentInfoRequest를 보내 초기 장비 상태를 받아야 함

---

## 7. Unity Client 구현 체크리스트

### 7.1 UI
- [ ] 장비 창 (4개 슬롯: 무기/투구/갑옷/바지)
- [ ] 장비 슬롯 클릭 시 C_UnequipItemRequest 전송
- [ ] 인벤토리 장비 아이템 클릭 시 C_EquipItemRequest 전송
- [ ] 장비 창 열기 시 C_EquipmentInfoRequest 전송
- [ ] 에러 메시지 토스트 UI

### 7.2 데이터 관리
- [ ] Equipment_data.json 파싱 및 로딩
- [ ] EquipmentManager (itemId → 장비 데이터 매핑)
- [ ] 현재 장착 중인 장비 상태 저장 (4개 슬롯)

### 7.3 패킷 핸들러
- [ ] S_EquipItemReply 핸들러
- [ ] S_UnequipItemReply 핸들러
- [ ] S_EquipmentInfoReply 핸들러
- [ ] S_BroadcastPlayerEquipment 핸들러

### 7.4 외형 시스템
- [ ] 플레이어 외형 렌더링 시스템
- [ ] 장비 변경 시 외형 업데이트
- [ ] 다른 플레이어 장비 브로드캐스트 처리

### 7.5 인벤토리 연동
- [ ] equipmentInstanceId 필드 추가 (InventorySlotInfo)
- [ ] 장비 아이템 구분 (EItemType.ITEM_TYPE_EQUIPMENT)
- [ ] 장비 장착/해제 시 인벤토리 UI 동기화

---

## 8. 테스트 시나리오

### 8.1 기본 장착/해제
1. 인벤토리에 장비 추가
2. 장비 장착 → 인벤토리에서 사라지는지 확인
3. 장비 창에 아이템 표시되는지 확인
4. 장비 해제 → 인벤토리로 돌아오는지 확인

### 8.2 장비 교체
1. 무기 A 장착
2. 무기 B 장착 시도 → A가 인벤토리로 돌아오고 B가 장착되는지 확인
3. equipmentInstanceId가 올바르게 추적되는지 확인

### 8.3 에러 케이스
1. 빈 슬롯 장착 시도 → 에러 메시지
2. 레벨 부족 장비 장착 시도 → 에러 메시지
3. 인벤토리 가득 찬 상태에서 장비 교체 → 에러 메시지

### 8.4 멀티플레이어
1. 다른 플레이어가 장비 변경 시 외형 변경 확인
2. 브로드캐스트 타이밍 확인

---

## 9. 서버 구현 참고사항

### 9.1 핵심 클래스
- **EquipmentManager**: 장비 데이터 관리 (Google Sheet & JSON)
- **EquipmentSystem**: 4개 슬롯 관리 및 장착/해제 로직
- **EquipmentRepository**: DB CRUD 작업
- **Player**: LoadEquipmentFromDB, EquipItem, UnequipItem 메서드

### 9.2 DB Stored Procedures
- `spGetCharacterEquipment`: 캐릭터 장비 로딩 (JOIN 사용)
- `spUpsertCharacterEquipment`: 장비 장착 상태 저장/업데이트
- `spDeleteCharacterEquipment`: 장비 해제
- `spGetEquipmentInstance`: 장비 인스턴스 조회
- `spInsertEquipmentInstance`: 새 장비 인스턴스 생성
- `spUpdateEquipmentInstance`: 강화 레벨 업데이트
- `spDeleteEquipmentInstance`: 장비 인스턴스 삭제
