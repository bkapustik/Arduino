#include "funshield.h"

class Time 
{
  private:
    unsigned long last_time;
  public:
    Time():last_time(0){}
    bool time_to_work(int interval)
    {
      auto cur_time = millis();
      if( cur_time >= last_time + interval) 
      {
        last_time = cur_time;
        return 1;
      }
      return 0;
    }
};

int mod(int a, int b)
{
    int r = a % b;
    return r < 0 ? r + b : r;
}

constexpr byte EMPTY_GLYPH = 0b11111111;
constexpr byte Glyph_for_D = 0b10100001;

class Button
{
  private:
    unsigned long last_clock;
    bool state;
    int button_number;
    bool stater = false;
    bool last_time = false;
  public:  
    Button(int button_number):button_number(button_number),last_clock(0){}
    void initialize()
    {
      pinMode( button_number, INPUT);
    }
    bool detection()
    {
      if (digitalRead(button_number)) return false;
      else return true;
    }
    bool just_press()
    {
      if (detection())
      {
        if(state)
        {
          state = false;
          return true;
        }
      }
      else state = true;
      return false;
    }
};

Button button1(A1),button2(A2),button3(A3);
Button button[]{button1,button2,button3};
constexpr int button_count = sizeof(button)/sizeof(button[0]);

void button_initialization( int button_count)
{
  for(int i = 0; i < button_count; ++i) button[i].initialize();
}

class Glyph
{
  public:
    void writeGlyphBitmask( byte glyph, byte pos_bitmask)
    {
      digitalWrite( latch_pin, LOW);
      shiftOut( data_pin, clock_pin, MSBFIRST, glyph);
      shiftOut( data_pin, clock_pin, MSBFIRST, pos_bitmask);
      digitalWrite( latch_pin, HIGH);
    }
    void writeGlyphSite(byte glyph, int pos, char site)
    {
      if (site == 'L')
      {
        int shift = 0x1;
        for (int i = 0; i < pos; ++i) shift *= 2; 
        writeGlyphBitmask(glyph, shift);
      }
      else
      {
        int shift = 0x8;
        for (int i = 0; i < pos; ++i) shift /= 2;
        writeGlyphBitmask(glyph, shift);
      }
    }
};

const int digit_count = 4;

class Display 
{
  private:
    byte data[digit_count] = {EMPTY_GLYPH,EMPTY_GLYPH,EMPTY_GLYPH,EMPTY_GLYPH};
    Glyph glyph;
    int pos = 0;
  public:
    void set(int number, bool configuration_mode = false, int number_of_throws = 0)
    {
      for (int i = 0; i < digit_count; ++i) data[i] = EMPTY_GLYPH;
      
      int count_of_digits_of_given_number = 0;
      int copy_of_number_to_get_his_digit_count = number;

      if (number == 0) 
      { 
        count_of_digits_of_given_number = 1;
      }
      else
      {
        while(copy_of_number_to_get_his_digit_count!=0)  
        {  
          copy_of_number_to_get_his_digit_count=copy_of_number_to_get_his_digit_count/10;  
          ++count_of_digits_of_given_number; 
        }
      }
      for (int i = 0; i < count_of_digits_of_given_number ; ++i )
      {
        int digit = mod(number,10);
        number /= 10;
        data[i] = digits[digit];
      }

      if( configuration_mode ) 
      {
        data[3] = digits[number_of_throws];
        data[2] = Glyph_for_D; 
      }
    }
    void loop()
    {
      glyph.writeGlyphSite(data[pos] ,pos,'R');
      pos = (pos + 1) % digit_count;
    }
};

Display d;

class Led 
{
  private:
    int led_number;
    bool on;
  public:
    Led(int led_number):led_number(led_number),on(false){}
    void ledOnOff(bool condition)
    {
      digitalWrite( led_number, condition ? LOW : HIGH);
    }
    void initialize()
    {
      pinMode(led_number,OUTPUT);
    }
};

Led led1(13),led2(12),led3(11),led4(10);
Led led[]{led1,led2,led3,led4};
constexpr int led_count = sizeof(led)/sizeof(led[0]);

void all_on_off(int led_count, bool condition)
{
  for(int i = 0;i < led_count; ++i) led[i].ledOnOff(condition);
}

void led_initialization(int led_count)
{
  for(int i = 0;i < led_count;++i) led[i].initialize();
  all_on_off(led_count, 0);  
}

class Application
{
  private:
    int place = 0;
    bool edge_was_hit = 0;
    Time timer1,timer2;
    bool configuration_mode = true;
    int type_of_dice = 4;
    int number_of_throws = 1;
    int number;
    unsigned long last_clock;
    bool generate = false;
  public:
    void x25_ball(int interval, bool condition, int number_of_balls)
    {
      if (timer1.time_to_work(interval))
      {
        for(int i = 0; i < led_count; ++i)
        {
          if (i <= place && i >= place - number_of_balls + 1) led[i].ledOnOff(1);
          else led[i].ledOnOff(0);
        }
        if (condition)
        {
          if (place == led_count-1+number_of_balls-1) edge_was_hit = 1;
          if (place == 0) edge_was_hit = 0;
          place = place + (edge_was_hit ? -1 : +1);
        }
        else
        {
          place += 1;
          if (place >= led_count + number_of_balls) place = 0;
        }
      }
    }
    void dungeons_and_dragons()
    {
      if (button[1].just_press()) 
      {
        if (configuration_mode)
        {
          ++number_of_throws;
          if (number_of_throws > 9) number_of_throws = 1;
        }
        else 
        {
          place = 0;
          all_on_off(led_count,false);
          edge_was_hit = 0;
          configuration_mode = true;
        }
      }
      

      if (button[2].just_press()) 
      {
        if (configuration_mode)
        {
          type_of_dice += 2; 
          if (type_of_dice > 100) {type_of_dice = 4; return;};
          if (type_of_dice > 20) {type_of_dice = 100; return;};
          if (type_of_dice > 12) {type_of_dice = 20; return;};
        }
        else 
        {
          place = 0;
          all_on_off(led_count,false);
          edge_was_hit = 0;
          configuration_mode = true;
        }
      }
      if (button[0].just_press()) 
      {
        configuration_mode = false;
      }
      if (button[0].just_press() && ! configuration_mode) generate = true;
      if (configuration_mode)
      {
        d.set(type_of_dice,true,number_of_throws);
      }
      else
      {
        if (button[0].detection())
          {
            x25_ball(30,1,1);
            if(timer2.time_to_work(5)) number = random(number_of_throws, (type_of_dice * number_of_throws) +1);
          }
        else 
        {
          all_on_off(led_count, false);
          place = 0;
        }
        d.set(number);
      }
    }
};

Application a;

void setup() {
  pinMode(latch_pin,OUTPUT);
  pinMode(clock_pin,OUTPUT);
  pinMode(data_pin,OUTPUT);
  button_initialization(button_count);
  led_initialization(led_count);
  Serial.begin( 9600);

}

void loop() {
 a.dungeons_and_dragons();
 d.loop();
}
