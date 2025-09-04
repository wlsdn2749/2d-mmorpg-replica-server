using System;
using System.Collections.Concurrent;
using UnityEngine;

public class MainThreadDispatcher : MonoBehaviour
{
    static readonly ConcurrentQueue<Action> Q = new();
    [RuntimeInitializeOnLoadMethod(RuntimeInitializeLoadType.BeforeSceneLoad)]
    static void Boot()
    {
        var go = new GameObject("[MainThreadDispatcher]");
        DontDestroyOnLoad(go);
        go.AddComponent<MainThreadDispatcher>();
    }
    public static void Enqueue(Action a) => Q.Enqueue(a);
    void Update() { while (Q.TryDequeue(out var a)) a?.Invoke(); }
}