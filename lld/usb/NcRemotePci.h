/******************************************************************************

Copyright (C) 2005, PLX Technology, Inc. (http://www.plxtech.com)

THIS CODE AND INFORMATION IS THE PROPERTY OF PLX TECHNOLOGY, INC. IT MAY
ONLY BE USED WITH PLX TECHNOLOGY PRODUCTS. NO REPRODUCTION OR DISTRIBUTION
IS ALLOWED WITHOUT THE EXPRESS WRITTEN PERMISSION OF PLX TECHNOLOGY.

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.

NCREMOTEPCI.H

PLX support: If you have technical problems, comments or feedback about 
this product, please contact us: www.plxtech.com/support/. Please 
include "RPCI" in the subject. In the body, please provide the 
software release code (e.g. RE010203), the NetChip chip you are using, 
plus any helpful details, including History trace logs with your comments
  
******************************************************************************/

///////////////////////////////////////////////////////////////////////////////
#ifndef REMOTEPCI_H
#define REMOTEPCI_H

//sskim20061226
//#include "usb.h"
#include "../../libusb-0.1.12/usb.h"

#ifdef __cplusplus
    #define NC_RPCI_API extern "C" 
#else
    #define NC_RPCI_API
#endif

//sskim
/*
#ifdef NC_REMOTEPCI_INTERNAL
    #define NC_RPCI_EXP __declspec(dllexport) __stdcall
#else
    #define NC_RPCI_EXP __declspec(dllimport) __stdcall
#endif
*/
#define NC_RPCI_EXP
typedef long LONG;
typedef unsigned long* PULONG;
typedef unsigned long* PLARGE_INTEGER;

#if 1
typedef unsigned int DWORD;
typedef unsigned int* PDWORD;
typedef unsigned long ULONG;
typedef unsigned char UCHAR;
typedef void VOID;
typedef void* PVOID;
typedef unsigned short USHORT;
typedef char* PCHAR;
typedef unsigned char* PBYTE;
#ifndef UINT32
typedef unsigned long UINT32;
#endif

typedef usb_dev_handle*  WDU_DEVICE_HANDLE;
	
#ifndef NULL
#define NULL 0UL
#endif

//#define CLIENT_USING_NET2280_DMA

#endif

#include "Net2282.h"

///////////////////////////////////////////////////////////////////////////////
// User-mode emulation:
//  - This RPCI kernel mode interface file can be compiled for user-mode 
//    by special user-mode RPCI applications
//  - In user mode compilations, various kernel objects must be defined
#ifndef DRIVER
// Compiling in a non-driver (i.e user-mode) context
typedef PVOID PDEVICE_OBJECT;
typedef PVOID PMDL;
typedef LONG USBD_STATUS;
typedef LONG NTSTATUS;
typedef PVOID PIRP;
typedef PVOID PKEVENT;

#define NT_SUCCESS(Status) ((NTSTATUS)(Status) >= 0)

typedef PVOID PNC_RPCI_INIT_STRUCT;
#undef NC_RPCI_EXP
#define NC_RPCI_EXP
#endif

///////////////////////////////////////////////////////////////////////////////
// PnP notification GUID:
//  - RPCI client start/stop notification. 
//  - RPCI registers this GUID and calls IoSetDeviceInterfaceState() when 
//    client calls RPCI's start and stop functions
//  - Applications can register to be notified of events on this GUID. 
//  - Tip: This GUID persists in Registry
/*
#include <initguid.h>
// From MS GuidGen.EXE:
// {36F0248C-9934-431d-90A7-3AF66D721E8B}
DEFINE_GUID(GUID_CLASS_REMOTE_PCI, 
0x36f0248c, 0x9934, 0x431d, 0x90, 0xa7, 0x3a, 0xf6, 0x6d, 0x72, 0x1e, 0x8b);
*/

///////////////////////////////////////////////////////////////////////////////
// Reference to RPCI's private Device Extension (PRX)
//  - PRX is passed  to virtually all RPCI functions
//  - PRX is created in AddDevice(). It is usually referenced in the client's
//    device extension as "PNC_RPCI_EXT prx"
//  - PRX is private to RPCI. The client should never need to know any 
//    contents of PRX
//sskim
/*
struct _NC_RPCI_EXT;                
typedef struct _NC_RPCI_EXT* PNC_RPCI_EXT;
*/
#define PNC_RPCI_EXT WDU_DEVICE_HANDLE
//+

///////////////////////////////////////////////////////////////////////////////
// Shift a single bit:
//  - Usage example: Shift one bit for DMA_DONE_INTERRUPT: NC_BIT(DMA_DONE_INTERRUPT)
#define NC_BIT(shift) (1<<shift)

///////////////////////////////////////////////////////////////////////////////
typedef struct _RPCI_OUTPUT_STRUCT_DRIVER_VERSION
{   // BCD coded driver version: MajorVersion, Year, Month, Day. 
    //  - Example: Version 1, 2002, March, 4th would be 
    //    coded as {0x01, 0x02, 0x03, 0x04}
    UCHAR       MajorVersion;
    UCHAR       Year;
    UCHAR       Month;
    UCHAR       Day;
} RPCI_OUTPUT_STRUCT_DRIVER_VERSION, *PRPCI_OUTPUT_STRUCT_DRIVER_VERSION;

///////////////////////////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
#ifdef DRIVER
#if !defined(_NTDDK_)
    #error You must include WDM.H or NTDDK.H before NcRemotePci.h
#endif

///////////////////////////////////////////////////////////////////////////////
typedef struct _NC_RPCI_INIT_STRUCT 
{
    // Size of this structure. Initialize this to sizeof(NC_RPCI_INIT_STRUCT)
    ULONG Size;

    // The device object being registered. This would generally be returned 
    // by IoCreateDevice()
    PDEVICE_OBJECT DeviceObject;

    // Next lower device object (expected to be MS-USBD). Set this equal 
    // to the return value from IoAttachDeviceToDeviceStack() for instance
    PDEVICE_OBJECT Ldo;

    // Dedicated endpoint mapping
    //  - These entries allow you to use different endpoint settings from 
    //    the NET2280 default dedicated endpoint settings
    //  - Recommended: Specify zero to use NET2280 default mapping
    //  - Specify valid USB data endpoint (0x01, 0x81, ...) to match endpoint
    //    programming and endpoint descriptors in the USB configuration
    //  - These settings must *match* device settings (These settings 
    //    do NOT change the device's settings!)
    //  - Tip: If you are not sure, specify zero for all dedicated endpoints
    UCHAR UsbEp_CfgIn;          // CFGIN default : 0x8d
    UCHAR UsbEp_CfgOut;         // CFGOUT default: 0x0d
    UCHAR UsbEp_PciIn;          // PCIIN default : 0x8e
    UCHAR UsbEp_PciOut;         // PCIOUT default: 0x0e
    UCHAR UsbEp_StatIn;         // STATIN default: 0x8f

} NC_RPCI_INIT_STRUCT, *PNC_RPCI_INIT_STRUCT;
#endif

