#ifndef SENSOR_H
#define SENSOR_H

#include "Dispositivo.h"

#include <string>

using namespace std;

/**
 * @file Sensor.h
 * @brief Sensores da fabrica: LUZ, HUMIDADE, FUMO e MISSEL.
 *
 * Campos comuns: ID, MARCA, VALOR, PROBABILIDADE_AVARIA, X, Y.
 * Cada tipo decide quando esta em aviso:
 * - LUZ: o valor desce abaixo do limiar.
 * - HUMIDADE: o valor atinge o limiar. O raio e a vizinhanca X
 *   dos motores que devem ser desligados.
 * - FUMO: o valor atinge o limiar.
 * - MISSEL: o valor chega a 1.
 */

/**
 * @brief Sensor generico. Os quatro tipos concretizam o aviso.
 */
class Sensor : public Dispositivo
{
    protected:
        double valor; ///< Leitura actual do sensor.
        bool avisoAnterior; ///< true se no passo anterior o sensor ja estava em aviso.

        /**
        * @brief Poe a leitura num valor que nao dispara aviso.
        *
        * Cada tipo define o seu valor seguro. Usado pela manutencao.
        */
        virtual void DefinirValorSeguro() = 0;

    public:
        /**
        * @brief Cria um sensor na posicao indicada.
        * @param id Identificador.
        * @param marca Fabricante.
        * @param valor Leitura inicial.
        * @param probabilidadeAvaria Probabilidade de avaria por hora.
        * @param posX Coordenada X, positiva.
        * @param posY Coordenada Y, positiva.
        */
        Sensor(int id, const string& marca, double valor, double probabilidadeAvaria, int posX, int posY);

        /** @brief Destrutor virtual. */
        virtual ~Sensor();

        /** @brief Devolve a leitura actual. */
        double GetValor() const;

        /**
        * @brief Substitui a leitura.
        * @param novoValor Nova leitura.
        */
        void SetValor(double novoValor);

        /** @brief Nome do tipo: LUZ, HUMIDADE, FUMO ou MISSEL. */
        virtual string GetTipo() const = 0;

        /**
        * @brief Indica se a leitura actual e um aviso.
        * @return true quando a regra do tipo e satisfeita.
        */
        virtual bool EmAviso() const = 0;

        /**
        * @brief Raio de vizinhanca, em celulas do campo.
        * @return Zero em todos os tipos excepto a humidade.
        */
        virtual int GetRaioVizinhanca() const;

        /**
        * @brief Detecta a passagem de normal para aviso.
        * @return true apenas no passo em que o aviso comeca.
        */
        bool Disparou();

        /**
        * @brief Actualiza a leitura com o horario e com o funcionamento dos motores.
        * @param motoresEmMarcha true se algum motor esta em RUN.
        * @param horaAtual Hora simulada, de 0 a 23.
        * @param horaAbertura Hora de abertura da fabrica.
        * @param horaEncerramento Hora de encerramento da fabrica.
        * @param fracaoHora Fracao de hora que este passo representa.
        */
        virtual void Actualizar(bool motoresEmMarcha, int horaAtual, int horaAbertura,
                                int horaEncerramento, double fracaoHora);

        /**
        * @brief Repoe o sensor: valor seguro e avaria limpa.
        */
        void Repor();
};

/**
 * @brief Sensor de luminosidade.
 *
 * Dispara quando o valor e inferior ao limiar.
 * Na ultima hora antes do encerramento a luz desce.
 * Na hora de abertura a luz volta a um valor seguro.
 */
class SensorLuz : public Sensor
{
    private:
        double limiar; ///< Abaixo deste valor ha aviso.

    public:
        /**
        * @brief Cria um sensor de luz.
        * @param limiar Limite inferior de luminosidade.
        */
        SensorLuz(int id, const string& marca, double valor, double probabilidadeAvaria,
                  int posX, int posY, double limiar);
        virtual ~SensorLuz();

        /** @brief Devolve o limiar de luminosidade. */
        double GetLimiar() const;
        virtual string GetTipo() const;
        virtual bool EmAviso() const;
        virtual void Actualizar(bool motoresEmMarcha, int horaAtual, int horaAbertura,
                                int horaEncerramento, double fracaoHora);
        virtual void DefinirValorSeguro();
};

