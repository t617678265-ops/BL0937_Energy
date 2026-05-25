#include "BL0937_Energy.h"

// Глобальный статический указатель для связи C-функций прерываний с объектом класса
static BL0937_Energy* _global_sensor_instance = nullptr;

// Чистые Си-функции прерываний верхнего уровня.
// Они вынесены за пределы класса, чтобы компилятор LibreTiny не перехватывал регистры чужих пинов [INDEX].
void globalIsrCF() {
    if (_global_sensor_instance != nullptr) {
        _global_sensor_instance->incrementCF();
    }
}

void globalIsrCF1() {
    if (_global_sensor_instance != nullptr) {
        _global_sensor_instance->incrementCF1();
    }
}

BL0937_Energy::BL0937_Energy() 
    : _cfPin(0), _cf1Pin(0), _selPin(0),
      _vCoeff(0.132f), _iCoeff(0.01f), _pCoeff(1.177f), 
      _measureInterval(500), _lastMeasureTime(0), _measureStep(0),
      _liveVoltage(0.0f), _liveCurrent(0.0f), _livePower(0.0f),
      _local_cf_pulses(0), _local_cf1_pulses(0) {
    _global_sensor_instance = this; 
}

void BL0937_Energy::begin(uint8_t cfPin, uint8_t cf1Pin, uint8_t selPin) {
    _cfPin = cfPin;
    _cf1Pin = cf1Pin;
    _selPin = selPin;

    // Сначала настраиваем управляющий пин SEL
    pinMode(_selPin, OUTPUT);
    digitalWrite(_selPin, HIGH); // Стартуем в режиме замера Напряжения [INDEX]

    // Настраиваем входы импульсов датчика
    pinMode(_cfPin, INPUT_PULLUP);
    pinMode(_cf1Pin, INPUT_PULLUP);

    _lastMeasureTime = millis();

    // Привязываем прерывания к изолированным Си-функциям верхнего уровня [INDEX]
    attachInterrupt(_cfPin, globalIsrCF, FALLING);
    attachInterrupt(_cf1Pin, globalIsrCF1, FALLING);
}

void BL0937_Energy::setCalibration(float vCoeff, float iCoeff, float pCoeff) {
    _vCoeff = vCoeff;
    _iCoeff = iCoeff;
    _pCoeff = pCoeff;
}

void BL0937_Energy::setMeasureInterval(uint32_t intervalMs) {
    _measureInterval = intervalMs;
}

void BL0937_Energy::update() {
    uint32_t now = millis();
    
    if (now - _lastMeasureTime > _measureInterval) {
        uint32_t duration = now - _lastMeasureTime; 
        _lastMeasureTime = now;

        // Копируем накопленные импульсы во временные переменные
        uint32_t active_cf_pulses = _local_cf_pulses;
        _local_cf_pulses = 0; 

        uint32_t cf1_pulses = _local_cf1_pulses;
        _local_cf1_pulses = 0; 

        // А. Расчет активной мощности (Ватты) [INDEX]
        _livePower = ((float)active_cf_pulses * 1000.0f * _pCoeff) / (float)duration;

        // Б. Автомат раздельного замера Тока / Напряжения [INDEX]
        if (_measureStep == 0) {
            // Напряжение [INDEX]
            _liveVoltage = ((float)cf1_pulses * 1000.0f * _vCoeff) / (float)duration;
            
            digitalWrite(_selPin, LOW); // Включаем режим ТОКА [INDEX]
            _measureStep = 1; 
        } 
        else {
            // Ток [INDEX]
            _liveCurrent = ((float)cf1_pulses * 1000.0f * _iCoeff) / (float)duration;

            digitalWrite(_selPin, HIGH); // Включаем режим НАПРЯЖЕНИЯ [INDEX]
            _measureStep = 0; 
        }
    }
}
