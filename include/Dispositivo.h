#ifndef DISPOSITIVO_H
#define DISPOSITIVO_H

#include <string>

using namespace std;

/**
 * @file Dispositivo.h
 * @brief Base de sensores e motores.
 *
 * Tanto o sensor como o motor tem identificador, marca,
 * probabilidade de avaria e uma posicao no campo da fabrica.
 * A posicao so e aceite quando X e Y sao nao-negativos (>= 0).
 */
class Dispositivo
{
    protected:
        int id; ///< Identificador unico dentro do seu grupo (motor ou sensor).
        string marca; ///< Fabricante do dispositivo.
        double probabilidadeAvaria; ///< Probabilidade de avaria por hora, entre 0 e 1.
        int x; ///< Coordenada X no campo. Valores negativos sao invalidos.
        int y; ///< Coordenada Y no campo. Valores negativos sao invalidos.
        int totalAvarias; ///< Numero de avarias registadas.
        bool avariado; ///< true enquanto a ultima avaria nao foi reparada.

        /**
        * @brief Garante que a probabilidade fica entre 0 e 1.
        * @param probabilidade Valor lido ou recebido.
        * @return Probabilidade limitada ao intervalo [0, 1].
        */
        double limitarProbabilidade(double probabilidade) const;

    public:
        /**
        * @brief Cria um dispositivo.
        * @param id Identificador. Deve ser positivo para entrar na fabrica.
        * @param marca Fabricante.
        * @param probabilidadeAvaria Probabilidade por hora. Valores fora de [0, 1] sao limitados.
        * @param posX Coordenada X. So e guardada se for nao-negativa.
        * @param posY Coordenada Y. So e guardada se for nao-negativa.
        */
        Dispositivo(int id, const string& marca, double probabilidadeAvaria, int posX, int posY);

        /** @brief Destrutor virtual, para apagar sensores e motores pela base. */
        virtual ~Dispositivo();

        /** @brief Devolve o identificador. */
        int GetId() const;

        /** @brief Devolve a marca. */
        string GetMarca() const;

        /** @brief Devolve a probabilidade de avaria por hora. */
        double GetProbabilidadeAvaria() const;

        /** @brief Devolve a coordenada X. */
        int GetX() const;

        /** @brief Devolve a coordenada Y. */
        int GetY() const;

        /** @brief Devolve o numero de avarias ja contadas. */
        int GetTotalAvarias() const;

        /**
        * @brief Indica se o dispositivo ja avariou e ainda nao foi reparado.
        * @return true depois de RegistarAvaria, ate RepararAvaria.
        */
        bool EstaAvariado() const;

        /**
        * @brief Altera a posicao.
        * @param posX Coordenada X. Tem de ser nao-negativa.
        * @param posY Coordenada Y. Tem de ser nao-negativa.
        * @return false se X ou Y forem negativos. Nesse caso a posicao nao muda.
        */
        bool SetPosicao(int posX, int posY);

        /**
        * @brief Conta uma avaria.
        *
        * So incrementa uma vez. Chamadas seguintes nao somam
        * enquanto o dispositivo nao for reparado.
        */
        void RegistarAvaria();

        /**
        * @brief Limpa o estado de avaria para a contagem poder voltar a subir.
        */
        void RepararAvaria();
};

#endif
