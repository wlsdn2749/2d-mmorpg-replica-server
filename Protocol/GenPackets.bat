pushd %~dp0

REM Proto 파일들 생성
set PROTOC="../third_party\grpc\install\bin\protoc.exe"
set GRPC_PLUGIN_CPP="../third_party\grpc\install\bin\grpc_cpp_plugin.exe"
set GRPC_PLUGIN_CS="../third_party\grpc\install\bin\grpc_csharp_plugin.exe"
set OUT_DIR_CPP="./"
set OUT_DIR_CS="./"

REM %PROTOC% --cpp_out=%OUT_DIR_CPP% --grpc_out=%OUT_DIR_CPP% --plugin=protoc-gen-grpc=%GRPC_PLUGIN_CPP% ./route_guide.proto
REM %PROTOC% --cpp_out=%OUT_DIR_CPP% --grpc_out=%OUT_DIR_CPP% --plugin=protoc-gen-grpc=%GRPC_PLUGIN_CPP% ./helloworld.proto

%PROTOC% --csharp_out=%OUT_DIR_CS% --grpc_out=%OUT_DIR_CS% --plugin=protoc-gen-grpc=%GRPC_PLUGIN_CS% ./greet.proto
%PROTOC% --csharp_out=%OUT_DIR_CS% --grpc_out=%OUT_DIR_CS% --plugin=protoc-gen-grpc=%GRPC_PLUGIN_CS% ./auth.proto


IF ERRORLEVEL 1 PAUSE

XCOPY /Y greet.cs "../gRPC-dummy-client\Protocols"
XCOPY /Y greetGrpc.cs "../gRPC-dummy-client\Protocols"
XCOPY /Y greet.cs "../AuthServer\Protocols"
XCOPY /Y greetGrpc.cs "../AuthServer\Protocols"

XCOPY /Y auth.cs "../gRPC-dummy-client\Protocols"
XCOPY /Y authGrpc.cs "../gRPC-dummy-client\Protocols"
XCOPY /Y auth.cs "../AuthServer\Protocols"
XCOPY /Y authGrpc.cs "../AuthServer\Protocols"


DEL /Q /F *.pb.h
DEL /Q /F *.pb.cc
DEL /Q /F *.h
DEL /Q /F *.cc

DEL /Q /F *.cs

PAUSE