#define LED_PIN 3
#define LED_NUM 60
#define STEP_NUM 20

#include "FastLED.h"

CRGB leds[LED_NUM];

#include <SimpleKeypad.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

const byte nb_rows = 4;                         // four rows
const byte nb_cols = 4;                         // four columns
char key_chars[nb_rows][nb_cols] = {            // The symbols of the keys
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[nb_rows] = {12, 11, 10, 9};        // The pins where the rows are connected
byte colPins[nb_cols] = {8, 7, 6, 5};           // The pins where the columns are connected
int game_score = 501;                           //количество очков в партии

SimpleKeypad kp1((char*)key_chars, rowPins, colPins, nb_rows, nb_cols);   // New keypad called kp1

void setup() {
  char text1[]="Done by Karp";
  char text2[]="Senior Prog.";
  lcd.clear();
  lcd.noBacklight();
  lcd.init();
  lcd.backlight();
  displayStart(text1, 0, 1);
  displayStart(text2, 1, 0);
  lcd.clear();
  displayGame_setup(game_score);
  highlight_player(0);
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, LED_NUM);
  FastLED.clear();
  FastLED.show();
  FastLED.setBrightness(10);
  draw_setup();
}


void loop() {
  static unsigned long int timer_blink_or_error = 0;     //таймер для мигании надписи ИЛИ ошибки
  static unsigned long int timer_change = 0;             //таймер для изменения очков на ленте
  static byte prev_number = 0;                           //сохраненное значение последнего хода
  static byte score_change_counter = 0;                  //счетчик отрисовки изменения очков
  static int number = 0;                                 //значение текущего хода
  static byte number_flag = 0;                           //количество символов в значении текущего хода
  static int player_score[2] = {game_score, game_score}; //текущее количество очков у игроков
  static boolean player = 0;                             //текущий игрок
  static boolean blink_phase = 1;                        //флаг мигания надписи GG

  static byte game_phase = 0;                            // = 0: если ждем нажатие кнопки и ничего не выводим на ленту
                                                         // = 1: состояние = 0 с возможностью отката хода
                                                         // = 2: состояние = 1 с выводом изменения очков (НЕ финальным)
                                                         // = 3: состояние = 0 с выводом отката очков
                                                         // = 4: отката нет, кнопку не ждем, выводим ошибку
                                                         // = 5: откат есть, кнопку не ждем, выводим ошибку
                                                         // 
                                                         // = 7: состояние = 1 с выводом изменения очков (ФИНАЛЬНЫМ)
                                                         // = 8: состояние = 1 с выводом победной анимации на ленту
                                                         // = 9: состояние = 1 с выводом возврата к началу на ленту
                                                         // = 10: состояние = 1 с возвратом к началу
  
  char key = kp1.getKey();                      
  if (key && game_phase <= 3) {                                                          
    switch (key){
      case '*': //стереть 1 цифру
        number -= (number % 10);
        number /= 10;
        if (number_flag > 0){
          number_flag --;
        }
        displayGame_input(player, number);
      break;
      case '#': //ввод -- обработка введенного числа
        //вычисления и переключение игрока, если игрок не "закрылся"
        if (player_score[(byte)player] > number && number <= 180){    //не полная проверка корректности, но да и похер
          player_score[(byte)player] -= number;                       //отняли очки у текущего игрока
          displayGame_score(player, player_score[(byte)player]);      //вывели на дисплей
          prev_number = number;                                       //сохранили отнятые очки с этого хода
          game_phase = 2;                                             //переключение фазы игры
          player = !player;                                           //переключение игрока
          number = 0;                                                 //обнулили текущее число очков
          number_flag = 0;                                            //и флаг для его ввода
          timer_change = millis();                                    //сетапим таймер для вывода анимации
          highlight_player(player);                                   //переключаем игрока на дисплее
        }
        //конец партии, если игрок "закрылся"
        else if (player_score[(byte)player] == number && number <= 170){//не полная проверка корректности, но да и похер
          game_phase = 7;                                             //переключение фазы игры
          timer_change = millis();                                    //запоминаем время конца партии для анимации
          timer_blink_or_error = millis();                            //запоминаем время конца партии для мигания GG          
          prev_number = player_score[(byte)player];
          player_score[(byte)player] = 0;                             //обнулили очки победителя
          number = 0;                                                 //обнулили текущее число очков
          number_flag = 0;                                            //и флаг для его ввода
          displayGame_score(player, player_score[(byte)player]);      //вывели очки на дисплей      
          displayGreetings(player);
        }
        else{                                                         //вывести сообщение об ошибке
          game_phase += 4;                                            //переключение фазы игры
          timer_blink_or_error = millis();                            //запоминаем время ошибки   
          displayError(player);                                       //выводим сообщение об ошибке
        }
      break;
      case 'A': //откат последнего ввода
        //переключение игрока обратно
        if (game_phase == 1){
          player = !player;                                           //вернули предыдущего игрока
          player_score[(byte)player] += prev_number;                  //добавили ему его очки
          game_phase = 3;                                             //переключение фазы игры          
          //изменение данных на дисплее
          displayGame_score(player, player_score[(byte)player]);      //вывели очки на дисплей
          timer_change = millis();                                    //сетапим таймер для вывода анимации
          highlight_player(player);                                   //переключаем игрока на дисплее
        }
        else{                                                         //вывести сообщение об ошибке
          game_phase = 4;                                             //переключение фазы игры
          timer_blink_or_error = millis();                            //запоминаем время ошибки   
          displayError(player);                                       //выводим сообщение об ошибке
        }
      break;
      case 'B': //закрыть партию победой текущего игрока, если хватает очков
        if (player_score[(byte)player] <= 170){                       //не полная проверка корректности, но да и похер
          prev_number = player_score[(byte)player];
          player_score[(byte)player] = 0;                             //обнулили очки текущего игрока (для отрисовки)
          displayGame_score(player, player_score[(byte)player]);      //вывели их на дисплей
          timer_change = millis();                                    //запоминаем время конца партии для анимации
          timer_blink_or_error = millis();                            //запоминаем время конца партии для мигания GG
          number = 0;                                                 //обнулили текущее число очков
          number_flag = 0;                                            //и флаг для его ввода
          displayGreetings(player);                                   //выводим GG на дисплее
          blink_phase = 0;                                            //готовим блинк GG
          game_phase = 7;                                             //переключение фазы игры                                             
        }
        else{//вывести сообщение об ошибке
          game_phase = 5;                                             //переключение фазы игры
          timer_blink_or_error = millis();                            //запоминаем время ошибки   
          displayError(player);                                       //выводим сообщение об ошибке     
        } 
      break;
      case 'C': //сброс игры в начало, старт игрока 0
        player_score[0] = game_score;
        player_score[1] = game_score;
        player = 0;
        //отрисовать изменение на дисплее
        displayGame_score(0, player_score[(byte)player]);
        displayGame_score(1, player_score[(byte)player]);
        highlight_player(player);
        //отрисовка на ленте изменения очков
        draw_setup();
        game_phase = 0;                                               //переключение фазы игры
      break;
      case 'D': //сброс игры в начало, старт игрока 1 
        player_score[0] = game_score;
        player_score[1] = game_score;
        player = 1;
        //отрисовать изменение на дисплее
        displayGame_score(0, game_score);
        displayGame_score(1, game_score);
        highlight_player(player);
        //отрисовка на ленте изменения очков
        draw_setup();
        game_phase = 0;                                               //переключение фазы игры
      break;
      
      default:  //введена цифра -- добавить ее в число
        if (number_flag < 3){
          number *= 10;
          number += ((byte) key - (byte) '0');
          number_flag ++;
          //отрисовать изменение на дисплее
          displayGame_input(player, number);
        }
      break;     
    } 
  }

  if (game_phase == 2 || game_phase == 7){   
    if (millis() >= timer_change + 10){
      if (game_phase == 7) 
        draw_change(player_score[(byte)(player)] + prev_number, player_score[(byte)(player)], player, score_change_counter);
      else
        draw_change(player_score[(byte)(!player)] + prev_number, player_score[(byte)(!player)], !player, score_change_counter);
      score_change_counter++;
      timer_change = millis();
      if (score_change_counter == STEP_NUM){
        score_change_counter = 0; 
        if (game_phase == 7) game_phase = 8;
        else game_phase = 1;
      }
    }
  }
  
  if (game_phase == 3){
    if (millis() >= timer_change + 10){
      draw_rollback(player_score[(byte)player], player_score[(byte)player] - prev_number, player, score_change_counter);
      score_change_counter++;
      timer_change = millis();
      if (score_change_counter == STEP_NUM){
        score_change_counter = 0;
        game_phase = 0;
      }
    }
  }
  
  if (game_phase == 4 || game_phase == 5){
    if (millis() >= timer_blink_or_error + 700){
      game_phase -= 4;                                                //переключение фазы игры
      highlight_player(player);
      displayGame_input(player, number);
    }   
  }

  if (game_phase >= 7){
    if (millis() >= timer_blink_or_error + 500){                       //мигаем GG на дисплее
      displayBlinkGG(player, blink_phase);
      blink_phase = !blink_phase;
      timer_blink_or_error = millis();
    }

    if (game_phase == 8){
      if (millis() >= timer_change + 40){
        draw_leds_win(player, score_change_counter);
        score_change_counter++;
        timer_change = millis();
        if (score_change_counter == LED_NUM / 2 + 1){
          score_change_counter = 0;
          game_phase = 9;
        }
      }
    }
    
    else if (game_phase == 9){
      if (millis() >= timer_change + 40){       
        draw_start_after_win(player, score_change_counter);
        timer_change = millis();
        score_change_counter++;
        if (score_change_counter == LED_NUM){
          score_change_counter = 0;
          game_phase = 10;
        }
      }
    }
    
    else if (game_phase == 10){                    //возвращаем партию к началу
      player_score[0] = game_score;
      player_score[1] = game_score;
      player = !player;
      //отрисовать изменение на дисплее
      lcd.clear();
      displayGame_setup(game_score);
      highlight_player(player);
      game_phase = 0;  
    }  
  }
  
  
}

