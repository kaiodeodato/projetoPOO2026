
#ifndef USER_H
#define USER_H

#include <string>
using namespace std;

/**
 * @file User.h
 * @brief Declaracao da classe User e dos seus tipos de utilizador.
 */

/**
 * @brief Classe base abstrata dos utilizadores da fabrica.
 *
 * Define a identificacao do utilizador e as permissoes
 * para executar as operacoes da fabrica.
 */
class User
{
protected:
    int id;       ///< Identificador do utilizador.
    string nome;  ///< Nome do utilizador.

public:
    /**
     * @brief Construtor da classe User.
     * @param id Identificador do utilizador.
     * @param nome Nome do utilizador.
     */
    User(int id, const string& nome);

    /**
     * @brief Destrutor virtual.
     */
    virtual ~User();

    /**
     * @brief Devolve o identificador do utilizador.
     * @return Identificador.
     */
    int GetId() const;

    /**
     * @brief Devolve o nome do utilizador.
     * @return Nome.
     */
    string GetNome() const;

    /**
     * @brief Devolve o tipo de utilizador.
     * @return Tipo de utilizador.
     */
    virtual string GetTipo() const = 0;

    /**
     * @brief Verifica se o utilizador pode adicionar.
     * @return true se tiver permissao, false caso contrario.
     */
    virtual bool PodeAdicionar() const;

    /**
     * @brief Verifica se o utilizador pode remover.
     * @return true se tiver permissao, false caso contrario.
     */
    virtual bool PodeRemover() const;

    /**
     * @brief Verifica se o utilizador pode listar.
     * @return true se tiver permissao, false caso contrario.
     */
    virtual bool PodeListar() const;

    /**
     * @brief Verifica se o utilizador pode executar motores (RUN).
     * @return true se tiver permissao, false caso contrario.
     */
    virtual bool PodeRun() const;

    /**
     * @brief Verifica se o utilizador pode realizar manutencao.
     * @return true se tiver permissao, false caso contrario.
     */
    virtual bool PodeManutencao() const;
};

/**
 * @brief Utilizador com todas as permissoes.
 */
class Admin : public User
{
public:
    /**
     * @brief Construtor do administrador.
     */
    Admin(int id, const string& nome);

    /**
     * @brief Destrutor do administrador.
     */
    virtual ~Admin();

    virtual string GetTipo() const;

    virtual bool PodeAdicionar() const;
    virtual bool PodeRemover() const;
    virtual bool PodeListar() const;
    virtual bool PodeRun() const;
    virtual bool PodeManutencao() const;
};

/**
 * @brief Utilizador com permissoes limitadas.
 *
 * Pode adicionar, listar, executar motores e realizar manutencao,
 * mas nao pode remover elementos.
 */
class Normal : public User
{
public:
    /**
     * @brief Construtor do utilizador normal.
     */
    Normal(int id, const string& nome);

    /**
     * @brief Destrutor do utilizador normal.
     */
    virtual ~Normal();

    virtual string GetTipo() const;

    virtual bool PodeAdicionar() const;
    virtual bool PodeListar() const;
    virtual bool PodeRun() const;
    virtual bool PodeManutencao() const;
};

/**
 * @brief Utilizador com permissoes apenas de consulta.
 *
 * Apenas pode listar os elementos da fabrica.
 */
class Visitante : public User
{
public:
    /**
     * @brief Construtor do visitante.
     */
    Visitante(int id, const string& nome);

    /**
     * @brief Destrutor do visitante.
     */
    virtual ~Visitante();

    virtual string GetTipo() const;

    virtual bool PodeListar() const;
};

#endif
