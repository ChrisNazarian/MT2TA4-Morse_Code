#include "fonts.h"
#include "mbed.h"
#include "LCD_DISCO_F429ZI.h"
#include <chrono>
#include "DebouncedInterrupt.h"
#include "stm32f429i_discovery_lcd.h"
#include <string>
#include <cstring>


#define LCD_WIDTH 240
#define LCD_HEIGHT 320
int x = 0;
int y = 180;

LCD_DISCO_F429ZI LCD;
InterruptIn User_Button(BUTTON1);
//InterruptIn Limit_switch(PG_9);
DebouncedInterrupt Space(PG_9);
DebouncedInterrupt Reset(PG_10);
PwmOut Buzzer(PE_6);

Timer timer1;
Timer timer2;
char word[5];
int position = 0;

struct Dictionary_mapping{
    const char* morse;
    char letter;
};
const Dictionary_mapping letter_map[]={
    {".-", 'A'},
    {"-...", 'B'},
    {"-.-.", 'C'},
    {"-..", 'D'},
    {".", 'E'},
    {"..-.", 'F'},
    {"--.", 'G'},
    {"....", 'H'},
    {"..", 'I'},
    {".---", 'J'},
    {"-.-", 'K'},
    {".-..", 'L'},
    {"--", 'M'},  
    {"-.", 'N'},
    {"---", 'O'},
    {".--.", 'P'},
    {"--.-", 'Q'},
    {".-.", 'R'},
    {"...", 'S'},
    {"-", 'T'},
    {"..-", 'U'},
    {"...-", 'V'},
    {".--", 'W'},
    {"-..-", 'X'},
    {"-.--", 'Y'},
    {"--..", 'Z'},

};
bool compare(const char*a, const char*b){
    while(*a && *b){
        if(*a!=*b){
            return false;
        }
        a++;
        b++;
    }
    return *a == *b;
}
char decode(char *morse_code){
    for(auto &mapping : letter_map){
        if(compare(morse_code, mapping.morse)){
            return mapping.letter;
        }
    }
    return '?';
}
void reset(){
    timer2.stop();
    timer2.reset();
    position = 0;
    x = 0;
    LCD.Clear(LCD_COLOR_WHITE);
}
void buzzer_on(){
    Buzzer.period_us(256);
    Buzzer.pulsewidth_us(50);
}
void buzzer_off(){
    Buzzer.period_us(256);
    Buzzer.pulsewidth_us(256);
}
void timer_start(){
    timer1.start();
    buzzer_on();
}
void space(){
    uint8_t written_word[30];
    word[position] = '\0';
    char letter = decode(word);
    sprintf((char *)written_word, "%c", letter);
    LCD.DisplayStringAt(x+=10, y, (uint8_t *)written_word, LEFT_MODE);
    memset(word,0,5);
    position = 0;
    x+=10;
    timer2.stop();
    timer2.reset();
}
void Input_check(){
    LCD.SetFont(&Font16);
    LCD.DisplayStringAt(0, 60, (uint8_t *)"Input check", CENTER_MODE);
    uint8_t time[30];
    uint8_t written_word[30];
    timer2.stop();
    timer1.stop();
    if(x>200){
        x=0;
        y+=20;
    }
    uint32_t space_ms = chrono::duration_cast<chrono::milliseconds>(timer2.elapsed_time()).count();
    uint32_t elapsed_ms = chrono::duration_cast<chrono::milliseconds>(timer1.elapsed_time()).count();

    if(space_ms > 1000 || (position >= 4 && position > 0)){
        word[position] = '\0';
        char letter = decode(word);
        LCD.SetFont(&Font16);
        sprintf((char *)written_word, "%c", letter);
        LCD.DisplayStringAt(x+=10, y, (uint8_t *)written_word, LEFT_MODE);
        memset(word,0,5);
        position = 0;
        //timer2.reset();
        //timer1.reset();
        //return;
        //decode previous word and add space
        
    }
    LCD.SetFont(&Font16);
    LCD.DisplayStringAt(0, 90, (uint8_t *)"Short<250 Long<1000", CENTER_MODE);
    sprintf((char *)time, "button time: %4d ms", elapsed_ms);
    LCD.DisplayStringAt(0, 120, (uint8_t *)&time, CENTER_MODE);

    if (elapsed_ms < 250){
        //LCD.DisplayStringAt(0, 170, (uint8_t *)"Short (.)", CENTER_MODE);
        word[position]='.';
    }
    if(elapsed_ms > 250 && elapsed_ms < 1000){
        //LCD.DisplayStringAt(0, 170, (uint8_t *)"Long (-)", CENTER_MODE);
        word[position]='-';
    }
    if(elapsed_ms > 1000){
        //LCD.DisplayStringAt(0, 170, (uint8_t *)"invalid input", CENTER_MODE);
    }
    
    timer2.reset();
    timer2.start();
    timer1.reset();
    buzzer_off();
    if (elapsed_ms < 1000 && position < 4){position++;}
}

void Input_check_ISR(){Input_check();}
void Timer_start_ISR(){timer_start();}

int main(){
    
    User_Button.rise(&timer_start);
    User_Button.fall(&Input_check);
    //Limit_switch.rise(&timer_start);
    //Limit_switch.fall(&Input_check);
    
    //Reset.fall(&reset);
    //Reset.rise(&timer_start);
    Space.attach(&space,IRQ_FALL,100,false);
    // Typing.attach(&typing_ISR,IRQ_RISE,50,false);
    Reset.attach(&reset,IRQ_FALL,100,false);

    while (true) {

    }
}

