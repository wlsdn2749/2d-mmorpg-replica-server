# NPC 시스템 Component-Based 구현 계획 (PRD/TaskList 기반)

## 1. 아키텍처 선택
**Component-Based 방식** (PRD 명시) - Player와 유사한 구조로 일관성 유지
- Monster의 복잡한 Service 패턴 대신 단순한 Component 조합 방식

## 2. 핵심 구현 방향
- **좌표 기반 상호작용** (PRD 명세: C_NPC_INTERACT with posX, posY)
- **Room에서 NPC 직접 관리** (별도 Service 불필요)
- **EntityKind::Npc 활용** (이미 정의됨)
- **코드/리소스 기반 데이터** (DB 미사용)

## 3. 파일 구조 (TaskList 기반)
```
GameServer/
├── NPC.h/cpp                      # 메인 NPC 클래스 + 타입 정의
├── NPCDialogComponent.h/cpp       # 대화 시스템
├── NPCShopComponent.h/cpp         # 상점 시스템
├── NPCQuestComponent.h/cpp        # 퀘스트 시스템
├── NPCManager.h/cpp               # 전역 데이터 관리
├── DialogManager.h/cpp            # 대화 데이터 관리
└── ShopDataManager.h/cpp          # 상점 데이터 관리
```

## 4. 주요 구현 특징
- **컴포넌트 조합**: 역할에 따라 Dialog/Shop/Quest 컴포넌트 선택적 생성
- **Room 통합**: 기존 Room 클래스에 NPC 관리 기능 추가
- **좌표 매핑**: _npcPositions로 좌표 → npcId 변환 지원
- **프로토콜 확장**: C_NPC_INTERACT, S_NPC_DIALOG 등 추가

## 5. 구현 단계 (TaskList 순서)
1. NPC.h/cpp - 메인 클래스 및 타입 정의
2. Dialog/Shop/Quest 컴포넌트 구현
3. Room에 좌표 기반 NPC 관리 기능 추가
4. 프로토콜 정의 및 패킷 핸들러 구현
5. NPCManager/DialogManager/ShopDataManager 구현

## 6. 상세 설계

### 6.1 NPC 메인 클래스 구조 (NPC.h에 타입 정의 포함)
```cpp
// NPC 관련 타입 정의
enum class ENPCRole : uint8_t {
    Dialog = 0,      // 단순 대화형
    Shop = 1,        // 상점형
    Quest = 2,       // 퀘스트형
    Mixed = 3        // 복합형 (Dialog + Shop/Quest)
};

enum class ENPCState : uint8_t {
    Idle = 0,
    Talking = 1,
    Trading = 2
};

enum class ENPCInteractionType : uint8_t {
    Talk = 0,
    Shop = 1,
    Quest = 2
};

struct NPCConfig {
    int npcId;
    ENPCRole role;
    string name;
    int x, y;
    int dialogId;
    int shopId;
    vector<int> questIds;
};

// NPC 메인 클래스
class NPC {
    EntityCore core;                    // 위치, 방향, EntityKind::Npc
    string name;
    ENPCRole role;                      // Dialog, Shop, Quest, Mixed
    ENPCState state;                    // 현재 상태
    
    // 컴포넌트들 (역할에 따라 선택적 생성)
    std::unique_ptr<NPCDialogComponent> _dialog;
    std::unique_ptr<NPCShopComponent> _shop;
    std::unique_ptr<NPCQuestComponent> _quest;

public:
    void Initialize(const NPCConfig& config);
    void HandleInteraction(int playerId, ENPCInteractionType type, int param1 = 0);
    bool IsNearby(const Pos2& playerPos, int maxDistance = 1) const;
    
    // Getters
    int GetNPCId() const { return core.id; }
    const string& GetName() const { return name; }
    ENPCRole GetRole() const { return role; }
    ENPCState GetState() const { return state; }
    Pos2 GetPosition() const { return core.pos; }
};
```