///////////////////////////////////////////////////////////////////////////////
// NET2280 dedicated endpoints
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// NET2280 dedicated endpoint USB addresses:
//  - Following are NET2280 default endpoint addresses. Reprogram addresses
//    using DEP_CFG. (Direction, bit 7, is hardcoded, and cannot be changed.)
//  - Advanced! Most implementations ignore these addresses, letting RPCI 
//    use NET2280 default address settings
//  - See NET2280 spec: 11.9.1: (DEP_CFG) Dedicated Endpoint Configuration
#define CFGIN_EP_ADDRESS    0x8d
#define CFGOUT_EP_ADDRESS   0x0d
#define PCIIN_EP_ADDRESS    0x8e
#define PCIOUT_EP_ADDRESS   0x0e
#define STATIN_EP_ADDRESS   0x8f

///////////////////////////////////////////////////////////////////////////////
// NET2280 dedicated endpoint structures:
//  - These structures are used with NET2280 dedicated endpoints for accessing
//    various NET2280 and remote PCI device registers
//  - Advanced! RPCI clients normally don't need these structures. RPCI exports
//    common register access functions making these structures unnecessary. 
//    However, if your PCI chip has uncommon register access requirements, 
//    it may be necessary to use these structures.
//  - See Net2280RegWrite() and Net2280RegRead()
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// NET2280 register accesses types
typedef enum _CFG_SPACE
{   // Space Select options for Config Register accesses
    //  - Advanced! RPCI clients normally don't use these enumerations. 
    //  - Space Select is handled by RPCI for common access usages
    //  - See NET2280 spec: 7.6.5.1: CFGOUT Endpoint: Space Select:
    PCI_CONFIG = (0<<4),            // PCI Configuration Registers (E.g. Device ID, Vendor ID)
    MEMORY_MAPPED_CONFIG = (1<<4),  // NET2280 Memory Mapped (PCIBAR0) Configuration Registers (E.g. DEVINIT, EECTL, PCIIRQENB0)
    CPU_8051_RAM = (2<<4)           // 8051 Program RAM (PCIBAR1)
} NC_CFG_SPACE;

///////////////////////////////////////////////////////////////////////////////
// NET2280 CFGOUT endpoint structure
//  - CFGOUT is used for configuration register writes, and to select the 
//    address for configuration register reads
//  - Advanced! CFGOUT is handled by RPCI for common access usages
//  - Refer to NET2280 spec: 7.6.5.1: CFGOUT Endpoint
//  - Override compiler's default packing alignment to comply 
//    with NET2280 CFGOUT endpoint
/*
#pragma pack (push)
#pragma pack (1)
*/
typedef struct _NC_CFGOUT_PACKET
{   // Packet format for CFGOUT transfers
    UCHAR SpaceSelByteEnables;
    UCHAR Reserved1;
    USHORT Address;
    USHORT Reserved2;
    ULONG Data;         // Tip: In this struct, Data is *not* 4-byte aligned!
} NC_CFGOUT_PACKET, *PNC_CFGOUT_PACKET;
//#pragma pack (pop)

///////////////////////////////////////////////////////////////////////////////
// NET2280 PCIOUT endpoint structure
//  - PCIOUT is used to initiate PCI master writes, and to select the 
//    address for PCI master register reads
//  - Advanced! PCIOUT is handled by RPCI for common access usages
//  - Refer to NET2280 spec: 7.6.5.3: PCIOUT Endpoint
//  - Override compiler's default packing alignment to comply 
//    with NET2280 CFGOUT endpoint
/*
#pragma pack (push)
#pragma pack (1)
*/
typedef struct _NC_PCIOUT_PACKET
{   // Packet format for PCIOUT transfers
    USHORT PciMstCtl;
    ULONG PciMstAddr;
    ULONG PciMstData;
} NC_PCIOUT_PACKET, *PNC_PCIOUT_PACKET;
//#pragma pack (pop)


///////////////////////////////////////////////////////////////////////////////
// 
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
typedef struct _NC_URB
{   // NetChip URB:
    //  - This structure applies to USB data transfers for Bulk or Interrupt 
    //    endpoints
    //  - Allocation: Client allocates persistent storage for this structure.
    //  - Ownership and persistence: This structure must persist (i.e. it 
    //    must not be deallocated) between the start of the transfer (when 
    //    the client calls NcRpci_StartUsbTransfer() and the completion of 
    //    the transfer (e.g. when RPCI calls CompletionCallback()). The
    //    structure is owned by RPCI during this time, and must not be
    //    modified by the client.
    //  - All members (except transfer result members) must be set *before*
    //    client calls NcRpci_StartUsbTransfer()
    //  - Some components in this structure are taken directly from (and 
    //    used in) MS-USBD URB. (MSDN: See _URB_BULK_OR_INTERRUPT_TRANSFER)
    //  - RPCI supports concurrent transfers, but it does not support queueing.
    //    That is, transfers can be started on multiple endpoints but for any
    //    single endpoint, its transfer must complete before starting another.

    ///////////////////////////////////////////////////////////////////////////
    // Client context:
    //  - Client can safely put anything here, at any time.
    PVOID ClientContext;

    ///////////////////////////////////////////////////////////////////////////
    // USB endpoint:
    //  - Specify a valid USB endpoint address, e.g. 0x01, 0x81, ...
    UCHAR UsbEp;

    ///////////////////////////////////////////////////////////////////////////
    // Host-side transfer address and length:
    //  - These members are put directly into a standard MS-USBD URB
    //  - For instance, if Windows passes your driver a 0x1234 byte buffer at
    //    kernel address 0xfc8e3900, you could pass 0xfc8e3900 for the
    //    Transfer Buffer, and 0x1234 for the Transfer Buffer Length. 
    //  - See MSDN and MS USB.H: _URB_BULK_OR_INTERRUPT_TRANSFER
    ULONG TransferBufferLength;
    PVOID TransferBuffer;
    PMDL TransferBufferMDL;     // Optional

    ///////////////////////////////////////////////////////////////////////////
    // Transfer completion methods:
    //  - RPCI provides several transfer completion methods:
    //     - Passive-level completion callback
    //     - Dispatch-level completion callback
    //     - Event synchronization object
    //  - Use one method: It is strongly advised to choose one completion
    //    method in a single request. RPCI does not restrict you from
    //    applying combinations in one request, however, results are not 
    //    specifically defined or guaranteed.
    ///////////////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////////////
    // Passive completion callback (recommended)
    //  - Client specifies a completion callback handler here
    //  - RPCI calls this handler when a USB transfer completes
    //  - Your client handler runs at PASSIVE_LEVEL
    //  - When called, ownership of the NC URB structure returns to your
    //    client: it can be recycled or freed
    //  - If successful, client handler may safely call NcRpci_StartUsbTransfer() 
    //    to start another transfer
    //  - Your client may safely call RPCI register access functions
    //  - Specify NULL to prevent RPCI from calling this handler
    VOID (*CompletionCallback)(PDEVICE_OBJECT, struct _NC_URB *);

    ///////////////////////////////////////////////////////////////////////////
    // Dispatch completion callback (Advanced):
    //  - Advanced! It is generally advised to set this entry to NULL
    //  - Client specifies a completion callback handler here
    //  - RPCI calls this handler when a USB transfer completes
    //  - Your client handler runs at DISPATCH_LEVEL
    //  - Because it runs at Dispatch, the handler must observe many
    //    restrictions. For instance RPCI register access functions
    //    cannot be called at Dispatch.
    //  - When called, ownership of the NC URB structure returns to your
    //    client: it can conditionally be recycled or freed. 
    //  - If successful, client handler may safely call NcRpci_StartUsbTransfer() 
    //    to start another transfer
    //  - Specify NULL to prevent RPCI from calling this handler
    VOID (*CompletionCallbackDispatch)(PDEVICE_OBJECT, struct _NC_URB *);

    ///////////////////////////////////////////////////////////////////////////
    // Completion event (Easy, less flexible)
    //  - Client specifies an initialized NT synchronization object, e.g. 
    //    see MSDN: KeInitializeEvent(). (Tip: A single event object can be 
    //    recycled; usually one per endpoint. Be sure to clear event between
    //    uses (i.e. be sure to call KeClearEvent()!))
    //  - When the USB transfer completes, RPCI calls KeSetEvent().
    //  - Client can wait for completion, e.g. see MSDN KeWaitForSingleObject()
    //  - Specify NULL to prevent RPCI from setting this event
    PKEVENT CompletionEvent;

    // USB transfer result:
    //  - Client completion handler (CompletionCallback) should examine results
    ULONG BytesTransferred;
    NTSTATUS NtStatus;
} NC_URB, *PNC_URB;


