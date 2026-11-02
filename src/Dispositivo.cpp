#include "Dispositivo.h"

using namespace std;

Dispositivo::Dispositivo(int id, const string& marca, double probabilidadeAvaria, int posX, int posY)
    : id(id), marca(marca), probabilidadeAvaria(0.0), x(0), y(0), totalAvarias(0), avariado(false)
{
    this->probabilidadeAvaria = limitarProbabilidade(probabilidadeAvaria);
    SetPosicao(posX, posY);
}

Dispositivo::~Dispositivo()
{
}

int Dispositivo::GetId() const
{
    return id;
}

string Dispositivo::GetMarca() const
{
    return marca;
}

double Dispositivo::GetProbabilidadeAvaria() const
{
    return probabilidadeAvaria;
}

int Dispositivo::GetX() const
{
    return x;
}

int Dispositivo::GetY() const
{
    return y;
}

int Dispositivo::GetTotalAvarias() const
{
    return totalAvarias;
}

bool Dispositivo::SetPosicao(int posX, int posY)
{
    if (posX <= 0 || posY <= 0)
    {
        return false;
    }

    x = posX;
    y = posY;
    return true;
}

bool Dispositivo::EstaAvariado() const
{
    return avariado;
}

void Dispositivo::RegistarAvaria()
{
    if (avariado)
    {
        return;
    }

    avariado = true;
    totalAvarias = totalAvarias + 1;
}

void Dispositivo::RepararAvaria()
{
    avariado = false;
}

double Dispositivo::limitarProbabilidade(double probabilidade) const
{
    if (probabilidade < 0.0)
    {
        return 0.0;
    }

    if (probabilidade > 1.0)
    {
        return 1.0;
    }

    return probabilidade;
}
