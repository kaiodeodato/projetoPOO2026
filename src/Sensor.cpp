#include "Sensor.h"

#include <cstdlib>

using namespace std;

Sensor::Sensor(int id, const string& marca, double valor, double probabilidadeAvaria, int posX, int posY)
    : Dispositivo(id, marca, probabilidadeAvaria, posX, posY), valor(valor), avisoAnterior(false)
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
    return "FUMO";
}

bool SensorFumo::EmAviso() const
{
    return valor >= limiar;
}

SensorMissel::SensorMissel(int id, const string& marca, double valor, double probabilidadeAvaria,
                           int posX, int posY)
    : Sensor(id, marca, valor, probabilidadeAvaria, posX, posY)
{
}

SensorMissel::~SensorMissel()
{
}

string SensorMissel::GetTipo() const
{
    return "MISSEL";
}

bool SensorMissel::EmAviso() const
{
    return valor >= 1.0;
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

void SensorHumidade::Actualizar(bool motoresEmMarcha, int, int, int, double)
{
    if (motoresEmMarcha && valor < 100.0)
    {
        valor = valor + 2.0;
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

void SensorFumo::Actualizar(bool motoresEmMarcha, int, int, int, double)
{
    if (motoresEmMarcha && valor < limiar + 5.0)
    {
        valor = valor + 0.8;
    }
}

void SensorFumo::DefinirValorSeguro()
{
    valor = 0.0;
}

void SensorMissel::Actualizar(bool, int, int, int, double fracaoHora)
{
    if (valor >= 1.0 || EstaAvariado())
    {
        return;
    }

    double sorteio = (rand() % 10000) / 10000.0;
    double limite = GetProbabilidadeAvaria() * fracaoHora;
    if (sorteio < limite)
    {
        valor = 1.0;
    }
}

void SensorMissel::DefinirValorSeguro()
{
    valor = 0.0;
}