void displayStart(char *text, byte line_number, boolean need_clear){
  byte fish_tail[] = {0x00, 0x00, 0x18, 0x15, 0x12, 0x12, 0x15, 0x18};
  byte fish_body[] = {0x00, 0x00, 0x1C, 0x02, 0x05, 0x01, 0x02, 0x1C};
  
  lcd.createChar(1, fish_tail);
  lcd.createChar(2, fish_body);

  if (need_clear) lcd.clear();
  
  for (int counter = 0; counter < 14; counter ++){
    lcd.setCursor(counter, line_number);
    if (counter > 1) lcd.print (text[counter - 2]);
    else lcd.print(" ");
    lcd.write(1);
    lcd.write(2);
    delay(150);
  }
  delay(1500);
}

void displayGame_setup(int score){
  lcd.setCursor(0, 0);
  lcd.print("Pl. 1: "); lcd.print(score);
  lcd.setCursor(0, 1);
  lcd.print("Pl. 2: "); lcd.print(score);
}

void displayGame_input(boolean player_number, int sign_number){
  lcd.setCursor(13, (byte) player_number);
  lcd.print("   ");
  lcd.setCursor(12, (byte) player_number);
  lcd.print(sign_number);
}

void highlight_player(boolean player_number){
  lcd.setCursor(11, (byte) player_number);
  lcd.print(">0");
  lcd.setCursor(11, (byte) !player_number);
  lcd.print("    ");
}

