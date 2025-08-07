using AuthServer;
using AuthServer.Context;
using AuthServer.Contracts.Repository;

using Grpc.AspNetCore.Server;
using GrpcGreeter; // GreetGrpc.cs, Greet.cs가 생성되었을 때 사용
using Microsoft.AspNetCore.Builder;
using Microsoft.AspNetCore.Hosting;
using Microsoft.AspNetCore.Server.Kestrel.Core;
using Microsoft.Extensions.Configuration;
using Microsoft.Extensions.DependencyInjection;



namespace AuthServer.Server
{
    class Program
    {
        static void Main(String[] args)
        {
            var builder = WebApplication.CreateBuilder(args);

            // cfg 등록
            builder.Configuration.AddJsonFile("appsettings.json");

            // Dapper 서비스 등록.
            builder.Services.AddSingleton<DapperContext>();
            builder.Services.AddControllers();

            // DI
            builder.Services.AddSingleton<IUserRepository, UserRepository>();
            builder.Services.AddSingleton<IJwtTokenGenerator, JwtTokenGenerator>();

            // gRPC 서비스 등록
            builder.Services.AddGrpc();
            

            // Kestrel 서버 HTTPS/HTTP2 설정 (기본값으로 충분함)
            builder.WebHost.ConfigureKestrel(options =>
            {
                // gRPC는 HTTP/2를 필요로 함
                options.ListenAnyIP(8443, listenOptions =>
                {
                    listenOptions.Protocols = HttpProtocols.Http2;
                });
            });

            var app = builder.Build();

            // gRPC 서비스 매핑
            app.MapGrpcService<GreeterService>();
            app.MapGrpcService<AuthService>();

            // HTTP 요청이 gRPC가 아닌 경우 안내 메시지
            app.MapGet("/", () => "This server only supports gRPC. Use a gRPC client to communicate.");

            app.Run();
        }
    }
}

