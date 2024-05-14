void read_Sensors(){
  /////////// VOLTAGE & CURRENT SENSORS  ///////////
  shuntvoltage_in             = 0.0000;
  busvoltage_in               = 0.0000;
  current_mA_in               = 0.0000;
  loadvoltage_in              = 0.0000;
  power_mW_in                 = 0.0000;
  shuntvoltage_out            = 0.0000;
  busvoltage_out              = 0.0000;
  current_mA_out              = 0.0000;
  loadvoltage_out             = 0.0000;
  power_mW_out                = 0.0000;
  cJin                        = 0.0000;
  pWin                        = 0.0000;
  lWin                        = 0.0000;
  cJout                       = 0.0000;
  pWout                       = 0.0000;
  lWout                       = 0.0000;
  zetaEfficiency              = 0.0000;
  avgVsStore = avgVS;
  avgCsStore = avgCS;
  
  for(int i = 0; i<avgVS; i++){
      cJin = (ina219_in.getCurrent_mA() / 1000);
      lWin = ina219_in.getBusVoltage_V() + (ina219_in.getShuntVoltage_mV() / 1000);
      if (( cJin >= 0.000 )){        
          current_mA_in = current_mA_in +  cJin;
          loadvoltage_in = loadvoltage_in + lWin;
      }else {
              avgVsStore--;
      }  
  }
  for(int i = 0; i<avgCS; i++){
      cJout = (ina219_out.getCurrent_mA() / 1000);
      lWout = ina219_out.getBusVoltage_V() + (ina219_out.getShuntVoltage_mV() / 1000);
      if (( lWout >= 0.00 )){        
          current_mA_out = current_mA_out +  cJout;
          loadvoltage_out = loadvoltage_out + lWout;
      }else {
              avgCsStore--;
      }  
  }
  Serial.print(" AVG IN:");    Serial.print(avgVsStore);
  Serial.print(" AVG OUT:");    Serial.print(avgCsStore);
  if (avgVsStore == 0){
    voltageInput  = 0;
    currentInput = 0;
  }else{
    voltageInput  = (loadvoltage_in/avgVsStore);
    currentInput = (current_mA_in/avgVsStore);
    if (voltageInput<1){voltageInput = 0;}
    if (currentInput<0.001){currentInput=0.0000;}
    powerInput = (voltageInput*currentInput); 
  }
  if (avgCsStore == 0){
    voltageOutput  = 0;
    currentOutput = 0;
  }else{
    voltageOutput  = (loadvoltage_out/avgCsStore);
    currentOutput = (current_mA_out/avgCsStore);
    if (voltageOutput<1){voltageOutput = 0;}
    if (currentOutput<0.001){currentOutput=0.0000;}
    powerOutput = (voltageOutput*currentOutput);
  }

  if (powerInput <0){
    Serial.print(" VI Error:");    Serial.print(voltageInput);
    Serial.print(" CI Error:");    Serial.print(currentInput);
    Serial.print(" AVG Error:");    Serial.print(avgVsStore);
  }
  //POWER COMPUTATION - Through computation
  
  
  if (powerInput && powerOutput >0){
  zetaEfficiency = (powerOutput/powerInput)*100.00;
  }
}