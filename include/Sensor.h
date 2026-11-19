#ifndef SENSOR_H
#define SENSOR_H

#include "Dispositivo.h"

#include <string>

using namespace std;

/**
 * @file Sensor.h
 * @brief Sensores da fabrica.
 *
 * O XML usa STEMPERATURA, SFOGO, SMISSEL, SLUZ e SHUMIDADE.
 * Campos comuns: ID, MARCA, VALOR_AVISO, POSICAO e PROB_AVARIA.
 * SFOGO e SMISSEL trazem ainda PROB_1, a probabilidade do evento.
 * SHUMIDADE pode trazer RAIO_VIZINHANCA; se faltar,
 * usa-se o raio geral da fabrica.
 */

/**
 * @brief Classe base dos sensores.
 *
 * SensorLuz, SensorHumidade, SensorFumo, SensorMissel e SensorTemperatura
 * derivam desta classe e concretizam o aviso.
 */
class Sensor : public Dispositivo
{
    protected:
        double valor; ///< Leitura actual do sensor.
        bool avisoAnterior; ///< true se no passo anterior o sensor ja estava em aviso.
        double probabilidadeUm; ///< Probabilidade do evento (PROB_1). Zero se o tipo nao a usa.

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
        * @param posX Coordenada X. Zero e aceite.
        * @param posY Coordenada Y. Zero e aceite.
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

        /** @brief Nome do tipo, igual a etiqueta do XML quando o sensor vem do ficheiro. */
        virtual string GetTipo() const = 0;

        /** @brief Limiar de aviso. Zero se o tipo nao o define. */
        virtual double GetLimiar() const;

        /** @brief Probabilidade do evento, ja em fracao entre 0 e 1. */
        double GetProbabilidadeUm() const;

        /**
        * @brief Guarda PROB_1. Valores iguais ou superiores a 1 sao percentagens.
        * @param probabilidade Fracao entre 0 e 1.
        */
        void SetProbabilidadeUm(double probabilidade);

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
        * @param id Identificador.
        * @param marca Fabricante.
        * @param valor Leitura inicial.
        * @param probabilidadeAvaria Probabilidade de avaria por hora.
        * @param posX Coordenada X.
        * @param posY Coordenada Y.
        * @param limiar Limite inferior de luminosidade.
        */
        SensorLuz(int id, const string& marca, double valor, double probabilidadeAvaria,
                  int posX, int posY, double limiar);

        /** @brief Destrutor. */
        virtual ~SensorLuz();

        /** @brief Devolve o limiar de luminosidade. */
        double GetLimiar() const;

        /** @brief Devolve "LUZ". */
        virtual string GetTipo() const;

        /** @brief Ha aviso quando a leitura desce abaixo do limiar. */
        virtual bool EmAviso() const;

        /** @brief Escurece na ultima hora e recupera na abertura. */
        virtual void Actualizar(bool motoresEmMarcha, int horaAtual, int horaAbertura,
                                int horaEncerramento, double fracaoHora);

        /** @brief Repoe uma leitura acima do limiar. */
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
        * @param id Identificador.
        * @param marca Fabricante.
        * @param valor Leitura inicial.
        * @param probabilidadeAvaria Probabilidade de avaria por hora.
        * @param posX Coordenada X.
        * @param posY Coordenada Y.
        * @param limiar Valor a partir do qual ha aviso.
        * @param raioVizinhanca Raio da vizinhanca. So e guardado se for positivo.
        */
        SensorHumidade(int id, const string& marca, double valor, double probabilidadeAvaria,
                       int posX, int posY, double limiar, int raioVizinhanca);

        /** @brief Destrutor. */
        virtual ~SensorHumidade();

        /** @brief Devolve o limiar de humidade. */
        double GetLimiar() const;

        /** @brief Devolve o raio em que os motores sao desligados. */
        virtual int GetRaioVizinhanca() const;

        /** @brief Devolve "HUMIDADE". */
        virtual string GetTipo() const;

        /** @brief Ha aviso quando a leitura atinge o limiar. */
        virtual bool EmAviso() const;

        /** @brief Sobe a humidade, por hora, enquanto houver motores em RUN. */
        virtual void Actualizar(bool motoresEmMarcha, int horaAtual, int horaAbertura,
                                int horaEncerramento, double fracaoHora);

        /** @brief Repoe uma leitura abaixo do limiar. */
        virtual void DefinirValorSeguro();
};