///////////////////////////////////////////////////////////////////////////////
// NET2280 DMA support
//  - RPCI's structures for NET2280 DMA
//  - Alternatively, your client can manually program NET2280 DMA or you can 
//    use your PCI adapter's DMA controller
//  - If you are using a non-NET2280 DMA controller, program it using RPCI NT 
//    HAL abstraction functions (e.g. NcRpci_WriteRegisterUlong())
//  - See NcRpci_KickNcDma()
//  - Before using NET2280 DMA, DMA registers must be mapped. See NcRpci_MapNcDma
//  - RPCI does not support NET2280 scatter/gather DMA
//  - Applies equally to NET2282
//  - See NET2280 spec: 
//     - Chapter 8: DMA Contoller
//     - 11.8: DMA Controller Registers
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
typedef struct _NC_DMA
{   // NET2280 DMA controller structure
    //  - Optional structure passed to NcRpci_StartUsbTransfer(); required
    //    structure passed to NcRpci_KickNcDma().
    //  - Applies only when using the NET2280 DMA controllers for data 
    //    transfers. (Otherwise pass NULL as the NC_DMA parameter.)
    //  - The NC_DMA structure includes values passed to NET2280 DMA
    //    registers such as the remote PCI transfer address and NET2280 DMA
    //    controller flags.
    //  - This structure must be allocated and initialized by the RPCI client,
    //    however its persistence is not required. After starting a transfer,
    //    or kicking the DMA controller NC_DMA memory can be safely discarded.
    //
    // Client context:
    //  - Client can safely put anything here (at any time)
    PVOID ClientContext;


    // Remote PCI address:
    //  - Client sets this to an address in a remote PCI BAR space. 
    //  - For instance if your PCI chip has a memory block mapped to PCI 
    //    space at 0x10000, you can set this Transfer Address to 0x10000.
    //  - Transfer address is written to NET2280 DMAADDR register
    ULONG RemotePciAddress;

    // Transfer control
    //  - Transfer control is written to NET2280 DMACTL register
    //  - Client may specify only the following fields. Other DMACTL fields 
    //    are managed by RPCI (See NET2280 spec for DMACTL):
    //     - DMA Address Hold (Specify TRUE to prevent address increment)
    //     - DMA FIFO Validate
    ULONG DmaControl;
} NC_DMA, *PNC_DMA;


///////////////////////////////////////////////////////////////////////////////
// RPCI functions called by your client:
//  - Clients call these functions to remotely control their PCI devices
//    and the NET2280 over USB.
//  - Function groups include:
//     - Plug and play functions: Initialize, start, stop and remove device
//     - NET2280 chip I/O functions: Configure NET2280 to work with your device
//     - Configuration register functions: Setup your PCI config registers
//     - NT HAL replacement functions: Chip I/O and interrupts
//     - Data transfer functions: Initiate large USB IN and OUT data transfers
//     - Optional: NET2280 DMA functions (or use your chip's DMA controller)
//     - RPCI driver management, such as version and initialization
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//	NC_RPCI_API RPCI_OUTPUT_STRUCT_DRIVER_VERSION NC_RPCI_EXP NcRpci_GetVersion(VOID);

///////////////////////////////////////////////////////////////////////////////
// RPCI initialization
//  - Called from client's AddDevice function
//  - Tip: For best results, see PLX sample client driver sources and docs!
//	NC_RPCI_API ULONG       NC_RPCI_EXP GetSizeofRemotePciExtension(VOID);
//	NC_RPCI_API NTSTATUS    NC_RPCI_EXP InitializeRemotePciExtension(PNC_RPCI_EXT, PNC_RPCI_INIT_STRUCT);

///////////////////////////////////////////////////////////////////////////////
// Plug and Play functions
//  - Client PnP functions must call these matching functions. E.g. your
//    StartDevice() function must call NcRpci_StartDevice()

///////////////////////////////////////////////////////////////////////////////
// Start Device:
//  - Your StartDevice() handler calls NcRpci_StartDevice().
//  - On success, the NET2280-based USB device is in a configured state
//  - Successful enumeration is a significant event: It means the NET2280 
//    and your remote PCI device is now ready for programming! Typically
//    you will setup NET2280 PCI configuration registers. (See 
//    Net2280RegWrite_UlongConfig()) EEPROM tip: An on-board EEPROM might 
//    also program NET2280 registers!
//  - From the USB device perspective, NcRpci_StartDevice() handles standard
//    USB enumeration only. It fetches standard USB descriptors, parses the 
//    configuration, and as the final step in enumeration, requests MS-USBD 
//    to "Set the Configuration." Although NcRpci_StartDevice() sets up
//    endpoint structures to be used later, it does not access any NET2280 
//    PCI or PCI configuration registers.
//  - Be sure to examine return code for success. If not successful, it is 
//    likely that the device did not enter the configured state; nothing
//    else will work. Return the failure status so the Device Manager can
//    show an appropriate message to the user.
NC_RPCI_API NTSTATUS    NC_RPCI_EXP NcRpci_StartDevice(PNC_RPCI_EXT);

