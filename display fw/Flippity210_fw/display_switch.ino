void toggleDaisyChainCount()
{
    bool daisyA = digitalRead(DAISY_A);
    bool daisyB = digitalRead(DAISY_B);

#ifdef USE_SERIAL
    Serial.print(daisyA);
    Serial.print(" ");
    Serial.println(daisyB);
#endif

    displaysCount = 4 - ((daisyA << 1) | daisyB);
}
