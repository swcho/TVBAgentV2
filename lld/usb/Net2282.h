/******************************************************************************

Copyright (C) 2003, NetChip Technology, Inc. (http://www.netchip.com)

THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND, 
EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED 
WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A PARTICULAR PURPOSE.

NET2282.H
  
NetChip NET2282 chip constants

These NET2282 register and bit field definitions are extracted 
from the NET2282 specification.

******************************************************************************/

////////////////////////////////////////////////////////////////////////////////////
// NET2282 register and bit field definitions:
//  - Definitions extracted from NET2282 chip specification
//  - Use the shift operator ('<<') to build bit masks where needed in your code: 
//       if (ChipReg & (1<<BIT_FIELD_NAME)) {DoSomething();}
////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////
#ifndef NET2282_H
#define NET2282_H

////////////////////////////////////////////////////////////////////////////////////
// NET2282 chip revisions
//  - See CHIPREV register
////////////////////////////////////////////////////////////////////////////////////
#define CHIPREV_REV_1                                       0x00000100      // NET2282 Rev-1 (first silicon)
#define CHIPREV_REV_1A                                      0x00000110      // NET2282 Rev-1A (mask change)
#define CHIPREV_EXPECTED                                    CHIPREV_REV_1A  // Build expects this CHIPREV

////////////////////////////////////////////////////////////////////////////////////
// PCI Configuration Registers
//  - Defined in PCI 2.3 specification
////////////////////////////////////////////////////////////////////////////////////

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define PCIVENDID                                           0x000

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define PCIDEVID                                            0x002

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define PCICMD                                              0x004
#define     INTERRUPT_DISABLE                                       10
#define     FAST_BACK_TO_BACK_ENABLE                                9
#define     SERR_ENABLE                                             8
#define     ADDRESS_STEPPING_ENABLE                                 7
#define     PARITY_CHECKING_ENABLE                                  6
#define     VGA_PALETTE_SNOOP                                       5
#define     MEMORY_WRITE_AND_INVALIDATE_ENABLE                      4
#define     SPECIAL_CYCLES                                          3
#define     BUS_MASTER_ENABLE                                       2
#define     MEMORY_SPACE_ENABLE                                     1
#define     IO_SPACE_ENABLE                                         0

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define PCISTAT                                             0x006
#define     PARITY_ERROR_DETECTED                                   15
#define     SERR_ASSERTED                                           14
#define     MASTER_ABORT_RECEIVED                                   13
#define     TARGET_ABORT_RECEIVED                                   12
#define     TARGET_ABORT_ASSERTED                                   11
#define     DEVSEL_TIMING                                           9
#define     DATA_PARITY_ERROR_DETECTED                              8
#define     FAST_BACK_TO_BACK                                       7
#define     USER_DEFINABLE                                          6
#define     CAPABILITY_VALID                                        4
#define     INTERRUPT_STATUS                                        3

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define PCIDEVREV                                           0x008

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define PCICLASS                                            0x009
#define     BASE_CLASS                                              16
#define     SUB_CLASS                                               8
#define     INTERFACE                                               0

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define PCICACHESIZE                                        0x00c

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define PCILATENCY                                          0x00d

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define PCIHEADER                                           0x00e

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define PCIBIST                                             0x00f

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define PCIBASE0                                            0x010
#define     PREFETCH_ENABLE                                         3
#define     ADDRESS_TYPE                                            1
#define     SPACE_TYPE                                              0

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define PCIBASE1                                            0x014

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define PCIBASE2                                            0x018

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define PCIBASE3                                            0x01c

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define PCIBASE4                                            0x020

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define PCIBASE5                                            0x024

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define CARDBUS                                             0x028

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define PCISUBVID                                           0x02c

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define PCISUBID                                            0x02e

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define PCIROMBASE                                          0x030

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define PCICAPPTR                                           0x034

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define PCIINTLINE                                          0x03c

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define PCIINTPIN                                           0x03d

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define PCIMINGNT                                           0x03e

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define PCIMINLAT                                           0x03f

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define PWRMNGID                                            0x040

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define PWRMNGNEXT                                          0x041

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define PWRMNGCAP                                           0x042
#define     PME_SUPPORT                                             11
#define     D2_SUPPORT                                              10
#define     D1_SUPPORT                                              9
#define     DEVICE_SPECIFIC_INITIALIZATION                          5
#define     PME_CLOCK                                               3
#define     PME_VERSION                                             0

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define PWRMNGCSR                                           0x044
#define     PME_STATUS                                              15
#define     DATA_SCALE                                              13
#define     DATA_SELECT                                             9
#define     PME_ENABLE                                              8
#define     POWER_STATE                                             0

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit ///Val ////
#define PWRMNGBRIDGE                                        0x046

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define PWRMNGDATA                                          0x047

