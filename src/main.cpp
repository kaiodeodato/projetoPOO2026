#include "Fabrica.h"
#include "User.h"

#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>

#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#else
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
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

// ---------------------------------------------------------------------------
// Utilitarios de terminal
// ---------------------------------------------------------------------------

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

// ---------------------------------------------------------------------------
// Menu interativo
// ---------------------------------------------------------------------------

/**
 * @brief Exibe o menu e executa a opcao escolhida pelo utilizador.
 * @param fabrica Referencia para a fabrica activa.
 */
static void executarMenu(Fabrica& fabrica)
{
    cout << endl;
    cout << "===============================" << endl;
    cout << "       MENU DA FABRICA         " << endl;
    cout << "===============================" << endl;
    cout << "1. Listar estado da fabrica"          << endl;
    cout << "2. Arrancar motor (START)"             << endl;
    cout << "3. Parar motor (STOP)"                 << endl;
    cout << "4. Manutencao geral"                   << endl;
    cout << "5. Adicionar utilizador"               << endl;
    cout << "6. Listar motores por tipo"            << endl;
    cout << "7. Ranking marcas menos fiaveis"       << endl;
    cout << "8. Ranking motores mais trabalhadores" << endl;
    cout << "9. Gerar relatorio XML"                << endl;
    cout << "0. Gerar listagem HTML"                << endl;
    cout << "S. Sair do menu"                       << endl;
    cout << "===============================" << endl;
    cout << "Opcao: ";

    string linha;
    getline(cin, linha);
    if (linha.empty())
    {
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
            getline(cin, linha);
            if (!linha.empty())
            {
                fabrica.Arrancar(atoi(linha.c_str()));
            }
            break;
        }
        case '3':
        {
            cout << "ID do motor a parar: ";
            getline(cin, linha);
            if (!linha.empty())
            {
                fabrica.Desligar(atoi(linha.c_str()));
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
            string tipo;
            getline(cin, tipo);
            cout << "ID: ";
            string sId;
            getline(cin, sId);
            cout << "Nome: ";
            string nome;
            getline(cin, nome);

            if (!tipo.empty() && !sId.empty() && !nome.empty())
            {
                int id = atoi(sId.c_str());
                User* novo = NULL;

                if (tipo == "admin")
                {
                    novo = new Admin(id, nome);
                }
                else if (tipo == "normal")
                {
                    novo = new Normal(id, nome);
                }
                else if (tipo == "visitante")
                {
                    novo = new Visitante(id, nome);
                }

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
            cout << "Tipo de motor: ";
            string tipo;
            getline(cin, tipo);
            list<Motor*>* lista = fabrica.Listar_Tipo(tipo, cout);
            if (lista != NULL)
            {
                if (lista->empty())
                {
                    cout << "Nenhum motor do tipo \"" << tipo << "\"." << endl;
                }
                delete lista;
            }
            break;
        }
        case '7':
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
            break;
        }
        case '8':
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
            break;
        }
        case '9':
        {
            cout << "Ficheiro XML: ";
            string fich;
            getline(cin, fich);
            if (!fich.empty())
            {
                fabrica.Relatorio(fich);
                cout << "Relatorio gravado em " << fich << "." << endl;
            }
            break;
        }
        case '0':
        {
            cout << "Ficheiro HTML: ";
            string fich;
            getline(cin, fich);
            if (!fich.empty())
            {
                fabrica.Listagem_HTML(fich);
                cout << "Listagem HTML gravada em " << fich << "." << endl;
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

// ---------------------------------------------------------------------------
// main
// ---------------------------------------------------------------------------

int main()
{
    srand((unsigned int)time(NULL));

    User* atual = new Admin(1, "Gestor");
    Fabrica fabrica(atual);

    if (!fabrica.Load("xml/fabrica.xml"))
    {
        cout << "Nao foi possivel carregar a configuracao." << endl;
        return 1;
    }

    cout << "Fabrica carregada. Prima 'M' para o menu, 'Q' para sair." << endl;
    fabrica.Listar(cout);

    int minutoPasso  = 0;    // minutos acumulados no dia simulado
    int horaSimulada = 8;    // hora de abertura definida no XML

    bool correr = true;

    while (correr)
    {
        // Verificar tecla pressionada (nao bloqueia)
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

        // Passo de simulacao (10 min simulados, 2 graus de aquecimento, 1.5 de arrefecimento)
        fabrica.PassoSimulacao(minutoPasso, horaSimulada, 10, 2.0, 1.5);

        // Verificar avisos de sensores
        {
            list<Motor*> desligados;
            fabrica.VerificarAvisosSensores(desligados);
        }

        // Avancar tempo simulado
        minutoPasso += 10;
        if (minutoPasso >= 1440)
        {
            minutoPasso = 0;
            fabrica.ReiniciarConsumoDia();
            cout << "[DIA NOVO] Consumo diario reiniciado." << endl;
        }
        horaSimulada = (minutoPasso / 60) % 24;

        // 500 ms de real por cada 10 min simulados
        pausar(500);
    }

    cout << "Simulador encerrado." << endl;
    return 0;
}
