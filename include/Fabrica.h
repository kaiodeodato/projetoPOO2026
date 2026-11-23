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
 * A configuracao vem de xml/fabrica.xml, no formato DADOS / DEFINICOES /
 * MOTORES / SENSORES. O campo vai de 0 ate DIMENSAO_FABRICA, inclusive.
 * Cada motor herda os intervalos de temperatura e a probabilidade de
 * avaria do seu tipo (MELETRICO, MCOMBUSTAO, MINDUCAO).
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
 * O simulador avanca um minuto de fabrica por cada segundo real.
 * Fora do horario os motores nao trabalham. Um alerta avisa o gestor
 * na hora. A tecla M para o simulador ate uma opcao do menu terminar.
 */

/**
 * @brief Fabrica: campo, dispositivos, acessos, XML e simulacao.
 */
class Fabrica
{
    private:
        /**
        * @brief Marca e o numero de avarias, para o ranking dos mais frageis.
        */
        struct MarcaAvarias
        {
            string marca; ///< Nome da marca.
            int avarias; ///< Total de avarias acumuladas.
        };

        /**
        * @brief Limiares e probabilidade lidos da definicao de um tipo de motor.
        */
        struct DefinicaoTipoMotor
        {
            IntervaloTemperatura verde; ///< Zona verde do tipo.
            IntervaloTemperatura amarelo; ///< Zona amarela do tipo.
            IntervaloTemperatura vermelho; ///< Zona vermelha do tipo.
            double probabilidade; ///< Probabilidade de avaria por hora, entre 0 e 1.
        };

        User* utilizadorAtual; ///< Utilizador que esta a operar a fabrica.
        list<User*> utilizadores; ///< Utilizadores pertencentes a fabrica.
        list<Motor*> motores; ///< Motores pertencentes a fabrica.
        list<Sensor*> sensores; ///< Sensores pertencentes a fabrica.
        int campoX; ///< Largura do campo, inclusive.
        int campoY; ///< Altura do campo, inclusive.
        int horaAbertura; ///< Primeira hora em que os motores podem trabalhar.
        int horaEncerramento; ///< Primeira hora em que os motores deixam de trabalhar.
        IntervaloTemperatura verde; ///< Intervalo verde do primeiro tipo lido.
        IntervaloTemperatura amarelo; ///< Intervalo amarelo do primeiro tipo lido.
        IntervaloTemperatura vermelho; ///< Intervalo vermelho do primeiro tipo lido.
        string nomeEmpresa; ///< Nome lido de NOME_EMPRESA.
        int vizinhancaAviso; ///< Raio, em celulas, usado quando o sensor nao traz o seu.
        list<int> paragemManual; ///< Motores desligados pelo operador ate ao turno seguinte.
        map<string, DefinicaoTipoMotor> definicoesMotores; ///< Definicoes MELETRICO, MCOMBUSTAO e MINDUCAO.
        int horaSimulada; ///< Hora actual da simulacao, de 0 a 23.
        int minutoSimulado; ///< Minuto actual da simulacao, de 0 a 59.
        bool fabricaActiva; ///< false depois de um aviso de missel.
        bool turnoAberto; ///< true enquanto o turno de hoje ja arrancou.

        /** @brief Indica se o utilizador actual pode adicionar. */
        bool podeAdicionar() const;

        /** @brief Indica se o utilizador actual pode remover. */
        bool podeRemover() const;

        /** @brief Indica se o utilizador actual pode listar. */
        bool podeListar() const;

        /** @brief Indica se o utilizador actual pode operar motores. */
        bool podeRun() const;

        /** @brief Indica se o utilizador actual pode fazer manutencao. */
        bool podeManutencao() const;

        /** @brief Indica se a hora simulada esta dentro do horario de funcionamento. */
        bool fabricaAberta() const;

        /**
        * @brief Indica se o operador pediu a paragem deste motor.
        * @param id Identificador do motor.
        */
        bool paragemManualContem(int id) const;

