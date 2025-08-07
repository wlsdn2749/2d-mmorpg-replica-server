#include "pch.h"
#include "NetAddress.h"


/*---------------
 *	NetAddress
 --------------*/
NetAddress::NetAddress(SOCKADDR_IN sockaddr) : _sockAddr(sockaddr)
{
}

NetAddress::NetAddress(wstring ip, uint16 port)
{
	::memset(&_sockAddr, 0, sizeof(_sockAddr));
	_sockAddr.sin_family = AF_INET;
	_sockAddr.sin_addr = ip2Address(ip.c_str());
	_sockAddr.sin_port = ::htons(port);
}

wstring NetAddress::GetIpAddress()
{
	WCHAR buffer[100];
	::InetNtopW(AF_INET, &_sockAddr.sin_addr, buffer, len32(buffer));
	return wstring(buffer);
}

IN_ADDR NetAddress::ip2Address(const WCHAR* ip)
{
	IN_ADDR address;
	::InetPtonW(AF_INET, ip, &address);
	return address;
}
