#include "User.h"
#include <iostream>

User::User(int id, const string& nome)
{
    this->id = id;
    this->nome = nome;
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

bool User::PodeAdicionar() const
{
    cout << "Nao tem permissao para adicionar." << endl;
    return false;
}

bool User::PodeRemover() const
{
    cout << "Nao tem permissao para remover." << endl;
    return false;
}

bool User::PodeListar() const
{
    cout << "Nao tem permissao para listar." << endl;
    return false;
}

bool User::PodeRun() const
{
    cout << "Nao tem permissao para executar motores." << endl;
    return false;
}

bool User::PodeManutencao() const
{
    cout << "Nao tem permissao para realizar manutencao." << endl;
    return false;
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

bool Visitante::PodeListar() const
{
    return true;
}
