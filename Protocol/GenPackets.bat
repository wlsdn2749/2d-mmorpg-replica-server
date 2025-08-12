pushd %~dp0

REM Auth Proto 파일들 생성
set PROTOC="../third_party\grpc\install\bin\protoc.exe"
set GRPC_PLUGIN_CPP="../third_party\grpc\install\bin\grpc_cpp_plugin.exe"
set GRPC_PLUGIN_CS="../third_party\grpc\install\bin\grpc_csharp_plugin.exe"
set OUT_DIR_CPP="./"
set OUT_DIR_CS="./"

%PROTOC% --csharp_out=%OUT_DIR_CS% --grpc_out=%OUT_DIR_CS% --plugin=protoc-gen-grpc=%GRPC_PLUGIN_CS% ./greet.proto
%PROTOC% --csharp_out=%OUT_DIR_CS% --grpc_out=%OUT_DIR_CS% --plugin=protoc-gen-grpc=%GRPC_PLUGIN_CS% ./auth.proto

IF ERRORLEVEL 1 PAUSE

XCOPY /Y auth.cs "../gRPC-dummy-client\Protocol"
XCOPY /Y authGrpc.cs "../gRPC-dummy-client\Protocol"
XCOPY /Y auth.cs "../AuthServer\Protocol"
XCOPY /Y authGrpc.cs "../AuthServer\Protocol"


REM Game Proto 파일들 생성
protoc.exe -I=./ --cpp_out=./ ./Protocol.proto
protoc.exe -I=./ --csharp_out=./ ./Protocol.proto

REM 자동화된 Handler 생성
GenPackets.exe --path=./Protocol.proto --output=ClientPacketHandler --recv=C_ --send=S_
GenPackets.exe --path=./Protocol.proto --output=ServerPacketHandler --recv=S_ --send=C_
GenPackets.exe --path=./Protocol.proto --output=ServerPacketManager --ext=cs --template=PacketManager.cs --recv=S_ --send=C_

REM 각자 위치에 옮겨주기

REM IOCP C++ GameServer
XCOPY /Y Protocol.pb.h "../GameServer"
XCOPY /Y Protocol.pb.cc "../GameServer"
XCOPY /Y ClientPacketHandler.h "../GameServer"

REM C# DummyClient
XCOPY /Y Protocol.cs "../DummyClientCS/Protocol"

XCOPY /Y ServerPacketManager.cs "../DummyClientCS/Packet"

DEL /Q /F *.pb.h
DEL /Q /F *.pb.cc
DEL /Q /F *.h
DEL /Q /F *.cc

DEL /Q /F *.cs

PAUSE