/**
 * @brief Sensor de humidade.
 *
 * Dispara quando o valor atinge o limiar.
 * Com motores em RUN a humidade sobe.
 * O raio define a vizinhanca X em que os motores sao desligados.
 */
class SensorHumidade : public Sensor
{
    private:
        double limiar; ///< A partir deste valor ha aviso.
        int raioVizinhanca; ///< Raio positivo da vizinhanca, em celulas.

    public:
        /**
        * @brief Cria um sensor de humidade.
        * @param limiar Valor a partir do qual ha aviso.
        * @param raioVizinhanca Raio da vizinhanca. So e guardado se for positivo.
        */
        SensorHumidade(int id, const string& marca, double valor, double probabilidadeAvaria,
                       int posX, int posY, double limiar, int raioVizinhanca);
        virtual ~SensorHumidade();

        /** @brief Devolve o limiar de humidade. */
        double GetLimiar() const;
        virtual int GetRaioVizinhanca() const;
        virtual string GetTipo() const;
        virtual bool EmAviso() const;
        virtual void Actualizar(bool motoresEmMarcha, int horaAtual, int horaAbertura,
                                int horaEncerramento, double fracaoHora);
        virtual void DefinirValorSeguro();
};

/**
 * @brief Sensor de fumo.
 *
 * Dispara quando o valor atinge o limiar.
 * Com motores em RUN o fumo acumula.
 */
class SensorFumo : public Sensor
{
    private:
        double limiar; ///< A partir deste valor ha aviso.

    public:
        /**
        * @brief Cria um sensor de fumo.
        * @param limiar Valor a partir do qual ha aviso.
        */
        SensorFumo(int id, const string& marca, double valor, double probabilidadeAvaria,
                   int posX, int posY, double limiar);
        virtual ~SensorFumo();

        /** @brief Devolve o limiar de fumo. */
        double GetLimiar() const;
        virtual string GetTipo() const;
        virtual bool EmAviso() const;
        virtual void Actualizar(bool motoresEmMarcha, int horaAtual, int horaAbertura,
                                int horaEncerramento, double fracaoHora);
        virtual void DefinirValorSeguro();
};

/**
 * @brief Sensor de missel.
 *
 * Dispara quando o valor e pelo menos 1.
 * Em cada passo ha uma hipotese, proporcional a probabilidade e a fracao de hora,
 * de a leitura passar a 1.
 */
class SensorMissel : public Sensor
{
    public:
        /**
        * @brief Cria um sensor de missel.
        */
        SensorMissel(int id, const string& marca, double valor, double probabilidadeAvaria,
                     int posX, int posY);
        virtual ~SensorMissel();

        virtual string GetTipo() const;
        virtual bool EmAviso() const;
        virtual void Actualizar(bool motoresEmMarcha, int horaAtual, int horaAbertura,
                                int horaEncerramento, double fracaoHora);
        virtual void DefinirValorSeguro();
};

/**
 * @brief Sensor de temperatura ambiente.
 *
 * Dispara quando o valor atinge o limiar.
 * Sobe gradualmente enquanto houver motores em RUN.
 */
class SensorTemperatura : public Sensor
{
    private:
        double limiar;

    public:
        SensorTemperatura(int id, const string& marca, double valor, double probabilidadeAvaria,
                          int posX, int posY, double limiar);
        virtual ~SensorTemperatura();

        double GetLimiar() const;
        virtual string GetTipo() const;
        virtual bool EmAviso() const;
        virtual void Actualizar(bool motoresEmMarcha, int horaAtual, int horaAbertura,
                                int horaEncerramento, double fracaoHora);
        virtual void DefinirValorSeguro();
};

/**
 * @brief Sensor de fogo/chama.
 *
 * Dispara quando o valor e pelo menos 1.
 * Comportamento semelhante ao sensor de fumo mas associado a chama directa.
 */
class SensorFogo : public Sensor
{
    private:
        double limiar;

    public:
        SensorFogo(int id, const string& marca, double valor, double probabilidadeAvaria,
                   int posX, int posY, double limiar);
        virtual ~SensorFogo();

        double GetLimiar() const;
        virtual string GetTipo() const;
        virtual bool EmAviso() const;
        virtual void Actualizar(bool motoresEmMarcha, int horaAtual, int horaAbertura,
                                int horaEncerramento, double fracaoHora);
        virtual void DefinirValorSeguro();
};

#endif
