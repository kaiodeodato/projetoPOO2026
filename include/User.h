#ifndef USER_H
#define USER_H

#include <string>

using namespace std;

/**
 * @file User.h
 * @brief Utilizadores da fabrica e os respectivos niveis de acesso.
 *
 * USER (ID, NOME), com tres perfis:
 * - ADMIN: ADD, REMOVER, LISTAR, RUN, MANUTENCAO
 * - NORMAL: ADD, LISTAR, RUN, MANUTENCAO
 * - VISITANTE: LISTAR
 */

/**
 * @brief Utilizador generico. Os perfis concretizam as permissoes.
 */
class User
{
    protected:
        int id; ///< Identificador. Tem de ser positivo para entrar na fabrica.
        string nome; ///< Nome apresentado nas listagens.

    public:
        /**
        * @brief Cria um utilizador.
        * @param id Identificador.
        * @param nome Nome.
        */
        User(int id, const string& nome);

        /** @brief Destrutor virtual. */
        virtual ~User();

        /** @brief Devolve o identificador. */
        int GetId() const;

        /** @brief Devolve o nome. */
        string GetNome() const;

        /** @brief Nome do perfil: ADMIN, NORMAL ou VISITANTE. */
        virtual string GetTipo() const = 0;

        /**
        * @brief Indica se pode adicionar utilizadores e motores.
        * A base recusa e avisa. Cada perfil autorizado redefine o metodo.
        */
        virtual bool PodeAdicionar() const;

        /**
        * @brief Indica se pode remover utilizadores, motores e sensores.
        * A base recusa e avisa. Cada perfil autorizado redefine o metodo.
        */
        virtual bool PodeRemover() const;

        /**
        * @brief Indica se pode listar o estado da fabrica.
        * A base recusa e avisa. Cada perfil autorizado redefine o metodo.
        */
        virtual bool PodeListar() const;

        /**
        * @brief Indica se pode ligar e desligar motores.
        * A base recusa e avisa. Cada perfil autorizado redefine o metodo.
        */
        virtual bool PodeRun() const;

        /**
        * @brief Indica se pode fazer a manutencao dos dispositivos.
        * A base recusa e avisa. Cada perfil autorizado redefine o metodo.
        */
        virtual bool PodeManutencao() const;
};

/**
 * @brief Administrador. Pode adicionar, remover, listar, operar e fazer manutencao.
 */
class Admin : public User
{
    public:
        /**
        * @brief Cria um administrador.
        * @param id Identificador.
        * @param nome Nome.
        */
        Admin(int id, const string& nome);

        /** @brief Destrutor. */
        virtual ~Admin();

        /** @brief Devolve "ADMIN". */
        virtual string GetTipo() const;

        /** @brief O administrador pode adicionar. */
        virtual bool PodeAdicionar() const;

        /** @brief O administrador pode remover. */
        virtual bool PodeRemover() const;

        /** @brief O administrador pode listar. */
        virtual bool PodeListar() const;

        /** @brief O administrador pode operar motores. */
        virtual bool PodeRun() const;

        /** @brief O administrador pode fazer manutencao. */
        virtual bool PodeManutencao() const;
};

/**
 * @brief Utilizador normal. Pode adicionar, listar, operar e fazer manutencao.
 */
class Normal : public User
{
    public:
        /**
        * @brief Cria um utilizador normal.
        * @param id Identificador.
        * @param nome Nome.
        */
        Normal(int id, const string& nome);

        /** @brief Destrutor. */
        virtual ~Normal();

        /** @brief Devolve "NORMAL". */
        virtual string GetTipo() const;

        /** @brief O utilizador normal pode adicionar. */
        virtual bool PodeAdicionar() const;

        /** @brief O utilizador normal pode listar. */
        virtual bool PodeListar() const;

        /** @brief O utilizador normal pode operar motores. */
        virtual bool PodeRun() const;

        /** @brief O utilizador normal pode fazer manutencao. */
        virtual bool PodeManutencao() const;
};

/**
 * @brief Visitante. So pode listar.
 */
class Visitante : public User
{
    public:
        /**
        * @brief Cria um visitante.
        * @param id Identificador.
        * @param nome Nome.
        */
        Visitante(int id, const string& nome);

        /** @brief Destrutor. */
        virtual ~Visitante();

        /** @brief Devolve "VISITANTE". */
        virtual string GetTipo() const;

        /** @brief O visitante pode listar. As restantes permissoes ficam na base. */
        virtual bool PodeListar() const;
};

#endif