////////////////////////////////////////////////////////////////////////////////////
// Main Control Registers
////////////////////////////////////////////////////////////////////////////////////

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define DEVINIT                                             0x000
#define     LOCAL_CLOCK_FREQUENCY                                   8
#define     FORCE_PCI_RESET                                         7
#define     PCI_ID                                                  6
#define     PCI_ENABLE                                              5
#define     FIFO_SOFT_RESET                                         4
#define     CFG_SOFT_RESET                                          3
#define     PCI_SOFT_RESET                                          2
#define     USB_SOFT_RESET                                          1
#define     M8051_RESET                                             0

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define EECTL                                               0x004
#define     EEPROM_ADDRESS_WIDTH                                    23
#define     EEPROM_CHIP_SELECT_ACTIVE                               22
#define     EEPROM_PRESENT                                          21
#define     EEPROM_VALID                                            20
#define     EEPROM_BUSY                                             19
#define     EEPROM_CHIP_SELECT_ENABLE                               18
#define     EEPROM_BYTE_READ_START                                  17
#define     EEPROM_BYTE_WRITE_START                                 16
#define     EEPROM_READ_DATA                                        8
#define     EEPROM_WRITE_DATA                                       0
#define         EEPROM_WRITE_ENABLE_CMD                                     6
#define         EEPROM_WRITE_CMD                                            2
#define         EEPROM_READ_CMD                                             3
#define         EEPROM_READ_STATUS_CMD                                      5
#define         EEPROM_WRITE_STATUS_CMD                                     1
#define         EEPROM_NOT_READY                                            0

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define EECLKFREQ                                           0x008

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define PCICTL                                              0x00C

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define PCIIRQENB0                                          0x010
#define     SETUP_PACKET_INTERRUPT_ENABLE                           7
#define     ENDPOINT_F_INTERRUPT_ENABLE                             6
#define     ENDPOINT_E_INTERRUPT_ENABLE                             5
#define     ENDPOINT_D_INTERRUPT_ENABLE                             4
#define     ENDPOINT_C_INTERRUPT_ENABLE                             3
#define     ENDPOINT_B_INTERRUPT_ENABLE                             2
#define     ENDPOINT_A_INTERRUPT_ENABLE                             1
#define     ENDPOINT_0_INTERRUPT_ENABLE                             0

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define PCIIRQENB1                                          0x014
#define     PCI_INTERRUPT_ENABLE                                    31
#define     POWER_STATE_CHANGE_INTERRUPT_ENABLE                     27
#define     PCI_ARBITER_TIMEOUT_INTERRUPT_ENABLE                    26
#define     PCI_PARITY_ERROR_INTERRUPT_ENABLE                       25
#define	    PCI_INTA_INTERRUPT_ENABLE                               24
#define     PCI_MASTER_ABORT_RECEIVED_INTERRUPT_ENABLE              20
#define     PCI_TARGET_ABORT_RECEIVED_INTERRUPT_ENABLE              19
#define     PCI_RETRY_ABORT_INTERRUPT_ENABLE                        17
#define     PCI_MASTER_CYCLE_DONE_INTERRUPT_ENABLE                  16
#define     VIRTUALIZED_ENDPOINT_INTERRUPT_ENABLE                   15
#define     SOF_DOWNCOUNT_INTERRUPT_ENABLE                          14
#define     GPIO_INTERRUPT_ENABLE                                   13
#define     DMA_D_INTERRUPT_ENABLE                                  12
#define     DMA_C_INTERRUPT_ENABLE                                  11
#define     DMA_B_INTERRUPT_ENABLE                                  10
#define     DMA_A_INTERRUPT_ENABLE                                  9
#define     EEPROM_DONE_INTERRUPT_ENABLE                            8
#define     VBUS_INTERRUPT_ENABLE                                   7
#define     CONTROL_STATUS_INTERRUPT_ENABLE                         6
#define     ROOT_PORT_RESET_INTERRUPT_ENABLE                        4
#define     SUSPEND_REQUEST_INTERRUPT_ENABLE                        3
#define     SUSPEND_REQUEST_CHANGE_INTERRUPT_ENABLE                 2
#define     RESUME_INTERRUPT_ENABLE                                 1
#define     SOF_INTERRUPT_ENABLE                                    0

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define CPUIRQENB0                                          0x018
#define     SETUP_PACKET_INTERRUPT_ENABLE                           7
#define     ENDPOINT_F_INTERRUPT_ENABLE                             6
#define     ENDPOINT_E_INTERRUPT_ENABLE                             5
#define     ENDPOINT_D_INTERRUPT_ENABLE                             4
#define     ENDPOINT_C_INTERRUPT_ENABLE                             3
#define     ENDPOINT_B_INTERRUPT_ENABLE                             2
#define     ENDPOINT_A_INTERRUPT_ENABLE                             1
#define     ENDPOINT_0_INTERRUPT_ENABLE                             0

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define CPUIRQENB1                                          0x01c
#define     CPU_INTERRUPT_ENABLE                                    31
#define     POWER_STATE_CHANGE_INTERRUPT_ENABLE                     27
#define     PCI_ARBITER_TIMEOUT_INTERRUPT_ENABLE                    26
#define     PCI_PARITY_ERROR_INTERRUPT_ENABLE                       25
#define     PCI_INTA_INTERRUPT_ENABLE                               24
#define     PCI_PME_INTERRUPT_ENABLE                                23
#define     PCI_SERR_INTERRUPT_ENABLE                               22
#define     PCI_PERR_INTERRUPT_ENABLE                               21
#define     PCI_MASTER_ABORT_RECEIVED_INTERRUPT_ENABLE              20
#define     PCI_TARGET_ABORT_RECEIVED_INTERRUPT_ENABLE              19
#define     PCI_RETRY_ABORT_INTERRUPT_ENABLE                        17
#define     PCI_MASTER_CYCLE_DONE_INTERRUPT_ENABLE                  16
#define     VIRTUALIZED_ENDPOINT_INTERRUPT_ENABLE                   15
#define     GPIO_INTERRUPT_ENABLE                                   13
#define     DMA_D_INTERRUPT_ENABLE                                  12
#define     DMA_C_INTERRUPT_ENABLE                                  11
#define     DMA_B_INTERRUPT_ENABLE                                  10
#define     DMA_A_INTERRUPT_ENABLE                                  9
#define     EEPROM_DONE_INTERRUPT_ENABLE                            8
#define     VBUS_INTERRUPT_ENABLE                                   7
#define     CONTROL_STATUS_INTERRUPT_ENABLE                         6
#define     ROOT_PORT_RESET_INTERRUPT_ENABLE                        4
#define     SUSPEND_REQUEST_INTERRUPT_ENABLE                        3
#define     SUSPEND_REQUEST_CHANGE_INTERRUPT_ENABLE                 2
#define     RESUME_INTERRUPT_ENABLE                                 1
#define     SOF_INTERRUPT_ENABLE                                    0

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define USBIRQENB1                                          0x024
#define     USB_INTERRUPT_ENABLE                                    31
#define     POWER_STATE_CHANGE_INTERRUPT_ENABLE                     27
#define     PCI_ARBITER_TIMEOUT_INTERRUPT_ENABLE                    26
#define     PCI_PARITY_ERROR_INTERRUPT_ENABLE                       25
#define     PCI_INTA_INTERRUPT_ENABLE                               24
#define     PCI_PME_INTERRUPT_ENABLE                                23
#define     PCI_SERR_INTERRUPT_ENABLE                               22
#define     PCI_PERR_INTERRUPT_ENABLE                               21
#define     PCI_MASTER_ABORT_RECEIVED_INTERRUPT_ENABLE              20
#define     PCI_TARGET_ABORT_RECEIVED_INTERRUPT_ENABLE              19
#define     PCI_RETRY_ABORT_INTERRUPT_ENABLE                        17
#define     PCI_MASTER_CYCLE_DONE_INTERRUPT_ENABLE                  16
#define     VIRTUALIZED_ENDPOINT_INTERRUPT_ENABLE                   15
#define     GPIO_INTERRUPT_ENABLE                                   13
#define     DMA_D_INTERRUPT_ENABLE                                  12
#define     DMA_C_INTERRUPT_ENABLE                                  11
#define     DMA_B_INTERRUPT_ENABLE                                  10
#define     DMA_A_INTERRUPT_ENABLE                                  9
#define     EEPROM_DONE_INTERRUPT_ENABLE                            8
#define     VBUS_INTERRUPT_ENABLE                                   7
#define     CONTROL_STATUS_INTERRUPT_ENABLE                         6
#define     ROOT_PORT_RESET_INTERRUPT_ENABLE                        4
#define     SUSPEND_REQUEST_INTERRUPT_ENABLE                        3
#define     SUSPEND_REQUEST_CHANGE_INTERRUPT_ENABLE                 2
#define     RESUME_INTERRUPT_ENABLE                                 1
#define     SOF_INTERRUPT_ENABLE                                    0

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define IRQSTAT0                                            0x028
#define     INTA_ASSERTED                                           12      // This bit is TRUE when the PCI INTA# pin is being asserted by the NET2282
#define     SETUP_PACKET_INTERRUPT                                  7
#define     ENDPOINT_F_INTERRUPT                                    6
#define     ENDPOINT_E_INTERRUPT                                    5
#define     ENDPOINT_D_INTERRUPT                                    4
#define     ENDPOINT_C_INTERRUPT                                    3
#define     ENDPOINT_B_INTERRUPT                                    2
#define     ENDPOINT_A_INTERRUPT                                    1
#define     ENDPOINT_0_INTERRUPT                                    0

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define IRQSTAT1                                            0x02c
#define     POWER_STATE_CHANGE_INTERRUPT                            27
#define     PCI_ARBITER_TIMEOUT_INTERRUPT                           26
#define     PCI_PARITY_ERROR_INTERRUPT                              25
#define     PCI_INTA_INTERRUPT                                      24
#define     PCI_PME_INTERRUPT                                       23
#define     PCI_SERR_INTERRUPT                                      22
#define     PCI_PERR_INTERRUPT                                      21
#define     PCI_MASTER_ABORT_RECEIVED_INTERRUPT                     20
#define     PCI_TARGET_ABORT_RECEIVED_INTERRUPT                     19
#define     PCI_RETRY_ABORT_INTERRUPT                               17
#define     PCI_MASTER_CYCLE_DONE_INTERRUPT                         16
#define     VIRTUALIZED_ENDPOINT_INTERRUPT                          15
#define     SOF_DOWNCOUNT_INTERRUPT                                 14
#define     GPIO_INTERRUPT                                          13
#define     DMA_D_INTERRUPT                                         12
#define     DMA_C_INTERRUPT                                         11
#define     DMA_B_INTERRUPT                                         10
#define     DMA_A_INTERRUPT                                         9
#define     EEPROM_DONE_INTERRUPT                                   8
#define     VBUS_INTERRUPT                                          7
#define     CONTROL_STATUS_INTERRUPT                                6
#define     ROOT_PORT_RESET_INTERRUPT                               4
#define     SUSPEND_REQUEST_INTERRUPT                               3
#define     SUSPEND_REQUEST_CHANGE_INTERRUPT                        2
#define     RESUME_INTERRUPT                                        1
#define     SOF_INTERRUPT                                           0

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define IDXADDR                                             0x030

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define IDXDATA                                             0x034

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define FIFOCTL                                             0x038
#define     PCI_BASE2_RANGE                                         16
#define     IGNORE_FIFO_AVAILABILITY                                3
#define     PCI_BASE2_SELECT                                        2
#define     FIFO_CONFIGURATION_SELECT                               0
#define     EP_ABCD_1K                                                      0
#define     EP_AB_2K                                                        1
#define     EP_A_2K_BC_1K                                                   2

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define MEMADDR                                             0x040
#define     START                                                   28
#define     DIRECTION                                               27
#define     FIFO_DIAGNOSTIC_SELECT                                  24
#define     MEMORY_ADDRESS                                          0

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define MEMDATA0                                            0x044

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define MEMDATA1                                            0x048

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define GPIOCTL                                             0x050
#define     GPIO3_LED_SELECT                                        12
#define     GPIO3_INTERRUPT_ENABLE                                  11
#define     GPIO2_INTERRUPT_ENABLE                                  10
#define     GPIO1_INTERRUPT_ENABLE                                  9
#define     GPIO0_INTERRUPT_ENABLE                                  8
#define     GPIO3_OUTPUT_ENABLE                                     7
#define     GPIO2_OUTPUT_ENABLE                                     6
#define     GPIO1_OUTPUT_ENABLE                                     5
#define     GPIO0_OUTPUT_ENABLE                                     4
#define     GPIO3_DATA                                              3
#define     GPIO2_DATA                                              2
#define     GPIO1_DATA                                              1
#define     GPIO0_DATA                                              0

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define GPIOSTAT                                            0x054
#define     GPIO3_INTERRUPT                                         3
#define     GPIO2_INTERRUPT                                         2
#define     GPIO1_INTERRUPT                                         1
#define     GPIO0_INTERRUPT                                         0