        /**
        * @brief Regista uma paragem pedida pelo operador.
        * @param id Identificador do motor.
        */
        void marcarParagemManual(int id);

        /**
        * @brief Esquece a paragem pedida pelo operador.
        * @param id Identificador do motor.
        */
        void limparParagemManual(int id);

        /**
        * @brief Distancia de Chebyshev entre dois pontos do campo.
        * @param x1 Coordenada X do primeiro ponto.
        * @param y1 Coordenada Y do primeiro ponto.
        * @param x2 Coordenada X do segundo ponto.
        * @param y2 Coordenada Y do segundo ponto.
        * @return O maior dos dois desvios absolutos.
        */
        int distancia(int x1, int y1, int x2, int y2) const;

        /**
        * @brief Indica se o motor esta dentro do raio do sensor.
        * @param motor Motor a testar.
        * @param sensor Sensor que define o centro.
        * @param raio Numero de celulas, inclusive.
        */
        bool motorNaVizinhanca(const Motor* motor, const Sensor* sensor, int raio) const;

        /** @brief Indica se algum sensor de fumo esta em aviso. */
        bool fumoEmAviso() const;

        /**
        * @brief Indica se um sensor de humidade em aviso cobre este motor.
        * @param motor Motor a testar.
        */
        bool humidadeBloqueia(const Motor* motor) const;

        /**
        * @brief Escreve um alerta imediato para o gestor.
        * @param mensagem Texto do alerta.
        */
        void avisarGestor(const string& mensagem) const;

        /**
        * @brief Substitui os caracteres especiais de XML.
        * @param texto Texto original.
        * @return Texto seguro para gravar num elemento XML.
        */
        string escaparXml(const string& texto) const;

        /**
        * @brief Grava um texto num ficheiro.
        * @param ficheiro Caminho de destino.
        * @param conteudo Texto completo.
        * @return false se o ficheiro nao abrir.
        */
        bool gravarTexto(const string& ficheiro, const string& conteudo) const;

        /**
        * @brief Actualiza sensores e pode registar a avaria de cada um.
        * @param motoresEmMarcha true se algum motor ficou em RUN neste minuto.
        */
        void actualizarSensores(bool motoresEmMarcha);

        /** @brief Chama o aviso correspondente a cada sensor que acabou de disparar. */
        void processarAlertas();

        /**
        * @brief Indica se a posicao cabe no campo, com o zero incluido.
        * @param posX Coordenada X.
        * @param posY Coordenada Y.
        * @param limiteX Largura do campo.
        * @param limiteY Altura do campo.
        */
        bool posicaoValida(int posX, int posY, int limiteX, int limiteY) const;

        /**
        * @brief Indica se a hora esta entre 0 e 23.
        * @param hora Hora a validar.
        */
        bool horaValida(int hora) const;

        /**
        * @brief Indica se as tres zonas de temperatura nao se sobrepoem.
        * @param zonaVerde Intervalo verde.
        * @param zonaAmarela Intervalo amarelo.
        * @param zonaVermelha Intervalo vermelho.
        */
        bool intervalosCoerentes(const IntervaloTemperatura& zonaVerde,
                                 const IntervaloTemperatura& zonaAmarela,
                                 const IntervaloTemperatura& zonaVermelha) const;

        /**
        * @brief Indica se a lista ja tem um utilizador com este identificador.
        * @param lista Lista a percorrer.
        * @param id Identificador procurado.
        */
        bool idUtilizadorExiste(const list<User*>& lista, int id) const;

        /**
        * @brief Indica se a lista ja tem um motor com este identificador.
        * @param lista Lista a percorrer.
        * @param id Identificador procurado.
        */
        bool idMotorExiste(const list<Motor*>& lista, int id) const;

        /**
        * @brief Indica se a lista ja tem um sensor com este identificador.
        * @param lista Lista a percorrer.
        * @param id Identificador procurado.
        */
        bool idSensorExiste(const list<Sensor*>& lista, int id) const;

