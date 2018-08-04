#include <iostream>		//用于输入输出流 
#include <fstream>		//用于文件操作 
#include <stdlib.h>		//用于程序流程控制 

using namespace std; 
/* run this program using the console pauser or add your own getch, system("pause") or input loop */
////////////////////////////////////////////////////////////////////////////////
// CRC校验，在上一轮校验的基础上继续作8位CRC校验
// 
//	输入参数：
//		chCurrByte	低8位数据有效，记录了上一次CRC校验的余数
//		chNextByte	低8位数据有效，记录了本次要继续校验的一个字节	
//
//	传出参数：

//		chCurrByte	低8位数据有效，记录了本次CRC校验的余数
////////////////////////////////////////////////////////////////////////////////
void CRCcheck(int &chCurrByte, int chNextByte)
{
	// CRC循环：每次调用进行8次循环，处理一个字节的数据。
	for(int nMask = 0x80; nMask > 0; nMask >>= 1)		
	{
		if((chCurrByte & 0x80) != 0)					//首位为1：移位，并进行异或运算 
		{
			chCurrByte <<= 1;							//移一位 
			if( (chNextByte & nMask) != 0)				//补一位 
			{
				chCurrByte |= 1;
			} 
			chCurrByte ^= 7;							//与00000111进行异或 
		}
		else											//首位为0，只移位，不异或 
		{
			chCurrByte <<= 1;							//移一位 
			if( (chNextByte & nMask) != 0)				//补一位 
				chCurrByte |= 1;	
		}
	}
	//cout<< "(" << ((int)chCurrByte & 0xff ) << ")";
}

