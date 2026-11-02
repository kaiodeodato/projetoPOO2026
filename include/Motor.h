#ifndef MOTOR_H
#define MOTOR_H

#include "Dispositivo.h"
#include "Tipos.h"

#include <string>

using namespace std;

/**
 * @file Motor.h
 * @brief Motor da fabrica.
 *
 * Campos: ID, MARCA, CONSUMO_HORA, TEMPERATURA_AVISO, TEMPERATURA_PARAGEM,
 * PROBABILIDADE_AVARIA, mais o tipo, a posicao (X, Y) e o estado.
 *
 * O tipo e uma cadeia (por exemplo "Esteira"). Um motor concreto pode
 * derivar desta classe e reimplementar RUN, START, RESTART, STOP e
 * ESTOU_QUENTE. A implementacao base ja cumpre as regras do enunciado.
 *
 * A classe de temperatura do proprio motor depende dos seus limiares:
 * abaixo de TEMPERATURA_AVISO fica VERDE, entre o aviso e a paragem
 * fica AMARELO, e a partir da paragem fica VERMELHO e passa a STOP.
 * Os intervalos globais do XML ficam na fabrica e servem a manutencao.
 *
 * Transicoes:
 * STOP -> START -> RUN -> STOP.
 * AVARIADO so sai com RESTART, e apenas se nao estiver VERMELHO.
 *
 * Dentro desta classe os metodos chamam-se STOP, START e RUN, por isso
 * o estado correspondente escreve-se ::STOP, ::START e ::RUN.
 */
class Motor : public Dispositivo
{
    private:
        string tipo;
        double consumoHora;
        double temperaturaAviso;
        double temperaturaParagem;
        double temperaturaAtual;
        double consumoDia;
        int tempoTrabalho;
        ESTADO_MOTOR estado; ///< STOP, START, RUN ou AVARIADO.

        /**
        * @brief Indica se a temperatura ja chegou a paragem.
        * @return true quando a classe actual e VERMELHO.
        */
        bool estaEmPerigo() const;

    public:
        Motor(int id, const string& tipo, const string& marca, double consumoHora,
              double temperaturaAviso, double temperaturaParagem, double probabilidadeAvaria,
              int posX, int posY);
        virtual ~Motor();

        virtual void RUN();
        virtual void START();
        virtual void RESTART();
        virtual void STOP();

        /**
        * @brief Classifica a temperatura e, se estiver VERMELHO, poe o motor em STOP.
        */
        virtual CLASSE_TEMPERATURA ESTOU_QUENTE();

        /**
        * @brief Classifica a temperatura sem alterar o estado.
        */
        CLASSE_TEMPERATURA GetClasseTemperatura() const;

        ESTADO_MOTOR GetEstado() const;
        string GetTipo() const;
        double GetConsumoHora() const;
        double GetConsumoDia() const;
        double GetTemperaturaAviso() const;
        double GetTemperaturaParagem() const;
        double GetTemperaturaAtual() const;
        int GetTempoTrabalho() const;

        /**
        * @brief Define a temperatura actual.
        * @param temperatura Nova temperatura.
        */
        void SetTemperaturaAtual(double temperatura);

        /**
        * @brief Soma graus a temperatura actual.
        * @param graus Valor positivo de aquecimento.
        */
        void Aquecer(double graus);

        /**
        * @brief Tira graus a temperatura, sem descer abaixo de zero.
        * @param graus Valor positivo de arrefecimento.
        */
        void Arrefecer(double graus);

        /**
        * @brief Poe o motor em AVARIADO e conta a avaria uma unica vez.
        * @return false se o motor ja estava avariado.
        */
        bool Avariar();

        /**
        * @brief Limpa a avaria e, se estava AVARIADO, fica em STOP.
        */
        void Reparar();

        /**
        * @brief Acumula minutos de trabalho e o consumo correspondente.
        * @param minutos Minutos em RUN neste passo. Valores nao positivos sao ignorados.
        */
        void RegistarTrabalho(int minutos);

        /** @brief Poe o consumo do dia a zero, no virar do dia. */
        void ReiniciarConsumoDia();
};

#endif
