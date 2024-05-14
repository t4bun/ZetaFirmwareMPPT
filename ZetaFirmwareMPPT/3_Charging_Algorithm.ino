
void PWM_Modulation(){
  PWM = constrain(PWM,0,pwmMaxLimited);              
  ledcWrite(pwmChannel,PWM);                                                         //Set PWM duty cycle and write to GPIO 
}
void Charging_Algorithm(void){
  switch (charger_state) {
    case 1:
      PWM = pwm_Fixedvalue;
      PWM_Modulation();
      break;
    case 2:
      voutMax = desiredVout;
      if (voltageOutput>(voutMax + 0.1)){PWM--;} 
      else if (voltageOutput<(voutMax - 0.5)){PWM++;}
      PWM_Modulation();
      break;
    case 3:
      // if((voltageInput < -10) && voltageOutput >=0){ trying = 1;}  // Uncomment this if using battery as load - Undervoltage system protection 
      // if((powerInput<1) && (voltageInput >=13) ){ trying = 1;}     // Uncomment this if using battery as load - Undervoltage system protection 
      if(voltageInput<2){PWM = 0; reb = 7;}                           // Uncomment this if using battery as load, change voltageinput< 2 to 8
      // else if(voltageOutput>voutMax){PWM--; reb = 0;}              // Uncomment this if using battery as load    
      else{
        if(trying == 1){ PWM = inD; reb = 1; trying = 0; } 
        else if(powerInput>powerInputPrev && voltageInput>voltageInputPrev){PWM--; reb = 2;}  //  ↑P ↑V ; →MPP  //D--
        else if(powerInput>powerInputPrev && voltageInput<voltageInputPrev){PWM++; reb = 3;}  //  ↑P ↓V ; MPP←  //D++
        else if(powerInput<powerInputPrev && voltageInput>voltageInputPrev){PWM++; reb = 4;}  //  ↓P ↑V ; MPP→  //D++
        else if(powerInput<powerInputPrev && voltageInput<voltageInputPrev){PWM--; reb = 5;}  //  ↓P ↓V ; ←MPP  //D--
        else if(voltageOutput<voutMax)                                     {PWM++; reb = 6;}  //  MP MV ; MPP Reached - 
        
        }
      PWM_Modulation();
      break;
  }
        powerInputPrev   = powerInput;                                               //Store Previous Recorded Power
        powerOutputPrev   = powerOutput; 
        voltageInputPrev = voltageInput;                                             //Store Previous Recorded Voltage      
}


