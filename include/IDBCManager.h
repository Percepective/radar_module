#ifndef _IDBCMANAGER_H_
#define _IDBCMANAGER_H_

#ifdef __cplusplus
#define EXTERN_C extern "C"
#else
#define EXTERN_C
#endif

typedef unsigned int        uint;
typedef unsigned int		DBCHandle;
typedef unsigned char       uint8;
typedef unsigned short       uint16;

#define INVALID_DBC_HANDLE -1 // ��Ч��DBC���

#define _MAX_FILE_PATH_                      1000 //��ļ�·��
#define _DBC_NAME_LENGTH_					 65  //���������
#define _DBC_COMMENT_MAX_LENGTH_             200 //ע�������
#define _DBC_UNIT_MAX_LENGTH_                10  //��λ�����
#define _DBC_SIGNAL_MAX_COUNT_                512 //һ����Ϣ���е��źŵ������Ŀ

struct DBCSignal
{
	char strName[_DBC_NAME_LENGTH_+1];  //����
	char strComment[_DBC_COMMENT_MAX_LENGTH_+1];  //ע��
	uint nStartBit; // ��ʼλ
	uint nLen;	// λ����
	double nFactor; // ת������
	double nOffset;	// ת��ƫ�� ʵ��ֵ=ԭʼֵ*nFactor+nOffset
	double nMin;    // ��Сֵ
	double nMax;	// ���ֵ
	char unit[_DBC_UNIT_MAX_LENGTH_+1];//��λ
	double nValue;  //ʵ��ֵ
};

struct  DBCMessage 
{
	char strName[_DBC_NAME_LENGTH_+1];  //����
	char strComment[_DBC_COMMENT_MAX_LENGTH_+1];    //ע��
	uint nID;
	uint8 nExtend; //1:��չ֡, 0:��׼֡
	uint nSize;	//��Ϣռ���ֽ���Ŀ
	DBCSignal vSignals[_DBC_SIGNAL_MAX_COUNT_+1]; //�źż���
	uint nSignalCount; //�ź�����
};

struct FileInfo
{
	char strFilePath[_MAX_FILE_PATH_+1]; //dbc�ļ�·��
	enum ProtocolType
	{
		DBC_J1939,
		DBC_CAN,
	} nType; //dbc��Э������, j1939ѡ��DBC_J1939, ����Э��ѡ��DBC_CAN
};

/*
����˵������ûʵ�ʷ������ݹ���, ֻ�ṩ���͵�����,
		  ��������Ҫʵ�ָûص���������ʵ�����ݷ���,
		  �򵥵���VCI_Transmit���ͼ��ɡ�
��    ��[in]��ctx-�ص�������������
��    ��[in]��pObj-Ҫ���͵�����
�� �� ֵ��void
*/
typedef void (*OnSend)(void* ctx, void* pObj);

/*
����˵������֡������ɺ�ص�, ������漰��֡����Ҳ���Բ�ʵ�ָûص�
��    ��[in]��ctx-�ص�������������
��    ��[in]��pMsg-���ص�DBC��Ϣ
��    ��[in]��data-��֡��ʵ������
��    ��[in]��nLen-��֡���ݵ��ֽ���Ŀ
��    ��[in]��nDirection-��֡�Ĵ��䷽�� 1:����, 0:����
�� �� ֵ��void
*/
typedef void (*OnMultiTransDone)(void* ctx, DBCMessage* pMsg, uint8* data, uint16 nLen, uint8 nDirection);

/*
����˵������ʼ������ģ�顣
�� �� ֵ��INVALID_DBC_HANDLE:��ʼ��ʧ��
		  ����ֵ:��ʼ���ɹ�
		  �����ֵ������ʹ�õ���
*/
EXTERN_C DBCHandle __stdcall DBC_Init();

/*
����˵�����ͷ���Դ, ��DBC_Init���ʹ��
��    ��[in]��hDBC-���, DBC_Load�ķ���ֵ
*/
EXTERN_C void __stdcall DBC_Release( DBCHandle hDBC );