### 6.2 Room NPC 관리 확장
```cpp
class Room {
    // 기존 멤버들...
    std::unordered_map<int, NPC> _npcs;           // npcId -> NPC
    std::unordered_map<Pos2, int> _npcPositions;  // 좌표 -> npcId 매핑

public:
    void LoadNPCs();  // 맵 로딩 시 호출
    void HandleNPCInteraction(int playerId, int posX, int posY);  // PRD 명세 따름
    NPC* FindNPCByPosition(int x, int y);
    NPC* FindNPCById(int npcId);
};
```

### 6.3 프로토콜 정의 (PRD 명세)
```protobuf
// NPC 상호작용 요청 (PRD 명세)
message C_NPC_INTERACT {
    int32 posX = 1;
    int32 posY = 2;
}

// NPC 응답들
message S_NPC_DIALOG {
    int32 npcId = 1;
    string text = 2;
    repeated string choices = 3;
}

message S_NPC_SHOP_OPEN {
    int32 npcId = 1;
    repeated ShopItemInfo items = 2;
}

message S_NPC_QUEST_LIST {
    int32 npcId = 1;
    repeated QuestInfo quests = 2;
}

message S_NPC_ERROR {
    string errorMessage = 1;
}
```

### 6.4 컴포넌트 구조

#### NPCDialogComponent
```cpp
class NPCDialogComponent {
public:
    void Initialize(int dialogTreeId);
    void StartDialog(int playerId);
    void HandleChoice(int playerId, int choiceId);
    
private:
    int _dialogTreeId;
    std::unordered_map<int, DialogNode> _nodes;
};
```

#### NPCShopComponent
```cpp
class NPCShopComponent {
public:
    void Initialize(int shopId);
    void ShowShop(int playerId);
    bool ProcessPurchase(int playerId, int itemId, int quantity);
    bool ProcessSell(int playerId, int itemId, int quantity);
    
private:
    int _shopId;
    vector<ShopItem> _items;
};
```

#### NPCQuestComponent
```cpp
class NPCQuestComponent {
public:
    void Initialize(const vector<int>& questIds);
    void CheckQuestStatus(int playerId);
    bool GiveQuest(int playerId, int questId);
    bool CompleteQuest(int playerId, int questId);
    
private:
    vector<int> _availableQuests;
    vector<int> _completableQuests;
};
```

## 7. 구현 흐름

### 7.1 초기화 흐름
1. **Room::LoadNPCs()** → NPCManager에서 해당 맵의 NPC 설정 로드
2. **NPC::Initialize()** → 역할에 따라 필요한 컴포넌트 생성
3. **Room에 등록** → _npcs와 _npcPositions에 추가

### 7.2 상호작용 흐름
1. **클라이언트** → C_NPC_INTERACT(posX, posY)
2. **Room::HandleNPCInteraction()** → 좌표로 NPC 검색
3. **NPC::HandleInteraction()** → 컴포넌트별 처리
4. **응답 전송** → S_NPC_DIALOG/SHOP_OPEN/QUEST_LIST

## 8. 핵심 특징

### 8.1 장점
- **PRD 명세 준수**: 좌표 기반 상호작용
- **기존 패턴 일관성**: Player와 유사한 Component 구조
- **단순성**: Monster보다 훨씬 단순한 구조
- **확장성**: 새 컴포넌트 쉽게 추가 가능
- **메모리 효율**: 필요한 컴포넌트만 생성

### 8.2 Monster vs NPC 차이점
- **Monster**: 복잡한 Service + Multiple Systems (실시간 AI)
- **NPC**: 단순한 Component-Based (트리거 기반)

## 9. 테스트 계획 (TaskList 기반)
- [ ] 특정 위치에서 NPC와 상호작용이 정상적으로 수행되는가?
- [ ] 각 Component별 응답이 클라이언트에 정확하게 전달되는가?
- [ ] 좌표가 잘못되었을 때 적절한 에러 응답을 반환하는가?
- [ ] 여러 역할을 동시에 가진 NPC가 정상 동작하는가?
- [ ] NPCManager가 맵 단위로 NPC를 정확히 로드/등록하는가?