void displayGame_score(boolean player_number, int score){
  lcd.setCursor(8, (byte) player_number);
  lcd.print("  ");
  lcd.setCursor(7, (byte) player_number);
  lcd.print(score);
}

void displayError(boolean player_number){
  lcd.setCursor(11, (byte) player_number);
  lcd.print("ERROR");
}

void displayGreetings(boolean player_number){
  lcd.clear();
  lcd.setCursor(0, (byte) player_number);
  lcd.print(" Player ");
  lcd.print((byte) player_number + 1);
  lcd.print(" won!");
  lcd.setCursor(0, (byte) !player_number);
  lcd.print("  Good Game!");
}

void displayBlinkGG(boolean player_number, boolean phase){
  lcd.setCursor(2, (byte) !player_number);
  if (phase) lcd.print("Good Game!");
  else lcd.print("          ");
}

void draw_change(int score_before, int score_after, boolean player, byte animation_step){
  int led_score_before = 0;
  int led_score_after = 0;

  if (player){
    animation_step = STEP_NUM - animation_step;
    led_score_before = map(score_before, 0, game_score, LED_NUM / 2, LED_NUM - 1);
    led_score_before = constrain(led_score_before, LED_NUM / 2 + 1, LED_NUM - 1);
    led_score_after = map(score_after, 0, game_score, LED_NUM / 2, LED_NUM - 1);
    led_score_after = constrain(led_score_after, LED_NUM / 2 + 1, LED_NUM - 1);
    animation_step = map(animation_step, 0, STEP_NUM, led_score_after, led_score_before);
    animation_step = constrain(animation_step, led_score_after, led_score_before);
    leds[animation_step].setHue(160);
  }
  else{
    led_score_before = map(game_score - score_before, 0, game_score, 0, (LED_NUM / 2) - 2);
    led_score_before = constrain(led_score_before, 0, (LED_NUM / 2) - 2);
    led_score_after = map(game_score - score_after, 0, game_score, 0, (LED_NUM / 2) - 2);
    led_score_after = constrain(led_score_after, 0, (LED_NUM / 2) - 2);
    animation_step = map(animation_step, 0, STEP_NUM, led_score_before, led_score_after);
    animation_step = constrain(animation_step, led_score_before, led_score_after);
    leds[animation_step].setHue(0);
  }

  FastLED.show();
}

