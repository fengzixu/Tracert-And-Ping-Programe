#include<iostream>
#include<WinSock2.h>
#include<WS2tcpip.h>
const BYTE ICMP_ECHO_REQUEST = 8;
const BYTE ICMP_ECHO_REPLY = 0;
const BYTE ICMP_TIMEOUT = 11;

const int DEF_ICMP_DATA_SIZE = 32;			//icmp报文的数据长度
const int MAX_ICMP_PACKET_SIZE = 1024;
const DWORD DEF_ICMP_TIMEOUT =3000;
const int DEF_MAX_HOP = 30;					//最大跳数
char HostName[255] = {0};		//接受输入的主机域名或者IP地址
using namespace std;
//创建ICMP报文头部结构体
//#pragma comment( lib, "ws2_32.lib" )    
typedef struct{

	BYTE type;  //报文类型字段
	BYTE code;	//报文代码字段
	USHORT cksum; //校验和
	USHORT id;	//和报文有关的线程ID
	USHORT seq; //16位报文序列号

}ICMP_HEADER;

//IP报头
typedef struct
{
    unsigned char hdr_len:4;        //4位头部长度
    unsigned char version:4;        //4位版本号
    unsigned char tos;            //8位服务类型
    unsigned short total_len;        //16位总长度
    unsigned short identifier;        //16位标识符
    unsigned short frag_and_flags;    //3位标志加13位片偏移
    unsigned char ttl;            //8位生存时间
    unsigned char protocol;        //8位上层协议号
    unsigned short checksum;        //16位校验和
    unsigned long sourceIP;        //32位源IP地址
    unsigned long destIP;        //32位目的IP地址
} IP_HEADER;

typedef struct{

	USHORT usSeqNo;
	DWORD dwRoundTripTime;
	in_addr ipadress;
}DECODE_RESULT;


//计算网际校验和函数
USHORT checksum(USHORT *pBuf,int iSize)
{
    unsigned long cksum=0;
    while(iSize>1)
    {
        cksum+=*pBuf++;
        iSize-=sizeof(USHORT);
    }
    if(iSize)
    {
        cksum+=*(UCHAR *)pBuf;
    }
    cksum=(cksum>>16)+(cksum&0xffff);
    cksum+=(cksum>>16);
    return (USHORT)(~cksum);
}

BOOL DecodeIcmpResponse(char * pBuf,int iPacketSize,DECODE_RESULT &DecodeResult,BYTE ICMP_ECHO_REPLY,BYTE  ICMP_TIMEOUT)
{
	IP_HEADER *piphdr = (IP_HEADER*)pBuf;
	int iphdrlen = piphdr->hdr_len*4;		//IP数据报的首部长度
	if(iPacketSize < (int)(iphdrlen+sizeof(ICMP_HEADER)))
		return FALSE;

	ICMP_HEADER *ptr_icmp = (ICMP_HEADER*)(pBuf+iphdrlen);
	USHORT usID, usSquNo;
	if(ptr_icmp->type == ICMP_ECHO_REPLY)
	{
		usID = ptr_icmp->id;
		usSquNo = ptr_icmp->seq;
	}
	else if(ptr_icmp->type == ICMP_TIMEOUT)
	{
		char *innerip = pBuf+iphdrlen+sizeof(ICMP_HEADER);
		int inneriplen = ((IP_HEADER*)innerip)->hdr_len*4;
		ICMP_HEADER *innericmp = (ICMP_HEADER*)(innerip+inneriplen);
		usID = innericmp->id;
		usSquNo = innericmp->seq;
	}
	else
		return false;


	if(usID!=(USHORT)GetCurrentProcessId()||usSquNo!=DecodeResult.usSeqNo)
		return false;

	DecodeResult.ipadress.S_un.S_addr = piphdr->sourceIP;
	DecodeResult.dwRoundTripTime = GetTickCount()-DecodeResult.dwRoundTripTime;//计算往返时间

	if(ptr_icmp->type == ICMP_ECHO_REPLY||ICMP_TIMEOUT)
	{
		if(DecodeResult.dwRoundTripTime){
			cout <<"	"<<DecodeResult.dwRoundTripTime<<"ms"<<flush;
		}
		else
			cout <<"	"<<"1ms"<<flush;
	}
	return true;
}