NC_RPCI_API VOID        NC_RPCI_EXP NcRpci_StopDevice(PNC_RPCI_EXT);
NC_RPCI_API VOID        NC_RPCI_EXP NcRpci_RemoveDevice(PNC_RPCI_EXT);

///////////////////////////////////////////////////////////////////////////////
// Is the RPCI system operational?
//  - NcRpci_IsOperational() returns STATUS_SUCCESS if the device and 
//    driver are believed to be in a usable, operational PnP state. 
//  - This function has fairly rare applicability: For instance, if your
//    driver loops 'forever' reading chip registers, it should occasionally 
//    call this function from within the loop. Otherwise, if the device is
//    disconnected during the loop, the device might appear to 'hang'; the
//    drivers will not stop and unload properly.
//  - Note: RPCI safely handles your calls even if the device or driver
//    is in a non-operational PnP state. This function is provided for
//    special cases where you might loop 'forever' calling RPCI's functions 
//    that do not provide return codes, such as the Library's NT-HAL register 
//    access functions.
//  - This function relies on recent plug and play results of the device
//    and driver. For instance when NcRpci_StartDevice() returns success,
//    the PnP state is changed to 'operational.' If MS-USBD ever
//    returns 'disconnected' or if NcRpci_StopDevice() is called, 
//    the PnP state is set to a non-operational value.
//  - This is a fast function. It simply tests RPCI's most-recent PnP state
//    and returns. It does not pass requests to MS-USBD or other drivers.
NC_RPCI_API NTSTATUS    NC_RPCI_EXP NcRpci_IsOperational(PNC_RPCI_EXT);

///////////////////////////////////////////////////////////////////////////////
// NET2280 PCI configuration register read/write ULONG functions
//  - These functions generate PCI read/write configuration cycles to NET2280.
//  - See NET2280 spec 11.3
NC_RPCI_API NTSTATUS    NC_RPCI_EXP Net2280RegWrite_UlongConfig (PNC_RPCI_EXT prx,  USHORT Address, ULONG Data);
NC_RPCI_API NTSTATUS    NC_RPCI_EXP Net2280RegRead_UlongConfig  (PNC_RPCI_EXT prx,  USHORT Address, PULONG Data);


///////////////////////////////////////////////////////////////////////////////
// NET2280 memory mapped read/write ULONG register functions
//  - Use these functions to access NET2280 (PCIBAR0) configuration registers
//  - See NET2280 spec 11.3
NC_RPCI_API NTSTATUS    NC_RPCI_EXP Net2280RegWrite_UlongBar0   (PNC_RPCI_EXT prx,  USHORT Address, ULONG Data);
NC_RPCI_API NTSTATUS    NC_RPCI_EXP Net2280RegRead_UlongBar0    (PNC_RPCI_EXT prx,  USHORT Address, PULONG Data);


///////////////////////////////////////////////////////////////////////////////
// Full NET2280 configuration register read/write access functions
//  - These two functions expose complete (and fairly complex) capabilities
//    of the NET2280's register access functionality. 
//  - Caller must provide a properly formatted Configuration OUT (CFGOUT) packet
//    specifying the PCI address, Space Select, Byte Enables and data
//  - See NET2280 spec: 7.6.5.1
//  - Commonly used functions (such as reading a NET2280 local ULONG  
//    configuration register) are provided elsewhere, with easier-to-use 
//    interfaces
//  - Use these functions when the common variations cannot be applied
NC_RPCI_API NTSTATUS    NC_RPCI_EXP Net2280RegWrite (PNC_RPCI_EXT prx,  PNC_CFGOUT_PACKET  CfgOutPacket);
NC_RPCI_API NTSTATUS    NC_RPCI_EXP Net2280RegRead  (PNC_RPCI_EXT prx,  PNC_CFGOUT_PACKET  CfgOutPacket,   PULONG Data);


///////////////////////////////////////////////////////////////////////////////
// Remote PCI configuration register read/write functions
//  - These functions generate PCI read/write configuration cycles to your
//    remotely controlled PCI device
NC_RPCI_API VOID    NC_RPCI_EXP NcRpci_WriteConfigUlong         (PNC_RPCI_EXT prx,  ULONG Config,   ULONG Value); 
NC_RPCI_API ULONG   NC_RPCI_EXP NcRpci_ReadConfigUlong          (PNC_RPCI_EXT prx,  ULONG Config);

///////////////////////////////////////////////////////////////////////////////
// Remote PCI HAL functions
//  - These functions replace a PCI driver's existing chip register accesses
//  - They are modeled strongly on the NT HAL functions; an extra parameter,
//    a pointer to RPCI's private extension (usually called 'prx') is required
//  - Cost: Any access to the PCI device over USB takes significantly more
//    time than direct PCI. Every access requires one (for write) or two (for
//    read) USB transfers. Tip: Minimize device accesses as much as possible!
//  - Advanced tip: For minimizing device accesses over USB, the NET2280's
//    on-board 8051 MCU might improve performance.
//
// Write single value to memory mapped register
NC_RPCI_API VOID    NC_RPCI_EXP NcRpci_WriteRegisterUchar       (PNC_RPCI_EXT prx,  ULONG Register, UCHAR Value);
NC_RPCI_API VOID    NC_RPCI_EXP NcRpci_WriteRegisterUshort      (PNC_RPCI_EXT prx,  ULONG Register, USHORT Value);
NC_RPCI_API VOID    NC_RPCI_EXP NcRpci_WriteRegisterUlong       (PNC_RPCI_EXT prx,  ULONG Register, ULONG Value); 

// Write single value to I/O port
NC_RPCI_API VOID    NC_RPCI_EXP NcRpci_WritePortUchar           (PNC_RPCI_EXT prx,  ULONG Port,     UCHAR Value);
NC_RPCI_API VOID    NC_RPCI_EXP NcRpci_WritePortUshort          (PNC_RPCI_EXT prx,  ULONG Port,     USHORT Value);
NC_RPCI_API VOID    NC_RPCI_EXP NcRpci_WritePortUlong           (PNC_RPCI_EXT prx,  ULONG Port,     ULONG Value);

// Read single value from memory mapped register
NC_RPCI_API UCHAR   NC_RPCI_EXP NcRpci_ReadRegisterUchar        (PNC_RPCI_EXT prx,  ULONG Register);
NC_RPCI_API USHORT  NC_RPCI_EXP NcRpci_ReadRegisterUshort       (PNC_RPCI_EXT prx,  ULONG Register);
NC_RPCI_API ULONG   NC_RPCI_EXP NcRpci_ReadRegisterUlong        (PNC_RPCI_EXT prx,  ULONG Register);

