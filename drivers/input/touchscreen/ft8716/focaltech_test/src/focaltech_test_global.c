/************************************************************************
* Copyright (C) 2012-2015, Focaltech Systems (R)��All Rights Reserved.
*
* File Name: focaltech_test_global.c
*
* Author: Software Development Team, AE
*
* Created: 2016-08-01
*
* Abstract: global function for test
*
************************************************************************/

#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/printk.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/delay.h>

#include "../include/focaltech_test_ini.h"
#include "../focaltech_test_config.h"
#include "../include/focaltech_test_main.h"
#include "../include/focaltech_ic_table.h"

#define DEVIDE_MODE_ADDR    0x00

/*buff length*/
#define BUFF_LEN_STORE_MSG_AREA     1024*10
#define BUFF_LEN_MSG_AREA_LINE2     1024*4
#define BUFF_LEN_STORE_DATA_AREA        1024*80
#define BUFF_LEN_TMP_BUFFER             1024*16


char *g_pTmpBuff = NULL;
char *g_pStoreMsgArea = NULL;
int g_lenStoreMsgArea = 0;
char *g_pMsgAreaLine2 = NULL;
int g_lenMsgAreaLine2 = 0;
char *g_pStoreDataArea = NULL;
int g_lenStoreDataArea = 0;
unsigned char m_ucTestItemCode = 0;
int m_iStartLine = 0;
int m_iTestDataCount = 0;

char *TestResult = NULL;
int TestResultLen = 0;

/*�ڴ����뷽ʽ*/
#define FTS_MALLOC_TYPE         1
enum enum_malloc_mode {
	kmalloc_mode = 0,
	vmalloc_mode = 1,
};

struct StruScreenSeting g_ScreenSetParam_sharp;
struct stTestItem g_stTestItem_sharp[1][MAX_TEST_ITEM];
struct structSCapConfEx g_stSCapConfEx_sharp;

int g_TestItemNum_sharp = 0;
char g_strIcName_sharp[20] = {0};
char *g_pStoreAllData = NULL;

int GetPrivateProfileString_sharp(char *section, char *ItemName, char *defaultvalue, char *returnValue, char *IniFile)
{
	char value[512] = {0};
	int len = 0;

	if (NULL == returnValue) {
		FTS_TEST_DBG("[FTS] returnValue==NULL in function %s.", __func__);
		return 0;
	}

	if (ini_get_key_sharp(IniFile, section, ItemName, value) < 0) {
		if (NULL != defaultvalue)
			memcpy(value, defaultvalue, strlen(defaultvalue));
		sprintf(returnValue, "%s", value);
		return 0;
	} else {
		len = sprintf(returnValue, "%s", value);
	}

	return len;
}

void focal_msleep_sharp(int ms)
{
	msleep(ms);
}

void SysDelay_sharp(int ms)
{
	msleep(ms);
}

int focal_abs_sharp(int value)
{
	if (value < 0)
		value = 0 - value;

	return value;
}

void *fts_malloc(size_t size)
{
	if (FTS_MALLOC_TYPE == kmalloc_mode) {
		return kmalloc(size, GFP_ATOMIC);
	} else if (FTS_MALLOC_TYPE == vmalloc_mode) {
		return vmalloc(size);
	} else {
		FTS_TEST_DBG("invalid malloc. \n");
		return NULL;
	}

	return NULL;
}

void fts_free(void *p)
{
	if (FTS_MALLOC_TYPE == kmalloc_mode) {
		return kfree(p);
	} else if (FTS_MALLOC_TYPE == vmalloc_mode) {
		return vfree(p);
	} else {
		FTS_TEST_DBG("invalid free. \n");
		return ;
	}

	return ;
}

