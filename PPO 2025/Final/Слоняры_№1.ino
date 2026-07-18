String decrypt(int arr[], int size){
  String result = "";

  for(int i = 0; i < size / 2;i++){
    char c = char(arr[i]-6);
    result += c; 

  }
  return result;
}




#include <LiquidCrystal.h>
LiquidCrystal lcd(6, 7, 8, 9, 10, 11);
float termo_value = 0;
int lazer = A0;
int termo = A1;
int DHT11 = A2;
int holla = A3;
float R1 = 10000;
float c1 = 0.001129148, c2 = 0.000234125, c3 = 0.0000000876741;
float logR2, R2, T; 
void setup() {
  int test_arr[] = {78, 107, 114, 114, 117, 78, 107, 114, 114, 117};
  String otvet = decrypt(test_arr, sizeof(test_arr));
  Serial.begin(9600);
  pinMode(lazer, INPUT);
  pinMode(lazer, INPUT_PULLUP);
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print(otvet);
  lcd.begin(16, 2);
  pinMode(termo, INPUT_PULLUP);
  
  Serial.println(otvet);
}
 
void loop() {
  float lazer_value = (5.0 * 20000 / (analogRead(lazer) * (5.0 / 1023.0))) - 20000;
  int t_value = analogRead(termo);
  float DHT11_value = analogRead(DHT11) / 12.342 + 3;
  int holla_value = analogRead(holla);
  termo_func(t_value);
  Serial.print(lazer_value);
  Serial.println("- лазер");
  Serial.print(termo_value);
  Serial.println("- термодатчик");
  Serial.print(DHT11_value);
  Serial.println("- DHT11");
  Serial.print(holla_value);
  Serial.println("- датчик Холла");
  Serial.println("-------------------");
  
  lcd.setCursor(0, 0);
  lcd.print("Lazer: ");
  lcd.print(lazer_value / 1000);
  lcd.setCursor(0, 1);
  lcd.print("Temp: ");
  lcd.print(termo_value);
  delay(1000);
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("DHT11: ");
  lcd.print(DHT11_value);
  lcd.setCursor(0, 1);
  lcd.print("Holla: ");
  lcd.print(holla_value);
  delay(1000);
  lcd.clear();
  
  
}

float termo_func(int value){
  const float nominalResistance = 10000; 
  const float nominalTemp = 25.0; 
  const float betaCoefficient = 3950; 
  const float seriesResistor = 10000; 
  float voltage = value * (5.0 / 1023.0);
  float resistance = seriesResistor * (5.0 / voltage - 1.0);
  float temperature;
  temperature = resistance / nominalResistance;    
  temperature = log(temperature);                 
  temperature /= betaCoefficient;                
  temperature += 1.0 / (nominalTemp + 273.15);  
  temperature = 1.0 / temperature;            
  temperature -= 273.15;
  termo_value = abs(temperature) + 20;

}