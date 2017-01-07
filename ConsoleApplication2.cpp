//
// ComPrinter.c - This program reads text from a
//   serial port and prints it to the console.
//
// Written by Ted Burke - last updated 3-5-2012
//
// To compile with MinGW:
//
//      gcc -o ComPrinter.exe ComPrinter.c
//
// Example commands:
//
//      ComPrinter /devnum 22 /baudrate 38400
//      ComPrinter /id 12 /quiet
//      ComPrinter /baudrate 38400 /keystrokes
//
// To stop the program, press Ctrl-C
//

#define _WIN64_WINNT 0x0500

#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>

#define MESSAGE_LENGTH 100

// Declare variables and structures
HANDLE hSerialRead = INVALID_HANDLE_VALUE;
DCB dcbSerialParams = { 0 };
COMMTIMEOUTS timeouts = { 0 };
DWORD dwBytesWritten = 0;
char Ldev_name[MAX_PATH] = "";
int dev_number = -1;
int baudrate = 9600;
int scan_max = 30;
int scan_min = 1;
int simulate_keystrokes = 0;
int debug = 1; // print some info by default
int id = -1;

HANDLE hSerialWrite = INVALID_HANDLE_VALUE;
DCB dcbSerialParamsWrite = { 0 };
COMMTIMEOUTS timeoutsWrite = { 0 };

void CloseSerialPort()
{
	if (hSerialRead != INVALID_HANDLE_VALUE)
	{
		// Close serial port
		fprintf(stderr, "\nClosing serial port...");
		if (CloseHandle(hSerialRead) == 0)
			fprintf(stderr, "Error\n");
		else fprintf(stderr, "OK\n");
	}
	if (hSerialWrite != INVALID_HANDLE_VALUE)
	{
		// Close serial port
		fprintf(stderr, "\nClosing serial port...");
		if (CloseHandle(hSerialWrite) == 0)
			fprintf(stderr, "Error\n");
		else fprintf(stderr, "OK\n");
	}
}

void exit_message(const char* error_message, int error)
{
	// Print an error message
	fprintf(stderr, error_message);
	fprintf(stderr, "\n");

	// Exit the program
	exit(error);
}

void simulate_keystroke(char c)
{
	// This structure will be used to create the keyboard
	// input event.
	INPUT ip;

	// Set up a generic keyboard event.
	ip.type = INPUT_KEYBOARD;
	ip.ki.wScan = 0; // hardware scan code for key
	ip.ki.time = 0;

	// Press the key
	// Currently only alphabet lettes, spaces, commas and full stops.
	if (c >= 0x61 && c <= 0x7A) c -= 0x20;
	if (c >= 0x30 && c <= 0x39) ip.ki.wVk = c;
	else if (c >= 0x41 && c <= 0x5A) ip.ki.wVk = c;
	else if (c == ' ') ip.ki.wVk = VK_SPACE;
	else if (c == ',') ip.ki.wVk = VK_OEM_COMMA;
	else if (c == '.') ip.ki.wVk = VK_OEM_PERIOD;
	else if (c == '\b') ip.ki.wVk = VK_BACK;
	else if (c == '\t') ip.ki.wVk = VK_TAB;
	else if (c == '\n') ip.ki.wVk = VK_RETURN;
	else return;

	ip.ki.dwFlags = 0; // 0 for key press
	SendInput(1, &ip, sizeof(INPUT));

	// Release the key
	ip.ki.dwFlags = KEYEVENTF_KEYUP; // KEYEVENTF_KEYUP for key release
	SendInput(1, &ip, sizeof(INPUT));
}