////////////////////////////////////////////////////////////////////////////////////
// USB Control Registers
////////////////////////////////////////////////////////////////////////////////////

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
// Standard Response (to standard USB requests)
//  - When an STDRSP bit is set, the NET2282 will 
//    automatically respond to the request
#define STDRSP                                              0x080
#define     STALL_UNSUPPORTED_REQUESTS                              31
#define     SET_TEST_MODE                                           16
#define     GET_OTHER_SPEED_CONFIGURATION                           15
#define     GET_DEVICE_QUALIFIER                                    14
#define     SET_ADDRESS__                                           13      // SET_ADDRESS already defined elsewhere
#define     ENDPOINT_SET_CLEAR_HALT                                 12
#define     DEVICE_SET_CLEAR_DEVICE_REMOTE_WAKEUP                   11
#define     GET_STRING_DESCRIPTOR_2                                 10
#define     GET_STRING_DESCRIPTOR_1                                 9
#define     GET_STRING_DESCRIPTOR_0                                 8
#define     GET_SET_INTERFACE                                       6
#define     GET_SET_CONFIGURATION                                   5
#define     GET_CONFIGURATION_DESCRIPTOR                            4
#define     GET_DEVICE_DESCRIPTOR                                   3
#define     GET_ENDPOINT_STATUS                                     2
#define     GET_INTERFACE_STATUS                                    1
#define     GET_DEVICE_STATUS                                       0

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define PRODVENDID                                          0x084
#define     PRODUCT_ID                                              16
#define     VENDOR_ID                                               0

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define RELNUM                                              0x088

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define USBCTL                                              0x08c
#define     SERIAL_NUMBER_INDEX                                     16
#define     PRODUCT_ID_STRING_ENABLE                                13
#define     VENDOR_ID_STRING_ENABLE                                 12
#define     USB_ROOT_PORT_WAKEUP_ENABLE                             11
#define     VBUS_PIN                                                10
#define     TIMED_DISCONNECT                                        9
#define     VIRTUAL_ENDPOINT_ENABLE                                 8
#define     SUSPEND_IMMEDIATELY                                     7
#define     SELF_POWERED_USB_DEVICE                                 6
#define     REMOTE_WAKEUP_SUPPORT                                   5
#define     PME_POLARITY                                            4
#define     USB_DETECT_ENABLE                                       3
#define     PME_WAKEUP_ENABLE                                       2
#define     DEVICE_REMOTE_WAKEUP_ENABLE                             1
#define     SELF_POWERED_STATUS                                     0

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define USBSTAT                                             0x090
#define     HIGH_SPEED                                              7
#define     FULL_SPEED                                              6
#define     GENERATE_RESUME                                         5
#define     GENERATE_DEVICE_REMOTE_WAKEUP                           4

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define XCVRDIAG                                            0x094
#define     FORCE_HIGH_SPEED_MODE                                   31
#define     FORCE_FULL_SPEED_MODE                                   30
#define     USB_TEST_MODE                                           24
#define     LINE_STATE                                              16
#define     TRANSCEIVER_OPERATION_MODE                              2
#define     TRANSCEIVER_SELECT                                      1
#define     TERMINATION_SELECT                                      0

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define SETUP0123                                           0x098
#define     SETUP_BYTE_3                                            24
#define     SETUP_BYTE_2                                            16
#define     SETUP_BYTE_1                                            8
#define     SETUP_BYTE_0                                            0

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define SETUP4567                                           0x09C
#define     SETUP_BYTE_7                                            24
#define     SETUP_BYTE_6                                            16
#define     SETUP_BYTE_5                                            8
#define     SETUP_BYTE_4                                            0

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define OURADDR                                             0x0a4
#define     FORCE_IMMEDIATE                                         7
#define     OUR_USB_ADDRESS                                         0

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define OURCONFIG                                           0x0a8

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define EP_EMPTY                                            0x0ac