void OnInit_InterfaceCfg_sharp(char *strIniFile)
{
	char str[128] = {0};

	FTS_TEST_FUNC_ENTER();

	GetPrivateProfileString_sharp("Interface", "IC_Type", "FT5X36", str, strIniFile);
	g_ScreenSetParam_sharp.iSelectedIC = fts_ic_table_get_ic_code_from_ic_name(str);
	FTS_TEST_INFO(" IC code :0x%02x. ", g_ScreenSetParam_sharp.iSelectedIC);


	GetPrivateProfileString_sharp("Interface", "Normalize_Type", 0, str, strIniFile);
	g_ScreenSetParam_sharp.isNormalize = fts_atoi(str);

	FTS_TEST_FUNC_EXIT();

}
/************************************************************************
* Name: ReadReg_sharp(Same function name as FT_MultipleTest)
* Brief:  Read Register
* Input: RegAddr
* Output: RegData
* Return: Comm Code. Code = 0x00 is OK, else fail.
***********************************************************************/
int ReadReg_sharp(unsigned char RegAddr, unsigned char *RegData)
{
	int iRet;

	if (NULL == fts_i2c_read_test_sharp) {
		FTS_TEST_DBG("[focal] %s fts_i2c_read_test_sharp == NULL  !!! ", __func__);
		return (ERROR_CODE_INVALID_COMMAND);
	}

	iRet = fts_i2c_read_test_sharp(&RegAddr, 1, RegData, 1);

	if (iRet >= 0)
		return (ERROR_CODE_OK);
	else
		return (ERROR_CODE_COMM_ERROR);
}

/************************************************************************
* Name: WriteReg_sharp(Same function name as FT_MultipleTest)
* Brief:  Write Register
* Input: RegAddr, RegData
* Output: null
* Return: Comm Code. Code = 0x00 is OK, else fail.
***********************************************************************/
int WriteReg_sharp(unsigned char RegAddr, unsigned char RegData)
{
	int iRet;
	unsigned char cmd[2] = {0};

	if (NULL == fts_i2c_write_test_sharp) {
		FTS_TEST_DBG("[focal] %s fts_i2c_write_test_sharp == NULL  !!!", __func__);
		return (ERROR_CODE_INVALID_COMMAND);
	}

	cmd[0] = RegAddr;
	cmd[1] = RegData;
	iRet = fts_i2c_write_test_sharp(cmd, 2);

	if (iRet >= 0)
		return (ERROR_CODE_OK);
	else
		return (ERROR_CODE_COMM_ERROR);
}
/************************************************************************
* Name: Comm_Base_IIC_IO_sharp(Same function name as FT_MultipleTest)
* Brief:  Write/Read Data by IIC
* Input: pWriteBuffer, iBytesToWrite, iBytesToRead
* Output: pReadBuffer
* Return: Comm Code. Code = 0x00 is OK, else fail.
***********************************************************************/
unsigned char Comm_Base_IIC_IO_sharp(unsigned char *pWriteBuffer, int  iBytesToWrite, unsigned char *pReadBuffer, int iBytesToRead)
{
	int iRet;

	if (NULL == fts_i2c_read_test_sharp) {
		FTS_TEST_DBG("[focal] %s fts_i2c_read_test_sharp == NULL  !!! ", __func__);
		return (ERROR_CODE_INVALID_COMMAND);
	}

	if (iBytesToRead > 0) {
		iRet = fts_i2c_read_test_sharp(pWriteBuffer, iBytesToWrite, pReadBuffer, iBytesToRead);
	} else {
		iRet = fts_i2c_write_test_sharp(pWriteBuffer, iBytesToWrite);
	}

	if (iRet >= 0)
		return (ERROR_CODE_OK);
	else
		return (ERROR_CODE_COMM_ERROR);
}
/************************************************************************
* Name: EnterWork_sharp(Same function name as FT_MultipleTest)
* Brief:  Enter Work Mode
* Input: null
* Output: null
* Return: Comm Code. Code = 0x00 is OK, else fail.
***********************************************************************/
unsigned char EnterWork_sharp(void)
{
	unsigned char RunState = 0;
	unsigned char ReCode = ERROR_CODE_COMM_ERROR;

	FTS_TEST_FUNC_ENTER();
	ReCode = ReadReg_sharp(DEVIDE_MODE_ADDR, &RunState);
	if (ReCode == ERROR_CODE_OK) {
		if (((RunState>>4)&0x07) == 0x00) {
			ReCode = ERROR_CODE_OK;
		} else {
			ReCode = WriteReg_sharp(DEVIDE_MODE_ADDR, 0);
			if (ReCode == ERROR_CODE_OK) {
				ReCode = ReadReg_sharp(DEVIDE_MODE_ADDR, &RunState);
				if (ReCode == ERROR_CODE_OK) {
					if (((RunState>>4)&0x07) == 0x00) {
						ReCode = ERROR_CODE_OK;
					} else {
						ReCode = ERROR_CODE_COMM_ERROR;
					}
				} else
					FTS_TEST_ERROR("EnterWork_sharp read DEVIDE_MODE_ADDR error 3.");
			} else
				FTS_TEST_ERROR("EnterWork_sharp write DEVIDE_MODE_ADDR error 2.");
		}
	} else
		FTS_TEST_ERROR("EnterWork_sharp read DEVIDE_MODE_ADDR error 1.");

	FTS_TEST_FUNC_EXIT();

	return ReCode;

}
/************************************************************************
* Name: EnterFactory_sharp
* Brief:  enter Fcatory Mode
* Input: null
* Output: null
* Return: Comm Code. Code = 0 is OK, else fail.
***********************************************************************/
unsigned char EnterFactory_sharp(void)
{
	unsigned char RunState = 0;
	int index = 0;
	unsigned char ReCode = ERROR_CODE_COMM_ERROR;

	FTS_TEST_FUNC_ENTER();
	ReCode = ReadReg_sharp(DEVIDE_MODE_ADDR, &RunState);
	if (ReCode == ERROR_CODE_OK) {
		if (((RunState>>4)&0x07) == 0x04) {
			ReCode = ERROR_CODE_OK;
		} else {
			ReCode = WriteReg_sharp(DEVIDE_MODE_ADDR, 0x40);
			if (ReCode == ERROR_CODE_OK) {
				for (index = 0; index < 20; ++index) {
					ReCode = ReadReg_sharp(DEVIDE_MODE_ADDR, &RunState);
					if (ReCode == ERROR_CODE_OK) {
						if (((RunState>>4)&0x07) == 0x04) {
							ReCode = ERROR_CODE_OK;
							break;
						} else {
							ReCode = ERROR_CODE_COMM_ERROR;
						}
					}
					SysDelay_sharp(50);
				}
				if (ReCode != ERROR_CODE_OK)
					FTS_TEST_ERROR("EnterFactory_sharp read DEVIDE_MODE_ADDR error 3.");
			} else
				FTS_TEST_ERROR("EnterFactory_sharp write DEVIDE_MODE_ADDR error 2.");
		}
	} else
		FTS_TEST_ERROR("EnterFactory_sharp read DEVIDE_MODE_ADDR error 1.");

	FTS_TEST_FUNC_EXIT();
	return ReCode;
}

