#ifndef TIPOS_H
#define TIPOS_H

using namespace std;

/**
 * @file Tipos.h
 * @brief Enumeracoes e intervalos partilhados pela fabrica.
 *
 * STOP, START, RUN e AVARIADO sao os estados dos motores.
 * VERDE, AMARELO e VERMELHO sao as classes de temperatura.
 */

/**
 * @brief Estado de funcionamento de um motor.
 */
enum ESTADO_MOTOR
{
    STOP, ///< Parado.
    START, ///< A arrancar. O passo seguinte da simulacao passa-o a RUN, se a fabrica estiver aberta.
    RUN, ///< Em funcionamento: aquece, consome e acumula tempo de trabalho.
    AVARIADO ///< Avariado. So volta a STOP com manutencao ou RESTART, se nao estiver vermelho.
};

/**
 * @brief Classe de temperatura de um motor.
 */
enum CLASSE_TEMPERATURA
{
    VERDE, ///< Abaixo da temperatura de aviso.
    AMARELO, ///< A aquecer. O gestor e avisado.
    VERMELHO ///< Temperatura de paragem. O gestor e avisado e o motor fica em STOP.
};

/**
 * @brief Intervalo fechado de temperatura, lido do XML.
 *
 * O minimo e o maximo vem do ficheiro de configuracao.
 * A manutencao usa o intervalo VERDE para repor a temperatura dos motores.
 */
struct IntervaloTemperatura
{
        double minimo; ///< Limite inferior, inclusive.
        double maximo; ///< Limite superior, inclusive.
};

#endif
