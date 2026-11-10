#include "Fabrica.h"
#include "Sensor.h"
#include "User.h"

#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>

#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#include <sys/select.h>
#endif

using namespace std;

/**
 * @file main.cpp
 * @brief Simulador continuo da Gestao de Fabrica.
 *
 * Pressionar 'M' (ou 'm') pausa o simulador e abre o menu interativo.
 * Pressionar 'Q' (ou 'q') termina o simulador.
 * Cada passo representa 10 minutos simulados.
 */

#ifndef _WIN32
static struct termios terminalOriginal;

/** @brief Ativa modo raw: sem eco e sem buffer de linha. */
static void ativarModoRaw()
{
    struct termios raw;
    tcgetattr(STDIN_FILENO, &terminalOriginal);
    raw = terminalOriginal;
    raw.c_lflag &= ~((tcflag_t)(ICANON | ECHO));
    raw.c_cc[VMIN]  = 0;
    raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
}

/** @brief Restaura o terminal ao estado original. */
static void restaurarTerminal()
{
    tcsetattr(STDIN_FILENO, TCSANOW, &terminalOriginal);
}
#endif

/** @brief Retorna true se houver uma tecla disponivel sem bloquear. */
static bool teclaPronta()
{
#ifdef _WIN32
    return _kbhit() != 0;
#else
    struct timeval tv = {0, 0};
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    return select(STDIN_FILENO + 1, &fds, NULL, NULL, &tv) > 0;
#endif
}

/** @brief Le uma tecla sem eco. Assume teclaPronta() == true. */
static char lerTecla()
{
#ifdef _WIN32
    return (char)_getch();
#else
    char c = '\0';
    read(STDIN_FILENO, &c, 1);
    return c;
#endif
}

/** @brief Suspende a execucao por ms milissegundos. */
static void pausar(int ms)
{
#ifdef _WIN32
    Sleep(ms);
#else
    usleep(ms * 1000);
#endif
}

/**
 * @brief Le uma linha do stdin, restaurando o terminal em modo normal primeiro.
 *
 * No Linux o modo raw e desativado para que getline funcione normalmente,
 * e reativado a seguir.
 */
static string lerLinha()
{
#ifndef _WIN32
    restaurarTerminal();
#endif
    string linha;
    getline(cin, linha);
#ifndef _WIN32
    ativarModoRaw();
#endif
    return linha;
}

/**
 * @brief Exibe o menu e executa a opcao escolhida pelo utilizador.
 * @param fabrica Referencia para a fabrica activa.
 */
