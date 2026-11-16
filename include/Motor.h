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
 * Motor e a classe base. MotorEletrico, MotorCombustao e MotorInducao
 * derivam dela e reimplementam RUN, START, RESTART, STOP e ESTOU_QUENTE.
 * O ritmo de aquecimento muda com o tipo. Os limiares vem do XML.
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

/**
 * @brief Motor generico. Os tipos concretos reimplementam o comando e o aquecimento.
 */
class Motor : public Dispositivo
{
    private:
        string tipo; ///< Etiqueta do motor, por exemplo MCOMBUSTAO.
        double consumoHora; ///< Consumo por hora de funcionamento.
        double temperaturaAviso; ///< A partir daqui a classe passa a AMARELO.
        double temperaturaParagem; ///< A partir daqui a classe passa a VERMELHO.
        double temperaturaAtual; ///< Temperatura neste momento.
        double consumoDia; ///< Consumo acumulado no dia simulado.
        int tempoTrabalho; ///< Minutos passados em RUN.
        ESTADO_MOTOR estado; ///< STOP, START, RUN ou AVARIADO.

        /**
        * @brief Indica se a temperatura ja chegou a paragem.
        * @return true quando a classe actual e VERMELHO.
        */
        bool estaEmPerigo() const;

    protected:
        /**
        * @brief Graus que o motor ganha por minuto em RUN.
        * @return Ritmo da classe base. Cada tipo concreto substitui o valor.
        */
        virtual double RitmoAquecimento() const;

    public:
        /**
        * @brief Cria um motor parado, na temperatura zero.
        * @param id Identificador.
        * @param tipo Etiqueta do tipo.
        * @param marca Fabricante.
        * @param consumoHora Consumo por hora.
        * @param temperaturaAviso Limiar da zona amarela.
        * @param temperaturaParagem Limiar da zona vermelha.
        * @param probabilidadeAvaria Probabilidade de avaria por hora.
        * @param posX Coordenada X.
        * @param posY Coordenada Y.
        */
        Motor(int id, const string& tipo, const string& marca, double consumoHora,
              double temperaturaAviso, double temperaturaParagem, double probabilidadeAvaria,
              int posX, int posY);

        /** @brief Destrutor virtual. */
        virtual ~Motor();

        /** @brief Passa de START para RUN, se a temperatura nao estiver vermelha. */
        virtual void RUN();

        /** @brief Passa a START, excepto se estiver avariado ou vermelho. */
        virtual void START();

        /** @brief Sai de AVARIADO para START, se a temperatura nao estiver vermelha. */
        virtual void RESTART();

        /** @brief Passa a STOP. Um motor avariado nao muda de estado. */
        virtual void STOP();

        /**
        * @brief Classifica a temperatura e, se estiver VERMELHO, poe o motor em STOP.
        * @return Classe actual: VERDE, AMARELO ou VERMELHO.
        */
        virtual CLASSE_TEMPERATURA ESTOU_QUENTE();

        /**
        * @brief Aquece um minuto de RUN com o ritmo do tipo concreto.
        */
        void AplicarAquecimento();

        /**
        * @brief Classifica a temperatura sem alterar o estado.
        * @return Classe actual: VERDE, AMARELO ou VERMELHO.
        */
        CLASSE_TEMPERATURA GetClasseTemperatura() const;

        /** @brief Devolve o estado actual. */
        ESTADO_MOTOR GetEstado() const;

        /** @brief Devolve a etiqueta do tipo. */
        virtual string GetTipo() const;

        /** @brief Devolve o consumo por hora. */
        double GetConsumoHora() const;

        /** @brief Devolve o consumo acumulado no dia. */
        double GetConsumoDia() const;

        /** @brief Devolve a temperatura de aviso. */
        double GetTemperaturaAviso() const;

        /** @brief Devolve a temperatura de paragem. */
        double GetTemperaturaParagem() const;

        /** @brief Devolve a temperatura actual. */
        double GetTemperaturaAtual() const;

        /** @brief Devolve os minutos de trabalho acumulados. */
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

/**
 * @brief Motor eletrico. Aquece mais devagar do que a base.
 */
class MotorEletrico : public Motor
{
    public:
        /**
        * @brief Cria um motor eletrico. O tipo fica MELETRICO.
        * @param id Identificador.
        * @param marca Fabricante.
        * @param consumoHora Consumo por hora.
        * @param temperaturaAviso Limiar amarelo, vindo do XML.
        * @param temperaturaParagem Limiar vermelho, vindo do XML.
        * @param probabilidadeAvaria Probabilidade de avaria por hora.
        * @param posX Coordenada X.
        * @param posY Coordenada Y.
        */
        MotorEletrico(int id, const string& marca, double consumoHora, double temperaturaAviso,
                      double temperaturaParagem, double probabilidadeAvaria, int posX, int posY);

