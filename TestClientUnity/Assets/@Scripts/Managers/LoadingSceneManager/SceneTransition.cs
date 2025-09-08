using System;
using UnityEngine;
using UnityEngine.SceneManagement;

public static class SceneTransition
{
    public static void RunAfterGameplaySceneLoaded(Action work)
    {
        void Handler(Scene s, LoadSceneMode m)
        {
            if (s.name == LoadingSceneManager.LOADING_SCENE_NAME) return; // 로딩씬 패스
            SceneManager.sceneLoaded -= Handler;
            work?.Invoke();
        }

        // 혹시 이미 게임 씬이 활성이라면 즉시 실행
        var active = SceneManager.GetActiveScene();
        if (active.IsValid() && active.name != LoadingSceneManager.LOADING_SCENE_NAME)
        {
            work?.Invoke();
            return;
        }
        SceneManager.sceneLoaded += Handler;
    }
}
