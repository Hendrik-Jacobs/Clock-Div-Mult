// Clock Divider and Multiplier for Eurorack Synthesizer.
// Based on Teensy microcontroller (LC or 3.2)
// Hendrik Jacobs, 2020

// Pins
int triggerPin = 21, outPin[2] = {2, 3};
int resetPin[2] = {20, 19}, stepPin[2] = {8, 9};
int syncPin[2] = {6, 7}, inLED = 5;
int cvPin[4] = {A0, A1, A3, A4};

// Variables
int diff_master, counter, last_counter[2], diff[2], cv[4];
byte first = 0, mode[2], high = 0, steps[2] = {0, 0}, sync[2];
byte res[2] = {0, 0}, sthigh[2], one[2] = {5, 6};
byte mult_two[11] = {32, 16, 8, 4, 2, 1, 2, 4, 8, 16, 32};
byte mult_seven[13] = {7, 6, 5, 4, 3, 2, 1, 2, 3, 4, 5, 6, 7};

void setup() {
  Serial.begin(9600);
  pinMode(triggerPin, INPUT);
  pinMode(inLED, OUTPUT);
  for (byte i = 0; i <= 1; i++) {
    pinMode(outPin[i], OUTPUT);
    pinMode(syncPin[i], INPUT_PULLUP);
    pinMode(resetPin[i], INPUT_PULLUP);
    pinMode(stepPin[i], INPUT_PULLUP);
    digitalWrite(outPin[i], HIGH);
  }
}

void loop() { 
  // Check reset inputs.
  if (digitalRead(resetPin[0]) == LOW && first != 0) res[0] = 1;   
  if (digitalRead(resetPin[1]) == LOW && first != 0) res[1] = 1;

  // Counter for input.
  counter++;
  // Counter for output 1.
  last_counter[0]++;
  // Counter for output 2.
  last_counter[1]++;

  // Resets when input clock gets faster.
  if (counter > diff_master * 2) {
    first = 0;
    for (byte i = 0; i <= 1; i++) {
      last_counter[i] = 0;
      digitalWrite(outPin[i], HIGH);
    }
  }
  
  byte trigger = digitalRead(triggerPin);

  // If trigger comes in.
  if (high == 0 && trigger == LOW) {
    if (first == 0) first = 1;
    else if (first == 1) {
      // Starting to count at second incoming trigger.
      first = 2;
      last_counter[0] = 0;
      last_counter[1] = 0;
    }
    else if (res[0] == 1) {
      // Reset 1
      res[0] = 0;
      last_counter[0] = 0;
    }
    if (res[1] == 1) {
      // Reset 2
      res[1] = 0;
      last_counter[1] = 0;
    }
    // diff_master saves the counts between the last two incoming trigger.
    diff_master = counter;
    counter = 0;
    // Input is High
    high = 1;
  }
  else if (trigger == HIGH) high = 0;
  digitalWrite(inLED, high);

  // After seciónd trigger.
  if (first != 0) {
    for (byte i = 0; i <= 1; i++) {
      sync[i] = digitalRead(syncPin[i]);
      steps[i] = digitalRead(stepPin[i]);

      // Read pot and CV.
      cv[i] = analogRead(cvPin[i]);
      cv[i+2] = analogRead(cvPin[i+2]);
      cv[i] += (cv[i+2] - 512) * (-1);
      cv[i] = min(1023, max(0, cv[i]));

      // Choose betwenn [...1,2,4,8,16,32] and [...1,2,4,5,6,7]
      if (steps[i] == 1)mode[i] = map(cv[i], 0, 1023, 10, 0);
      else mode[i] = map(cv[i], 0, 1023, 12, 0);
      
      // Calculate counts between outputs
      if (mode[i] < one[steps[i]] && steps[i] == 0) 
        diff[i] = diff_master / mult_two[mode[i]];
      else if (mode[i] >= one[steps[i]] && steps[i] == 0) 
        diff[i] = diff_master * mult_two[mode[i]];
      else if (mode[i] < one[steps[i]] && steps[i] == 1) 
        diff[i] = diff_master / mult_seven[mode[i]];
      else if (mode[i] >= one[steps[i]] && steps[i] == 1) 
        diff[i] = diff_master * mult_seven[mode[i]];

      // When rate gets faster and last_count is out of range
      if (last_counter[i] > diff[i]) last_counter[i] -= diff[i];

      if (last_counter[i] >= diff[i] / 2 && last_counter[i] < diff[i]) 
      {
        // Set output to Low
        digitalWrite(outPin[i], HIGH);      
      }
      else if (last_counter[i] == diff[i]) {
        if (mode[i] > one[i] && counter > diff_master / 2 && sync[i] == LOW) {
          res[i] = 1;
          last_counter[i] = 0;
        }
        else if (mode[i] > one[i] && counter <= diff_master / 2 && sync[i] == LOW) 
          last_counter[i] = counter;
        else last_counter[i] = 0;
        
        if (mode[i] <= one[steps[i]] && counter >= diff[i] && sync[i] == LOW) 
          res[i] = 1;
        else if (mode[i] <= one[steps[i]] && counter < diff[i] && sync[i] == LOW) 
          last_counter[i] = counter;
      }
      if (last_counter[i] >= 0 && last_counter[i] < diff[i] / 2) 
        digitalWrite(outPin[i], LOW);
    }
  }
}
