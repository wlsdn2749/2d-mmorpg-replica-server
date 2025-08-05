using Grpc.Net.Client;
using GrpcGreeter;

var channel = GrpcChannel.ForAddress("http://localhost:5001");
var client = new Greeter.GreeterClient(channel);

var reply = await client.SayHelloAsync(new HelloRequest { Name = "Jinwoo" });
Console.WriteLine("Server said: " + reply.Message);