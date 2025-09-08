using UnityEngine;

public static class SessionStore
{
    public static ulong SelectedCharId;
    public static bool EnterGameOk;
    public static int TargetSceneIndex;       // int·Î º¯°æ
    public static Vector2? OverrideSpawnPos;

    public static void Reset()
    {
        EnterGameOk = false;
        TargetSceneIndex = -1;
        OverrideSpawnPos = null;
        SelectedCharId = 0;
    }
}