        /** @brief Destrutor. */
        virtual ~MotorEletrico();

        /** @brief Reimplementa RUN e usa a transicao da classe base. */
        virtual void RUN();

        /** @brief Reimplementa START e usa a transicao da classe base. */
        virtual void START();

        /** @brief Reimplementa RESTART e usa a transicao da classe base. */
        virtual void RESTART();

        /** @brief Reimplementa STOP e usa a transicao da classe base. */
        virtual void STOP();

        /** @brief Reimplementa ESTOU_QUENTE e usa a classificacao da classe base. */
        virtual CLASSE_TEMPERATURA ESTOU_QUENTE();

        /** @brief Devolve "MELETRICO". */
        virtual string GetTipo() const;

    protected:
        /** @brief Aquece 0,8 graus por minuto em RUN. */
        virtual double RitmoAquecimento() const;
};

/**
 * @brief Motor de combustao. Aquece mais depressa do que a base.
 */
class MotorCombustao : public Motor
{
    public:
        /**
        * @brief Cria um motor de combustao. O tipo fica MCOMBUSTAO.
        * @param id Identificador.
        * @param marca Fabricante.
        * @param consumoHora Consumo por hora.
        * @param temperaturaAviso Limiar amarelo, vindo do XML.
        * @param temperaturaParagem Limiar vermelho, vindo do XML.
        * @param probabilidadeAvaria Probabilidade de avaria por hora.
        * @param posX Coordenada X.
        * @param posY Coordenada Y.
        */
        MotorCombustao(int id, const string& marca, double consumoHora, double temperaturaAviso,
                       double temperaturaParagem, double probabilidadeAvaria, int posX, int posY);

        /** @brief Destrutor. */
        virtual ~MotorCombustao();

        /** @brief Reimplementa RUN e usa a transicao da classe base. */
        virtual void RUN();

        /** @brief Reimplementa START e usa a transicao da classe base. */
        virtual void START();

        /** @brief Reimplementa RESTART e usa a transicao da classe base. */
        virtual void RESTART();

        /** @brief Reimplementa STOP e usa a transicao da classe base. */
        virtual void STOP();

        /** @brief Reimplementa ESTOU_QUENTE e usa a classificacao da classe base. */
        virtual CLASSE_TEMPERATURA ESTOU_QUENTE();

        /** @brief Devolve "MCOMBUSTAO". */
        virtual string GetTipo() const;

    protected:
        /** @brief Aquece 2,2 graus por minuto em RUN. */
        virtual double RitmoAquecimento() const;
};

/**
 * @brief Motor de inducao. O aquecimento fica entre o eletrico e a combustao.
 */
class MotorInducao : public Motor
{
    public:
        /**
        * @brief Cria um motor de inducao. O tipo fica MINDUCAO.
        * @param id Identificador.
        * @param marca Fabricante.
        * @param consumoHora Consumo por hora.
        * @param temperaturaAviso Limiar amarelo, vindo do XML.
        * @param temperaturaParagem Limiar vermelho, vindo do XML.
        * @param probabilidadeAvaria Probabilidade de avaria por hora.
        * @param posX Coordenada X.
        * @param posY Coordenada Y.
        */
        MotorInducao(int id, const string& marca, double consumoHora, double temperaturaAviso,
                     double temperaturaParagem, double probabilidadeAvaria, int posX, int posY);

        /** @brief Destrutor. */
        virtual ~MotorInducao();

        /** @brief Reimplementa RUN e usa a transicao da classe base. */
        virtual void RUN();

        /** @brief Reimplementa START e usa a transicao da classe base. */
        virtual void START();

        /** @brief Reimplementa RESTART e usa a transicao da classe base. */
        virtual void RESTART();

        /** @brief Reimplementa STOP e usa a transicao da classe base. */
        virtual void STOP();

        /** @brief Reimplementa ESTOU_QUENTE e usa a classificacao da classe base. */
        virtual CLASSE_TEMPERATURA ESTOU_QUENTE();

        /** @brief Devolve "MINDUCAO". */
        virtual string GetTipo() const;

    protected:
        /** @brief Aquece 1,2 graus por minuto em RUN. */
        virtual double RitmoAquecimento() const;
};

#endif
