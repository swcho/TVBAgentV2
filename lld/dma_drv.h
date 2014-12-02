//=================================================================	
//	DMA open/close for TVB370/380/390
//
//	Copyright (C) 2007
//	Teleview Corporation
//
//	Author : 
//  	Last Modified : 27 March. 2006
//=================================================================	

#ifndef	__TLV_DMA_DRV_H_
#define	__TLV_DMA_DRV_H_

#include 	"tvb360u.h"

#ifdef __cplusplus
extern "C" {
#endif

//=================================================================	
//
extern	int	fDMA_Busy;

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
class CLldDmaDrv	:	public	CLldTvb360u
{
private:

public:
	CLldDmaDrv(void);
	~CLldDmaDrv();

private:
#if 0 //2010/9/30 PCI/USB MULTIBOARD
#else
#ifdef WIN32
	int Set_PCI9054_Reg(HANDLE hDevice, unsigned char nOffset, DWORD dwData);
#else
	int Set_PCI9054_Reg_usb(usb_dev_handle* hDevice, unsigned char nOffset, DWORD dwData);
	int Set_PCI9054_Reg(int hDevice, unsigned char nOffset, DWORD dwData);
#endif
#endif

public:
	int _stdcall TSPL_GET_DMA_STATUS(void);
	int _stdcall TSPL_PUT_DATA(DWORD dwDMASize,DWORD *dest);
	void* _stdcall TSPL_GET_DATA(long dwDMASize);
	int _stdcall TSPL_SET_FIFO_CONTROL(int nDMADirection, int nDMASize);
	int _stdcall TSPL_GET_FIFO_CONTROL(int nDMADirection, int nDMASize);
	void* _stdcall TSPL_READ_BLOCK(long dwDMASize);
	void	_stdcall TSPL_WRITE_BLOCK_TEST(DWORD *pdwSrcBuff, unsigned long dwBuffSize, DWORD *dest);
#ifdef WIN32
	int _stdcall TSPL_WRITE_BLOCK(DWORD dwBuffSize,DWORD *dest);
#else
	int TSPL_WRITE_BLOCK(DWORD *pdwSrcBuff, ULONG dwBuffSize, DWORD *dest);
#endif
	void* _stdcall TSPL_GET_DMA_ADDR(void);
	int _stdcall TSPL_GET_DMA_REG_INFO(int nOffset);
	int _stdcall TSPL_SET_DMA_REG_INFO(unsigned char nOffset, DWORD dwData);
	int TSPL_SET_DEMUX_CONTROL_TEST(int nState);

};

#ifdef __cplusplus
}
#endif



#endif