void draw_rollback(int score_before, int score_after, boolean player, byte animation_step){
  int led_score_before = 0;
  int led_score_after = 0;

  if (player){
    led_score_before = map(score_before, 0, game_score, LED_NUM / 2, LED_NUM - 1);
    led_score_before = constrain(led_score_before, LED_NUM / 2 + 1, LED_NUM - 1);
    led_score_after = map(score_after, 0, game_score, LED_NUM / 2, LED_NUM - 1);
    led_score_after = constrain(led_score_after, LED_NUM / 2 + 1, LED_NUM - 1);
    animation_step = map(animation_step, 0, STEP_NUM, led_score_after, led_score_before);
    animation_step = constrain(animation_step, led_score_after, led_score_before);
    leds[animation_step].setRGB(0,0,0);
  }
  else{
    animation_step = STEP_NUM - animation_step;
    led_score_before = map(game_score - score_before, 0, game_score, 0, (LED_NUM / 2) - 2);
    led_score_before = constrain(led_score_before, 0, (LED_NUM / 2) - 2);
    led_score_after = map(game_score - score_after, 0, game_score, 0, (LED_NUM / 2) - 2);
    led_score_after = constrain(led_score_after, 0, (LED_NUM / 2) - 2);
    animation_step = map(animation_step, 0, STEP_NUM, led_score_before, led_score_after);
    animation_step = constrain(animation_step, led_score_before, led_score_after);
    leds[animation_step].setRGB(0,0,0);
  }

  FastLED.show();
}

void draw_setup(){
  FastLED.clear();
  leds[LED_NUM - 1].setHue(160);
  leds[0].setHue(0);
  leds[LED_NUM / 2 - 1].setRGB(255, 255, 255);
  leds[LED_NUM / 2].setRGB(255, 255, 255);
  FastLED.show();
}

void draw_leds_win(boolean player, byte animation_step){
  
  if (!player){
    leds[(LED_NUM / 2) - 2 + animation_step].setHue(0);
  }

  if (player){
    leds[(LED_NUM / 2) + 1 - animation_step].setHue(160);
  }

  FastLED.show();
}

void draw_start_after_win(boolean player, byte animation_step){
  if (player) animation_step = LED_NUM - animation_step - 1;
  Serial.println(animation_step);
  if (animation_step == (LED_NUM / 2) || animation_step == (LED_NUM / 2) - 1){
    leds[animation_step].setRGB(255, 255, 255);
  }
  else if (animation_step != 0 && animation_step != LED_NUM - 1){
    leds[animation_step].setRGB(0, 0, 0);
  }
  FastLED.show();
}
