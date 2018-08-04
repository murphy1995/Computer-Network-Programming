#include <iostream>		//������������� 
#include <fstream>		//�����ļ����� 
#include <stdlib.h>		//���ڳ������̿��� 

using namespace std; 
/* run this program using the console pauser or add your own getch, system("pause") or input loop */
////////////////////////////////////////////////////////////////////////////////
// CRCУ�飬����һ��У��Ļ����ϼ�����8λCRCУ��
// 
//	���������
//		chCurrByte	��8λ������Ч����¼����һ��CRCУ�������
//		chNextByte	��8λ������Ч����¼�˱���Ҫ����У���һ���ֽ�	
//
//	����������

//		chCurrByte	��8λ������Ч����¼�˱���CRCУ�������
////////////////////////////////////////////////////////////////////////////////
void CRCcheck(int &chCurrByte, int chNextByte)
{
	// CRCѭ����ÿ�ε��ý���8��ѭ��������һ���ֽڵ����ݡ�
	for(int nMask = 0x80; nMask > 0; nMask >>= 1)		
	{
		if((chCurrByte & 0x80) != 0)					//��λΪ1����λ��������������� 
		{
			chCurrByte <<= 1;							//��һλ 
			if( (chNextByte & nMask) != 0)				//��һλ 
			{
				chCurrByte |= 1;
			} 
			chCurrByte ^= 7;							//��00000111������� 
		}
		else											//��λΪ0��ֻ��λ������� 
		{
			chCurrByte <<= 1;							//��һλ 
			if( (chNextByte & nMask) != 0)				//��һλ 
				chCurrByte |= 1;	
		}
	}
	//cout<< "(" << ((int)chCurrByte & 0xff ) << ")";
}

