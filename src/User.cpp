#include "User.h"

using namespace std;

User::User(int id, const string& nome)
    : id(id), nome(nome)
{
}

User::~User()
{
}

int User::GetId() const
{
    return id;
}

string User::GetNome() const
{
    return nome;
}

Admin::Admin(int id, const string& nome)
    : User(id, nome)
{
}

Admin::~Admin()
{
}

string Admin::GetTipo() const
{
    return "ADMIN";
}

bool Admin::PodeAdicionar() const
{
    return true;
}

bool Admin::PodeRemover() const
{
    return true;
}

bool Admin::PodeListar() const
{
    return true;
}

bool Admin::PodeRun() const
{
    return true;
}

bool Admin::PodeManutencao() const
{
    return true;
}

Normal::Normal(int id, const string& nome)
    : User(id, nome)
{
}

Normal::~Normal()
{
}

string Normal::GetTipo() const
{
    return "NORMAL";
}

bool Normal::PodeAdicionar() const
{
    return true;
}

bool Normal::PodeRemover() const
{
    return false;
}

bool Normal::PodeListar() const
{
    return true;
}

bool Normal::PodeRun() const
{
    return true;
}

bool Normal::PodeManutencao() const
{
    return true;
}

Visitante::Visitante(int id, const string& nome)
    : User(id, nome)
{
}

Visitante::~Visitante()
{
}

string Visitante::GetTipo() const
{
    return "VISITANTE";
}

bool Visitante::PodeAdicionar() const
{
    return false;
}

bool Visitante::PodeRemover() const
{
    return false;
}

bool Visitante::PodeListar() const
{
    return true;
}

bool Visitante::PodeRun() const
{
    return false;
}

bool Visitante::PodeManutencao() const
{
    return false;
}
