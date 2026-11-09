#ifndef FABRICA_H
#define FABRICA_H

#include "Motor.h"
#include "Sensor.h"
#include "Tipos.h"
#include "User.h"

#include <iostream>
#include <list>
#include <map>
#include <string>

using namespace std;

/**
 * @file Fabrica.h
 * @brief Gestao da fabrica: campo, dispositivos, acessos e configuracao XML.
 *
 * A fabrica e um campo de campoX por campoY. Cada motor e cada sensor
 * ocupa uma posicao (X, Y). So entram valores positivos, e apenas
 * dentro do campo (1..campoX e 1..campoY).
 *
 * Posse de memoria:
 * - A fabrica e dona de todos os ponteiros que guarda.
 * - O utilizador passado ao construtor passa a ser da fabrica.
 * - Add, quando devolve true, fica dona do ponteiro.
 * - Add, quando devolve false, nao fica dona: quem chamou apaga o ponteiro.
 * - Listar_Tipo e os rankings devolvem uma lista nova. Quem chamou
 *   apaga a lista com delete e nao apaga os motores que ela aponta.
 * - O destrutor liberta utilizadores, motores e sensores.
 *
 * O ciclo continuo do simulador (tecla M) nao faz parte desta fase.
 * Manutencao, avisos, relatorio XML e listagem HTML ficam declarados
 * e sao preenchidos quando o simulador for ligado.
 */

class Fabrica
{
    private:
        struct MarcaAvarias
        {
            string marca;
            int avarias;
        };

        User* utilizadorAtual;
        list<User*> utilizadores;
        list<Motor*> motores;
        list<Sensor*> sensores;
        int campoX;
        int campoY;
        int horaAbertura;
        int horaEncerramento;
        IntervaloTemperatura verde;
        IntervaloTemperatura amarelo;
        IntervaloTemperatura vermelho;

        bool podeAdicionar() const;
        bool podeListar() const;
        bool podeRun() const;
        bool podeManutencao() const;

        bool posicaoValida(int posX, int posY, int limiteX, int limiteY) const;
        bool horaValida(int hora) const;
        bool intervalosCoerentes(const IntervaloTemperatura& zonaVerde,
                                 const IntervaloTemperatura& zonaAmarela,
                                 const IntervaloTemperatura& zonaVermelha) const;

        bool idUtilizadorExiste(const list<User*>& lista, int id) const;
        bool idMotorExiste(const list<Motor*>& lista, int id) const;
        bool idSensorExiste(const list<Sensor*>& lista, int id) const;
        Motor* procurarMotor(int id) const;

        void apagarUtilizadores(list<User*>& lista);
        void apagarMotores(list<Motor*>& lista);
        void apagarSensores(list<Sensor*>& lista);
        void moverUtilizadores(list<User*>& origem);
        void moverMotores(list<Motor*>& origem);
        void moverSensores(list<Sensor*>& origem);
        void libertarCargaParcial(list<User*>& users, list<Motor*>& mots, list<Sensor*>& sens);

        bool lerLinhas(const string& ficheiro, list<string>& linhas) const;
        string limparLinha(const string& linha) const;
        bool linhaDoTipo(const string& linha, const string& tag) const;
        bool linhaIgnorada(const string& linha) const;
        bool linhaEstrutural(const string& linha) const;
        string lerAtributo(const string& linha, const string& atributo) const;
        bool atributoInteiro(const string& linha, const string& atributo, int& destino) const;
        bool atributoReal(const string& linha, const string& atributo, double& destino) const;
        bool lerLocalizacao(const string& linha, int& id, string& marca,
                            double& probabilidade, int& posX, int& posY) const;

        bool lerDimensoes(const string& linha, int& novoX, int& novoY) const;
        bool lerHorario(const string& linha, int& abertura, int& encerramento) const;
        bool lerIntervalo(const string& linha, IntervaloTemperatura& intervalo) const;
        bool lerEstrutura(const list<string>& linhas, int& novoX, int& novoY,
                          int& novaAbertura, int& novoEncerramento,
                          IntervaloTemperatura& novoVerde, IntervaloTemperatura& novoAmarelo,
                          IntervaloTemperatura& novoVermelho) const;
        bool lerConteudo(const list<string>& linhas, int limiteX, int limiteY,
                         list<User*>& novosUsers, list<Motor*>& novosMotores,
                         list<Sensor*>& novosSensores) const;

        User* criarUtilizador(const string& linha) const;
        Sensor* criarSensor(const string& linha) const;
        Motor* criarMotor(const string& linha) const;
        bool registarUtilizadorXml(const string& linha, list<User*>& destino) const;
        bool registarSensorXml(const string& linha, int limiteX, int limiteY,
                               list<Sensor*>& destino) const;
        bool registarMotorXml(const string& linha, int limiteX, int limiteY,
                              list<Motor*>& destino) const;

        string textoEstado(ESTADO_MOTOR estado) const;
        string textoClasse(CLASSE_TEMPERATURA classe) const;

        void acumularAvarias(map<string, int>& totais) const;
        void inserirMarca(list<MarcaAvarias>& ranking, const string& marca, int avarias) const;
        void inserirPorTrabalho(list<Motor*>& destino, Motor* motor) const;

    public:
        explicit Fabrica(User* ut);
        ~Fabrica();

        Fabrica(const Fabrica& outra) = delete;
        Fabrica& operator=(const Fabrica& outra) = delete;

        bool Load(const string& ficheiro);
        bool Add(User* ut);
        bool Add(Motor* m);
        void Listar(ostream& f = cout) const;
        void Desligar(int id_motor);
        ESTADO_MOTOR Get_Estado(int id_motor) const;
        list<Motor*>* Listar_Tipo(string Tipo, ostream& f = cout) const;
        bool Manutencao();
        list<string>* Ranking_Dos_Fracos() const;
        list<Motor*>* Ranking_Dos_Mais_Trabalhadores() const;
        void Relatorio(string fich_xml) const;
        int Aviso_Humidade(list<Motor*>& lm);
        int Aviso_Fumo(list<Motor*>& lm, string fich_video);
        int Aviso_Luz(string fich_video);
        void Aviso_Missel(string fvideo, string festado = "Estado.txt");
        void Listagem_HTML(string fich_html) const;

        int GetCampoX() const;
        int GetCampoY() const;

        /**
        * @brief Coloca um motor em START (se tiver permissao e o motor existir).
        * @param id_motor Identificador do motor.
        */
        void Arrancar(int id_motor);

        /**
        * @brief Executa um passo de simulacao: aquece/arrefece motores, avanca START para RUN.
        * @param minutoAtual Minuto do dia simulado (0-1439).
        * @param horaAtual Hora simulada (0-23).
        * @param minutosPasso Minutos simulados neste passo.
        * @param grausPasso Graus de aquecimento por passo em RUN.
        * @param arrefPasso Graus de arrefecimento por passo em STOP.
        */
        void PassoSimulacao(int minutoAtual, int horaAtual,
                            int minutosPasso = 10, double grausPasso = 2.0, double arrefPasso = 1.5);

        /**
        * @brief Verifica todos os sensores e dispara os avisos correspondentes.
        * @param desligados Lista preenchida com motores desligados neste passo.
        */
        void VerificarAvisosSensores(list<Motor*>& desligados);

        /**
        * @brief Reinicia o consumo do dia de todos os motores (virar do dia).
        */
        void ReiniciarConsumoDia();
};

#endif
