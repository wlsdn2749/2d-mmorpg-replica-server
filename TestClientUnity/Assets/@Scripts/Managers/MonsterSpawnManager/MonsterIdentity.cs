using UnityEngine;
using Google.Protobuf.Protocol;

public class MonsterIdentity : MonoBehaviour
{
    public int MonsterId { get; private set; }
    public int MonsterTypeId { get; private set; }
    public EDirection Direction { get; private set; }

    Animator _anim;

    void Awake() { _anim = GetComponent<Animator>(); }

    public void Init(int monsterId, int typeId, EDirection dir)
    {
        MonsterId = monsterId;
        MonsterTypeId = typeId;
        Direction = dir;
        if (_anim) _anim.SetInteger("Dir", (int)dir);
    }
}