/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define VIRT_EP                                             0x0b0
#define     VIRTUAL_IN_INTERRUPTS                                   17
#define     VIRTUAL_OUT_INTERRUPTS                                  1

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define VIRT_EP_ENB                                         0x0b4
#define     VIRTUAL_IN_INTERRUPT_ENABLE                             17
#define     VIRTUAL_OUT_INTERRUPT_ENABLE                            1

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define EP_ENABLE                                           0x0b8

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define EP_DISABLE                                          0x0bc

////////////////////////////////////////////////////////////////////////////////////
//PCI Control Registers
////////////////////////////////////////////////////////////////////////////////////

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
// PCI Master Control
#define PCIMSTCTL                                           0x100
#define     PCI_ARBITER_PARK_SELECT                                 13              // Bits 15:13
#define         PARK_LAST_GRANTEE                                           0       // 000b = Last Grantee
#define         PARK_8051_USB                                               1       // 001b = 8051/USB (default)
#define         PARK_EXTERNAL_REQUESTOR                                     2       // 010b = External Requester
#define         PARK_DMA_CHANNEL_A                                          4       // 100b = DMA Channel A
#define         PARK_DMA_CHANNEL_B                                          5       // 101b = DMA Channel B
#define         PARK_DMA_CHANNEL_C                                          6       // 110b = DMA Channel C
#define         PARK_DMA_CHANNEL_D                                          7       // 111b = DMA Channel D
#define     PCI_MULTI_LEVEL_ARBITER                                 12
#define     PCI_RETRY_ABORT_ENABLE                                  11
#define     DMA_MEMORY_WRITE_AND_INVALIDATE_ENABLE                  10
#define     DMA_READ_MULTIPLE_ENABLE                                9
#define     DMA_READ_LINE_ENABLE                                    8
#define     PCI_MASTER_COMMAND_SELECT                               6
#define         MEM_READ_OR_WRITE                                           0
#define         IO_READ_OR_WRITE                                            1
#define         CFG_READ_OR_WRITE                                           2
#define     PCI_MASTER_START                                        5
#define     PCI_MASTER_READ_WRITE                                   4
#define         PCI_MASTER_WRITE                                            0
#define         PCI_MASTER_READ                                             1
//XXXXXXXXX This was PCI_MASTER_BYTE_WRITE_ENABLES
#define     PCI_MASTER_BYTE_ENABLES                                 0

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
// PCI Master Address
#define PCIMSTADDR                                          0x104

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
// PCI Master Data
#define PCIMSTDATA                                          0x108

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
// PCI Master Status
#define PCIMSTSTAT                                          0x10C
#define     PCI_ARBITER_CLEAR                                       2
#define     PCI_EXTERNAL_ARBITER                                    1
#define     PCI_HOST_MODE                                           0

