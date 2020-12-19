# Clock-Div-Mult

### Clock Divider/Multiplier for eurorack synthesizer with one input and two outputs, based on a microcontroller.

Functions per output:
  - Speed potentiometer and control voltage
  - Sync-Button: If you switch from multiplying to dividing the output syncs to the input, 
                 so the output will not get high somewhere between the inputs.
  - Mode-Button: Multiply/Divide with "1, 2, 4, 8, 16, 32" or "1, 2, 3, 4, 5, 6, 7"
  - Reset-Button and trigger/gate input.
  - Visualisation with LEDs.
  
  I used a teensy 3.2 microcontroller, but other microcontrollers will also work.
  No libraries needed.
