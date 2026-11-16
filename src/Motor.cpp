/**
 * @file Motor.cpp
 * @brief Implementacao da classe base Motor e dos motores concretos.
 */

#include "Motor.h"

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

    if (classe == VERMELHO)
    {
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

double Motor::RitmoAquecimento() const
{
    return 1.5;
}

void Motor::AplicarAquecimento()
{
    Aquecer(RitmoAquecimento());
}

MotorEletrico::MotorEletrico(int id, const string& marca, double consumoHora, double temperaturaAviso,
                             double temperaturaParagem, double probabilidadeAvaria, int posX, int posY)
    : Motor(id, "MELETRICO", marca, consumoHora, temperaturaAviso, temperaturaParagem, probabilidadeAvaria, posX, posY)
{
}

MotorEletrico::~MotorEletrico()
{
}

void MotorEletrico::RUN()
{
    Motor::RUN();
}

void MotorEletrico::START()
{
    Motor::START();
}

void MotorEletrico::RESTART()
{
    Motor::RESTART();
}

void MotorEletrico::STOP()
{
    Motor::STOP();
}

CLASSE_TEMPERATURA MotorEletrico::ESTOU_QUENTE()
{
    return Motor::ESTOU_QUENTE();
}

string MotorEletrico::GetTipo() const
{
    return "MELETRICO";
}

double MotorEletrico::RitmoAquecimento() const
{
    return 0.8;
}

MotorCombustao::MotorCombustao(int id, const string& marca, double consumoHora, double temperaturaAviso,
                               double temperaturaParagem, double probabilidadeAvaria, int posX, int posY)
    : Motor(id, "MCOMBUSTAO", marca, consumoHora, temperaturaAviso, temperaturaParagem, probabilidadeAvaria, posX, posY)
{
}

MotorCombustao::~MotorCombustao()
{
}

void MotorCombustao::RUN()
{
    Motor::RUN();
}

void MotorCombustao::START()
{
    Motor::START();
}

void MotorCombustao::RESTART()
{
    Motor::RESTART();
}

void MotorCombustao::STOP()
{
    Motor::STOP();
}

CLASSE_TEMPERATURA MotorCombustao::ESTOU_QUENTE()
{
    return Motor::ESTOU_QUENTE();
}

string MotorCombustao::GetTipo() const
{
    return "MCOMBUSTAO";
}

double MotorCombustao::RitmoAquecimento() const
{
    return 2.2;
}

MotorInducao::MotorInducao(int id, const string& marca, double consumoHora, double temperaturaAviso,
                           double temperaturaParagem, double probabilidadeAvaria, int posX, int posY)
    : Motor(id, "MINDUCAO", marca, consumoHora, temperaturaAviso, temperaturaParagem, probabilidadeAvaria, posX, posY)
{
}

MotorInducao::~MotorInducao()
{
}

void MotorInducao::RUN()
{
    Motor::RUN();
}

void MotorInducao::START()
{
    Motor::START();
}

void MotorInducao::RESTART()
{
    Motor::RESTART();
}

void MotorInducao::STOP()
{
    Motor::STOP();
}

CLASSE_TEMPERATURA MotorInducao::ESTOU_QUENTE()
{
    return Motor::ESTOU_QUENTE();
}

string MotorInducao::GetTipo() const
{
    return "MINDUCAO";
}

double MotorInducao::RitmoAquecimento() const
{
    return 1.2;
}