int main()
{

	cout << "请输入目的目的主机的IP地址或者域名>>>" << endl;
	cin >> HostName;


	//初始化套接字库

	WSADATA m_wsadata;
	int initFlag = WSAStartup(MAKEWORD(2,2),&m_wsadata);

	if(initFlag != 0)																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																						
	{
		cout <<"初始化错误"<<endl;
		return 0;
	}

	//将字符串形式的IP地址转化为程序中方便操作的u_long类型
	u_long long_ip = inet_addr(HostName);

	if(long_ip == INADDR_NONE)			//如果按照IP地址解析失败
	{
		//按照主机名解析
		hostent *ptrhostname = gethostbyname(HostName); 
		if(ptrhostname == NULL)
		{
			cout << "解析主机名失败,输入的地址名无效" <<endl;
			WSACleanup();
			return 0;
		}
		else{
			long_ip  = (*((in_addr*)ptrhostname->h_addr_list[0])).S_un.S_addr;
			//目的主机的IP地址是
			cout << "目的主机的IP地址是 ：";
			cout <<inet_ntoa(*(in_addr*)ptrhostname->h_addr_list) <<endl;

		}
	}
	
	//存储目的主机的地址信息，还有接收信息的地址信息
	sockaddr_in sendinfo;
	ZeroMemory(&sendinfo,sizeof(sockaddr_in));
	cout << "Tracing roote to    "<<HostName<<"    with a maximun of 30 hops.\n" << endl;
	//填充目的主机地址信息
	sendinfo.sin_family = AF_INET;
	sendinfo.sin_addr.S_un.S_addr = long_ip;

	SOCKET sockRaw = WSASocket(AF_INET,SOCK_RAW,IPPROTO_ICMP,NULL,0,WSA_FLAG_OVERLAPPED);
	if(INVALID_SOCKET == WSAGetLastError())
	{
		cout <<"创建套接字失败"<<endl;
		WSACleanup();
		return 0;
	}
	//设置超时时间
	int TimeOut = 3000;

	//设置主机发送和接收数据的超时时间
	setsockopt(sockRaw,SOL_SOCKET,SO_SNDTIMEO,(char*)&TimeOut,sizeof(TimeOut));
	setsockopt(sockRaw,SOL_SOCKET,SO_RCVTIMEO,(char*)&TimeOut,sizeof(TimeOut));
	char IcmpSendBuff[DEF_ICMP_DATA_SIZE+sizeof(ICMP_HEADER)] = {0};		//定义发送缓冲区
	char IcmpRecvBuff[MAX_ICMP_PACKET_SIZE] = {0};				//定义接收缓冲区

	//初始化ICMP报文头部
	//依靠先申请一段内存空间，然后再对内存空间进行操作，把结构体中的成员赋值进去
	ICMP_HEADER* ptr_icmpheader = (ICMP_HEADER*)IcmpSendBuff;

	ptr_icmpheader->type = ICMP_ECHO_REQUEST;		//设定报文类型
	ptr_icmpheader->code = 0;
	ptr_icmpheader->id = (USHORT)GetCurrentProcessId();
	memset(IcmpSendBuff+sizeof(ICMP_HEADER),'E',DEF_ICMP_DATA_SIZE);

	USHORT usSeqNo = 0;
	int TTL = 1;
	BOOL QuitFlag = FALSE;
	int maxhop = DEF_MAX_HOP;
	DECODE_RESULT DecodeResult;
	while(!QuitFlag&&maxhop--)				//开始追踪路由测试
	{
		//设置每次发送的IP数据报的TTL
		setsockopt(sockRaw,IPPROTO_IP,IP_TTL,(char*)&TTL,sizeof(TTL));
		cout << TTL << flush;
		ptr_icmpheader->cksum = 0;
		ptr_icmpheader->seq = htons(usSeqNo++);
		ptr_icmpheader->cksum = checksum((USHORT*)IcmpSendBuff,sizeof(ICMP_HEADER)+DEF_ICMP_DATA_SIZE);


		DecodeResult.usSeqNo = ((ICMP_HEADER*)IcmpSendBuff)->seq;
		DecodeResult.dwRoundTripTime = GetTickCount();
		//发送数据报
		sendto(sockRaw,IcmpSendBuff,sizeof(IcmpSendBuff),0,(sockaddr*)&sendinfo,sizeof(sendinfo));
		sockaddr_in recvinfo;
		int recvlen = 0;
		int fromlen = sizeof(recvinfo);
		//准备接受发回来的ICMP数据报
		while(1)
		{
			//memset(IcmpRecvBuff,0,sizeof(IcmpRecvBuff));
			recvlen = recvfrom(sockRaw,IcmpRecvBuff,MAX_ICMP_PACKET_SIZE,0,(SOCKADDR*)&recvinfo,&fromlen);
			if(recvlen!=SOCKET_ERROR)
			{
				if(DecodeIcmpResponse(IcmpRecvBuff,recvlen,DecodeResult,ICMP_ECHO_REPLY,ICMP_TIMEOUT))
				{
					if(DecodeResult.ipadress.S_un.S_addr == sendinfo.sin_addr.S_un.S_addr)
						QuitFlag = true;
					cout << "\t" << inet_ntoa(DecodeResult.ipadress)<<endl;
					break;
				}
			}
			else if (WSAGetLastError() == WSAETIMEDOUT)
			{
				cout <<"		*"<<"	"<<"Request timed out" <<endl;
				break;
			}
			else
				break;
		}
		//准备发送下一次数据报
		TTL++;
	}
	return 0;
}