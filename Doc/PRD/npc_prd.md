# 🧾 NPC 시스템 PRD (Component-based)

## 1. 🎯 목적

MMORPG 게임 내 NPC는 고정 위치에 존재하며, 플레이어와의 상호작용을 통해 **대화, 상점, 퀘스트** 기능을 제공합니다.  
본 시스템은 **Component-based 구조**로 구현되며, 각 기능은 독립적으로 모듈화됩니다.

---

## 2. 📦 디렉터리 및 클래스 구성

```

GameServer/
├── NPC.h / NPC.cpp                  # 메인 NPC 클래스
├── NPCTypes.h                       # NPC 타입, 상태, 설정 정의
├── NPCDialogComponent.h / .cpp     # 대화 시스템
├── NPCShopComponent.h / .cpp       # 상점 시스템
├── NPCQuestComponent.h / .cpp      # 퀘스트 시스템
├── NPCManager.h / .cpp             # NPC 전역 관리
├── NPCRepository.h / .cpp          # DB 접근 레이어 (현재 미사용)
├── DialogManager.h / .cpp          # 대화 데이터 관리
└── ShopDataManager.h / .cpp        # 상점 데이터 관리

````

---

## 3. 🧩 NPC Entity 및 Component 구조

### 3.1. EntityCore

```cpp
struct EntityCore
{
	EntityId	id{};
	EntityKind	kind{};
	Pos2		pos{};
	Protocol::EDirection	dir{ Protocol::EDirection::DIR_DOWN };
};
````

### 3.2. EntityKind 확장

```cpp
enum class EntityKind : uint8 {
	Player,
	Monster,
	Npc,
	ItemObject
};
```

### 3.3. NPC Component 종류

| Component 이름         | 설명           |
| -------------------- | ------------ |
| `NPCDialogComponent` | 플레이어와의 대화 처리 |
| `NPCShopComponent`   | 아이템 판매 기능 처리 |
| `NPCQuestComponent`  | 퀘스트 시작/완료 처리 |

---

## 4. 🧠 NPC 역할 정의

현재 정의된 역할은 다음과 같습니다:

| 역할 이름   | 포함 Component                              |
| ------- | ----------------------------------------- |
| 단순 대화형  | `NPCDialogComponent`                      |
| 상점형 NPC | `NPCDialogComponent`, `NPCShopComponent`  |
| 퀘스트 제공형 | `NPCDialogComponent`, `NPCQuestComponent` |

---

## 5. 🗺️ NPC 초기화

* 맵 로딩 시, NPC 목록을 직접 코드/리소스로부터 로딩
* 각 NPC는 `EntityCore`를 생성하고, 역할에 맞는 Component를 attach
* `NPCManager`에 등록됨

---

## 6. 🔁 상호작용 흐름

### 6.1. 클라이언트 → 서버 요청

```cpp
struct C_NPC_INTERACT
{
	int posX;
	int posY;
};
```

> 📌 `targetId` 생략: NPC는 고정 위치이므로, 좌표 기반으로 NPC를 식별

### 6.2. 서버 처리

1. 좌표 기준으로 NPC 검색
2. 해당 Entity가 `EntityKind::Npc`인지 확인
3. NPC의 Component 구조를 기반으로 상호작용 처리
4. 해당 기능에 맞는 응답 전송

---

## 7. 📤 서버 → 클라이언트 응답 예시

| 기능  | 응답 구조체                                           |
| --- | ------------------------------------------------ |
| 대화  | `S_NPC_DIALOG { string text; }`                  |
| 상점  | `S_NPC_SHOP_OPEN { vector<ItemInfo> items; }`    |
| 퀘스트 | `S_NPC_QUEST_LIST { vector<QuestInfo> quests; }` |

---

## 8. ⛔ DB 미사용

* 현재 NPC 관련 데이터는 모두 코드 또는 리소스 기반으로 로드
* `NPCRepository`는 존재하지만 사용되지 않음 (향후 확장 대비)

---

## 9. ✅ 테스트 항목

* [ ] 특정 위치에서 NPC와 상호작용이 정상적으로 수행되는가?
* [ ] 각 Component별 응답이 클라이언트에 정확하게 전달되는가?
* [ ] 좌표가 잘못되었을 때 적절한 에러 응답을 반환하는가?
* [ ] 여러 역할을 동시에 가진 NPC가 정상 동작하는가?
* [ ] `NPCManager`가 맵 단위로 NPC를 정확히 로드/등록하는가?

---

## 10. 📌 향후 고려사항 (TODO)

* DB 기반 NPC 정의 및 역할 확장 (`NPCRepository`)
* NPC별 스크립트 기반 동작 지원 (Lua 등)
* NPC별 애니메이션, 이펙트, 사운드 처리

