#ifndef BL0937_ENERGY_H
#define BL0937_ENERGY_H

#include <Arduino.h>

/**
 * @class BL0937_Energy
 * @brief Драйвер измерителя мощности BL0937 для LibreTiny (LN882HK) [INDEX].
 */
class BL0937_Energy {
public:
    BL0937_Energy();

    // Инициализация пинов (физические лапы передаются из main.cpp) [INDEX]
    void begin(uint8_t cfPin, uint8_t cf1Pin, uint8_t selPin);

    // Автомат сбора данных и переключения ноги SEL (вызывается в loop) [INDEX]
    void update();

    // Динамическая установка калибровочных коэффициентов (аргументы по умолчанию) [INDEX]
    void setCalibration(float vCoeff = 0.132f, float iCoeff = 0.01f, float pCoeff = 1.177f);

    // Динамическая установка интервала работы автомата (по умолчанию 500 мс) [INDEX]
    void setMeasureInterval(uint32_t intervalMs = 500);

    // Открытые функции-запросы (Геттеры) для передачи данных [INDEX]
    float getVoltage() const { return _liveVoltage; }
    float getCurrent() const { return _liveCurrent; }
    float getPower()   const { return _livePower; }

    // Публичные методы для инкремента счетчиков (вызываются из глобальных ISR)
    void incrementCF()  { _local_cf_pulses++; }
    void incrementCF1() { _local_cf1_pulses++; }

private:
    uint8_t _cfPin;
    uint8_t _cf1Pin;
    uint8_t _selPin;

    float _vCoeff;
    float _iCoeff;
    float _pCoeff;

    uint32_t _measureInterval;
    uint32_t _lastMeasureTime;
    uint8_t  _measureStep; 

    float _liveVoltage;
    float _liveCurrent;
    float _livePower;

    // Внутренние счетчики импульсов экземпляра класса
    volatile uint32_t _local_cf_pulses;
    volatile uint32_t _local_cf1_pulses;
};

#endif // BL0937_ENERGY_H