// Read single value from I/O port
NC_RPCI_API UCHAR   NC_RPCI_EXP NcRpci_ReadPortUchar            (PNC_RPCI_EXT prx,  ULONG Port);
NC_RPCI_API USHORT  NC_RPCI_EXP NcRpci_ReadPortUshort           (PNC_RPCI_EXT prx,  ULONG Port);
NC_RPCI_API ULONG   NC_RPCI_EXP NcRpci_ReadPortUlong            (PNC_RPCI_EXT prx,  ULONG Port);

//XXXXXXXXXXXXXXXXXXX Still need support for NT HAL's 12 "buffered" variations (NcRpci_ReadRegisterBufferUlong(), etc.)

///////////////////////////////////////////////////////////////////////////////
// USB interrupt support
//  - Interrupts supported through NET2280 STATIN endpoint
//  - See NET2280 spec: 7.6.5.5 STATIN Endpoint

///////////////////////////////////////////////////////////////////////////////
// Normalize client callback parameter passed to NcRpci_InitializeUsbInterrupt
//  - This typedef casts the Device Object parameter to PVOID allowing *any*
//    type of device object to be applied. 
//  - Standard WDM clients pass their Functional Device Object; an AVStream 
//    client however might pass their PKSDEVICE.
typedef VOID(*NC_CLIENT_CALLBACK)(PVOID AnyDeviceObject, ULONG Interrupt_IrqStat1);

///////////////////////////////////////////////////////////////////////////////
// Initialize USB interrupts
//  - Enable (or disable) interrupt subsystem on remote USB device
//  - NET2280 uses dedicated endpoint STATIN for interrupt support
//  - See NET2280 spec: Chapter 9
//  - Caller specifies enable bits (UsbIrqEnb1). Minimally, USB Interrupt Enable,
//    plus at least one other bit must be set to enable interrupts; clear all
//    bits to disable interrupts. See NET2280 spec: 11.5.8 USBIRQENB1
//  - After this initialization, client arms for USB interrupts by 
//    calling NcRpci_ArmUsbInterrupt()
//  - Callback: Client supplies an interrupt callback function. On an interrupt
//    (which is when a STATIN transfer completes) the client's callback 
//    is called (at PASSIVE_LEVEL) with the supplied Device Object 
//    and value matching NET280 IRQSTAT1. 
//  - Re-arming: The client's callback function can re-arm for another 
//    interrupt by calling NcRpci_ArmUsbInterrupt() again. The client must 
//    not write to IRQSTAT1, or interrupt events might become lost.
//  - Be sure to check error codes: If a STATIN transfer completes with
//    an error, it is considered a broken interrupt. The callback function
//    is not called.
//  - Disabling: Specify zero for UsbIrqEnb1. !!IMPORTANT!! All parameters 
//    must be valid when disabling. The client's Interrupt Callback routine
//    must be prepared to run (at least one time) after disabling.
//  - Advanced tip: Client does not have to use RPCI's interrupt
//    support. RPCI includes this mechanism for ease-of-use. Interrupts
//    can be managed "manually" by any client using STATIN and various RPCI 
//    functions
NC_RPCI_API NTSTATUS NC_RPCI_EXP 
NcRpci_InitializeUsbInterrupt(
    PNC_RPCI_EXT prx, 
    ULONG UsbIrqEnb1,   // See NET2280 spec: 11.5.8 USBIRQENB1
    NC_CLIENT_CALLBACK InterruptCallback,
    PVOID AnyDeviceObject
    );

///////////////////////////////////////////////////////////////////////////////
// Arm for USB interrupt
//  - Interrupts supported using NET2280 STATIN endpoint
//  - Arming the interrupt simply means starting a USB transfer on 
//    the NET2280 STATIN endpoint. When an enabled NET2280 interrupt event
//    occurs (via USBIRQENB1), the NET2280 completes the transfer, returning 
//    a value matching NET2280 IRQSTAT1 (four bytes)
//  - If successful, return code is STATUS_PENDING, indicating the transfer
//    on the STATIN endpoint is waiting for an interrupt event. (Tip: 
//    NcRpci_ArmUsbInterrupt simply calls NcRpci_StartUsbTransfer)
//  - RPCI must be initialized for USB interrupts first. 
//  - See NcRpci_InitializeUsbInterrupt()
NC_RPCI_API NTSTATUS NC_RPCI_EXP 
NcRpci_ArmUsbInterrupt(
    PNC_RPCI_EXT prx
    );


///////////////////////////////////////////////////////////////////////////////
// NET2280 DMA controller functions:
//  - The NET2280 features four on-board DMA controllers. These controllers 
//    are optimized for USB transfers. They are optional; you are free to use
//    your own PCI controller's DMA as well. (You can even mix and match!)
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Map DMA controllers (Optional):
//  - One-time NET2280 DMA preparation function called during initialization
//  - Not required if you do not use the NET2280's DMA controllers
//  - Establishes NET2280 "endpoint to DMA controller" mapping
//  - Your Start Device handler calls this function once after 
//    NcRpci_StartDevice() returns successfully.
NC_RPCI_API NTSTATUS    NC_RPCI_EXP NcRpci_MapNcDma(PNC_RPCI_EXT prx);

///////////////////////////////////////////////////////////////////////////////
// Start DMA cycles on an endpoint using a NET2280 DMA controller (Optional):
//  - This function programs and starts a NET2280 DMA controller independently
//    of a USB transfer.
//  - Typically this function is not used. It is provided for special cases,
//    perhaps where a DMA transfer is fragmented over a single USB transfer.
//  - In most cases, clients can call NcRpci_StartUsbTransfer() to start
//    the USB transfer and (optionally) program and start the NET2280 DMA 
//    controller.
NC_RPCI_API NTSTATUS    NC_RPCI_EXP NcRpci_KickNcDma(PNC_RPCI_EXT, PNC_URB, PNC_DMA);

///////////////////////////////////////////////////////////////////////////////
// USB transfer functions
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Start USB transfer:
//  - Call this function to start a USB transfer on an endpoint. It applies
//    when using your own DMA controller or the NET2280 DMA controller.
//  - (Optional) Specify a structure to start the NET2280 DMA controller
//  - Parameters (See structure definitions for more details):
//     - PNC_URB: Your client specifies a USB endpoint, a buffer, a
//       completion callback function (and more) in this structure.
//     - NC_DMA: (Optional) This parameter applies when your transfer uses
//       a NET2280 DMA controller. When using your own DMA controller set 
//       this parameter to NULL and program your DMA controller independently. 
//  - Return code is normally STATUS_PENDING, indicating the USB transfer
//    started, but has not completed. Any other code indicates an error.
//  - This function can safely be called from IRQL <= DISPATCH_LEVEL
NC_RPCI_API NTSTATUS    NC_RPCI_EXP NcRpci_StartUsbTransfer(PNC_RPCI_EXT, PNC_URB, PNC_DMA);


