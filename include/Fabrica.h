#ifndef FABRICA_H
#define FABRICA_H

#include "Motor.h"
#include "Sensor.h"
#include "TiposMotor.h"
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

        struct DefinicoesTipoMotor
        {
            IntervaloTemperatura verde;
            IntervaloTemperatura amarelo;
            IntervaloTemperatura vermelho;
            double probAvaria;
        };

        User* utilizadorAtual;
        list<User*> utilizadores;
        list<Motor*> motores;
        list<Sensor*> sensores;
        int campoX;
        int campoY;
        int horaAbertura;
        int horaEncerramento;
        int vizinhancaAviso;
        string nomeFabrica;
        IntervaloTemperatura verde;
        IntervaloTemperatura amarelo;
        IntervaloTemperatura vermelho;
        DefinicoesTipoMotor defCombustao;
        DefinicoesTipoMotor defEletrico;
        DefinicoesTipoMotor defInducao;

        bool podeAdicionar() const;
        bool podeRemover() const;
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
        string tagAbertura(const string& linha) const;
        bool eTagFecho(const string& linha, const string& tag) const;
        string valorEntreTag(const string& linha, const string& tag) const;
        bool lerInteiroDaTag(const list<string>& bloco, const string& tag, int& dest) const;
        bool lerRealDaTag(const list<string>& bloco, const string& tag, double& dest) const;
        string lerTextoDaTag(const list<string>& bloco, const string& tag) const;
        bool lerPar(const string& texto, double& a, double& b) const;
        bool lerIntervaloBloco(const list<string>& bloco, const string& tag,
                               IntervaloTemperatura& intervalo) const;
        bool lerPosicao(const list<string>& bloco, int& posX, int& posY) const;

        bool extrairBloco(const list<string>& linhas, const string& tag,
                          list<string>& bloco) const;
        bool extrairTodosOsBlocos(const list<string>& linhas, const string& tag,
                                  list< list<string> >& blocos) const;

        bool lerDefinicoes(const list<string>& bloco,
                           int& novoX, int& novoY,
                           int& novaAbertura, int& novoEncerramento,
                           int& novaVizinhanca, string& novoNome,
                           DefinicoesTipoMotor& combDef,
                           DefinicoesTipoMotor& eletDef,
                           DefinicoesTipoMotor& indDef) const;
        bool lerTipoMotorBloco(const list<string>& bloco,
                               DefinicoesTipoMotor& def) const;

        Motor*  criarMotorDoBloco(const list<string>& bloco, const string& tipo,
                                  const DefinicoesTipoMotor& def) const;
        Sensor* criarSensorDoBloco(const list<string>& bloco, const string& tipo) const;
        bool lerMotores(const list<string>& blocoMotores,
                        int limX, int limY, list<Motor*>& dest) const;
        bool lerSensores(const list<string>& blocoSensores,
                         int limX, int limY, list<Sensor*>& dest) const;

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
        /**
        * @brief Adiciona um sensor a fabrica.
        * @param s Ponteiro para o sensor. A fabrica fica dona do ponteiro se devolver true.
        * @return true se adicionado; false se NULL, sem permissao, posicao invalida ou ID duplicado.
        */
        bool Add(Sensor* s);

        /**
        * @brief Remove um utilizador pelo ID. Apenas o ADMIN pode remover.
        * @param id Identificador do utilizador a remover.
        * @return true se removido, false se sem permissao ou nao encontrado.
        */
        bool RemoverUtilizador(int id);

        /**
        * @brief Remove um motor pelo ID. Apenas o ADMIN pode remover.
        * @param id Identificador do motor a remover.
        * @return true se removido, false se sem permissao ou nao encontrado.
        */
        bool RemoverMotor(int id);

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
        * @brief Coloca um motor em START, se o utilizador tiver permissao RUN.
        *
        * Nao faz nada se o motor nao existir ou o utilizador nao puder usar RUN.
        * @param id_motor Identificador do motor.
        */
        void Arrancar(int id_motor);

        /**
        * @brief Executa um passo de simulacao.
        *
        * Por cada motor em START passa a RUN se a fabrica estiver aberta.
        * Motores em RUN aquecem, acumulam tempo de trabalho e podem avariar.
        * Motores em STOP arrefecem.
        * O gestor e avisado quando um motor entra em AMARELO ou VERMELHO.
        * Os sensores sao actualizados com o estado dos motores e a hora.
        *
        * @param minutoAtual Minuto do dia simulado (0-1439).
        * @param horaAtual   Hora simulada (0-23).
        * @param minutosPasso Duracao do passo em minutos simulados (default 10).
        * @param grausPasso   Aquecimento por passo em RUN (default 2.0).
        * @param arrefPasso   Arrefecimento por passo em STOP (default 1.5).
        */
        void PassoSimulacao(int minutoAtual, int horaAtual,
                            int minutosPasso = 10, double grausPasso = 2.0, double arrefPasso = 1.5);

        /**
        * @brief Verifica todos os sensores e activa os avisos quando disparam pela primeira vez.
        *
        * Chama Aviso_Humidade, Aviso_Fumo, Aviso_Luz ou Aviso_Missel consoante o tipo.
        * So activa o aviso no primeiro passo em que o sensor entra em aviso (Disparou()).
        *
        * @param desligados Lista preenchida com todos os motores desligados neste passo.
        */
        void VerificarAvisosSensores(list<Motor*>& desligados);

        /**
        * @brief Reinicia o consumo diario de todos os motores.
        *
        * Deve ser chamado no virar do dia simulado.
        */
        void ReiniciarConsumoDia();

        /**
        * @brief Troca o utilizador actual por um novo.
        *
        * O utilizador anterior e apagado se nao estiver na lista de utilizadores.
        * A fabrica fica dona do novo ponteiro.
        * @param novo Novo utilizador. NULL e ignorado.
        */
        void SetUtilizadorAtual(User* novo);
};

#endif