int main(int argc, char *argv[]) 
{
	//检测命令行参数的正确性 
	if(argc != 3)
	{
		cout<<"请输入'-e [封装文件名]'封装帧或'-u [数据帧文件路径]'解析帧";
		exit(0);
	}
	//检测参数是否为帧封装命令 
	if (argv[1][0] == '-' && argv[1][1] == 'e')
	{
		
		cout<<"请输入一段信息："<<endl;
		//打开帧封装文件，默认无文件时创建 
		ofstream out(argv[2]); 
		if (!out.is_open())
		{
			cout << "无法打开帧封装包文件" << endl;
			exit(0);
		}
	
		int preamble = 0xaa;										//前导码 
		int locator  = 0xab;										//帧前定界符 
		int destaddr[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};		//目的地址 
		int souraddr[6] = {0x00, 0x16, 0x76, 0xB4, 0xE4, 0x77};		//源地址 
		int type[2] = {0x08, 0x06};									//类型字段 
		int ncheck = 0;												//校验码 
		char pdata;													//数据字段读入字符 
		int datasize = 0;											//数据字段长度 
		bool LengthCont = true;										//分帧符号 
		bool EndFlag = false;                                      	//读入数据字段结束标志 
		//帧封装主控循环		
		while(LengthCont) 
		{
			//写入前导码 
			for(int i = 0; i < 7; i++)
			{
				out.put(preamble);
			}
			//写入帧前定界符 
			out.put(locator);
			//写入目的地址并校验 
			for( int i = 0; i < 6; i++)
			{
				if(i == 0)
					ncheck = destaddr[i];							//将第一个字符作为“余数”等待下一个bit 
				else
					CRCcheck(ncheck, destaddr[i]);					//开始校验 
					out.put(destaddr[i]);
			}
			//写入源地址并校验 
			for(int i = 0; i < 6; i++)
			{
				CRCcheck(ncheck, souraddr[i]);
				out.put(souraddr[i]);
			}
			//写入类型字段并校验 
			for(int i = 0; i < 2; i++)
			{
				CRCcheck(ncheck, type[i]);
				out.put(type[i]);
			}
			datasize = 0; 									//将数据字段长度清零
			//输入数据字段并校验 
			while(cin.get(pdata))
			{
				if(pdata == '\n') 							//以回车作为结束 ，跳出循环 
				{
					EndFlag = true;							//将结束标志置为真 
					break;	
				}
				datasize ++;                                //数据字段长度计数 
				CRCcheck(ncheck, pdata);
				out.put(pdata);
					
				if(datasize >= 1500)						//长度大于1500，分帧 
					break;
			}
			if(datasize <= 1500 && EndFlag) 				//同时满足1500字符以内和结束时中止循环，解决恰好等于1500的情况 
				LengthCont = false;
			//cout<<datasize;
			CRCcheck(ncheck, 0);							//最后一步校验
			//输入校验码 
			out.put(ncheck);
			
		}
			//关闭文件 
			out.close();
			cout<<"帧封装完毕！"<<endl;
	}
	//检测参数是否为帧解析命令 
	else if (argv[1][0] == '-' && argv[1][1] == 'u')
	{
		//检测输入文件是否存在，并可以按所需的权限和方式打开 
		ifstream input(argv[2], ios::in|ios::binary);
		if(!input.is_open())
		{
			cout<< "无法打开帧封装包文件，请检查文件是否存在并且为损坏" <<endl;
			exit(0); 
		}
		
		//变量声明及初始化
		 int nSN = 1;                //帧序号 
		 int ncheck = 0;            //校验码
		 int nCurrDataOffset = 22;   //帧头偏移量
		 int nCurrDataLength = 0;    //数据字段长度
		 bool bParseCont = true;     //是否继续对输入文件进行解析
		 int nFileEnd = 0;           //输入文件的长度
		 
		 //计算输入文件的长度 
		 input.seekg(0, ios::end);    //把文件指针移到文件的末尾
		 nFileEnd = input.tellg();    //取得输入文件的长度
		 input.seekg(0, ios::beg);    //文件指针位置初始化
		 
		 cout.fill('0');              //显示初始化
		 cout.setf(ios::uppercase);  //以大写字母输出 
		 
		 //定位到输入文件的第一个有效帧
		 //从文件头开始，找到第一个连续的“AA-AA-AA-AA-AA-AA-AA-AB”
		 while( true )
		 {
		 	for(int j = 0; j < 7; j++)
		 	{
		 		if(input.tellg() >= nFileEnd)
		 		{
		 			cout<<"没有找到合法的帧"<<endl;
		 			input.close();
		 			exit(0);
		 		}
		 		//看当前字符是不是0xaa，如果不是，则重新寻找7个连续的0xaa
				 if(input.get() != 0xaa)
				 {
				 	j = -1;
				 } 
		 	}
		 	
		 	if(input.tellg() >= nFileEnd)          //安全性检测 
		 	{
		 		cout<<"没有找到合法的帧"<<endl;
		 		input.close();
		 		exit(0);
		 	}
		 	
		 	if(input.get() == 0xab)				   //判断7个连续的0xaa之后是否0xab 
		 	{
		 		break;
		 	}
		 } 
		 
		 //将数据字段偏移量定位在上述二进制之后14字节处，并准备进入解析阶段
		 nCurrDataOffset = input.tellg() ;
		 nCurrDataOffset+=14;
		 input.seekg(-8,ios::cur);
		 
		 //主控循环
		 while( bParseCont )//当仍然可以继续解析输入文件时，继续解析
		 {
		 	// 检测剩余文件是否可能包含完整帧头
		 	int tempOffset = input.tellg();
		 	if(tempOffset + 14 > nFileEnd)
		 	{
		 		cout<<endl<<"没有找到完整帧头，解析终止"<<endl;
		 		input.close();
		 		exit(0);
		 	}
		 	
		 	int c;                //读入字节
			int EtherType = 0;     //由帧中读出的类型字段
			bool bAccept = true;   //是否接受该帧 
			
			//输出帧的序号 
			cout << endl << "序号：\t\t" <<nSN;
			
			//输出前导码，只输出，不校验
			cout << endl <<"前导码：\t";
			for (int i = 0; i < 7; i++)             //输出格式为： AA AA AA AA AA AA AA 
			{
				cout.width(2);
				cout << hex <<input.get() <<dec << " ";
			}  
			
			//输出帧前定界符，只输出，不校验
			cout << endl << "帧前定界符：\t";       //输出格式为： AB 
			cout.width(2);
			cout << hex << input.get();
			
			//输出目的地址，并校验
			cout << endl <<"目的地址：\t";
			for(int i = 0; i < 6; i++)
			{
				c = input.get();
				cout.width(2);
				cout << hex << c << dec << (i==5? "" : "-");
				if(i == 0)                         //第一个字节，作为“余数”等待下一个bit
				{
					ncheck = c;
				} 
				else                               //开始校验 
				{
					CRCcheck(ncheck, c);
				}
			} 
			//输出源地址，并校验
			cout << endl <<"源地址：\t";
			for(int i = 0; i < 6; i++)
			{
				c = input.get();
				cout.width(2);
				cout << hex << c << dec << (i==5? "" : "-");
				CRCcheck(ncheck, c);               //继续校验 
			} 
			
			//输出类型字段并校验
			cout << endl << "类型字段：\t";
			cout.width(2);
			// 输出类型字段的高8位
			c = input.get();
			cout<< hex << c << dec << " ";
			CRCcheck(ncheck, c);					// CRC校验
			EtherType = c;
			// 输出类型字段的低8位
			c = input.get();						
			cout.width(2);
			cout<< hex << c;
			CRCcheck(ncheck,c);						// CRC校验
			EtherType <<= 8;                        //转换成主机格式
			EtherType |= c;
			
			// 定位下一个帧，以确定当前帧的结束位置
			while ( bParseCont )
			{
	
				for (int i = 0; i < 7; i++)					//找下一个连续的7个0xaa
				{				
					if (input.tellg() >= nFileEnd)			//到文件末尾，退出循环
					{
						bParseCont = false;
						break;
					}
					// 看当前字符是不是0xaa，如果不是，则重新寻找7个连续的0xaa
					if (input.get() != 0xaa)
					{
						i = -1;
					}
				}
				
				// 如果直到文件结束仍没找到上述比特串，将终止主控循环的标记bParseCont置为true
				bParseCont = bParseCont && (input.tellg() < nFileEnd);													
	
				// 判断7个连续的0xaa之后是否为0xab
				if (bParseCont && input.get() == 0xab)		
				{
					break;
				}
			} 
			
			// 计算数据字段的长度
			tempOffset = input.tellg();
			nCurrDataLength = 
			   bParseCont ?                                  //是否到达文件末尾 
			   (tempOffset - 8 - 1 - nCurrDataOffset) :   // 没到文件末尾：下一帧头位置 - 前导码和定界符长度 - CRC校验码长度 - 数据字段起始位置
			   (tempOffset - 1 - nCurrDataOffset);        // 已到达文件末尾：文件末尾位置 - CRC校验码长度 - 数据字段起始位置
			
			// 以文本格式数据字段，并校验
			cout << endl << "数据字段：\t";	 
			 char * pData = new  char [nCurrDataLength]; //创建缓存区
			input.seekg(bParseCont ? (-8 - 1 -nCurrDataLength) : (-1 - nCurrDataLength), ios::cur);
			input.read(pData, nCurrDataLength);               //读入数据字段
			
			 int nCount = 50;                                 //每行的基本字符数量 
			 for (int i = 0; i < nCurrDataLength; i++)            //输出数据字段文本 
			 {
			 	nCount--;
				cout << pData[i];                             //字符输出
				CRCcheck(ncheck, pData[i]);                   //CRC校验 
				
				if ( nCount < 0)                              //换行处理
				{
					//将行尾的单词写完整
					if(pData[i] == ' ')
					{
						cout << endl << "\t\t";
						nCount = 50;
					} 
					//处理过长的行尾单词：换行并使用连字符
					if( nCount < -10)
					{
						cout << "-" << endl << "\t\t";
						nCount = 50;
					} 
				} 
			 } 
			 delete[] pData;                                 //释放缓冲空间 
			 
			 //输入CRC校验码，如果CRC校验有误，则输出正确的CRC校验码
			 cout << endl << "CRC校验";
			 c = input.get();                               //读入CRC校验码
			 int nTmpCRC = ncheck;
			 CRCcheck(ncheck, c);                           //最后一步校验
			 
			 if((ncheck & 0xff) == 0)                        //CRC校验无误
			 {
			 	cout.width(2);
			 	cout <<"(正确)：\t" << hex << c;
			 } 
			 else                                            //CRC校验有误
			 {
			 	cout.width(2);
			 	cout << "(错误)：\t" << hex << c;
			 	CRCcheck(nTmpCRC, 0x00);
			 	cout << "\t应为：" << hex << (nTmpCRC & 0xff);
				bAccept = false;                             // 将帧的接收标记置为false
			 } 
			 	//	如果数据字段长度不足46字节或数据字段长度超过1500字节，则将帧的接收标记置为false	
			if (nCurrDataLength < 46 ||	nCurrDataLength > 1500 )							
			{
				bAccept = false;
			}
			// 输出帧的接收状态
			cout<< endl << "状态：\t\t" << (bAccept ? "Accept" : "Discard") << endl <<endl;
	
			nSN++;									// 帧序号加1
			tempOffset = input.tellg();								
			nCurrDataOffset = tempOffset + 22;	// 将数据字段偏移量更新为下一帧的帧头结束位置
		} 
		 // 关闭输入文件
		input.close();
	}
	else
	{
		cout<<"请输入'-e [封装文件名]'封装帧或'-u [数据帧文件路径]'解析帧";
		exit(0);
	}
	return 0;
}