///////////////////////////////////////////////////////////////////////////////
// Cancel USB transfer:
//  - Call this function to cancel a USB transfer
//  - This function calls IoCancelIrp() on the an IRP being used by the 
//    endpoint specified in the NC_URB. The return value is the same value
//    returned from IoCancelIrp(). See MSDN for more details. 
//  - This function performs host-side operations only. It does not access 
//    the remote device.
//  - This function returns immediately. Later, MS-USBD completes your
//    transfer with an error (STATUS_CANCELLED); the Library completes your 
//    canceled transfer normally.
//  - Use with care! Canceling a transfer might create synchronization 
//    problems. For instance, you might stop your DMA controller then cancel,
//    but a few packets might still be 'in-transit' over the USB or 
//    in NET2282 FIFOs. 
//  - Beware of race conditions. The transfer might complete normally *after*
//    your call to cancel, but before MS-USBD actually cancels the transfer.
//  - Callers must be running at IRQL <= DISPATCH_LEVEL
NC_RPCI_API ULONG       NC_RPCI_EXP NcRpci_CancelUsbTransfer(PNC_RPCI_EXT, PNC_URB);

///////////////////////////////////////////////////////////////////////////////
// Cancel USB transfer and wait:
//  - Call this function to cancel a USB transfer and wait for it to complete.
//  - Operational overview: 
//     - Calls IoCancelIrp() on the an IRP being used by the endpoint 
//       specified in the NC_URB. 
//     - Calls KeWaitForSingleObject() to allow MS-USBD time to cancel
//     - MS-USBD completes your transfer with error (STATUS_CANCELLED)
//     - Library completes your canceled transfer normally.
//  - Timeout (optional): Provide (LARGE_INTEGER) Timeout to override 200mSec
//    default timeout. See MSDN: KeWaitForSingleObject() for Timeout 
//    parameter format.
//  - This function performs host-side operations only. It does not access 
//    the remote device.
//  - Return codes:
//     - STATUS_SUCCESS: IoCancelIrp() canceled the IRP and MS-USBD completed
//       the transfer within the Timeout limit.
//     - STATUS_UNSUCCESSFUL: IoCancelIrp() returned FALSE (IRP not cancelable)
//     - Failure status code from KeWaitForSingleObject(), e.g. STATUS_TIMEOUT
//  - Callers must be running at IRQL == PASSIVE_LEVEL
NC_RPCI_API NTSTATUS    NC_RPCI_EXP NcRpci_CancelUsbTransferAndWait(PNC_RPCI_EXT, PNC_URB, PLARGE_INTEGER);


///////////////////////////////////////////////////////////////////////////////
// Debug, history and monitor support
//  - Used during development
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Get PRX
//  - This function should only be used by debug and development tools. See
//    PLX's RPCI monitor driver and user-mode apps.
//  - A monitor driver uses this function to acquire the 'handle' of one RPCI 
//    device. In the code, the handle is usually named PRX). PRX references 
//    a portion of a device extension reserved for RPCI, usually created 
//    at AddDevice time.
//  - The monitor only needs to do get PRX once, during its initialization.
//  - Members of PRX are opaque to the monitor. Just like any client, PRX is
//    simply passed as a parameter to all RPCI library functions.
//  - Not guaranteed to be supported in Free-Build versions
//  - Characteristics of this function are subject to change; not guaranteed.
NC_RPCI_API PNC_RPCI_EXT NC_RPCI_EXP NcRpciMon_GetPrx(ULONG DeviceInstance);


///////////////////////////////////////////////////////////////////////////////
// History support:
//  - History (AKA Trace Logging) applies to your driver's development cycle
//  - Think of trace logging as a logic analyzer for software
//  - The Checked-Build of RPCI logs information on every critical code 
//    branch to a circular buffer. When an unexpected event occurs the logs 
//    can be studied to help understand why things happened the way they did.
//  - If needed, trace logs can be emailed to PLX support for expert analysis
//  - Dumping trace logs: RPCI monitor tools can acquire and show trace logs
//    in user-mode or they can be displayed using a kernel debugger (KD, 
//    WinDbg). RPCI start-up messages should show trace log buffer addresses.
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
typedef struct _NC_HISTORY_ENTRY
{   // A single history (or trace log) entry looks like this:
    ULONG Text;     // Human readable keyword  (e.g. "Intr", "TxEn", "Susp")
    ULONG Arg1;     // Anything you want (e.g. any register, pointer, variable)
    ULONG Arg2;     // Anything you want (e.g. any register, pointer, variable)
    ULONG Arg3;     // Anything you want (e.g. any register, pointer, variable)
} NC_HISTORY_ENTRY, *PNC_HISTORY_ENTRY;

///////////////////////////////////////////////////////////////////////////////
typedef struct _NC_HISTORY_INFO
{   // Describe current history array:
    //  - History's base address, number of entries, current entry
    //  - History is a circular array of multi-byte entries
    PNC_HISTORY_ENTRY HistoryBase;
    ULONG HistoryIndex;
    ULONG HistoryMaxEntries;
    ULONG SizeofHistoryEntry;
} NC_HISTORY_INFO, *PNC_HISTORY_INFO;

///////////////////////////////////////////////////////////////////////////////
// Get updated information on the history trace log
//	NC_RPCI_API PNC_HISTORY_INFO    NC_RPCI_EXP NcRpci_GetHistoryInfo(VOID);

///////////////////////////////////////////////////////////////////////////////
// Add history entry:
//  - Client calls here to add one entry to RPCI's history log
//  - This feature helps you visualize how your client inter-operates 
//    with RPCI. 
//	NC_RPCI_API VOID                NC_RPCI_EXP NcRpci_History(PCHAR, ULONG, ULONG, ULONG);


///////////////////////////////////////////////////////////////////////////////
// Debug print volume 
//  - Get (or set) debugging print volume level
//  - Pass a print volume level (0 through 5)
//     0: All messages suppressed
//     5: All possible messages printed
//  - Recommendation: Use 5 during earliest development, reducing the number 
//    as development progresses. Avoid using zero, as it blocks even serious
//    errors from printing.
//  - Pass negative one (-1) to read the current volume setting
//  - Checked build only! Does not apply to Free build
//	NC_RPCI_API ULONG               NC_RPCI_EXP NcRpciDebug_PrintVolume(ULONG);


