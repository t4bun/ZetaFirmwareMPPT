



void padding100(int padVar){
  if(padVar<10){lcd.print("  ");}
  else if(padVar<100){lcd.print(" ");}
}
void padding10(int padVar){
  if(padVar<10){lcd.print(" ");}
}

void showLCD(){

  lcd.setCursor(0,1); lcd.print(powerInput,2);  lcd.print("W");padding100(powerInput);    
  lcd.setCursor(0,2); lcd.print(voltageInput,4);lcd.print("V");padding100(voltageInput);            
  lcd.setCursor(0,3);lcd.print(currentInput,4);lcd.print("A");padding100(currentInput);    

  lcd.setCursor(13,1); lcd.print(PWM,1);lcd.print(" ");padding10(PWM); 
  lcd.setCursor(13,2); lcd.print("DUTY");
  lcd.setCursor(13,3); lcd.print(pret,1);lcd.print("%");padding100(pret); 
}
