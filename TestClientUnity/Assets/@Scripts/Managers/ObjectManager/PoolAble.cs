using UnityEngine;
using UnityEngine.Pool;

public class PoolAble : MonoBehaviour
{
    bool _released = false;

    public IObjectPool<GameObject> Pool { get; set; }
    public void ReleaseObject()
    {
        if (_released)
        {
            // 디버깅용 로그만 찍고 무시
            Debug.LogWarning($"[PoolAble] Double release detected on {name}");
            return;
        }

        _released = true;
        Pool.Release(gameObject);
    }
    // 풀에서 다시 꺼낼 때 호출할 초기화 포인트가 있어야 함
    public void OnTakenFromPool()
    {
        _released = false;
    }
}