/******************************************************************************
Trace log debugging reference:
 - The following trace log reference is extracted from RPCI library source
   files. All of RPCI's trace logging calls are shown. (Disclaimer: This 
   list is believed to be accurate and up-to-date, but cannot be guaranteed!)
 - As RPCI driver code executes, its critical code paths call RPCI's HISTO() 
   macro. HISTO quickly adds compact entries sequentially to RPCI's trace 
   log. (The log is a large circular buffer of NC_HISTORY_ENTRY structures.)
 - Tip: If RPCI's PrintVolume is set high enough, HISTO prints in real-time 
   to the kernel's debug console (DbgPrint()). High print volume is 
   recommended during early debug and development. See NcRpciDebug_PrintVolume. 
 - At low print volume levels, HISTO still adds entries into the trace log 
   buffer. The log can be dumped later for analysis. (For instance, you can
   dump the log after an overnight stress-test unexpectedly stops!)
 - Use an RPCI developer's tool (e.g. RpciMon.EXE) to dump RPCI's history 
   log from user-mode.
 - Each line in your log indicates what RPCI is doing, and in what order.
 - As problems arise, you examine your trace log, matching keywords in this 
   list to keywords from your trace log. (Keywords are short strings, up to 
   four-characters, e.g. "Upc"). 
 - Three parameters (to the right of a keyword) might offer helpful clues.
 - Parenthesis tip: By convention, RPCI's development team usually surround
   substantial calls with parenthesis. E.g. between "Upc(" and ")Upc" you
   might see more entries due to other RPCI calls or callbacks to your client.
 - Trace logs apply to RPCI's Checked-build drivers only.
 - History volume: Like RPCI's print volume, trace logging supports 
   a "History Volume" concept. Under default conditions all lines below 
   can appear in a trace log except for lines with VOLUME_MAXIMUM. (These 
   lines tend to fill the trace log with unimportant entries.) Default:
     gHistoryVolume = DEFAULT_HISTORY = VOLUME_HIGH
 - Need help with trace logs? Submit your trace log results (with your
   in-line diagnosis and comments please!) to RPCI@plxtech.com.

///////////////////////////////////////////////////////////////////////////////
// RPCI trace log reference:

HISTO(DEFAULT_HISTORY, "prx", 0, 0, size);  // GetSizeofRemotePciExtension()
HISTO(DEFAULT_HISTORY, "RPCI", ris, prx, (VERSION_MAJOR<<24) + (VERSION_YEAR<<16) + (VERSION_MONTH<<8) + (VERSION_DAY<<0)); // InitializeRemotePciExtension
HISTO(VOLUME_MINIMUM, "Vol", PrintVolume, &gPrintVolume, gPrintVolume); // NcRpciDebug_PrintVolume()
HISTO(DEFAULT_HISTORY, "St!!", -1, -1, -1); // AllStop in SendAwaitUrb()
HISTO(VOLUME_MAXIMUM, "SAU~", NtStatus, 0, prx->PnpState);    // SendAwaitUrb()
HISTO(VOLUME_MAXIMUM, "SAU", Irp, prx, urb);    // SendAwaitUrb()
HISTO(VOLUME_MINIMUM, "SAU!", NtStatus, IoStatus.Status, urb->UrbHeader.Status);
HISTO(DEFAULT_HISTORY, "Fre", EpInfo->Irp, EpInfo->Urb, EpInfo->UsbdPipeInfo->EndpointAddress); // FreeEpResources()
HISTO(DEFAULT_HISTORY, "Invl", prx, 0, 0);  // InvalidateConfiguration()
HISTO(DEFAULT_HISTORY, "Cfg0", prx, 0, 0);  // SelectConfigurationZero()
HISTO(DEFAULT_HISTORY, "SCfg", prx, 0, RequestedConfiguration); // SelectConfiguration()
HISTO(DEFAULT_HISTORY, "RStt", prx->PnpState, prx, prx->DeviceObject);  // NcRpci_StartDevice()
HISTO(DEFAULT_HISTORY, "ViPi", prx, prx->dd.idVendor, prx->dd.idProduct);   // NcRpci_StartDevice(): VID and PID
HISTO(DEFAULT_HISTORY, "Conf", ii, pcd, size);  // NcRpci_StartDevice(): Configuration info
HISTO(DEFAULT_HISTORY, "RStp", prx->PnpState, prx, prx->DeviceObject);  // NcRpci_StopDevice()
HISTO(DEFAULT_HISTORY, "RRmv", prx->PnpState, prx, prx->DeviceObject);  // NcRpci_RemoveDevice()
HISTO(VOLUME_MAXIMUM, "IsOp", 0, 0, prx->PnpState);     // NcRpci_IsOperational()
HISTO(DEFAULT_HISTORY, "Cncl", EpInfo->Irp, NcUrb->UsbEp, EpInfo->Irp->IoStatus.Information);   // NcRpci_CancelUsbTransfer()
HISTO(DEFAULT_HISTORY, "C&W(", EpInfo->Irp, NcUrb->UsbEp, EpInfo->Irp->IoStatus.Status);   // NcRpci_CancelUsbTransferAndWait()
HISTO(DEFAULT_HISTORY, ")C&W", NtStatus, NcUrb->UsbEp, Canceled);   // NcRpci_CancelUsbTransferAndWait()
HISTO(DEFAULT_HISTORY, "Upc", EpInfo->Irp, EpInfo->UsbdPipeInfo->EndpointAddress, NtStatus);    // PassiveUrbComplete()
HISTO(DEFAULT_HISTORY, "Upc(", NtStatus, NcUrb->UsbEp, NcUrb->BytesTransferred);    // PassiveUrbComplete()
HISTO(DEFAULT_HISTORY, "Upc!", URB_STATUS(EpInfo->Urb), NtStatus, Irp->IoStatus.Information);   // PassiveUrbComplete(): Transfer not perfectly successful!
HISTO(DEFAULT_HISTORY, ")Upc", CompletionCount, EpInfo->CompletionCount, 0);    // PassiveUrbComplete()
HISTO(DEFAULT_HISTORY, "UCmp", Irp->IoStatus.Status, EpInfo->NcUrb->UsbEp, EpInfo->Urb->UrbBulkOrInterruptTransfer.TransferBufferLength);   // OnUrbComplete(): USB transfer completed from MS-USBD
HISTO(DEFAULT_HISTORY, "UCD(", NcUrb->CompletionCallbackDispatch, 0, 0);    // OnUrbComplete(): Completion callback at DISPATCH_LEVEL
HISTO(DEFAULT_HISTORY, ")UCD", 0, 0, 0);    // OnUrbComplete
HISTO(DEFAULT_HISTORY, "UCEv", NcUrb->NtStatus, NcUrb->UsbEp, NcUrb->BytesTransferred); // OnUrbComplete(): Event completion
HISTO(DEFAULT_HISTORY, "UCPa", EpInfo->CompletionCount, 0, 0);  // OnUrbComplete(): Queue passive-level work item
HISTO(DEFAULT_HISTORY, "Stp!", -1, -1, -1); // NcRpci_StartUsbTransfer(): Stopping due to AllStop!
HISTO(DEFAULT_HISTORY, "USt(", EpInfo->Irp, NcUrb->UsbEp, NcUrb->TransferBufferLength); // NcRpci_StartUsbTransfer()
HISTO(DEFAULT_HISTORY, "USt~", NtStatus, 0, prx->PnpState); // NcRpci_StartUsbTransfer()
HISTO(DEFAULT_HISTORY, ")USt", 0, prx->MsUsbdDevObj, NtStatus); // NcRpci_StartUsbTransfer()
HISTO(DEFAULT_HISTORY, "Int(", 0, prx->Interrupt_AnyDeviceObject, prx->Interrupt_IrqStat1);  // OnStatinInterrupt(): Calling client interrupt handler
HISTO(DEFAULT_HISTORY, ")Int", 0, 0, 0);    // OnStatinInterrupt(): Return from calling client interrupt handler
HISTO(DEFAULT_HISTORY, "IniI", 0, 0, UsbIrqEnb1);   // NcRpci_InitializeUsbInterrupt()
HISTO(DEFAULT_HISTORY, "ArmI", 0, Dbg_IrqStat1, prx->Interrupt_IrqStat1);   // NcRpci_ArmUsbInterrupt()
HISTO(DEFAULT_HISTORY, "DMap", 0, 0, 0);    // NcRpci_MapNcDma()
HISTO(DEFAULT_HISTORY, "!Ep", PhysEp, 0, EpCfg);    // NcRpci_MapNcDma(): Endpoint not enabled?
HISTO(DEFAULT_HISTORY, "PhEp", PhysEp, EpInfo->NcDmaOffset, EpCfg); // NcRpci_MapNcDma(): Map physical to USB EP
HISTO(DEFAULT_HISTORY, "KDma", NcDma->RemotePciAddress, DmaCount, DmaCtl);  // NcRpci_KickNcDma()
HISTO(VOLUME_HIGH, "80Wr", CfgOutPacket->SpaceSelByteEnables, CfgOutPacket->Address, CfgOutPacket->Data);
HISTO(VOLUME_HIGH, "80Ad", CfgOutPacket->SpaceSelByteEnables, CfgOutPacket->Address, Data);
HISTO(VOLUME_HIGH, "80Rd", NtStatus, CfgOutPacket->Address, *Data);
HISTO(VOLUME_HIGH, "RgWr", PciOutPacket->PciMstCtl, PciOutPacket->PciMstAddr, PciOutPacket->PciMstData);
HISTO(VOLUME_MAXIMUM, "RgAd", PciOutPacket->PciMstCtl, PciOutPacket->PciMstAddr, Data);
HISTO(VOLUME_HIGH, "RgRd", PciOutPacket->PciMstCtl, PciOutPacket->PciMstAddr, *Data);

******************************************************************************/

