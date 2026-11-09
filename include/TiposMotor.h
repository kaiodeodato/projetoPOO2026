#ifndef TIPOS_MOTOR_H
#define TIPOS_MOTOR_H

#include "Motor.h"
#include "Tipos.h"

#include <string>

using namespace std;

/**
 * @file TiposMotor.h
 * @brief Subclasses concretas de Motor: combustao, electrico e inducao.
 *
 * Cada subclasse guarda os seus proprios intervalos de temperatura
 * (VERDE/AMARELO/VERMELHO) e a sua probabilidade de avaria global,
 * tal como definidos em <DEFINICOES> no ficheiro XML do simulador.
 *
 * Os intervalos sao usados por ESTOU_QUENTE para classificar a
 * temperatura actual em relacao aos limiares globais do tipo,
 * alem dos limiares individuais herdados de Motor.
 */

/**
 * @brief Motor de combustao interna.
 */
class MotorCombustao : public Motor
{
    private:
        IntervaloTemperatura verde;
        IntervaloTemperatura amarelo;
        IntervaloTemperatura vermelho;

    public:
        /**
        * @brief Cria um motor de combustao.
        * @param id Identificador.
        * @param marca Fabricante.
        * @param consumoHora Consumo em kWh por hora.
        * @param probabilidadeAvaria Probabilidade de avaria por hora.
        * @param posX Posicao X no campo.
        * @param posY Posicao Y no campo.
        * @param verde Intervalo VERDE de temperatura.
        * @param amarelo Intervalo AMARELO de temperatura.
        * @param vermelho Intervalo VERMELHO de temperatura.
        */
        MotorCombustao(int id, const string& marca, double consumoHora,
                       double probabilidadeAvaria, int posX, int posY,
                       const IntervaloTemperatura& verde,
                       const IntervaloTemperatura& amarelo,
                       const IntervaloTemperatura& vermelho);
        virtual ~MotorCombustao();

        virtual string GetTipo() const;

        /** @brief Devolve o intervalo VERDE deste tipo de motor. */
        IntervaloTemperatura GetIntervaloVerde() const;
        /** @brief Devolve o intervalo AMARELO deste tipo de motor. */
        IntervaloTemperatura GetIntervaloAmarelo() const;
        /** @brief Devolve o intervalo VERMELHO deste tipo de motor. */
        IntervaloTemperatura GetIntervaloVermelho() const;
};

/**
 * @brief Motor electrico.
 */
class MotorEletrico : public Motor
{
    private:
        IntervaloTemperatura verde;
        IntervaloTemperatura amarelo;
        IntervaloTemperatura vermelho;

    public:
        MotorEletrico(int id, const string& marca, double consumoHora,
                      double probabilidadeAvaria, int posX, int posY,
                      const IntervaloTemperatura& verde,
                      const IntervaloTemperatura& amarelo,
                      const IntervaloTemperatura& vermelho);
        virtual ~MotorEletrico();

        virtual string GetTipo() const;

        IntervaloTemperatura GetIntervaloVerde() const;
        IntervaloTemperatura GetIntervaloAmarelo() const;
        IntervaloTemperatura GetIntervaloVermelho() const;
};

/**
 * @brief Motor de inducao.
 */
class MotorInducao : public Motor
{
    private:
        IntervaloTemperatura verde;
        IntervaloTemperatura amarelo;
        IntervaloTemperatura vermelho;

    public:
        MotorInducao(int id, const string& marca, double consumoHora,
                     double probabilidadeAvaria, int posX, int posY,
                     const IntervaloTemperatura& verde,
                     const IntervaloTemperatura& amarelo,
                     const IntervaloTemperatura& vermelho);
        virtual ~MotorInducao();

        virtual string GetTipo() const;

        IntervaloTemperatura GetIntervaloVerde() const;
        IntervaloTemperatura GetIntervaloAmarelo() const;
        IntervaloTemperatura GetIntervaloVermelho() const;
};

#endif