int main(int argc, char *argv[])
{
	// Parse command line arguments. Available options:
	//
	// /devnum DEVICE_NUMBER
	// /baudrate BITS_PER_SECOND
	// /id ROBOT_ID_NUMBER
	// /keystrokes
	// /debug
	// /quiet
	//
	int n = 1;
	while (n < argc)
	{
		// Process next command line argument
		if (strcmp(argv[n], "/devnum") == 0)
		{
			if (++n >= argc) exit_message("Error: no device number specified", 1);

			// Set device number to specified value
			dev_number = atoi(argv[n]);
		}
		else if (strcmp(argv[n], "/baudrate") == 0)
		{
			if (++n >= argc) exit_message("Error: no baudrate value specified", 1);

			// Set baudrate to specified value
			baudrate = atoi(argv[n]);
		}
		else if (strcmp(argv[n], "/id") == 0)
		{
			if (++n >= argc) exit_message("Error: no id number specified", 1);

			// Set id to specified value
			id = atoi(argv[n]);
		}
		else if (strcmp(argv[n], "/keystrokes") == 0)
		{
			simulate_keystrokes = 1;
		}
		else if (strcmp(argv[n], "/debug") == 0)
		{
			debug = 2;
		}
		else if (strcmp(argv[n], "/quiet") == 0)
		{
			debug = 0;
		}
		else
		{
			// Unknown command line argument
			if (debug) fprintf(stderr, "Unrecognised option: %s\n", argv[n]);
		}

		n++; // Increment command line argument counter
	}

	// Welcome message
	if (debug)
	{
		fprintf(stderr, "\nComPrinter.exe - written by Ted Burke\n");
		fprintf(stderr, "https://batchloaf.wordpress.com\n");
		fprintf(stderr, "This version: 3-5-2012\n\n");
	}

	// Debug messages
	if (debug > 1) fprintf(stderr, "dev_number = %d\n", dev_number);
	if (debug > 1) fprintf(stderr, "baudrate = %d\n\n", baudrate);

	// Register function to close serial port at exit time
	atexit(CloseSerialPort);

	if (dev_number != -1)
	{
		// Limit scan range to specified COM port number
		scan_max = dev_number;
		scan_min = dev_number;
	}

	// Scan for an available COM port in _descending_ order
	for (n = scan_max; n >= scan_min; --n)
	{
		// Try next device
		sprintf(Ldev_name, "\\\\.\\COM%d", n);
		if (debug > 1) fprintf(stderr, "Trying %s...", Ldev_name);
		hSerialRead = CreateFileA(_T(dev_name), GENERIC_READ | GENERIC_WRITE, 0, 0,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
		hSerialWrite = CreateFileA(_T(dev_name), GENERIC_READ | GENERIC_WRITE, 0, 0,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 0);
		if (hSerialRead != INVALID_HANDLE_VALUE && hSerialWrite!=INVALID_HANDLE_VALUE)
		{
			if (debug > 1) fprintf(stderr, "OK\n");
			dev_number = n;
			break; // stop scanning COM ports
		}
		else if (debug > 1) fprintf(stderr, "FAILED\n");
	}
	
	// Check in case no serial port could be opened
	if (hSerialRead == INVALID_HANDLE_VALUE)
		exit_message("Error: could not open read serial port", 1);
	//Check if write Serial port could not be opened 
	if (hSerialWrite == INVALID_HANDLE_VALUE)
		exit_message("Error: could not open write serial port", 1);

	// If we get this far, a serial port was successfully opened
	if (debug) fprintf(stderr, "Opening COM%d at %d baud\n\n", dev_number, baudrate);

	// Set device parameters:
	//  baudrate (default 2400), 1 start bit, 1 stop bit, no parity
	dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
	dcbSerialParamsWrite.DCBlength = sizeof(dcbSerialParamsWrite);

	if (GetCommState(hSerialRead, &dcbSerialParams) == 0) {
		fprintf(stderr, "Error getting device state\n");
		CloseHandle(hSerialRead);
		return 1;	
	}
	if (GetCommState(hSerialWrite, &dcbSerialParamsWrite) == 0) {
		fprintf(stderr, "Error getting device state\n");
		CloseHandle(hSerialWrite);
		return 1;
	}
	
	dcbSerialParamsWrite.BaudRate = CBR_19200; 
	dcbSerialParams.ByteSize = 8;
	dcbSerialParams.StopBits = ONESTOPBIT;
	dcbSerialParams.Parity = NOPARITY;

	dcbSerialParams.BaudRate = baudrate;
	dcbSerialParams.ByteSize = 8;
	dcbSerialParams.StopBits = ONESTOPBIT;
	dcbSerialParams.Parity = NOPARITY;

	if (SetCommState(hSerialRead, &dcbSerialParams) == 0)
		exit_message("Error setting device parameters while reading", 1);
	if (SetCommState(hSerialWrite, &dcbSerialParamsWrite) == 0)
		exit_message("Error setting device parameters while writing", 1);

	// Set COM port timeout settings
	timeouts.ReadIntervalTimeout = 50;
	timeouts.ReadTotalTimeoutConstant = 50;
	timeouts.ReadTotalTimeoutMultiplier = 10;
	timeouts.WriteTotalTimeoutConstant = 50;
	timeouts.WriteTotalTimeoutMultiplier = 10;
	if (SetCommTimeouts(hSerialRead, &timeouts) == 0)
		exit_message("Error setting timeouts while reading", 1);
	if (SetCommTimeouts(hSerialWrite, &timeouts) == 0)
		exit_message("Error setting timeouts while writing", 1);
	// Read text and print to console (and maybe simulate keystrokes)
	int state = 1;
	//int i;
	char c;
	char message_buffer[MESSAGE_LENGTH];
	DWORD bytes_read;
	//Variables to write 
	DWORD bytes_written, total_bytes_written = 0;
	//For testing Purposes 
	char bytes_to_send[5] = { 104,101,108,108,111 };
	//char* bytes_to_send;
	//Take the size of bytes to send and ask for user to input 
	fprintf(stderr, "Sending bytes...");
	if (!WriteFile(hSerialWrite, bytes_to_send, 5, &bytes_written, NULL))
	{
		fprintf(stderr, "Error\n");
		CloseHandle(hSerialWrite);
		return 1;
	}
	fprintf(stderr, "%d bytes written\n", bytes_written);

	// Close serial port
	fprintf(stderr, "Closing serial port...");
	if (CloseHandle(hSerialWrite) == 0)
	{
		fprintf(stderr, "Error\n");
		return 1;
	}
	fprintf(stderr, "OK\n");
	// Depending on whether a robot id has been specified, either
	// print all incoming characters to the console or filter by
	// the specified id number
	if (id == -1)
	{
		// No robot id specified - print everything to console
		while (1)
		{
			ReadFile(hSerialRead, &c, 1, &bytes_read, NULL);
			if (bytes_read == 1)
			{
				printf("%c", c);
				if (simulate_keystrokes == 1) simulate_keystroke(c);
			}
		}
	}
	else
	{
		// A robot id was specified - use a state machine to parse
		// incoming text and display only messages in the correct
		// format matching the specified id number
		while (1)
		{
			// Read next character
			ReadFile(hSerialRead, &c, 1, &bytes_read, NULL);
			if (bytes_read != 1) continue;

			if (state == 1)
			{
				// State 1: Waiting for '<' character to start message
				n = 0;
				strcpy(message_buffer, "");
				if (c == '<') state = 2;
			}
			else if (state == 2)
			{
				// State 2: Reading robot id one digit at a time
				if (c >= '0' && c <= '9') n = 10 * n + (c - 48);
				else if (c == '@') state = 3;
				else state = 1;
			}
			else if (state == 3)
			{
				// Got a '@' character, so now reading actual message
				if (n != id) state = 1;
				else if (c != '>') strncat(message_buffer, &c, 1);
				else if (c == '>') state = 4;
				if (strlen(message_buffer) >= MESSAGE_LENGTH - 1) state = 4;
			}
			else if (state == 4)
			{
				// State 4: Display complete message
				fprintf(stdout, "%s\n", message_buffer);
				state = 1;
			}
			else state = 1;
		}
	}

	// We should never get to this point because when the user
	// presses Ctrl-C, the atexit function will be called instead.
	return 0;
}
