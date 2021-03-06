/*
 * TempHumidity.c
 *
 * Created: 4/6/2015 3:52:06 PM
 *  Author: shuffleres
 */ 

#include "TempHumidity.h"

int main(void)
{
	//Turn on serial for 8N1
	//Delay it to make sure everything boots up in time
	serial_open(19200, SERIAL_8N1);
	_delay_ms(400);
    while(1)
    {
		//Turn off interrupts just in case
		//Pull low then change to output and wait
		//Change to pullup and wait to see if the sensor responds
		cli();
		digitalWrite(7,LOW);
		pinMode(7,OUTPUT);
        _delay_us(600);
		pinMode(7,INPUT_PULLUP);
		_delay_us(70);
		//Two checks to be sure the sensor is responding
		//If it's not it will break out of the while loop
		if(digitalRead(7))
		{
			prints("\nNo response from sensor, check connection\n");
			break;
		}
		_delay_us(80);
		if (!digitalRead(7))
		{
			prints("\nNo response from sensor, check connection\n");
			break;
		}
		//Sensor is responding so we get ready to read
		//We will read the 5 bytes individually (makes it easier to do the check sum later)
		for (uint8_t byteVal = 0; byteVal < 5; byteVal++)
		{
			getByte = 0;
			//Loop through all 8 bits of the byte
			for (uint8_t i = 0; i < 8; i++)
			{
				getByte <<= 1;
				//Wait for it to go back to low
				while(digitalRead(PIN))
				{
					;
				}
				_delay_us(5);
				//wait for it to go back to high
				while(!digitalRead(PIN))
				{
					;
				}
				//Delay to the middle point
				_delay_us(50);
				//Check if the pin is high
				//If it is then |= 1
				if(digitalRead(PIN))
				{
					getByte |= 1;
				}
			}
			//Set what we just calculated into the array
			bytes[byteVal] = getByte;
		}
		//Print data or say there was a checksum error
		if (checkSum())
		{
			printHumidity(bytes[0], bytes[1]);
			printTempInF(bytes[2], bytes[3]);
		}
		else {
			prints("\n checksum is not equal \n");
		}
		//Turn interrupts back on and clear all the bytes
		sei();
		int iter;
		for (iter = 0; iter < 5; iter++)
		{
			bytes[iter] = 0;
		}
		//Wait for a second for the next iteration
		//I can make this 500ms instead but I feel like it's too fast
		_delay_ms(400);
	}
}

//Grab the correct bits and set them to a unsigned int value
//That value / 10 is the temp, and that value mod 10 is the decimal
//Then print the correct string, had to use an array of chars for printing
//Mod and division on decimal values only
void printTempInF(uint8_t bytes2, uint8_t bytes3)
{
	char buffer[8];
	int temp = ((bytes2) << 8) | bytes3;
	temp = temp / 10;
	int remainder = temp % 10;
	//C -> F conversion
	temp = ((temp * 9)/5) + 32;
	//Check for negative
	if(bytes2  < 0)
	{
		temp = temp * -1;
	}
	itoa(temp,buffer,10);
	prints(" TMP= ");
	prints(buffer);
	itoa(remainder,buffer,10);
	prints(".");
	prints(buffer);
	prints(" \n");
}

//Grab the correct bits and set them to a unsigned int value
//That value / 10 is the RH, and that value mod 10 is the decimal
//Then print the correct string, had to use an array of chars for printing
//Mod and division on decimal values only
void printHumidity(uint8_t bytes0, uint8_t bytes1)
{
	char buffer[8];
	int relHum = (bytes0 << 8) | bytes1;
	int remainder = relHum % 10;
	relHum = relHum / 10;
	prints("RHUM= ");
	itoa(relHum,buffer,10);
	prints(buffer);
	itoa(remainder,buffer,10);
	prints(".");
	prints(buffer);
	prints(", ");
}

//Make a sum variable
//Add the four other bytes and store
//If the sum variable is equal to the byte 4 value it passes
int checkSum()
{
	unsigned char sum;
	sum = bytes[0] + bytes[1] + bytes[2] + bytes[3];
	if(bytes[4] == sum)
	{
		return 1;
	}
	return 0;
}

//Selects which mode to set a pin to.
//Tests to see if the pin and mode is in bounds
void pinMode(int pin, char mode)
{
	if (pin > 53 || pin < 0)
	{
		return -1;
	}
	if (mode < 0 || mode > 2)
	{
		return -1;
	}
	//Input
	if (mode == 0)
	{
		*(map[pin].ddr) &= ~(1 << map[pin].bitShift);
		*(map[pin].ddr + 1) &= ~(1 << map[pin].bitShift);
	}
	//Output
	else if (mode == 1)
	{
		*(map[pin].ddr) |= (1 << map[pin].bitShift);
	}
	//Pullup
	else if(mode == 2)
	{
		*(map[pin].ddr) &= ~(1 << map[pin].bitShift);
		*(map[pin].ddr + 1) |= (1 << map[pin].bitShift);
	}
	
}

//Tests to see if there is a voltage on the pin
int digitalRead(int pin)
{
	return (*(map[pin].ddr - 1) & (1 << map[pin].bitShift)) ? 1 : 0;

}

//Changes the bit to turn it on or off
void digitalWrite(int pin, int value)
{
	if (value < 0 || value > 1)
	{
		return -1;
	}
	if (value == 1)
	{
		*(map[pin].ddr + 1) |= (1 << map[pin].bitShift);
	}
	else
	{
		*(map[pin].ddr + 1) &= ~(1 << map[pin].bitShift);
	}
}