        /**
        * @brief Procura um motor pelo identificador.
        * @param id Identificador.
        * @return Ponteiro para o motor, ou NULL se nao existir.
        */
        Motor* procurarMotor(int id) const;

        /**
        * @brief Apaga os utilizadores e esvazia a lista.
        * @param lista Lista dona dos ponteiros.
        */
        void apagarUtilizadores(list<User*>& lista);

        /**
        * @brief Apaga os motores e esvazia a lista.
        * @param lista Lista dona dos ponteiros.
        */
        void apagarMotores(list<Motor*>& lista);

        /**
        * @brief Apaga os sensores e esvazia a lista.
        * @param lista Lista dona dos ponteiros.
        */
        void apagarSensores(list<Sensor*>& lista);

        /**
        * @brief Passa os utilizadores para a fabrica e esvazia a origem.
        * @param origem Lista temporaria da leitura.
        */
        void moverUtilizadores(list<User*>& origem);

        /**
        * @brief Passa os motores para a fabrica e esvazia a origem.
        * @param origem Lista temporaria da leitura.
        */
        void moverMotores(list<Motor*>& origem);

        /**
        * @brief Passa os sensores para a fabrica e esvazia a origem.
        * @param origem Lista temporaria da leitura.
        */
        void moverSensores(list<Sensor*>& origem);

        /**
        * @brief Apaga uma carga de XML que nao chegou a ser aceite.
        * @param users Utilizadores criados durante a leitura.
        * @param mots Motores criados durante a leitura.
        * @param sens Sensores criados durante a leitura.
        */
        void libertarCargaParcial(list<User*>& users, list<Motor*>& mots, list<Sensor*>& sens);

        /**
        * @brief Le as linhas nao vazias de um ficheiro, ja sem espacos nas pontas.
        * @param ficheiro Caminho do XML.
        * @param linhas Destino das linhas.
        * @return false se o ficheiro nao abrir.
        */
        bool lerLinhas(const string& ficheiro, list<string>& linhas) const;

        /**
        * @brief Remove espacos e tabulacoes no inicio e no fim.
        * @param linha Linha original.
        * @return Linha limpa.
        */
        string limparLinha(const string& linha) const;

        /**
        * @brief Le o XML no formato de fabrica.xml.
        *
        * PROB_AVARIA e PROB_1 com valor igual ou superior a 1 sao percentagens.
        * @param linhas Linhas ja limpas do ficheiro.
        * @param nome Nome da empresa, preenchido em caso de sucesso.
        * @param novoX Largura do campo.
        * @param novoY Altura do campo.
        * @param abertura Hora de inicio.
        * @param fecho Hora de fecho.
        * @param vizinhanca Raio de aviso da fabrica.
        * @param novoVerde Intervalo verde do primeiro tipo.
        * @param novoAmarelo Intervalo amarelo do primeiro tipo.
        * @param novoVermelho Intervalo vermelho do primeiro tipo.
        * @param novosMotores Motores criados. Quem chamou liberta-os se a leitura falhar.
        * @param novosSensores Sensores criados. Quem chamou liberta-os se a leitura falhar.
        * @return false se o XML nao cumprir o formato.
        */
        bool lerDados(const list<string>& linhas, string& nome, int& novoX, int& novoY,
                      int& abertura, int& fecho, int& vizinhanca,
                      IntervaloTemperatura& novoVerde, IntervaloTemperatura& novoAmarelo,
                      IntervaloTemperatura& novoVermelho, list<Motor*>& novosMotores,
                      list<Sensor*>& novosSensores);

        /**
        * @brief Nome legivel de um estado de motor.
        * @param estado Estado a converter.
        */
        string textoEstado(ESTADO_MOTOR estado) const;

        /**
        * @brief Nome legivel de uma classe de temperatura.
        * @param classe Classe a converter.
        */
        string textoClasse(CLASSE_TEMPERATURA classe) const;

        /**
        * @brief Soma as avarias de motores e sensores por marca.
        * @param totais Mapa marca para total. E alterado.
        */
        void acumularAvarias(map<string, int>& totais) const;

