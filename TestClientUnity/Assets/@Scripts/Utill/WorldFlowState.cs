using UnityEngine;

public static class WorldFlowState
{
    public static bool HasEnteredWorld { get; set; } = false; // 최초 접속 여부
    public static bool OnCharacterChange { get; set; } = false; // 월드에서 캐릭터 선택화면으로 돌아갈 때
    public static bool TransitionInProgress { get; set; } = false; // Begin~Commit 구간
    public static int ActiveMapId { get; set; } = -1;          // 현재 맵
    public static int? CurrentTransitionId { get; set; } = null;

    public static void ResetForBegin(int mapId, int transitionId)
    {
        TransitionInProgress = true;
        CurrentTransitionId = transitionId;
        ActiveMapId = mapId; // 타겟 맵
    }

    public static void FinishSceneActivated()
    {
        // 씬 활성 직후 Ready 보낼 때 호출
        TransitionInProgress = false;
    }
}