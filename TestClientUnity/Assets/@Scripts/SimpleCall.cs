using UnityEngine;
using System.Threading.Tasks;
using Grpc.Net.Client;
using Cysharp.Net.Http;
using Mmorpg2d.Auth;

public class SimpleCall : MonoBehaviour
{
    
    //public static SimpleCall Instance { get; private set; }
    //void Awake()
    //{
    //    // 중복 인스턴스 방지
    //    if (Instance != null && Instance != this)
    //    {
    //        Destroy(gameObject);
    //        return;
    //    }
    //    Instance = this;
    //}
    //public string jwt = "";

    //public async Task<bool> CallGrpc(Auth.AuthClient client, string id, string password )
    //{
    //    try
    //    {
    //        var reply = await client.LoginAsync(new LoginRequest() { Email = id, Password = password});
    //        Debug.Log("gRPC 응답: " + reply.Success + "\ngRPC Detail" + reply.Detail + "\nJwt" + reply.Jwt);
    //        jwt = reply.Jwt;
    //        return reply.Success;
    //    }
    //    catch (System.Exception ex)
    //    {
    //        Debug.LogError("gRPC 오류: " + ex.Message);
    //        return false;
    //    }
    //}
}