        /**
        * @brief Insere a marca por ordem decrescente de avarias.
        * @param ranking Lista ja ordenada.
        * @param marca Nome da marca.
        * @param avarias Total dessa marca.
        */
        void inserirMarca(list<MarcaAvarias>& ranking, const string& marca, int avarias) const;

        /**
        * @brief Insere o motor por ordem decrescente de tempo de trabalho.
        * @param destino Lista ja ordenada. Nao fica dona do motor.
        * @param motor Motor a inserir.
        */
        void inserirPorTrabalho(list<Motor*>& destino, Motor* motor) const;

    public:
        /**
        * @brief Cria a fabrica e fica dona do utilizador actual.
        * @param ut Utilizador inicial. Pode ser NULL.
        */
        explicit Fabrica(User* ut);

        /** @brief Liberta utilizadores, motores e sensores. */
        ~Fabrica();

        /** @brief Copia proibida, porque a fabrica e dona dos ponteiros. */
        Fabrica(const Fabrica& outra) = delete;

        /** @brief Atribuicao proibida, porque a fabrica e dona dos ponteiros. */
        Fabrica& operator=(const Fabrica& outra) = delete;

        /**
        * @brief Carrega a configuracao no formato de fabrica.xml.
        * @param ficheiro Caminho do XML.
        * @return false se o ficheiro falhar ou a fabrica ja tiver dispositivos.
        */
        bool Load(const string& ficheiro);

        /**
        * @brief Adiciona um utilizador, se o perfil actual o permitir.
        * @param ut Utilizador. Em caso de sucesso passa a ser da fabrica.
        * @return false se o ponteiro continuar a pertencer a quem chamou.
        */
        bool Add(User* ut);

        /**
        * @brief Adiciona um motor, se o perfil actual o permitir.
        * @param m Motor. Em caso de sucesso passa a ser da fabrica.
        * @return false se o ponteiro continuar a pertencer a quem chamou.
        */
        bool Add(Motor* m);

        /**
        * @brief Cria um motor de um tipo definido no XML e adiciona-o.
        * @param id Identificador positivo e ainda livre.
        * @param tipo MELETRICO, MCOMBUSTAO ou MINDUCAO.
        * @param marca Fabricante.
        * @param consumo Consumo por hora, nao negativo.
        * @param x Coordenada X.
        * @param y Coordenada Y.
        * @return false se o tipo nao existir ou o motor for rejeitado.
        */
        bool CriarMotor(int id, const string& tipo, const string& marca, double consumo, int x, int y);

        /**
        * @brief Remove um utilizador que nao seja o utilizador actual.
        * @param id Identificador.
        * @return false se o perfil nao puder remover ou o id nao existir.
        */
        bool RemoverUtilizador(int id);

        /**
        * @brief Remove um motor e liberta-o.
        * @param id Identificador.
        * @return false se o perfil nao puder remover ou o id nao existir.
        */
        bool RemoverMotor(int id);

        /**
        * @brief Remove o sensor com este identificador e este tipo.
        * @param id Identificador. Pode repetir-se entre tipos diferentes.
        * @param tipo Etiqueta do sensor, por exemplo SFOGO.
        * @return false se o perfil nao puder remover ou o sensor nao existir.
        */
        bool RemoverSensor(int id, const string& tipo);

        /**
        * @brief Pede o arranque de um motor.
        * @param id Identificador.
        * @return false se o perfil nao puder operar ou o motor nao existir.
        */
        bool Ligar(int id);

        /**
        * @brief Escreve o estado actual da fabrica.
        * @param f Destino. Por omissao e a consola.
        */
        void Listar(ostream& f = cout) const;

        /**
        * @brief Desliga um motor e mantem-no parado ate ao proximo turno.
        * @param id_motor Identificador do motor.
        */
        void Desligar(int id_motor);

        /**
        * @brief Devolve o estado de um motor.
        * @param id_motor Identificador.
        * @return STOP se o motor nao existir ou o perfil nao puder listar.
        */
        ESTADO_MOTOR Get_Estado(int id_motor) const;

