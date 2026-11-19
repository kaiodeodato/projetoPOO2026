/**
 * @file Sensor.cpp
 * @brief Implementacao da classe base Sensor e dos sensores concretos.
 */

#include "Sensor.h"

#include <cstdlib>

using namespace std;

Sensor::Sensor(int id, const string& marca, double valor, double probabilidadeAvaria, int posX, int posY)
    : Dispositivo(id, marca, probabilidadeAvaria, posX, posY), valor(valor), avisoAnterior(false), probabilidadeUm(0.0)
{
}

Sensor::~Sensor()
{
}

double Sensor::GetValor() const
{
    return valor;
}

void Sensor::SetValor(double novoValor)
{
    valor = novoValor;
}

int Sensor::GetRaioVizinhanca() const
{
    return 0;
}

double Sensor::GetLimiar() const
{
    return 0.0;
}

double Sensor::GetProbabilidadeUm() const
{
    return probabilidadeUm;
}

void Sensor::SetProbabilidadeUm(double probabilidade)
{
    if (probabilidade < 0.0)
    {
        probabilidadeUm = 0.0;
        return;
    }

    if (probabilidade > 1.0)
    {
        probabilidadeUm = 1.0;
        return;
    }

    probabilidadeUm = probabilidade;
}

bool Sensor::Disparou()
{
    bool agora = EmAviso();
    bool novo = agora && !avisoAnterior;
    avisoAnterior = agora;
    return novo;
}

void Sensor::Actualizar(bool, int, int, int, double)
{
}

void Sensor::Repor()
{
    DefinirValorSeguro();
    RepararAvaria();
    avisoAnterior = false;
}

SensorLuz::SensorLuz(int id, const string& marca, double valor, double probabilidadeAvaria,
                     int posX, int posY, double limiar)
    : Sensor(id, marca, valor, probabilidadeAvaria, posX, posY), limiar(limiar)
{
}

SensorLuz::~SensorLuz()
{
}

double SensorLuz::GetLimiar() const
{
    return limiar;
}

string SensorLuz::GetTipo() const
{
    return "LUZ";
}

bool SensorLuz::EmAviso() const
{
    return valor < limiar;
}

SensorHumidade::SensorHumidade(int id, const string& marca, double valor, double probabilidadeAvaria,
                               int posX, int posY, double limiar, int raioVizinhanca)
    : Sensor(id, marca, valor, probabilidadeAvaria, posX, posY), limiar(limiar), raioVizinhanca(0)
{
    if (raioVizinhanca > 0)
    {
        this->raioVizinhanca = raioVizinhanca;
    }
}

SensorHumidade::~SensorHumidade()
{
}

double SensorHumidade::GetLimiar() const
{
    return limiar;
}

int SensorHumidade::GetRaioVizinhanca() const
{
    return raioVizinhanca;
}

string SensorHumidade::GetTipo() const
{
    return "HUMIDADE";
}

bool SensorHumidade::EmAviso() const
{
    return valor >= limiar;
}

SensorFumo::SensorFumo(int id, const string& marca, double valor, double probabilidadeAvaria,
                       int posX, int posY, double limiar)
    : Sensor(id, marca, valor, probabilidadeAvaria, posX, posY), limiar(limiar)
{
}

SensorFumo::~SensorFumo()
{
}

double SensorFumo::GetLimiar() const
{
    return limiar;
}

string SensorFumo::GetTipo() const
{
    return "SFOGO";
}

bool SensorFumo::EmAviso() const
{
    return valor >= limiar;
}

SensorMissel::SensorMissel(int id, const string& marca, double valor, double probabilidadeAvaria,
                           int posX, int posY, double limiar)
    : Sensor(id, marca, valor, probabilidadeAvaria, posX, posY), limiar(limiar)
{
}

SensorMissel::~SensorMissel()
{
}

double SensorMissel::GetLimiar() const
{
    return limiar;
}

string SensorMissel::GetTipo() const
{
    return "SMISSEL";
}

bool SensorMissel::EmAviso() const
{
    return valor >= limiar;
}

void SensorLuz::Actualizar(bool, int horaAtual, int horaAbertura, int horaEncerramento, double)
{
    if (horaAtual == horaAbertura)
    {
        valor = limiar + 40.0;
    }

    if (horaEncerramento > 0 && horaAtual >= horaEncerramento - 1 && horaAtual < horaEncerramento)
    {
        valor = limiar - 1.0;
    }
}

void SensorLuz::DefinirValorSeguro()
{
    valor = limiar + 40.0;
}

void SensorHumidade::Actualizar(bool motoresEmMarcha, int, int, int, double fracaoHora)
{
    if (motoresEmMarcha && valor < 100.0)
    {
        valor = valor + 2.0 * fracaoHora;
    }
}

void SensorHumidade::DefinirValorSeguro()
{
    if (limiar > 1.0)
    {
        valor = limiar - 1.0;
    }
    else
    {
        valor = 0.0;
    }
}


void SensorFumo::Actualizar(bool motoresEmMarcha, int, int, int, double fracaoHora)
{
    if (EstaAvariado())
    {
        return;
    }

    if (probabilidadeUm > 0.0 && valor < limiar)
    {
        double sorteio = (rand() % 10000) / 10000.0;

        if (sorteio < probabilidadeUm * fracaoHora)
        {
            valor = limiar;
            return;
        }
    }

    if (motoresEmMarcha)
    {
        if (valor < limiar + 5.0)
        {
            valor += 0.8 * fracaoHora;

            if (valor > limiar + 5.0)
            {
                valor = limiar + 5.0;
            }
        }
    }
    else
    {
        valor -= 1.0 * fracaoHora;

        if (valor < 0.0)
        {
            valor = 0.0;
        }
    }
}


void SensorFumo::DefinirValorSeguro()
{
    valor = 0.0;
}

void SensorMissel::Actualizar(bool, int, int, int, double fracaoHora)
{
    if (valor >= limiar || EstaAvariado())
    {
        return;
    }

    double sorteio = (rand() % 10000) / 10000.0;
    double limite = probabilidadeUm * fracaoHora;
    if (sorteio < limite)
    {
        valor = limiar;
    }
}

void SensorMissel::DefinirValorSeguro()
{
    valor = 0.0;
}

SensorTemperatura::SensorTemperatura(int id, const string& marca, double valor, double probabilidadeAvaria,
                                     int posX, int posY, double limiar)
    : Sensor(id, marca, valor, probabilidadeAvaria, posX, posY), limiar(limiar)
{
}

SensorTemperatura::~SensorTemperatura()
{
}

double SensorTemperatura::GetLimiar() const
{
    return limiar;
}

string SensorTemperatura::GetTipo() const
{
    return "STEMPERATURA";
}

bool SensorTemperatura::EmAviso() const
{
    return valor >= limiar;
}

void SensorTemperatura::DefinirValorSeguro()
{
    valor = 0.0;
}
