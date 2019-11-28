#include <p18cxxx.h>
#include <stdio.h>
#include "usb.h"

// Note: there are timing related problems associated with GET_FEATURE
// when run at less than 48 MHz
#define CLK_48MHZ 1

#pragma config PLLDIV=5, CPUDIV=OSC1_PLL2, USBDIV=2, FOSC=HSPLL_HS// FCMEM=OFF
#pragma config IESO=OFF, PWRT=OFF, BOR=ON_ACTIVE, BORV=3, VREGEN=ON, WDT=OFF
#pragma config MCLRE=OFF, PBADEN=OFF, CCP2MX=OFF, STVREN=ON, LVP=OFF, XINST=OFF, DEBUG=OFF

// HID feature buffer
volatile unsigned char HIDFeatureBuffer[HID_FEATURE_REPORT_BYTES];

unsigned char PreviousButtonStates =0;

#pragma code low_vector=0x8
void low_interrupt (void)
{
}


#pragma code

// Allocate buffers in RAM for storage of bytes that have either just
// come in from the SIE or are waiting to go out to the SIE.
char txBuffer[HID_INPUT_REPORT_BYTES];
char rxBuffer[HID_OUTPUT_REPORT_BYTES];
unsigned int timestamp=0;
extern byte transferType;

// Entry point for user initialization
void UserInit(void)
{
	LATA=0;
	TRISA=0b11111000;
	LATB=0;
    TRISB=0;
    T0CON=0x80;			// Timer0 period = T*2^16:2 = 5.46ms:2 = 10.93ms (PS=2)
}

#define LOBYTE(x) (*((char *)&x))
#define HIBYTE(x) (*(((char *)&x)+1))
// If we got some bytes from the host, then echo them back.

static void SendUSBMessage(void)
{
	byte i;
	// put first byte in PORTB
	LATB=rxBuffer[0];
	// x01 to signal interrupt transfer
	LATBbits.LATB1=0;
	LATBbits.LATB0=1;
	
	//read PORTA levels and store in first byte of report
	txBuffer[0]=PORTA;
	txBuffer[1]=0xF0;	//this is to differentiate between different transfer types
	txBuffer[2]=HIBYTE(timestamp);
	txBuffer[3]=LOBYTE(timestamp);
	txBuffer[4]=PreviousButtonStates;
    // Copy input bytes to the output buffer
	for (i=5;i<HID_OUTPUT_REPORT_BYTES;i++)
        txBuffer[i] = rxBuffer[i];            

    // As long as the SIE is owned by the processor, we let USB tasks continue.
	while (ep1Bi.Stat & UOWN)
        ProcessUSBTransactions(); 

    // The report will be sent in the next interrupt IN transfer.
    HIDTxReport(txBuffer, HID_INPUT_REPORT_BYTES);
}


static void USBEcho(void)
{
    byte rxCnt;

    // Find out if an Output report has been received from the host.
    rxCnt = HIDRxReport(rxBuffer, HID_OUTPUT_REPORT_BYTES);

	// If no bytes in, then nothing to do
	if (rxCnt == 0)
		return;
	SendUSBMessage();
}

// Central processing loop.  Whenever the firmware isn't busy servicing
// the USB, we will get control here to do other processing.
void ProcessIO(void)
	{

	unsigned char CurrentButtonStates;

#if DEBUG_PRINT
	// Process USART
    checkEcho();
#endif


    // User Application USB tasks
    if ((deviceState < CONFIGURED) || (UCONbits.SUSPND==1)) return;

	// Process USB: Echo back any bytes that have come in.
    USBEcho();
}

// Initialization for a SET_FEATURE request.  This routine will be
// invoked during the setup stage and is used to set up the buffer
// for receiving data from the host
void SetupFeatureReport(byte reportID)
{
    if (reportID == 0)
    {
        // When the report arrives in the data stage, the data will be  
        // stored in HIDFeatureBuffer.
        inPtr = (byte*)&HIDFeatureBuffer;
    }
}