static void executarMenu(Fabrica& fabrica)
{
    cout << endl;
    cout << "======================================" << endl;
    cout << "          MENU DA FABRICA             " << endl;
    cout << "======================================" << endl;
    cout << "1.  Listar estado da fabrica"           << endl;
    cout << "2.  Arrancar motor (START)"              << endl;
    cout << "3.  Parar motor (STOP)"                  << endl;
    cout << "4.  Manutencao geral"                    << endl;
    cout << "5.  Adicionar utilizador"                << endl;
    cout << "6.  Adicionar motor"                     << endl;
    cout << "7.  Adicionar sensor"                    << endl;
    cout << "8.  Remover utilizador (so ADMIN)"       << endl;
    cout << "9.  Remover motor (so ADMIN)"             << endl;
    cout << "10. Listar motores por tipo"             << endl;
    cout << "11. Ranking marcas menos fiaveis"        << endl;
    cout << "12. Ranking motores mais trabalhadores"  << endl;
    cout << "13. Gerar relatorio XML"                 << endl;
    cout << "14. Gerar listagem HTML"                 << endl;
    cout << "15. Mudar utilizador atual"              << endl;
    cout << "S.  Sair do menu"                        << endl;
    cout << "======================================" << endl;
    cout << "Opcao: ";

    string linha = lerLinha();
    if (linha.empty())
    {
        return;
    }

    if (linha == "10")
    {
        cout << "Tipo de motor: ";
        string tipo = lerLinha();
        list<Motor*>* lista = fabrica.Listar_Tipo(tipo, cout);
        if (lista != NULL)
        {
            if (lista->empty())
            {
                cout << "Nenhum motor do tipo \"" << tipo << "\"." << endl;
            }
            delete lista;
        }
        return;
    }
    if (linha == "11")
    {
        list<string>* ranking = fabrica.Ranking_Dos_Fracos();
        if (ranking != NULL)
        {
            cout << "Marcas (mais avarias -> mais fiaveis):" << endl;
            int pos = 1;
            for (list<string>::iterator it = ranking->begin(); it != ranking->end(); ++it)
            {
                cout << "  " << pos++ << ". " << *it << endl;
            }
            delete ranking;
        }
        return;
    }
    if (linha == "12")
    {
        list<Motor*>* ranking = fabrica.Ranking_Dos_Mais_Trabalhadores();
        if (ranking != NULL)
        {
            cout << "Motores (mais trabalharam):" << endl;
            int pos = 1;
            for (list<Motor*>::iterator it = ranking->begin(); it != ranking->end(); ++it)
            {
                Motor* m = *it;
                cout << "  " << pos++ << ". ID " << m->GetId()
                     << " " << m->GetTipo()
                     << " (" << m->GetTempoTrabalho() << " min)" << endl;
            }
            delete ranking;
        }
        return;
    }
    if (linha == "13")
    {
        cout << "Ficheiro XML: ";
        string fich = lerLinha();
        if (!fich.empty())
        {
            fabrica.Relatorio(fich);
            cout << "Relatorio gravado em " << fich << "." << endl;
        }
        return;
    }
    if (linha == "14")
    {
        cout << "Ficheiro HTML: ";
        string fich = lerLinha();
        if (!fich.empty())
        {
            fabrica.Listagem_HTML(fich);
            cout << "Listagem HTML gravada em " << fich << "." << endl;
        }
        return;
    }
    if (linha == "15")
    {
        cout << "Tipo do novo utilizador (admin/normal/visitante): ";
        string tipo = lerLinha();
        cout << "ID: ";
        string sId = lerLinha();
        cout << "Nome: ";
        string nome = lerLinha();

        if (!tipo.empty() && !sId.empty() && !nome.empty())
        {
            int id = atoi(sId.c_str());
            User* novo = NULL;

            if (tipo == "admin")           { novo = new Admin(id, nome); }
            else if (tipo == "normal")     { novo = new Normal(id, nome); }
            else if (tipo == "visitante")  { novo = new Visitante(id, nome); }

            if (novo != NULL)
            {
                fabrica.SetUtilizadorAtual(novo);
                cout << "Utilizador atual alterado para " << nome << " [" << tipo << "]." << endl;
            }
            else
            {
                cout << "Tipo invalido." << endl;
            }
        }
        return;
    }

    char opcao = linha[0];

    switch (opcao)
    {
        case '1':
        {
            fabrica.Listar(cout);
            break;
        }
        case '2':
        {
            cout << "ID do motor a arrancar: ";
            string s = lerLinha();
            if (!s.empty())
            {
                fabrica.Arrancar(atoi(s.c_str()));
            }
            break;
        }
        case '3':
        {
            cout << "ID do motor a parar: ";
            string s = lerLinha();
            if (!s.empty())
            {
                fabrica.Desligar(atoi(s.c_str()));
                cout << "Motor parado." << endl;
            }
            break;
        }
        case '4':
        {
            if (fabrica.Manutencao())
            {
                cout << "Manutencao concluida." << endl;
            }
            else
            {
                cout << "Sem permissao para manutencao." << endl;
            }
            break;
        }
        case '5':
        {
            cout << "Tipo (admin/normal/visitante): ";
            string tipo = lerLinha();
            cout << "ID: ";
            string sId = lerLinha();
            cout << "Nome: ";
            string nome = lerLinha();

            if (!tipo.empty() && !sId.empty() && !nome.empty())
            {
                int id = atoi(sId.c_str());
                User* novo = NULL;

                if (tipo == "admin")       { novo = new Admin(id, nome); }
                else if (tipo == "normal") { novo = new Normal(id, nome); }
                else if (tipo == "visitante") { novo = new Visitante(id, nome); }

                if (novo != NULL)
                {
                    if (fabrica.Add(novo))
                    {
                        cout << "Utilizador adicionado." << endl;
                    }
                    else
                    {
                        cout << "Nao foi possivel adicionar (sem permissao ou ID duplicado)." << endl;
                        delete novo;
                    }
                }
                else
                {
                    cout << "Tipo invalido." << endl;
                }
            }
            break;
        }
        case '6':
        {
            cout << "ID: ";         string sId    = lerLinha();
            cout << "Tipo: ";       string tipo   = lerLinha();
            cout << "Marca: ";      string marca  = lerLinha();
            cout << "Consumo/h: ";  string sCons  = lerLinha();
            cout << "Temp aviso: "; string sAviso = lerLinha();
            cout << "Temp paragem: "; string sPar = lerLinha();
            cout << "Prob avaria [0-1]: "; string sProb = lerLinha();
            cout << "X: ";          string sX     = lerLinha();
            cout << "Y: ";          string sY     = lerLinha();

            if (!sId.empty() && !tipo.empty() && !marca.empty() &&
                !sCons.empty() && !sAviso.empty() && !sPar.empty() &&
                !sProb.empty() && !sX.empty() && !sY.empty())
            {
                Motor* m = new Motor(atoi(sId.c_str()), tipo, marca,
                                     atof(sCons.c_str()),
                                     atof(sAviso.c_str()),
                                     atof(sPar.c_str()),
                                     atof(sProb.c_str()),
                                     atoi(sX.c_str()),
                                     atoi(sY.c_str()));
                if (fabrica.Add(m))
                {
                    cout << "Motor adicionado." << endl;
                }
                else
                {
                    cout << "Nao foi possivel adicionar (sem permissao, posicao invalida ou ID duplicado)." << endl;
                    delete m;
                }
            }
            break;
        }
        case '7':
        {
            cout << "Tipo (luz/humidade/fumo/missel): ";
            string tipo = lerLinha();
            cout << "ID: ";         string sId   = lerLinha();
            cout << "Marca: ";      string marca = lerLinha();
            cout << "Valor: ";      string sVal  = lerLinha();
            cout << "Prob avaria [0-1]: "; string sProb = lerLinha();
            cout << "X: ";          string sX    = lerLinha();
            cout << "Y: ";          string sY    = lerLinha();

            if (!tipo.empty() && !sId.empty() && !marca.empty() &&
                !sVal.empty() && !sProb.empty() && !sX.empty() && !sY.empty())
            {
                int    id    = atoi(sId.c_str());
                double val   = atof(sVal.c_str());
                double prob  = atof(sProb.c_str());
                int    px    = atoi(sX.c_str());
                int    py    = atoi(sY.c_str());
                Sensor* s    = NULL;

                if (tipo == "luz")
                {
                    cout << "Limiar: ";
                    string sLim = lerLinha();
                    if (!sLim.empty())
                    {
                        s = new SensorLuz(id, marca, val, prob, px, py, atof(sLim.c_str()));
                    }
                }
                else if (tipo == "humidade")
                {
                    cout << "Limiar: ";  string sLim  = lerLinha();
                    cout << "Raio: ";    string sRaio = lerLinha();
                    if (!sLim.empty() && !sRaio.empty())
                    {
                        s = new SensorHumidade(id, marca, val, prob, px, py,
                                               atof(sLim.c_str()), atoi(sRaio.c_str()));
                    }
                }
                else if (tipo == "fumo")
                {
                    cout << "Limiar: ";
                    string sLim = lerLinha();
                    if (!sLim.empty())
                    {
                        s = new SensorFumo(id, marca, val, prob, px, py, atof(sLim.c_str()));
                    }
                }
                else if (tipo == "missel")
                {
                    s = new SensorMissel(id, marca, val, prob, px, py);
                }

                if (s != NULL)
                {
                    if (fabrica.Add(s))
                    {
                        cout << "Sensor adicionado." << endl;
                    }
                    else
                    {
                        cout << "Nao foi possivel adicionar (sem permissao, posicao invalida ou ID duplicado)." << endl;
                        delete s;
                    }
                }
                else
                {
                    cout << "Tipo invalido." << endl;
                }
            }
            break;
        }
        case '8':
        {
            cout << "ID do utilizador a remover: ";
            string s = lerLinha();
            if (!s.empty())
            {
                if (fabrica.RemoverUtilizador(atoi(s.c_str())))
                {
                    cout << "Utilizador removido." << endl;
                }
                else
                {
                    cout << "Nao foi possivel remover (sem permissao, nao encontrado ou utilizador actual)." << endl;
                }
            }
            break;
        }
        case '9':
        {
            cout << "ID do motor a remover: ";
            string s = lerLinha();
            if (!s.empty())
            {
                if (fabrica.RemoverMotor(atoi(s.c_str())))
                {
                    cout << "Motor removido." << endl;
                }
                else
                {
                    cout << "Nao foi possivel remover (sem permissao ou nao encontrado)." << endl;
                }
            }
            break;
        }
        case 's':
        case 'S':
        {
            cout << "A voltar ao simulador..." << endl;
            break;
        }
        default:
        {
            cout << "Opcao desconhecida." << endl;
            break;
        }
    }
}