void serial_open(long speed, int config)
{
	//Switch for baud rate
	//Uses table from the data sheet
	//Sets U2X0 to 1 or 0 depending on which has a lower % error
	switch (speed)
	{
		case 2400:
		UBRR0 = 832;
		UCSR0A |= (1<<U2X0);
		break;
		case 4800:
		UBRR0 = 207;
		UCSR0A &= ~(1<<U2X0);
		break;
		case 9600:
		UBRR0 = 103;
		UCSR0A &= ~(1<<U2X0);
		break;
		case 14400:
		UBRR0 = 68;
		UCSR0A &= ~(1<<U2X0);
		break;
		case 19200:
		UBRR0 = 51;
		UCSR0A &= ~(1<<U2X0);
		break;
		case 28800:
		UBRR0 = 68;
		UCSR0A |= (1<<U2X0);
		break;
		case 38400:
		UBRR0 = 25;
		UCSR0A &= ~(1<<U2X0);
		break;
		case 57600:
		UBRR0 = 16;
		UCSR0A &= ~(1<<U2X0);
		break;
		case 76800:
		UBRR0 = 12;
		UCSR0A &= ~(1<<U2X0);
		break;
		case 115200:
		UBRR0 = 16;
		UCSR0A |= (1<<U2X0);
		break;
		case 230400:
		UBRR0 = 3;
		UCSR0A &= ~(1<<U2X0);
		break;
		case 250000:
		UBRR0 = 3;
		UCSR0A &= ~(1<<U2X0);
		break;
		default:
		return;
	}
	
	//Switch for the configuration
	//Defaults to 8N1 if a valid choice was not entered
	//Sets certain bits on or off
	//                      UCSR0C
	//   7       6      5     4     3      2      1     0
	//UMSEL01-UMSEL00-UPM01-UPM00-USBS0-UCSZ01-UCSZ00-UCPOL0
	// Synch   Synch   Par   Par  Stop   Data   Data  Data
	switch (config)
	{
		case SERIAL_5N1:
		UCSR0C = UCSR0C & 00000000;
		break;
		case SERIAL_6N1:
		UCSR0C |= (1<<UCPOL0);
		break;
		case SERIAL_7N1:
		UCSR0C |= (1<<UCSZ00);
		break;
		case SERIAL_8N1:
		UCSR0C &= (3<<UCSZ00);
		break;
		case SERIAL_5N2:
		UCSR0C |= (1<<USBS0);
		break;
		case SERIAL_6N2:
		UCSR0C |= (1<<USBS0) | (1<<UCPOL0);
		break;
		case SERIAL_7N2:
		UCSR0C |= (1<<USBS0) | (1<<UCSZ00);
		break;
		case SERIAL_8N2:
		UCSR0C |= (1<<USBS0) | (3<<UCSZ00);
		break;
		case SERIAL_5E1:
		UCSR0C |= (1<<UMSEL00);
		break;
		case SERIAL_6E1:
		UCSR0C |= (1<<UMSEL00) | (1<<UCPOL0);
		break;
		case SERIAL_7E1:
		UCSR0C |= (1<<UMSEL00) | (1<<UCSZ00);
		break;
		case SERIAL_8E1:
		UCSR0C |= (1<<UMSEL00) | (3<<UCSZ00);
		break;
		case SERIAL_5E2:
		UCSR0C |= (1<<UMSEL00) | (1<<USBS0);
		break;
		case SERIAL_6E2:
		UCSR0C |= (1<<UMSEL00) | (1<<USBS0) | (1<<UCPOL0);
		break;
		case SERIAL_7E2:
		UCSR0C |= (1<<UMSEL00) | (1<<USBS0) | (1<<UCSZ00);
		break;
		case SERIAL_8E2:
		UCSR0C |= (1<<UMSEL00) | (1<<USBS0) | (3<<UCSZ00);
		break;
		case SERIAL_5O1:
		UCSR0C |= (3<<UMSEL00);
		break;
		case SERIAL_6O1:
		UCSR0C |= (3<<UMSEL00) | (1<<UCPOL0);
		break;
		case SERIAL_7O1:
		UCSR0C |= (3<<UMSEL00) | (1<<UCSZ00);
		break;
		case SERIAL_8O1:
		UCSR0C |= (3<<UMSEL00) | (3<<UCSZ00);
		break;
		case SERIAL_5O2:
		UCSR0C |= (3<<UMSEL00) | (1<<USBS0);
		break;
		case SERIAL_6O2:
		UCSR0C |= (3<<UMSEL00) | (1<<USBS0) | (1<<UCPOL0);
		break;
		case SERIAL_7O2:
		UCSR0C |= (3<<UMSEL00) | (1<<USBS0) | (1<<UCSZ00);
		break;
		case SERIAL_8O2:
		UCSR0C |= (3<<UMSEL00) | (1<<USBS0) | (3<<UCSZ00);
		break;
		default:
		UCSR0C &= (3<<UCSZ00);
	}
	
	
	//Set UBRRH to the highest 4 bits
	UBRR0H = (UBRR0 >> 8);
	//Set UBRRL to the rest
	UBRR0L =  UBRR0;
	//Enables TX and RX
	UCSR0B = (1 << TXEN0) | (1 << RXEN0);
	
}

void printOut(char data)
{
	while(!((UCSR0A) & (1<<UDRE0)))
	{
		;
	}
	UDR0 = data;
}

void prints(char* data)
{
	while(*data)
	{
		printOut(*data++);
	}
}
