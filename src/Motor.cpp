#include "Motor.h"

#include <iostream>

using namespace std;

Motor::Motor(int id, const string& tipo, const string& marca, double consumoHora,
             double temperaturaAviso, double temperaturaParagem, double probabilidadeAvaria,
             int posX, int posY)
    : Dispositivo(id, marca, probabilidadeAvaria, posX, posY),
      tipo(tipo),
      consumoHora(consumoHora),
      temperaturaAviso(temperaturaAviso),
      temperaturaParagem(temperaturaParagem),
      temperaturaAtual(0.0),
      consumoDia(0.0),
      tempoTrabalho(0),
      estado(::STOP)
{
}

Motor::~Motor()
{
}

void Motor::RUN()
{
    if (estaEmPerigo())
    {
        estado = ::STOP;
        return;
    }

    if (estado != ::START)
    {
        return;
    }

    estado = ::RUN;
}

void Motor::START()
{
    if (estaEmPerigo())
    {
        estado = ::STOP;
        return;
    }

    if (estado == AVARIADO)
    {
        return;
    }

    estado = ::START;
}

void Motor::RESTART()
{
    if (estado == AVARIADO)
    {
        return;
    }

    if (estaEmPerigo())
    {
        estado = ::STOP;
        return;
    }

    estado = ::START;
}

void Motor::STOP()
{
    if (estado != AVARIADO)
    {
        estado = ::STOP;
    }
}

CLASSE_TEMPERATURA Motor::ESTOU_QUENTE()
{
    CLASSE_TEMPERATURA classe = GetClasseTemperatura();

    if (classe == AMARELO)
    {
        cout << "[AVISO GESTOR] Motor " << GetId() << " (" << GetMarca() << ") esta em AMARELO ("
             << temperaturaAtual << " graus)." << endl;
    }
    else if (classe == VERMELHO)
    {
        cout << "[AVISO GESTOR] Motor " << GetId() << " (" << GetMarca() << ") esta em VERMELHO ("
             << temperaturaAtual << " graus) - a parar!" << endl;
        estado = ::STOP;
    }

    return classe;
}

CLASSE_TEMPERATURA Motor::GetClasseTemperatura() const
{
    if (temperaturaAtual >= temperaturaParagem)
    {
        return VERMELHO;
    }

    if (temperaturaAtual >= temperaturaAviso)
    {
        return AMARELO;
    }

    return VERDE;
}

bool Motor::estaEmPerigo() const
{
    return GetClasseTemperatura() == VERMELHO;
}

ESTADO_MOTOR Motor::GetEstado() const
{
    return estado;
}

string Motor::GetTipo() const
{
    return tipo;
}

double Motor::GetConsumoHora() const
{
    return consumoHora;
}

double Motor::GetConsumoDia() const
{
    return consumoDia;
}

double Motor::GetTemperaturaAviso() const
{
    return temperaturaAviso;
}

double Motor::GetTemperaturaParagem() const
{
    return temperaturaParagem;
}

double Motor::GetTemperaturaAtual() const
{
    return temperaturaAtual;
}

int Motor::GetTempoTrabalho() const
{
    return tempoTrabalho;
}

void Motor::SetTemperaturaAtual(double temperatura)
{
    temperaturaAtual = temperatura;
}

void Motor::Aquecer(double graus)
{
    if (graus > 0.0)
    {
        temperaturaAtual = temperaturaAtual + graus;
    }
}

void Motor::Arrefecer(double graus)
{
    if (graus <= 0.0)
    {
        return;
    }

    temperaturaAtual = temperaturaAtual - graus;
    if (temperaturaAtual < 0.0)
    {
        temperaturaAtual = 0.0;
    }
}

bool Motor::Avariar()
{
    if (estado == AVARIADO)
    {
        return false;
    }

    estado = AVARIADO;
    RegistarAvaria();
    return true;
}

void Motor::Reparar()
{
    RepararAvaria();
    if (estado == AVARIADO)
    {
        estado = ::STOP;
    }
}

void Motor::RegistarTrabalho(int minutos)
{
    if (minutos <= 0)
    {
        return;
    }

    tempoTrabalho = tempoTrabalho + minutos;
    consumoDia = consumoDia + consumoHora * (minutos / 60.0);
}

void Motor::ReiniciarConsumoDia()
{
    consumoDia = 0.0;
}

IntervaloTemperatura Motor::GetIntervaloVerde() const
{
    IntervaloTemperatura iv;
    iv.minimo = 0.0;
    iv.maximo = 0.0;
    return iv;
}

IntervaloTemperatura Motor::GetIntervaloAmarelo() const
{
    IntervaloTemperatura iv;
    iv.minimo = 0.0;
    iv.maximo = 0.0;
    return iv;
}

IntervaloTemperatura Motor::GetIntervaloVermelho() const
{
    IntervaloTemperatura iv;
    iv.minimo = 0.0;
    iv.maximo = 0.0;
    return iv;
}
