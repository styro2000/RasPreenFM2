#ifndef _global_obj_h
#define _global_obj_h

#include <circle/memory.h>
#include <circle/actled.h>
#include <circle/types.h>
#include <circle/gpiopin.h>

char* itoa(int num, char* str, int base); 
void reverse(char str[], int length); 

class GPIO_OUT
{
public:
	GPIO_OUT (int PinNr);
	~GPIO_OUT (void);

	boolean Initialize (void);
	boolean Set (int out);
    void Invert(void);
//	GPIO_OUT *Get (void);

	CGPIOPin OutPin;
 
private:
	// do not change this order
//	static GPIO_OUT *s_pThis;
	
};

extern GPIO_OUT out_led1;
extern GPIO_OUT out_led2;
extern GPIO_OUT out_led3;


#endif
