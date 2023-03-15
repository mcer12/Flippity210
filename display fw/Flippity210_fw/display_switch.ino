
void toggleDaisyChainCount() {

#ifdef USE_SERIAL
  Serial.print(digitalRead(DAISY_A));
  Serial.print(" ");
  Serial.println(digitalRead(DAISY_B));
#endif

  if (digitalRead(DAISY_A) == 0 && digitalRead(DAISY_B) == 0) {
    displaysLimit = 1;
  }
  else if (digitalRead(DAISY_A) == 0 && digitalRead(DAISY_B) == 1) {
    displaysLimit = 2;
  }
  else if (digitalRead(DAISY_A) == 1 && digitalRead(DAISY_B) == 0) {
    displaysLimit = 3;
  }
  else if (digitalRead(DAISY_A) == 1 && digitalRead(DAISY_B) == 1) {
    displaysLimit = 4;
  }
}
