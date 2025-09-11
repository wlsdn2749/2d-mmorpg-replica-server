using System.Collections.Generic;
using UnityEngine;
using UnityEngine.Pool;

public class ObjectPoolManager : MonoBehaviour
{
    [System.Serializable]
    private class ObjectInfo
    {
        public string objectName;
        public GameObject prefab;
        public int count;
    }

    public static ObjectPoolManager Instance;
    public bool IsReady { get; private set; }

    [SerializeField] private ObjectInfo[] objectInfos;

    private Dictionary<string, IObjectPool<GameObject>> objectPoolDic = new Dictionary<string, IObjectPool<GameObject>>();
    private Dictionary<string, GameObject> prefabDic = new Dictionary<string, GameObject>();

    private void Awake()
    {
        if (Instance == null)
        {

            Instance = this;
            DontDestroyOnLoad
                (this.gameObject);
        }
        else
        {
            Destroy(this.gameObject);
            return;
        }

        Init();
    }

    private void Init()
    {
        IsReady = false;

        foreach (var info in objectInfos)
        {
            if (prefabDic.ContainsKey(info.objectName))
            {
                Debug.LogWarning($"{info.objectName} 은 이미 등록된 오브젝트입니다.");
                continue;
            }

            prefabDic.Add(info.objectName, info.prefab);

            var pool = new ObjectPool<GameObject>(
                () => CreatePooledItem(info.objectName),
                OnTakeFromPool,
                OnReturnedToPool,
                OnDestroyPoolObject,
                true,
                info.count,
                info.count
            );

            objectPoolDic.Add(info.objectName, pool);

            // 미리 생성해서 풀에 채워넣기
            for (int i = 0; i < info.count; i++)
            {
                var obj = pool.Get();
                obj.GetComponent<PoolAble>().ReleaseObject();
            }
        }

        Debug.Log("오브젝트풀 준비 완료");
        IsReady = true;
    }

    private GameObject CreatePooledItem(string objectName)
    {
        GameObject poolObject = Instantiate(prefabDic[objectName], this.transform);
        poolObject.GetComponent<PoolAble>().Pool = objectPoolDic[objectName];
        return poolObject;
    }

    private void OnTakeFromPool(GameObject obj)

    {
        obj.SetActive(true);
    }

    private void OnReturnedToPool(GameObject obj)
    {
        obj.SetActive(false);
    }

    private void OnDestroyPoolObject(GameObject obj)
    {
        Destroy(obj);
    }

    public GameObject GetObject(string objectName)
    {
        if (!objectPoolDic.ContainsKey(objectName))
        {
            Debug.LogError($"{objectName} 은 오브젝트풀에 등록되어 있지 않습니다.");
            return null;
        }

        return objectPoolDic[objectName].Get();
    }
    public void ReleaseObject(GameObject obj)
    {
        var poolable = obj.GetComponent<PoolAble>();
        if (poolable == null)
        {
            Debug.LogError("PoolAble 컴포넌트가 없습니다. 풀에 반환할 수 없습니다.");
            return;
        }

        poolable.ReleaseObject();
    }
}