////////////////////////////////////////////////////////////////////////////////////
// DMA Control Registers:
//  - There are four sets of DMA registers, one for each data endpoint, A, B, C and D
//  - DMA registers sets start at offset 0x180, and 0x20 bytes apart:
//     Offset to endpoint A DMA register set: 0x180
//     Offset to endpoint B DMA register set: 0x1a0
//     Offset to endpoint C DMA register set: 0x1c0
//     Offset to endpoint D DMA register set: 0x1e0
//  - Macros are provided to simplify accessing DMA register sets
//
// DMA Scatter/Gather descriptors (See NET2282 spec, section 8.3:
//  - You may associate the following descriptor DWORDs with NET2282 DMA registers:
//     DMACOUNT:   First DWORD of descriptor (containing DMA Byte Count)
//     DMAADDR:    Second DWORD (containing PCI Starting Address)
//     DMADESC:    Third DWORD (containing Next Descriptor Address)
////////////////////////////////////////////////////////////////////////////////////

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define DMACTL                                              0x180
#define     DMA_ABORT_DONE_INTERRUPT_ENABLE                         27
#define     DMA_PAUSE_DONE_INTERRUPT_ENABLE                         26
#define     DMA_SCATTER_GATHER_DONE_INTERRUPT_ENABLE                25
#define     DMA_CLEAR_COUNT_ENABLE                                  21
#define     DESCRIPTOR_POLLING_RATE                                 19
#define         POLL_CONTINUOUS                                             0
#define         POLL_1_USEC                                                 1
#define         POLL_100_USEC                                               2
#define         POLL_1_MSEC                                                 3
#define     DMA_VALID_BIT_POLLING_ENABLE                            18
#define     DMA_VALID_BIT_ENABLE                                    17
#define     DMA_SCATTER_GATHER_ENABLE                               16
#define     DMA_OUT_AUTO_START_ENABLE                               4
#define     DMA_PREEMPT_ENABLE                                      3
#define     DMA_FIFO_VALIDATE                                       2           // Terminate current S/G entry with short packet
#define     DMA_ENABLE                                              1
#define     DMA_ADDRESS_HOLD                                        0

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define DMASTAT                                             0x184
#define     DMA_ABORT_DONE_INTERRUPT                                27
#define     DMA_PAUSE_DONE_INTERRUPT                                26
#define     DMA_SCATTER_GATHER_DONE_INTERRUPT                       25
#define     DMA_TRANSACTION_DONE_INTERRUPT                          24
#define     DMA_ABORT                                               1
#define     DMA_START                                               0

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define DMACOUNT                                            0x190
#define     VALID_BIT                                               31
#define     DMA_DIRECTION                                           30          // TRUE: USB IN (PCI to USB)
#define     DMA_DONE_INTERRUPT_ENABLE                               29
#define     END_OF_CHAIN                                            28
#define     DMA_SG_FIFO_VALIDATE                                    27
#define         DMA_BYTE_COUNT_MASK                                         ((1<<24)-1)
#define     DMA_BYTE_COUNT                                          0

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define DMAADDR                                             0x194

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define DMADESC                                             0x198
#define     NEXT_DESCRIPTOR_ADDRESS                                 4

