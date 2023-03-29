
void toggleDaisyChainCount() {

#ifdef USE_SERIAL
  Serial.print(digitalRead(DAISY_A));
  Serial.print(" ");
  Serial.println(digitalRead(DAISY_B));
#endif

  bool daisyA = digitalRead(DAISY_A);
  bool daisyB = digitalRead(DAISY_B);

  if (daisyA && daisyB) {
    displaysCount = 1;
  }
  else if (daisyA && !daisyB) {
    displaysCount = 2;
  }
  else if (!daisyA && daisyB) {
    displaysCount = 3;
  }
  else if (!daisyA && !daisyB) {
    displaysCount = 4;
  }
}