int main()
{
    srand((unsigned int)time(NULL));

#ifndef _WIN32
    ativarModoRaw();
#endif

    User* atual = new Admin(1, "Gestor");
    Fabrica fabrica(atual);

    if (!fabrica.Load("xml/fabrica.xml"))
    {
        cout << "Nao foi possivel carregar a configuracao." << endl;
#ifndef _WIN32
        restaurarTerminal();
#endif
        return 1;
    }

    cout << "Fabrica carregada. Prima 'M' para o menu, 'Q' para sair." << endl;
    fabrica.Listar(cout);

    int minutoPasso  = 0;
    int horaSimulada = 8;

    bool correr = true;

    while (correr)
    {
        if (teclaPronta())
        {
            char tecla = lerTecla();
            if (tecla == 'M' || tecla == 'm')
            {
                executarMenu(fabrica);
            }
            else if (tecla == 'Q' || tecla == 'q')
            {
                correr = false;
                break;
            }
        }

        fabrica.PassoSimulacao(minutoPasso, horaSimulada, 10, 2.0, 1.5);

        {
            list<Motor*> desligados;
            fabrica.VerificarAvisosSensores(desligados);
        }

        minutoPasso += 10;
        if (minutoPasso >= 1440)
        {
            minutoPasso = 0;
            fabrica.ReiniciarConsumoDia();
            cout << "[DIA NOVO] Consumo diario reiniciado." << endl;
        }
        horaSimulada = (minutoPasso / 60) % 24;

        pausar(500);
    }

    cout << "Simulador encerrado." << endl;

#ifndef _WIN32
    restaurarTerminal();
#endif
    return 0;
}
