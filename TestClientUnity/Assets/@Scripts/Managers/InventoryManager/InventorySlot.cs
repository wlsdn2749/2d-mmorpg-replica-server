using System;


[Serializable]
public class InventorySlot
{
    public int slotIndex;
    public int itemId;
    public int count;
    public bool isQuickslot;
    public bool IsEmpty => itemId <= 0 || count <= 0;
}