        /**
        * @brief Lista e devolve os motores de um tipo.
        * @param Tipo Etiqueta, por exemplo MCOMBUSTAO.
        * @param f Destino da listagem.
        * @return Lista nova. Quem chamou apaga a lista e nao os motores. NULL se nao puder listar.
        */
        list<Motor*>* Listar_Tipo(string Tipo, ostream& f = cout) const;

        /**
        * @brief Poe cada motor numa temperatura aleatoria da zona verde e repoe os sensores.
        * @return false se o perfil nao puder fazer manutencao ou a fabrica estiver desligada.
        */
        bool Manutencao();

        /**
        * @brief Marcas ordenadas da que mais avaria para a mais fiavel.
        * @return Lista nova de nomes. Quem chamou apaga a lista. NULL se nao puder listar.
        */
        list<string>* Ranking_Dos_Fracos() const;

        /**
        * @brief Motores ordenados do que mais trabalhou para o que menos trabalhou.
        * @return Lista nova. Quem chamou apaga a lista e nao os motores. NULL se nao puder listar.
        */
        list<Motor*>* Ranking_Dos_Mais_Trabalhadores() const;

        /**
        * @brief Grava o estado no formato DADOS / DEFINICOES / MOTORES / SENSORES.
        * @param fich_xml Caminho do relatorio.
        */
        void Relatorio(string fich_xml) const;

        /**
        * @brief Desliga os motores na vizinhanca de cada sensor de humidade em aviso.
        * @param lm Fica com os motores que foram desligados. Nao e dona deles.
        * @return Numero de motores desligados.
        */
        int Aviso_Humidade(list<Motor*>& lm);

        /**
        * @brief Desliga todos os motores e grava o video das saidas.
        * @param lm Fica com os motores que foram desligados. Nao e dona deles.
        * @param fich_video Caminho do ficheiro de video.
        * @return Numero de motores desligados.
        */
        int Aviso_Fumo(list<Motor*>& lm, string fich_video);

        /**
        * @brief Grava o video de regresso a casa.
        * @param fich_video Caminho do ficheiro de video.
        * @return 1 se o ficheiro foi gravado, 0 em caso contrario.
        */
        int Aviso_Luz(string fich_video);

        /**
        * @brief Grava o video dos abrigos, guarda o estado e desliga a fabrica.
        * @param fvideo Caminho do video.
        * @param festado Caminho do ficheiro de estado. Por omissao e Estado.txt.
        */
        void Aviso_Missel(string fvideo, string festado = "Estado.txt");

        /**
        * @brief Grava uma pagina HTML com o estado actual, para abrir num browser.
        * @param fich_html Caminho do ficheiro.
        */
        void Listagem_HTML(string fich_html) const;

        /**
        * @brief Avanca um minuto de funcionamento da fabrica.
        *
        * Dentro do horario os motores arrancam, aquecem, consomem e podem avariar.
        * Fora do horario ficam parados. Cada alerta e dito ao gestor de imediato.
        */
        void AvancarMinuto();

        /** @brief Indica se a fabrica ainda nao foi desligada por um missel. */
        bool EstaActiva() const;

        /** @brief Devolve a hora simulada, de 0 a 23. */
        int GetHoraSimulada() const;

        /** @brief Devolve o minuto simulado, de 0 a 59. */
        int GetMinutoSimulado() const;

        /** @brief Devolve o nome da empresa. */
        string GetNomeEmpresa() const;

        /** @brief Devolve a largura do campo. */
        int GetCampoX() const;

        /** @brief Devolve a altura do campo. */
        int GetCampoY() const;

         /**
         * @brief Altera o utilizador atualmente ativo.
         * @param id Identificador do utilizador a selecionar.
         * @return true se encontrou e selecionou o utilizador.
         */
        bool MudarUtilizador(int id);

        void ListarUtilizadores(ostream& f = cout) const;
};

#endif
