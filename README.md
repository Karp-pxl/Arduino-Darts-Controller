# Arduino-Darts-Controller
This porject is an Arduino-based controller for darts match log and LED indication. 
1) Assemble the components in a way described in pdf file (Fritzing scheme).
2) Arduino NANO should be powered from USB.
3) Load the firmware to the Arduino NANO.

The controller works this way:

1) Initial animation on the display appears.
2) Game starts:
  - Press number keys to input current player score;
  - Press "*" to remove last symbol;
  - Press "#" to confirm the score;
  - Press "A" to revert last score input;
  - Press "B" if current player performs a checkout;
  - Press "C" to start new game having player 1 beginning;
  - Press "D" to start new game having player 2 beginning.
3) During the game, player's scores will be visualized on LED strip as red and blue strips.
4) After someone winning the game, animation on the LED strip appears.