////////////////////////////////////////////////////////////////////////////////////
// Dedicated Endpoint Registers
////////////////////////////////////////////////////////////////////////////////////

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define DEP_CFG                                             0x200
// DEP_CFG bits are identical to bits defined in EP_CFG:
//  - ENDPOINT_ENABLE
//  - ENDPOINT_TYPE
//  - ENDPOINT_NUMBER

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define DEP_RSP                                             0x204
// DEP_RSP bits are identical to bits defined in EP_RSP:
//  - SET_NAK_PACKETS
//  - SET_ENDPOINT_TOGGLE
//  - SET_ENDPOINT_HALT
//  - CLEAR_NAK_PACKETS
//  - CLEAR_ENDPOINT_TOGGLE
//  - CLEAR_ENDPOINT_HALT

////////////////////////////////////////////////////////////////////////////////////
// Configurable Endpoint/FIFO Registers
//  - The set of configurable endpoint registers shown below is for endpoint zero.
//  - Each endpoint (EPA through EPF) has a set of registers identical to this one.
//  - EP_CFG for EPA is at 0x320, EP_CFG for EPB is at 0x340, ...
////////////////////////////////////////////////////////////////////////////////////

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define EP_CFG                                              0x300
#define     ENDPOINT_BYTE_COUNT                                     16
#define     ENDPOINT_ENABLE                                         10
#define     ENDPOINT_TYPE                                           8
#define     ENDPOINT_DIRECTION                                      7
#define     ENDPOINT_NUMBER                                         0

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
// Endpoint Byte Count is a special sub-register in EP_CFG:
//  - Endpoint Byte Count should be treated as an independent 16 
//    bit register; it does not align with other NET2282 registers
//  - Usage: to terminate an IN transfer with a short packet 
//    set Endpoint Byte Count to 0, 1, 2, 3, or 4 then write
//    the final (32 bit) word of the transfer to EP_DATA. The
//    NET2282 will automatically write the word to the FIFO and
//    validate the FIFO with 0, 1, 2, 3 or 4 bytes.
//  - EP_DATA must be written even if Endpoint Byte Count is set to zero
//  - Endpoint Byte Count defaults to 4, and is restored to 4 after
//    every write to EP_DATA (or FIFO Flush)
// Care must be taken for terminating transfers that are exact packet 
// multiples. (i.e transfers that are an exact multiple of the endpoint
// MaxPacketSize.) These transfers require Zero Length Packets (ZLPs) to
// follow the final data packet:
//  - After writing the final data to EP_DATA, software must make sure
//    there is room available in the FIFO for at least 4 bytes before 
//    setting Endpoint Byte Count to zero, then writing EP_DATA
#define EP_BYTE_COUNT                                       0x302
                                                                    
