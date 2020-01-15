
#include "global_obj.h"
// #include <circle/gpiopin.h>
#include <circle/timer.h>


GPIO_OUT::GPIO_OUT (int PinNr)
: OutPin (PinNr, GPIOModeOutput)
{
    OutPin.Write(0);
}

GPIO_OUT::~GPIO_OUT (void)
{
}

boolean GPIO_OUT::Initialize (void)
{
	return TRUE;
}

boolean GPIO_OUT::Set(int out)
{
    OutPin.Write(out);
	return TRUE;
}

void GPIO_OUT::Invert(void)
{
    OutPin.Invert ();
}

// GPIO_OUT *GPIO_OUT::Get (void)
// {
// 	return this->s_pThis;
// }

GPIO_OUT out_led1(17);
GPIO_OUT out_led2(23);
GPIO_OUT out_led3(24);

  
/* A utility function to reverse a string  */
void reverse(char s[], int length) 
{ 
    // int start = 0; 
    // int end = length -1; 
    // while (start < end) 
    // { 
    //     swap(*(str+start), *(str+end)); 
    //     start++; 
    //     end--; 
    // } 
     int i, j;
     char c;
 
     for (i = 0, j = length-1; i<j; i++, j--) {
         c = s[i];
         s[i] = s[j];
         s[j] = c;
     }

} 
  
// Implementation of itoa() 
char* itoa(int num, char* str, int base) 
{ 
    int i = 0; 
    bool isNegative = false; 
  
    /* Handle 0 explicitely, otherwise empty string is printed for 0 */
    if (num == 0) 
    { 
        str[i++] = '0'; 
        str[i] = '\0'; 
        return str; 
    } 
  
    // In standard itoa(), negative numbers are handled only with  
    // base 10. Otherwise numbers are considered unsigned. 
    if (num < 0 && base == 10) 
    { 
        isNegative = true; 
        num = -num; 
    } 
  
    // Process individual digits 
    while (num != 0) 
    { 
        int rem = num % base; 
        str[i++] = (rem > 9)? (rem-10) + 'a' : rem + '0'; 
        num = num/base; 
    } 
  
    // If number is negative, append '-' 
    if (isNegative) 
        str[i++] = '-'; 
  
    str[i] = '\0'; // Append string terminator 
  
    // Reverse the string 
    reverse(str, i); 
  
    return str; 
} 
