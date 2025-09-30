using System.Collections.Generic;
using Google.Protobuf;
using UnityEngine;

public class PacketQueue
{
    public static PacketQueue Instance { get; } = new PacketQueue();

    private Queue<IMessage> _packetQueue = new Queue<IMessage>();
    private object _lock = new object();

    public void Push(IMessage packet)
    {
        lock (_lock)
        {
            _packetQueue.Enqueue(packet);
        }
    }

    public IMessage Pop()
    {
        lock (_lock)
        {
            if (_packetQueue.Count == 0)
                return null;

            return _packetQueue.Dequeue();
        }
    }

    public List<IMessage> PopAll()
    {
        List<IMessage> list = new List<IMessage>();

        lock (_lock)
        {
            while(_packetQueue.Count > 0)
                list.Add(_packetQueue.Dequeue());
        }

        return list;
    }
}