/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define EP_RSP                                              0x304
#define     SET_NAK_PACKETS                                         15
#define     SET_EP_HIDE_STATUS_PHASE                                14
#define     SET_ENDPOINT_FORCE_CRC_ERROR                            13
#define     SET_INTERRUPT_MODE                                      12
#define     SET_CONTROL_STATUS_PHASE_HANDSHAKE                      11
#define     SET_NAK_OUT_PACKETS_MODE                                10
#define     SET_ENDPOINT_TOGGLE                                     9
#define     SET_ENDPOINT_HALT                                       8
#define     CLEAR_NAK_PACKETS                                       7
#define     CLEAR_EP_HIDE_STATUS_PHASE                              6
#define     CLEAR_ENDPOINT_FORCE_CRC_ERROR                          5
#define     CLEAR_INTERRUPT_MODE                                    4
#define     CLEAR_CONTROL_STATUS_PHASE_HANDSHAKE                    3
#define     CLEAR_NAK_OUT_PACKETS_MODE                              2
#define     CLEAR_ENDPOINT_TOGGLE                                   1
#define     CLEAR_ENDPOINT_HALT                                     0

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define EP_IRQENB                                           0x308
#define     SHORT_PACKET_OUT_DONE_INTERRUPT_ENABLE                  6
#define     SHORT_PACKET_TRANSFERRED_INTERRUPT_ENABLE               5
#define     DATA_PACKET_RECEIVED_INTERRUPT_ENABLE                   3
#define     DATA_PACKET_TRANSMITTED_INTERRUPT_ENABLE                2
#define     DATA_OUT_PING_TOKEN_INTERRUPT_ENABLE                    1
#define     DATA_IN_TOKEN_INTERRUPT_ENABLE                          0

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define EP_STAT                                             0x30c
#define     FIFO_VALID_COUNT                                        24
#define     HIGH_BANDWIDTH_OUT_TRANSACTION_PID                      22
#define     TIMEOUT                                                 21
#define     USB_STALL_SENT                                          20
#define     USB_IN_NAK_SENT                                         19
#define     USB_IN_ACK_RCVD                                         18
#define     USB_OUT_NAK_SENT                                        17
#define     USB_OUT_ACK_SENT                                        16
#define     ENABLE_STATUS_CHANGE_INTERRUPT                          12
#define     FIFO_FULL                                               11
#define     FIFO_EMPTY                                              10
#define     FIFO_FLUSH                                              9
#define     FIFO_VALIDATE                                           8
#define     SHORT_PACKET_TRANSFERRED_STATUS                         7
#define     SHORT_PACKET_OUT_DONE_INTERRUPT                         6
#define     SHORT_PACKET_TRANSFERRED_INTERRUPT                      5
#define     NAK_PACKETS                                             4
#define     DATA_PACKET_RECEIVED_INTERRUPT                          3
#define     DATA_PACKET_TRANSMITTED_INTERRUPT                       2
#define     DATA_OUT_PING_TOKEN_INTERRUPT                           1
#define     DATA_IN_TOKEN_INTERRUPT                                 0


/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define EP_AVAIL                                            0x310

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define EP_DATA                                             0x314

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define EP_DATA1                                            0x318
#define     ENDPOINT_DATA_END_OF_PACKET                             4
#define     ENDPOINT_DATA_BYTE_ENABLES                              0

////////////////////////////////////////////////////////////////////////////////////
// Indexed Registers
//  - Indexed registers are 32 bits wide, and accessed by 
//    selecting a register ordinal in IDXADDR then reading 
//    or writing its content via IDXDATA.
////////////////////////////////////////////////////////////////////////////////////

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define DIAG                                                0x0
#define     RETRY_COUNTER                                           16
#define     FORCE_PCI_SERR                                          11
#define     FORCE_PCI_INTERRUPT                                     10
#define     FORCE_USB_INTERRUPT                                     9
#define     FORCE_CPU_INTERRUPT                                     8
#define     ILLEGAL_BYTE_ENABLES                                    5
#define     FAST_TIMES                                              4
#define     FORCE_RECEIVE_ERROR                                     2
#define     FORCE_TRANSMIT_CRC_ERROR                                0

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define PKTLEN                                              0x1

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define FRAME                                               0x2

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define CHIPREV                                             0x3

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define UFRAME                                              0x4

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define FRAME_COUNT                                         0x5

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define HS_MAXPOWER                                         0x6

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define FS_MAXPOWER                                         0x7

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define HS_INTPOLL_RATE                                     0x8

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define FS_INTPOLL_RATE                                     0x9

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define HS_NAK_RATE                                         0xA

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define SCRATCH                                             0xB

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define EP_A_HS_MAXPKT                                      0x20
#define     ADDITIONAL_TRANSACTION_OPPORTUNITIES                    11
#define     HIGH_SPEED_MAX_PACKET_SIZE                              0

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define EP_A_FS_MAXPKT                                      0x21

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define EP_B_HS_MAXPKT                                      0x30
#define EP_B_FS_MAXPKT                                      0x31

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define EP_C_HS_MAXPKT                                      0x40
#define EP_C_FS_MAXPKT                                      0x41

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define EP_D_HS_MAXPKT                                      0x50
#define EP_D_FS_MAXPKT                                      0x51

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define EP_E_HS_MAXPKT                                      0x60
#define EP_E_FS_MAXPKT                                      0x61

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define EP_F_HS_MAXPKT                                      0x70
#define EP_F_FS_MAXPKT                                      0x71

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define STATIN_HS_INTPOLL_RATE                              0x84