/************************************************************************
* Name: fts_SetTestItemCodeName
* Brief:  set test item code and name
* Input: null
* Output: null
* Return:
**********************************************************************/

void fts_SetTestItemCodeName(unsigned char ucitemcode)
{
	g_stTestItem_sharp[0][g_TestItemNum_sharp].ItemCode = ucitemcode;

	g_stTestItem_sharp[0][g_TestItemNum_sharp].TestNum = g_TestItemNum_sharp;
	g_stTestItem_sharp[0][g_TestItemNum_sharp].TestResult = RESULT_NULL;
	g_TestItemNum_sharp++;
}

/************************************************************************
* Name: InitTest
* Brief:  Init all param before test
* Input: none
* Output: none
* Return: none
***********************************************************************/
int InitTest(void)
{
	int ret = 0;
	ret = AllocateMemory();
	if (ret < 0)
		return -EPERM;

	InitStoreParamOfTestData();

	g_stSCapConfEx_sharp.ChannelXNum = 0;
	g_stSCapConfEx_sharp.ChannelYNum = 0;
	g_stSCapConfEx_sharp.KeyNum = 0;
	g_stSCapConfEx_sharp.KeyNumTotal = 6;

	return 0;

}

/************************************************************************
* Name: FinishTest
* Brief:  Init all param before test
* Input: none
* Output: none
* Return: none
***********************************************************************/
void FinishTest(void)
{
	MergeAllTestData();
	FreeMemory();
}