/*
����˵��������DBC��ʽ�ļ���
��    ��[in]��hDBC-���, DBC_Load�ķ���ֵ
��    ��[in]��pFileInfo-�ļ���Ϣ, �ο�struct FileInfo
�� �� ֵ��true���سɹ�
*/
EXTERN_C bool __stdcall DBC_LoadFile( DBCHandle hDBC, const FileInfo* pFileInfo );

/*
����˵������ȡ��һ����Ϣ��
��    ��[in]��hDBC-���, DBC_Load�ķ���ֵ
��    ��[out]��pMsg �洢��Ϣ����Ϣ
�� �� ֵ��true��ʾ�ɹ�
*/
EXTERN_C bool __stdcall DBC_GetFirstMessage( DBCHandle hDBC, DBCMessage* pMsg );

/*
����˵������ȡ��һ����Ϣ��
��    ��[in]��hDBC-���, DBC_Load�ķ���ֵ
��    ��[out]��pMsg �洢��Ϣ����Ϣ
�� �� ֵ��true��ʾ�ɹ�
*/
EXTERN_C bool __stdcall DBC_GetNextMessage( DBCHandle hDBC, DBCMessage* pMsg );

/*
����˵��������ID��ȡ��Ϣ��
��    ��[in]��hDBC-���, DBC_Load�ķ���ֵ
��    ��[in]��nID-��Ϣ��ID
��    ��[out]��pMsg �洢��Ϣ����Ϣ
�� �� ֵ��true��ʾ�ɹ�
*/
EXTERN_C bool __stdcall DBC_GetMessageById( DBCHandle hDBC, uint nID, DBCMessage* pMsg );
/*
����˵������ȡ��Ϣ������
��    ��[in]��hDBC-���, DBC_Load�ķ���ֵ
�� �� ֵ����Ϣ����
*/
EXTERN_C uint __stdcall DBC_GetMessageCount( DBCHandle hDBC );

/*
����˵��������DBCЭ�����֡����
��    ��[in]��hDBC-���, DBC_Load�ķ���ֵ
��    ��[in]��pObj ԭʼ֡����
��    ��[out]��pMsg �������
�� �� ֵ��true��ʾ�ɹ�
*/
EXTERN_C bool DBC_Analyse( DBCHandle hDBC, const void* pObj, DBCMessage* pMsg );

/*
����˵������������Ҫ���øú����ѽ��յ���֡���ݴ�����,
		  �漰��֡�������Ҫ��������, �����޷�ʵ���豸������
��    ��[in]��hDBC-���, DBC_Load�ķ���ֵ
��    ��[in]��pObj ԭʼ֡����
�� �� ֵ��void
*/
EXTERN_C void DBC_OnReceive( DBCHandle hDBC, const void* pObj );

/*
����˵��������ʵ�ʷ������ݵĴ�����, �漰���ݷ���ʱ�������ã�ֻ��Ҫ����һ�Ρ�
��    ��[in]��hDBC-���, DBC_Load�ķ���ֵ
��    ��[in]��sender�ص�����ָ��
��    ��[in]��ctx-�ص�������������
�� �� ֵ��void
*/
EXTERN_C void DBC_SetSender( DBCHandle hDBC, OnSend sender, void* ctx );

/*
����˵�������ô����֡�������ݵĻص�������ֻ��Ҫ����һ��
��    ��[in]��hDBC-���, DBC_Load�ķ���ֵ
��    ��[in]��OnMultiTransDone�ص�����ָ��
��    ��[in]��ctx-�ص�������������
�� �� ֵ��void
*/
EXTERN_C void DBC_SetOnMultiTransDoneFunc( DBCHandle hDBC, OnMultiTransDone func, void* ctx);
/*
����˵��������DBC��Ϣ��
��    ��[in]��hDBC-���, DBC_Load�ķ���ֵ
��    ��[in]��pMsg Ҫ���͵���Ϣ
�� �� ֵ��true��ʾ�ɹ�
*/
EXTERN_C bool DBC_Send( DBCHandle hDBC, const DBCMessage* pMsg );

#endif // _IDBCMANAGER_H_