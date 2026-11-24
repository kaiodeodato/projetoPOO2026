#ifndef USER_H
#define USER_H

#include <string>

using namespace std;

class User
{
    protected:
        int id;
        string nome;

    public:
        User(int id, const string& nome);

        virtual ~User();

        int GetId() const;

        string GetNome() const;

        virtual string GetTipo() const = 0;

        virtual bool PodeAdicionar() const;

        virtual bool PodeRemover() const;

        virtual bool PodeListar() const;

        virtual bool PodeRun() const;

        virtual bool PodeManutencao() const;
};

class Admin : public User
{
    public:
        Admin(int id, const string& nome);

        virtual ~Admin();

        virtual string GetTipo() const;

        virtual bool PodeAdicionar() const;

        virtual bool PodeRemover() const;

        virtual bool PodeListar() const;

        virtual bool PodeRun() const;

        virtual bool PodeManutencao() const;
};

class Normal : public User
{
    public:
        Normal(int id, const string& nome);

        virtual ~Normal();

        virtual string GetTipo() const;

        virtual bool PodeAdicionar() const;

        virtual bool PodeListar() const;

        virtual bool PodeRun() const;

        virtual bool PodeManutencao() const;
};

class Visitante : public User
{
    public:
        Visitante(int id, const string& nome);

        virtual ~Visitante();

        virtual string GetTipo() const;

        virtual bool PodeListar() const;
};

#endif