/************************************************************************
* Name: InitStoreParamOfTestData
* Brief:  Init store param of test data
* Input: none
* Output: none
* Return: none
***********************************************************************/
void InitStoreParamOfTestData(void)
{
	g_lenStoreMsgArea = 0;

	g_lenStoreMsgArea += sprintf(g_pStoreMsgArea, "ECC, 85, 170, IC Name, %s, IC Code, %x\n",  g_strIcName_sharp,  g_ScreenSetParam_sharp.iSelectedIC);



	g_lenMsgAreaLine2 = 0;



	g_lenStoreDataArea = 0;
	m_iStartLine = 11;

	m_iTestDataCount = 0;
}
/************************************************************************
* Name: MergeAllTestData
* Brief:  Merge All Data of test result
* Input: none
* Output: none
* Return: none
***********************************************************************/
void MergeAllTestData(void)
{
	int iLen = 0;


	iLen = sprintf(g_pTmpBuff, "TestItem, %d, ", m_iTestDataCount);
	memcpy(g_pStoreMsgArea+g_lenStoreMsgArea, g_pTmpBuff, iLen);
	g_lenStoreMsgArea += iLen;


	memcpy(g_pStoreMsgArea+g_lenStoreMsgArea, g_pMsgAreaLine2, g_lenMsgAreaLine2);
	g_lenStoreMsgArea += g_lenMsgAreaLine2;


	iLen = sprintf(g_pTmpBuff, "\n\n\n\n\n\n\n\n\n");
	memcpy(g_pStoreMsgArea+g_lenStoreMsgArea, g_pTmpBuff, iLen);
	g_lenStoreMsgArea += iLen;


	memcpy(g_pStoreAllData, g_pStoreMsgArea, g_lenStoreMsgArea);


	if (0 != g_lenStoreDataArea) {
		memcpy(g_pStoreAllData+g_lenStoreMsgArea, g_pStoreDataArea, g_lenStoreDataArea);
	}

	FTS_TEST_DBG("lenStoreMsgArea=%d,  lenStoreDataArea = %d",  g_lenStoreMsgArea, g_lenStoreDataArea);
}



/************************************************************************
* Name: AllocateMemory
* Brief:  Allocate pointer Memory
* Input: none
* Output: none
* Return: none
***********************************************************************/
int AllocateMemory(void)
{

	g_pStoreMsgArea = NULL;
	if (NULL == g_pStoreMsgArea)
		g_pStoreMsgArea = fts_malloc(BUFF_LEN_STORE_MSG_AREA);
	if (NULL == g_pStoreMsgArea)
		goto ERR;

	g_pMsgAreaLine2 = NULL;
	if (NULL == g_pMsgAreaLine2)
		g_pMsgAreaLine2 = fts_malloc(BUFF_LEN_MSG_AREA_LINE2);
	if (NULL == g_pMsgAreaLine2)
		goto ERR;

	g_pStoreDataArea = NULL;
	if (NULL == g_pStoreDataArea)
		g_pStoreDataArea = fts_malloc(BUFF_LEN_STORE_DATA_AREA);
	if (NULL == g_pStoreDataArea)
		goto ERR;

	g_pTmpBuff = NULL;
	if (NULL == g_pTmpBuff)
		g_pTmpBuff = fts_malloc(BUFF_LEN_TMP_BUFFER);
	if (NULL == g_pTmpBuff)
		goto ERR;

	TestResult = NULL;
	if (NULL == TestResult)
		TestResult = fts_malloc(BUFF_LEN_TMP_BUFFER);
	if (NULL == TestResult)
		goto ERR;

	return 0;

ERR:
	FTS_TEST_ERROR("fts_malloc memory failed in function.");
	return -EPERM;

}

/************************************************************************
* Name: FreeMemory
* Brief:  Release pointer memory
* Input: none
* Output: none
* Return: none
***********************************************************************/
void FreeMemory(void)
{

	if (NULL != g_pStoreMsgArea)
		fts_free(g_pStoreMsgArea);

	if (NULL != g_pMsgAreaLine2)
		fts_free(g_pMsgAreaLine2);

	if (NULL != g_pStoreDataArea)
		fts_free(g_pStoreDataArea);

	if (NULL != g_pTmpBuff)
		fts_free(g_pTmpBuff);
}