int main(int argc, char *argv[]) 
{
	//��������в�������ȷ�� 
	if(argc != 3)
	{
		cout<<"������'-e [��װ�ļ���]'��װ֡��'-u [����֡�ļ�·��]'����֡";
		exit(0);
	}
	//�������Ƿ�Ϊ֡��װ���� 
	if (argv[1][0] == '-' && argv[1][1] == 'e')
	{
		
		cout<<"������һ����Ϣ��"<<endl;
		//��֡��װ�ļ���Ĭ�����ļ�ʱ���� 
		ofstream out(argv[2]); 
		if (!out.is_open())
		{
			cout << "�޷���֡��װ���ļ�" << endl;
			exit(0);
		}
	
		int preamble = 0xaa;										//ǰ���� 
		int locator  = 0xab;										//֡ǰ����� 
		int destaddr[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};		//Ŀ�ĵ�ַ 
		int souraddr[6] = {0x00, 0x16, 0x76, 0xB4, 0xE4, 0x77};		//Դ��ַ 
		int type[2] = {0x08, 0x06};									//�����ֶ� 
		int ncheck = 0;												//У���� 
		char pdata;													//�����ֶζ����ַ� 
		int datasize = 0;											//�����ֶγ��� 
		bool LengthCont = true;										//��֡���� 
		bool EndFlag = false;                                      	//���������ֶν�����־ 
		//֡��װ����ѭ��		
		while(LengthCont) 
		{
			//д��ǰ���� 
			for(int i = 0; i < 7; i++)
			{
				out.put(preamble);
			}
			//д��֡ǰ����� 
			out.put(locator);
			//д��Ŀ�ĵ�ַ��У�� 
			for( int i = 0; i < 6; i++)
			{
				if(i == 0)
					ncheck = destaddr[i];							//����һ���ַ���Ϊ���������ȴ���һ��bit 
				else
					CRCcheck(ncheck, destaddr[i]);					//��ʼУ�� 
					out.put(destaddr[i]);
			}
			//д��Դ��ַ��У�� 
			for(int i = 0; i < 6; i++)
			{
				CRCcheck(ncheck, souraddr[i]);
				out.put(souraddr[i]);
			}
			//д�������ֶβ�У�� 
			for(int i = 0; i < 2; i++)
			{
				CRCcheck(ncheck, type[i]);
				out.put(type[i]);
			}
			datasize = 0; 									//�������ֶγ�������
			//���������ֶβ�У�� 
			while(cin.get(pdata))
			{
				if(pdata == '\n') 							//�Իس���Ϊ���� ������ѭ�� 
				{
					EndFlag = true;							//��������־��Ϊ�� 
					break;	
				}
				datasize ++;                                //�����ֶγ��ȼ��� 
				CRCcheck(ncheck, pdata);
				out.put(pdata);
					
				if(datasize >= 1500)						//���ȴ���1500����֡ 
					break;
			}
			if(datasize <= 1500 && EndFlag) 				//ͬʱ����1500�ַ����ںͽ���ʱ��ֹѭ�������ǡ�õ���1500����� 
				LengthCont = false;
			//cout<<datasize;
			CRCcheck(ncheck, 0);							//���һ��У��
			//����У���� 
			out.put(ncheck);
			
		}
			//�ر��ļ� 
			out.close();
			cout<<"֡��װ��ϣ�"<<endl;
	}
	//�������Ƿ�Ϊ֡�������� 
	else if (argv[1][0] == '-' && argv[1][1] == 'u')
	{
		//��������ļ��Ƿ���ڣ������԰������Ȩ�޺ͷ�ʽ�� 
		ifstream input(argv[2], ios::in|ios::binary);
		if(!input.is_open())
		{
			cout<< "�޷���֡��װ���ļ��������ļ��Ƿ���ڲ���Ϊ��" <<endl;
			exit(0); 
		}
		
		//������������ʼ��
		 int nSN = 1;                //֡��� 
		 int ncheck = 0;            //У����
		 int nCurrDataOffset = 22;   //֡ͷƫ����
		 int nCurrDataLength = 0;    //�����ֶγ���
		 bool bParseCont = true;     //�Ƿ�����������ļ����н���
		 int nFileEnd = 0;           //�����ļ��ĳ���
		 
		 //���������ļ��ĳ��� 
		 input.seekg(0, ios::end);    //���ļ�ָ���Ƶ��ļ���ĩβ
		 nFileEnd = input.tellg();    //ȡ�������ļ��ĳ���
		 input.seekg(0, ios::beg);    //�ļ�ָ��λ�ó�ʼ��
		 
		 cout.fill('0');              //��ʾ��ʼ��
		 cout.setf(ios::uppercase);  //�Դ�д��ĸ��� 
		 
		 //��λ�������ļ��ĵ�һ����Ч֡
		 //���ļ�ͷ��ʼ���ҵ���һ�������ġ�AA-AA-AA-AA-AA-AA-AA-AB��
		 while( true )
		 {
		 	for(int j = 0; j < 7; j++)
		 	{
		 		if(input.tellg() >= nFileEnd)
		 		{
		 			cout<<"û���ҵ��Ϸ���֡"<<endl;
		 			input.close();
		 			exit(0);
		 		}
		 		//����ǰ�ַ��ǲ���0xaa��������ǣ�������Ѱ��7��������0xaa
				 if(input.get() != 0xaa)
				 {
				 	j = -1;
				 } 
		 	}
		 	
		 	if(input.tellg() >= nFileEnd)          //��ȫ�Լ�� 
		 	{
		 		cout<<"û���ҵ��Ϸ���֡"<<endl;
		 		input.close();
		 		exit(0);
		 	}
		 	
		 	if(input.get() == 0xab)				   //�ж�7��������0xaa֮���Ƿ�0xab 
		 	{
		 		break;
		 	}
		 } 
		 
		 //�������ֶ�ƫ������λ������������֮��14�ֽڴ�����׼����������׶�
		 nCurrDataOffset = input.tellg() ;
		 nCurrDataOffset+=14;
		 input.seekg(-8,ios::cur);
		 
		 //����ѭ��
		 while( bParseCont )//����Ȼ���Լ������������ļ�ʱ����������
		 {
		 	// ���ʣ���ļ��Ƿ���ܰ�������֡ͷ
		 	int tempOffset = input.tellg();
		 	if(tempOffset + 14 > nFileEnd)
		 	{
		 		cout<<endl<<"û���ҵ�����֡ͷ��������ֹ"<<endl;
		 		input.close();
		 		exit(0);
		 	}
		 	
		 	int c;                //�����ֽ�
			int EtherType = 0;     //��֡�ж����������ֶ�
			bool bAccept = true;   //�Ƿ���ܸ�֡ 
			
			//���֡����� 
			cout << endl << "��ţ�\t\t" <<nSN;
			
			//���ǰ���룬ֻ�������У��
			cout << endl <<"ǰ���룺\t";
			for (int i = 0; i < 7; i++)             //�����ʽΪ�� AA AA AA AA AA AA AA 
			{
				cout.width(2);
				cout << hex <<input.get() <<dec << " ";
			}  
			
			//���֡ǰ�������ֻ�������У��
			cout << endl << "֡ǰ�������\t";       //�����ʽΪ�� AB 
			cout.width(2);
			cout << hex << input.get();
			
			//���Ŀ�ĵ�ַ����У��
			cout << endl <<"Ŀ�ĵ�ַ��\t";
			for(int i = 0; i < 6; i++)
			{
				c = input.get();
				cout.width(2);
				cout << hex << c << dec << (i==5? "" : "-");
				if(i == 0)                         //��һ���ֽڣ���Ϊ���������ȴ���һ��bit
				{
					ncheck = c;
				} 
				else                               //��ʼУ�� 
				{
					CRCcheck(ncheck, c);
				}
			} 
			//���Դ��ַ����У��
			cout << endl <<"Դ��ַ��\t";
			for(int i = 0; i < 6; i++)
			{
				c = input.get();
				cout.width(2);
				cout << hex << c << dec << (i==5? "" : "-");
				CRCcheck(ncheck, c);               //����У�� 
			} 
			
			//��������ֶβ�У��
			cout << endl << "�����ֶΣ�\t";
			cout.width(2);
			// ��������ֶεĸ�8λ
			c = input.get();
			cout<< hex << c << dec << " ";
			CRCcheck(ncheck, c);					// CRCУ��
			EtherType = c;
			// ��������ֶεĵ�8λ
			c = input.get();						
			cout.width(2);
			cout<< hex << c;
			CRCcheck(ncheck,c);						// CRCУ��
			EtherType <<= 8;                        //ת����������ʽ
			EtherType |= c;
			
			// ��λ��һ��֡����ȷ����ǰ֡�Ľ���λ��
			while ( bParseCont )
			{
	
				for (int i = 0; i < 7; i++)					//����һ��������7��0xaa
				{				
					if (input.tellg() >= nFileEnd)			//���ļ�ĩβ���˳�ѭ��
					{
						bParseCont = false;
						break;
					}
					// ����ǰ�ַ��ǲ���0xaa��������ǣ�������Ѱ��7��������0xaa
					if (input.get() != 0xaa)
					{
						i = -1;
					}
				}
				
				// ���ֱ���ļ�������û�ҵ��������ش�������ֹ����ѭ���ı��bParseCont��Ϊtrue
				bParseCont = bParseCont && (input.tellg() < nFileEnd);													
	
				// �ж�7��������0xaa֮���Ƿ�Ϊ0xab
				if (bParseCont && input.get() == 0xab)		
				{
					break;
				}
			} 
			
			// ���������ֶεĳ���
			tempOffset = input.tellg();
			nCurrDataLength = 
			   bParseCont ?                                  //�Ƿ񵽴��ļ�ĩβ 
			   (tempOffset - 8 - 1 - nCurrDataOffset) :   // û���ļ�ĩβ����һ֡ͷλ�� - ǰ����Ͷ�������� - CRCУ���볤�� - �����ֶ���ʼλ��
			   (tempOffset - 1 - nCurrDataOffset);        // �ѵ����ļ�ĩβ���ļ�ĩβλ�� - CRCУ���볤�� - �����ֶ���ʼλ��
			
			// ���ı���ʽ�����ֶΣ���У��
			cout << endl << "�����ֶΣ�\t";	 
			 char * pData = new  char [nCurrDataLength]; //����������
			input.seekg(bParseCont ? (-8 - 1 -nCurrDataLength) : (-1 - nCurrDataLength), ios::cur);
			input.read(pData, nCurrDataLength);               //���������ֶ�
			
			 int nCount = 50;                                 //ÿ�еĻ����ַ����� 
			 for (int i = 0; i < nCurrDataLength; i++)            //��������ֶ��ı� 
			 {
			 	nCount--;
				cout << pData[i];                             //�ַ����
				CRCcheck(ncheck, pData[i]);                   //CRCУ�� 
				
				if ( nCount < 0)                              //���д���
				{
					//����β�ĵ���д����
					if(pData[i] == ' ')
					{
						cout << endl << "\t\t";
						nCount = 50;
					} 
					//�����������β���ʣ����в�ʹ�����ַ�
					if( nCount < -10)
					{
						cout << "-" << endl << "\t\t";
						nCount = 50;
					} 
				} 
			 } 
			 delete[] pData;                                 //�ͷŻ���ռ� 
			 
			 //����CRCУ���룬���CRCУ�������������ȷ��CRCУ����
			 cout << endl << "CRCУ��";
			 c = input.get();                               //����CRCУ����
			 int nTmpCRC = ncheck;
			 CRCcheck(ncheck, c);                           //���һ��У��
			 
			 if((ncheck & 0xff) == 0)                        //CRCУ������
			 {
			 	cout.width(2);
			 	cout <<"(��ȷ)��\t" << hex << c;
			 } 
			 else                                            //CRCУ������
			 {
			 	cout.width(2);
			 	cout << "(����)��\t" << hex << c;
			 	CRCcheck(nTmpCRC, 0x00);
			 	cout << "\tӦΪ��" << hex << (nTmpCRC & 0xff);
				bAccept = false;                             // ��֡�Ľ��ձ����Ϊfalse
			 } 
			 	//	��������ֶγ��Ȳ���46�ֽڻ������ֶγ��ȳ���1500�ֽڣ���֡�Ľ��ձ����Ϊfalse	
			if (nCurrDataLength < 46 ||	nCurrDataLength > 1500 )							
			{
				bAccept = false;
			}
			// ���֡�Ľ���״̬
			cout<< endl << "״̬��\t\t" << (bAccept ? "Accept" : "Discard") << endl <<endl;
	
			nSN++;									// ֡��ż�1
			tempOffset = input.tellg();								
			nCurrDataOffset = tempOffset + 22;	// �������ֶ�ƫ��������Ϊ��һ֡��֡ͷ����λ��
		} 
		 // �ر������ļ�
		input.close();
	}
	else
	{
		cout<<"������'-e [��װ�ļ���]'��װ֡��'-u [����֡�ļ�·��]'����֡";
		exit(0);
	}
	return 0;
}