/////// Reg/Bit/Val /////////////////////////////////////// Reg /// Bit /// Val ////
#define STATIN_FS_INTPOLL_RATE                              0x85

////////////////////////////////////////////////////////////////////////////////////
// Miscellaneous NET2282 constants
////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////
// Configurable endpoints:
//  - EP0, EPA, EPB, ..., EPF
//  - Endpoint numbers map one-to-one to their bit positions in IRQSTAT0
//  - Base address for each endpoint's registers reside at a fixed address
//  - Tip: Last two endpoints (EPE and EPF) have limited resources
//  - Use macros to get address of a configurable endpoint register:
//     AddressOf_EpStat_For_EpB = EPPAGEOFFSET(EPB) + EP_STAT;
#define EP0                                                 ENDPOINT_0_INTERRUPT
#define EPA                                                 ENDPOINT_A_INTERRUPT
#define EPB                                                 ENDPOINT_B_INTERRUPT
#define EPC                                                 ENDPOINT_C_INTERRUPT
#define EPD                                                 ENDPOINT_D_INTERRUPT
#define EPE                                                 ENDPOINT_E_INTERRUPT
#define EPF                                                 ENDPOINT_F_INTERRUPT

////////////////////////////////////////////////////////////////////////////////////
// Dedicated endpoints:
//  - CFGOUT, CFGIN, PCIOUT, PCIIN, STATIN
//  - Use macros to get address of a dedicated endpoint register:
//     AddressOf_DepCfg_For_PciIn = DEDEPPAGEOFFSET(PCIIN) + DEP_CFG;
#define CFGOUT                                              0
#define CFGIN                                               1
#define PCIOUT                                              2
#define PCIIN                                               3
#define STATIN                                              4

// Helper macros for dedicated endpoints
#define SIZEOF_DEDICATEDEP                                  0x10
#define DEDEPPAGEOFFSET(dep)                              ((dep)*SIZEOF_DEDICATEDEP)
//XXXXXXXXXX #define NUM_DED_EPREGISTERS                                 4
#define FIRSTDEDEPREG                                       DEP_CFG
#define DEDENDPOINT_COUNT                                   5


////////////////////////////////////////////////////////////////////////////////////
#define MAX_ENDPOINT_PER_DIRECTION                          0xF
#define ENDPOINT_NUMBER_BITS                                0xF
#define MAX_ENDPOINT_BUFFER_SIZE                            0x400

////////////////////////////////////////////////////////////////////////////////////
#define NUMDMACHANNELREGISTERS                              8
#define FIRSTDMACHANNELREG                                  DMACTL

#define SIZEOF_DMACTLREG                                    0x20
#define DMACHANNELOFFSET(ep)                                ((ep-1)*SIZEOF_DMACTLREG)
#define DMACHANNEL_INT_ENABLE_BIT(ep)                       (ep+8)

#define SIZEOF_ENDPOINT                                     0x20    // Number of bytes in a set of Endpoint/FIFO registers
#define NUMEPREGISTERS                                      8
#define EPPAGEOFFSET(ep)                                    ((ep)*SIZEOF_ENDPOINT)
#define FIRSTEPREG                                          EP_CFG
#define EP_HS_MAXPKT(ep)                                    ((ep)*0x10+0x10)
#define EP_FS_MAXPKT(ep)                                    ((ep)*0x10+0x11)

////////////////////////////////////////////////////////////////////////////////////
#define DMACHANNEL_COUNT                                    4

#define EP0_MAX_PACKET_SIZE                                 0x40

#define FIRST_PHYSICAL_ENDPOINT                             EPA
#define LAST_PHYSICAL_ENDPOINT                              EPD

#define FIRST_USER_ENDPOINT                                 EPA
#define LAST_USER_ENDPOINT                                  EPD
#define ENDPOINT_COUNT                             (LAST_USER_ENDPOINT+1)

////////////////////////////////////////////////////////////////////////////////////
// NET2282 8051 microcontroller
////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////
#define MAX8051PROGRAMSIZE                                  0x8000

////////////////////////////////////////////////////////////////////////////////////
#endif // NET2282_H

////////////////////////////////////////////////////////////////////////////////////
//  End of file
////////////////////////////////////////////////////////////////////////////////////
