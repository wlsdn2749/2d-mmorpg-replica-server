using UnityEngine;

public enum LoginEntryMode
{
    ColdStart,                 // 앱 처음 실행(로그인 필요)
    AfterLeaveToCharacterSelect, // 인게임 → 캐선 복귀 (세션 유지)
}

public static class LoginManagement
{
    public static LoginEntryMode NextLoginEntryMode = LoginEntryMode.ColdStart;

    // 어디서든 로그인씬으로 이동할 때 이걸 사용
    public static void GoToLogin(LoginEntryMode mode)
    {
        NextLoginEntryMode = mode;
        LoadingSceneManager.OnSceneActivated = null; // 혹시 남아있던 콜백 정리
        LoadingSceneManager.LoadScene("LoginScene");
    }
    public static void SetLoingEntryMode(LoginEntryMode mode)
    {
        NextLoginEntryMode = mode;
    }
}