/**
 * @brief Sensor de fumo.
 *
 * Dispara quando o valor atinge o limiar.
 * Com motores em RUN o fumo acumula.
 * PROB_1, quando existe, pode levar a leitura ao limiar.
 * No XML a etiqueta e SFOGO.
 */
class SensorFumo : public Sensor
{
    private:
        double limiar; ///< A partir deste valor ha aviso.

    public:
        /**
        * @brief Cria um sensor de fumo.
        * @param id Identificador.
        * @param marca Fabricante.
        * @param valor Leitura inicial.
        * @param probabilidadeAvaria Probabilidade de avaria por hora.
        * @param posX Coordenada X.
        * @param posY Coordenada Y.
        * @param limiar Valor a partir do qual ha aviso.
        */
        SensorFumo(int id, const string& marca, double valor, double probabilidadeAvaria,
                   int posX, int posY, double limiar);

        /** @brief Destrutor. */
        virtual ~SensorFumo();

        /** @brief Devolve o limiar de fumo. */
        double GetLimiar() const;

        /** @brief Devolve "SFOGO". */
        virtual string GetTipo() const;

        /** @brief Ha aviso quando a leitura atinge o limiar. */
        virtual bool EmAviso() const;

        /** @brief Acumula fumo por hora e pode saltar ao limiar segundo PROB_1. */
        virtual void Actualizar(bool motoresEmMarcha, int horaAtual, int horaAbertura,
                                int horaEncerramento, double fracaoHora);

        /** @brief Repoe a leitura a zero. */
        virtual void DefinirValorSeguro();
};

/**
 * @brief Sensor de missel.
 *
 * Dispara quando o valor atinge VALOR_AVISO.
 * Em cada passo ha uma hipotese, proporcional a PROB_1 e a fracao de hora,
 * de a leitura passar ao limiar. No XML a etiqueta e SMISSEL.
 */
class SensorMissel : public Sensor
{
    private:
        double limiar; ///< VALOR_AVISO. A leitura chega aqui quando o evento ocorre.

    public:
        /**
        * @brief Cria um sensor de missel.
        * @param id Identificador.
        * @param marca Fabricante.
        * @param valor Leitura inicial.
        * @param probabilidadeAvaria Probabilidade de avaria por hora.
        * @param posX Coordenada X.
        * @param posY Coordenada Y.
        * @param limiar VALOR_AVISO. A leitura chega a este valor quando o evento ocorre.
        */
        SensorMissel(int id, const string& marca, double valor, double probabilidadeAvaria,
                     int posX, int posY, double limiar);

        /** @brief Destrutor. */
        virtual ~SensorMissel();

        /** @brief Devolve o VALOR_AVISO. */
        virtual double GetLimiar() const;

        /** @brief Devolve "SMISSEL". */
        virtual string GetTipo() const;

        /** @brief Ha aviso quando a leitura atinge o limiar. */
        virtual bool EmAviso() const;

        /** @brief Pode levar a leitura ao limiar, com probabilidade PROB_1 por hora. */
        virtual void Actualizar(bool motoresEmMarcha, int horaAtual, int horaAbertura,
                                int horaEncerramento, double fracaoHora);

        /** @brief Repoe a leitura a zero. */
        virtual void DefinirValorSeguro();
};

/**
 * @brief Sensor de temperatura. No XML a etiqueta e STEMPERATURA.
 *
 * Dispara quando a leitura atinge VALOR_AVISO.
 */
class SensorTemperatura : public Sensor
{
    private:
        double limiar; ///< VALOR_AVISO. A partir deste valor ha aviso.

    public:
        /**
        * @brief Cria um sensor de temperatura.
        * @param id Identificador.
        * @param marca Fabricante.
        * @param valor Leitura inicial.
        * @param probabilidadeAvaria Probabilidade de avaria por hora.
        * @param posX Coordenada X.
        * @param posY Coordenada Y.
        * @param limiar VALOR_AVISO.
        */
        SensorTemperatura(int id, const string& marca, double valor, double probabilidadeAvaria,
                          int posX, int posY, double limiar);

        /** @brief Destrutor. */
        virtual ~SensorTemperatura();

        /** @brief Devolve o VALOR_AVISO. */
        virtual double GetLimiar() const;

        /** @brief Devolve "STEMPERATURA". */
        virtual string GetTipo() const;

        /** @brief Ha aviso quando a leitura atinge o limiar. */
        virtual bool EmAviso() const;

        /** @brief Repoe a leitura a zero. */
        virtual void DefinirValorSeguro();
};

#endif