// Post processing for a SET_FEATURE request.  After all the data has
// been delivered from host to device, this will be invoked to perform
// application specific processing.
void SetFeatureReport(byte reportID)
{
#if DEBUG_PRINT
	//printf("SetFeatureReport(0x%hx)\r\n", reportID);
#endif
    // Currently only handling report 0, ignore any others.
    if (reportID == 0)
    {
        // Set the state of PORTB based on the first byte
        // of the feature report.
        LATB = HIDFeatureBuffer[0];
		// x10 to signal feature transfer
		LATBbits.LATB1=1;
		LATBbits.LATB0=0;
    }
}

// Handle a feature report request on the control pipe
void GetFeatureReport(byte reportID)
{
#if DEBUG_PRINT
	//printf("GetFeatureReport(0x%uhx): 0x%hx, 0x%hx\r\n",
	//	(byte)reportID, (byte)HIDFeatureBuffer[0],
	//	(byte)HIDFeatureBuffer[1]);
#endif

	if (reportID == 0)
	{
		// Handle report #0
		outPtr = (byte *)&HIDFeatureBuffer;
		HIDFeatureBuffer[0]=PORTA;
		HIDFeatureBuffer[1]=0xF1;	//this is to differentiate between different transfer types
		HIDFeatureBuffer[2]=HIBYTE(timestamp);
		HIDFeatureBuffer[3]=LOBYTE(timestamp);
		wCount = HID_FEATURE_REPORT_BYTES;
		transferType=0;
	}

}

// Handle control out.  This might be an alternate way of processing
// an output report, so all that's needed is to point the output
// pointer to the output buffer
// Initialization for a SET_REPORT request.  This routine will be
// invoked during the setup stage and is used to set up the buffer
// for receiving data from the host
void SetupOutputReport(byte reportID)
{
	if (reportID == 0)
	{
		// When the report arrives in the data stage, the data will be  
		// stored in HIDFeatureBuffer
		inPtr = (byte*)&HIDRxBuffer;
	}
}

// Post processing for a SET_REPORT request.  After all the data has
// been delivered from host to device, this will be invoked to perform
// application specific processing.
void SetOutputReport(byte reportID)
{
#if DEBUG_PRINT
	//printf("SetOutputReport(0x%hx)\r\n", reportID);
#endif
	// Currently only handling report 0, ignore any others.
	if (reportID != 0)
		return;

	LATB=HIDRxBuffer[0];
	// x11 to signal SET_REPORT transfer
	LATBbits.LATB1=1;
	LATBbits.LATB0=1;
}

// Handle a control input report
void GetInputReport(byte reportID)
{
#if DEBUG_PRINT
	printf("GetInputReport: 0x%uhx\r\n", reportID);
#endif
	if (reportID == 0)
	{
		byte i;
		// Send back the contents of the HID report
		// TBD: provide useful information...
		outPtr = (byte *)&HIDTxBuffer;
		HIDTxBuffer[0]=PORTA;
		HIDTxBuffer[1]=0xF2;	//this is to differentiate between different transfer types
		HIDTxBuffer[2]=HIBYTE(timestamp);
		HIDTxBuffer[3]=LOBYTE(timestamp);
		HIDTxBuffer[4]=PreviousButtonStates;
		for(i=5;i<HID_INPUT_REPORT_BYTES;i++) HIDTxBuffer[i]=HIDRxBuffer[i];
		// The number of bytes in the report (from usb.h).
		wCount = HID_INPUT_REPORT_BYTES;
		transferType=0;
	}
}

// Entry point of the firmware
void main(void)
{
	byte led_cnt=0;
	// Set all I/O pins to digital
    ADCON1 |= 0x0F;
    
	// Initialize USB
    UCFG = 0x14; // Enable pullup resistors; full speed mode

    deviceState = DETACHED;
    remoteWakeup = 0x00;
    currentConfiguration = 0x00;

	// Call user initialization function
	UserInit(); 

	while(1)
    {
        // Ensure USB module is available
		EnableUSBModule();
		
		// As long as we aren't in test mode (UTEYE), process
		// USB transactions.
		if(UCFGbits.UTEYE != 1)
			ProcessUSBTransactions();

        // Application specific tasks
        ProcessIO();
    }
}
