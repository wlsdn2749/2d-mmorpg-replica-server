#pragma once

/*---------------
 *	NetAddress
 --------------*/





class NetAddress
{
public:
	NetAddress() = default;
	NetAddress(SOCKADDR_IN sockaddr);
	NetAddress(wstring ip, uint16 port); // wstring은 하나가 2byte인 string임

	SOCKADDR_IN&	GetSockAddr() { return _sockAddr; }
	wstring			GetIpAddress();
	uint16			GetPort() {return ::ntohs(_sockAddr.sin_port); }

public:
	static IN_ADDR ip2Address(const WCHAR* ip);
private:
	SOCKADDR_IN _sockAddr = {};
};