///////////////////////////////////////////////////////////////////////////////
// PCI enumeration
//  - Required only when using an "unknown" PCI adapter: We dynamically
//    configure the PCI device. It shouldn't be needed once a single, 
//    known PCI adapter chip is connected. Then fixed addresses can be applied!
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// On the NET2280 and NET2282 RDK boards, IDSEL is routed to AD16
//  - New hardware designs can place IDSEL on any AD pin above AD15
#define IDSEL (1<<16)

///////////////////////////////////////////////////////////////////////////////
// PCI BARs:
//  - Minimum address spacing (there's plenty of room!)
#define BASE_INCREMENT          0x10000

/************************************************************************/
/* IO Control Command 							*/
/************************************************************************/
#define IOCTL_TEST_TSP				0x800
#define IOCTL_WRITE_TO_MEMORY		0x801
#define IOCTL_READ_FROM_MEMORY	0x802
#define IOCTL_READ_PCI9054_REG		0x803
#define IOCTL_WRITE_PCI9054_REG	0x804
#define IOCTL_TSP100_EPLD_DOWN	0x805
#define IOCTL_TVB350_EPLD_DOWN	0x806
#define IOCTL_FILE_DATA_DOWN		0x807
#define IOCTL_FILE_DATA_UP			0x808
#define IOCTL_GET_DMA_MEMORY		0x809

// TVB595V1
#define IOCTL_REGISTER_EVENT		0x901
#define IOCTL_SIGNAL_EVENT			0x902
#define IOCTL_SET_FIFO_CONTROL		0x903

#define IOCTL_END_OF_APPLICATION		0x811
#define IOCTL_GET_FAST_DOWN			0x812
#define IOCTL_READ_PEX8111_PCI_EXPRESS_REG		0x813
#define IOCTL_READ_PEX8111_PCI_LOCAL_REG		0x814
#define IOCTL_ENABLED_RW_EEPROM			0x815
#define IOCTL_READ_EEPROM_AT25640		0x816
#define IOCTL_WRITE_EEPROM_AT25640		0x817
#define IOCTL_OSRUSBFX2_NET2282_READ		0x910

// TVB595V1
#define DMA_BUFFER_SIZE	0x100000

extern "C"	ULONG DispatchControl(WDU_DEVICE_HANDLE hDevice, void* SystemBuffer, int ControlCode);
extern "C"	void StartDevice(WDU_DEVICE_HANDLE hDevice);

//kslee:modify ========================================================================================
extern "C"	int Net2282_Eeprom_Read(WDU_DEVICE_HANDLE hDevice, UINT32 offset, USHORT *value);
extern "C"	int Net2282_EepromDataRead(WDU_DEVICE_HANDLE hDevice, UCHAR *data);

extern "C"	int Net2282_EepromWait(WDU_DEVICE_HANDLE hDevice);
extern "C"	int Net2282_Eeprom_CheckBusy(WDU_DEVICE_HANDLE hDevice);

extern "C"	int Net2282_Eeprom_Write(WDU_DEVICE_HANDLE hDevice, UINT32 offset, USHORT value);
extern "C"	int Net2282_EepromDataWrite(WDU_DEVICE_HANDLE hDevice, UCHAR data);
//==================================================================================================
#ifdef __cplusplus
extern "C" 
{
#endif

//	void StartDevice(WDU_DEVICE_HANDLE hDevice);

#ifdef __cplusplus
}
#endif

///////////////////////////////////////////////////////////////////////////////
#define USB_TRANSFER_REQUEST	_IOWR('U', 100, struct USB_TRANSFER_PARAM)

/* Define these values to match your devices */
#define USB_VENDOR_DEFAULT_ID		0x0525 
#define USB_PRODUCT_DEFAULT_ID	0x1820

#define USB_VENDOR_ID	0x9445 
#define USB_PRODUCT_ID	0x4961

struct USB_TRANSFER_PARAM {
	unsigned long dwPipeNum;
	unsigned long fRead; 
    	unsigned long dwOptions; 
    	unsigned long* pBuffer; 
    	unsigned long dwBufferSize; 
    	unsigned long* pdwBytesTransferred; 
       unsigned char* pSetupPacket; 
       unsigned long dwTimeout;
};

///////////////////////////////////////////////////////////////////////////////
// End of file
///////////////////////////////////////////////////////////////////////////////

#endif  // REMOTEPCI_H
