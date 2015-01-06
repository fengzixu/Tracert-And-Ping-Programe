//#include<iostream>
//#include<WinSock2.h>
//#include<string>
//#include<cstdio>
//
//using namespace std;
//#pragma comment(lib, "ws2_32.lib")
//const BYTE ICMP_ECHO_REQUEST = 8;
//const BYTE ICMP_ECHO_REPLY = 0;
//const BYTE ICMP_TIMEOUT = 11;
//const int DEF_ICMP_DATA_SIZE = 32;			//icmp报文的数据长度
//const int MAX_ICMP_PACKET_SIZE = 1024;
//typedef struct
//{
//	unsigned char hdr_len:4;        //4位头部长度
//	unsigned char version:4;        //4位版本号
//	unsigned char tos;            //8位服务类型
//	unsigned short total_len;        //16位总长度
//	unsigned short identifier;        //16位标识符
//	unsigned short frag_and_flags;    //3位标志加13位片偏移
//	unsigned char ttl;            //8位生存时间
//	unsigned char protocol;        //8位上层协议号
//	unsigned short checksum;        //16位校验和
//	unsigned long sourceIP;        //32位源IP地址
//	unsigned long destIP;        //32位目的IP地址
//} IP_HEADER;
//
//typedef struct{
//
//	BYTE type;  //报文类型字段
//	BYTE code;	//报文代码字段
//	USHORT cksum; //校验和
//	USHORT id;	//和报文有关的线程ID
//	USHORT seq; //16位报文序列号
//
//}ICMP_HEADER;
//
//unsigned short checkSum (char *pBuffer, int nLen)
//{
//    unsigned short nWord;
//    unsigned int nSum = 0;
//    int i;
//
//    //Make 16 bit words out of every two adjacent 8 bit words in the packet
//    //and add them up
//    for (i = 0; i < nLen; i = i + 2)
//    {
//        nWord =((pBuffer [i] << 8)& 0xFF00) + (pBuffer [i + 1] & 0xFF);
//        nSum = nSum + (unsigned int)nWord;    
//    }
//
//    //Take only 16 bits out of the 32 bit sum and add up the carries
//    while (nSum >> 16)
//    {
//        nSum = (nSum & 0xFFFF) + (nSum >> 16);
//    }
//
//    //One's complement the result
//    nSum = ~nSum;
//
//    return ((unsigned short) nSum);
//}
//
//
//int main()
//{
//	string DestIp;
//	//创建发送和接收缓冲区
//	char SendBuff[1024] = {0};
//	char RecvBuff[1024] = {0};
//	unsigned long totaltime = 0;
//	unsigned long minroundtime = -1;
//	unsigned long maxroundtime = 0;
//	unsigned long pingjuntime = 0;
//	unsigned long recvtime = 0;
//	unsigned long roundtime = 0;
//	int SendCount = 0;
//	int RecvCount = 0;
//	int losscount = 0;
//	int sendicmp_seq = 0;
//	//初始化套接字
//	WSADATA wsadata;
//	int initFlag = WSAStartup(MAKEWORD(2,2),&wsadata);
//
//	if(initFlag != 0)
//	{
//		cout<<"套接字初始化失败"<<endl;
//		WSACleanup();
//		return 0;
//	}
//	int  m_TimeOut = 1000;
//	//创建套接字
//	SOCKET sock_raw = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
//	if(GetLastError() == INVALID_SOCKET)
//	{
//		cout<<"套接字创建失败"<<endl;
//		WSACleanup();
//		return 0;
//	}
//	//设定套接字的超时时间，防止接收和发送消息的时候阻塞，使得程序不能进行
//
//	setsockopt(sock_raw,SOL_SOCKET,SO_SNDTIMEO,(char*)&m_TimeOut,sizeof(int));
//	setsockopt(sock_raw,SOL_SOCKET,SO_RCVTIMEO,(char*)&m_TimeOut,sizeof(int));
//
//	cout << "Ping>> ";
//	cin >> DestIp;
//
//
//	sockaddr_in destsockinfo;
//	sockaddr_in fromsockinfo;
//	destsockinfo.sin_family = AF_INET;
//	memset(&destsockinfo.sin_zero, 0, 8);
//	//确定目的IP地址
//	u_long long_ip = inet_addr(DestIp.c_str());
//
//	if(long_ip == INADDR_NONE)			//如果按照IP地址解析失败
//	{
//		//按照主机名解析
//		hostent *ptrhostname = gethostbyname(DestIp.c_str()); 
//		if(ptrhostname == NULL)
//		{
//			cout << "解析主机名失败,输入的地址名无效" <<endl;
//			WSACleanup();
//			return 0;
//		}
//		else{
//			long_ip  = (*((in_addr*)ptrhostname->h_addr_list[0])).S_un.S_addr;
//			//目的主机的IP地址是
//			cout << "目的主机的IP地址是 ：";
//			cout <<inet_ntoa(*(in_addr*)ptrhostname->h_addr_list) <<endl;
//
//		}
//	}
//	destsockinfo.sin_addr.S_un.S_addr = long_ip;
//	//循环发送四个ICMP请求回显数据报
//	while(SendCount<4)
//	{
//		SendCount++;
//		//给数据报填写要发送的信息
//		ICMP_HEADER * ptr_send_icmp = (ICMP_HEADER*)SendBuff;
//		ptr_send_icmp->cksum = 0;
//		ptr_send_icmp->type = 8;
//		ptr_send_icmp->code = 0;
//		ptr_send_icmp->id = (USHORT)GetCurrentProcessId();
//		ptr_send_icmp->seq = (USHORT)(sendicmp_seq++);
//
//		memset(SendBuff+sizeof(ICMP_HEADER),'*',DEF_ICMP_DATA_SIZE);
//		ptr_send_icmp->cksum = htons(checkSum(SendBuff,sizeof(ICMP_HEADER)+DEF_ICMP_DATA_SIZE));
//
//
//		int sendflag = sendto(sock_raw,SendBuff,sizeof(ICMP_HEADER)+32,0,(sockaddr*)&destsockinfo,sizeof(destsockinfo));
//
//		if(sendflag == SOCKET_ERROR)
//		{
//			cout<<"Send error" << WSAGetLastError()<<endl;
//			break;
//		}
//		unsigned long sendtime = GetTickCount();
//		int fromlen = sizeof(fromsockinfo);
//		int recvflag = recvfrom(sock_raw,RecvBuff,MAX_ICMP_PACKET_SIZE,0,(sockaddr*)&fromsockinfo,&fromlen);
//
//		if(recvflag == SOCKET_ERROR)
//		{
//			if(WSAGetLastError() == WSAETIMEDOUT)
//			{
//				cout<<"请求超时"<<endl;
//				losscount++;
//				continue;
//			}
//			break;
//		}
//		else
//		{
//			IP_HEADER *ptr_ip = (IP_HEADER*)RecvBuff;
//			ICMP_HEADER *ptr_icmp_recv = (ICMP_HEADER*)(RecvBuff+sizeof(IP_HEADER));
//			//检查收到的是否是我们需要的数据报
//			if(ptr_icmp_recv->code == 0 && ptr_icmp_recv->id == ptr_send_icmp->id && ptr_icmp_recv->seq == ptr_send_icmp->seq)
//			{
//				RecvCount++;
//				recvtime = GetTickCount();
//				roundtime = recvtime-sendtime;
//				totaltime+=roundtime;
//				if(minroundtime == -1)
//				{
//					minroundtime = roundtime;
//					maxroundtime = roundtime;
//				}
//				if(roundtime < minroundtime)
//				{
//					minroundtime = roundtime;
//				}
//				if(roundtime > maxroundtime)
//				{
//					maxroundtime = roundtime;
//				}
//
//				cout <<"Reply from " << inet_ntoa(fromsockinfo.sin_addr) << ": bytes = " << recvflag-(sizeof(IP_HEADER)+sizeof(ICMP_HEADER)) << ", time = " \
//					<< roundtime << "ms, TTL = "<<(int)ptr_ip->ttl<<endl;
//			}
//			else
//			{
//				cout << "The echo reply is not correct!" << endl;
//			}
//			Sleep(1000);
//		}
//	}
//	cout<<endl;
//	cout << "180.97.33.108 的 Ping 统计信息:"<<endl;
//	printf("\t数据包: 已发送 = %d，已接收 = %d，丢失 = %d (%d% 丢失)，\n",SendCount,RecvCount,losscount,losscount/SendCount);
//	cout<<"往返行程的估计时间(以毫秒为单位):"<<endl;
//	printf("\t最短 = %dms，最长 = %dms，平均 = %dms\n",minroundtime,maxroundtime,totaltime/4);
//
//
//	return 0;
//}
//
