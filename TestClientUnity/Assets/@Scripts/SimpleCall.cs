using UnityEngine;
using System.Threading.Tasks;
using Grpc.Net.Client;
using Cysharp.Net.Http;
using Mmorpg2d.Auth;

public class SimpleCall : MonoBehaviour
{
    
    public static SimpleCall Instance { get; private set; }
    void Awake()
    {
        // 중복 인스턴스 방지
        if (Instance != null && Instance != this)
        {
            Destroy(gameObject);
            return;
        }
        Instance = this;
    }
    public string jwt = "";
    
    private async void Start()
    {
        await CallGrpc();
    }

    private async Task CallGrpc()
    {
        // gRPC 채널 생성
        using var handler = new YetAnotherHttpHandler() { Http2Only = true }; // Unity에서 HttpClient 호환을 위해 필요
        using var channel = GrpcChannel.ForAddress("http://182.231.5.187:8080", new GrpcChannelOptions
        {
            HttpHandler = handler,
            DisposeHttpClient = true
        });

        var client = new Auth.AuthClient(channel);

        try
        {
            var reply = await client.LoginAsync(new LoginRequest() { Email = "123@123.com", Password = "1234"});
            Debug.Log("gRPC 응답: " + reply.Success + "\ngRPC Detail" + reply.Detail + "\nJwt" + reply.Jwt);
            jwt = reply.Jwt;
        }
        catch (System.Exception ex)
        {
            Debug.LogError("gRPC 오류: " + ex.Message); 
        }
    }
}