/**
 * @file main.cpp
 * @brief Arranque da fabrica, pergunta do relatorio XML e menu da tecla M.
 */

#include "Fabrica.h"
#include "User.h"

#include <conio.h>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <windows.h>
#define WIN32_LEAN_AND_MEAN

using namespace std;

/** @brief Descarta o resto da linha depois de uma leitura falhada. */
static void limparEntrada()
{
    cin.clear();
    cin.ignore(10000, '\n');
}

/**
 * @brief Pede um inteiro ao operador.
 * @param pedido Texto mostrado antes da leitura.
 * @param destino Fica com o valor lido.
 * @return false se a leitura falhar.
 */
static bool lerInteiro(const string& pedido, int& destino)
{
    cout << pedido;
    if (!(cin >> destino))
    {
        limparEntrada();
        cout << "Valor invalido." << endl;
        return false;
    }

    limparEntrada();
    return true;
}

/**
 * @brief Pede um numero real ao operador.
 * @param pedido Texto mostrado antes da leitura.
 * @param destino Fica com o valor lido.
 * @return false se a leitura falhar.
 */
static bool lerReal(const string& pedido, double& destino)
{
    cout << pedido;
    if (!(cin >> destino))
    {
        limparEntrada();
        cout << "Valor invalido." << endl;
        return false;
    }

    limparEntrada();
    return true;
}

/**
 * @brief Pede uma linha de texto ao operador.
 * @param pedido Texto mostrado antes da leitura.
 * @param destino Fica com a linha, sem o Enter.
 * @return false se a linha vier vazia.
 */
static bool lerTexto(const string& pedido, string& destino)
{
    cout << pedido;
    getline(cin, destino);
    return !destino.empty();
}

/**
 * @brief Escreve a hora simulada com dois digitos nos minutos.
 * @param fabrica Fabrica cuja hora se mostra.
 */
static void mostrarHora(const Fabrica& fabrica)
{
    cout << "Hora simulada " << fabrica.GetHoraSimulada() << ":";
    if (fabrica.GetMinutoSimulado() < 10)
    {
        cout << "0";
    }
    cout << fabrica.GetMinutoSimulado() << endl;
}

/**
 * @brief Pergunta os dados e tenta adicionar um utilizador.
 * @param fabrica Fabrica que recebe o utilizador se o Add tiver sucesso.
 */
static void adicionarUtilizador(Fabrica& fabrica)
{
    int id = 0;
    int perfil = 0;
    string nome;
    if (!lerInteiro("ID do utilizador: ", id) || !lerTexto("Nome: ", nome))
    {
        return;
    }

    cout << "Perfil: 1 ADMIN, 2 NORMAL, 3 VISITANTE: ";
    if (!(cin >> perfil))
    {
        limparEntrada();
        cout << "Valor invalido." << endl;
        return;
    }
    limparEntrada();

    User* utilizador = NULL;
    if (perfil == 1)
    {
        utilizador = new Admin(id, nome);
    }
    else if (perfil == 2)
    {
        utilizador = new Normal(id, nome);
    }
    else if (perfil == 3)
    {
        utilizador = new Visitante(id, nome);
    }
    else
    {
        cout << "Perfil desconhecido." << endl;
        return;
    }

    if (!fabrica.Add(utilizador))
    {
        delete utilizador;
        cout << "Nao foi possivel adicionar o utilizador." << endl;
        return;
    }

    cout << "Utilizador adicionado." << endl;
}

/**
 * @brief Pergunta os dados e tenta criar um motor de um tipo do XML.
 * @param fabrica Fabrica que recebe o motor.
 */
static void adicionarMotor(Fabrica& fabrica)
{
    int id = 0;
    int x = 0;
    int y = 0;
    double consumo = 0.0;
    string tipo;
    string marca;
    if (!lerInteiro("ID do motor: ", id) || !lerTexto("Tipo (MELETRICO, MCOMBUSTAO, MINDUCAO): ", tipo) || !lerTexto("Marca: ", marca))
    {
        return;
    }

    if (!lerReal("Consumo por hora: ", consumo) || !lerInteiro("X: ", x) || !lerInteiro("Y: ", y))
    {
        return;
    }

    if (!fabrica.CriarMotor(id, tipo, marca, consumo, x, y))
    {
        cout << "Nao foi possivel adicionar o motor." << endl;
        return;
    }

    cout << "Motor adicionado." << endl;
}

/**
 * @brief Pergunta o que remover e chama o metodo correspondente.
 * @param fabrica Fabrica sobre a qual se remove.
 */
static void remover(Fabrica& fabrica)
{
    int opcao = 0;
    int id = 0;
    cout << "Remover: 1 utilizador, 2 motor, 3 sensor: ";
    if (!(cin >> opcao))
    {
        limparEntrada();
        cout << "Valor invalido." << endl;
        return;
    }
    limparEntrada();

    if (!lerInteiro("ID: ", id))
    {
        return;
    }

    bool ok = false;
    if (opcao == 1)
    {
        ok = fabrica.RemoverUtilizador(id);
    }
    else if (opcao == 2)
    {
        ok = fabrica.RemoverMotor(id);
    }
    else if (opcao == 3)
    {
        string tipo;
        if (!lerTexto("Tipo do sensor (STEMPERATURA, SFOGO, SMISSEL, LUZ): ", tipo))
        {
            return;
        }
        ok = fabrica.RemoverSensor(id, tipo);
    }
    else
    {
        cout << "Opcao desconhecida." << endl;
        return;
    }

    if (ok)
    {
        cout << "Removido." << endl;
    }
    else
    {
        cout << "Nao foi possivel remover." << endl;
    }
}

/**
 * @brief Mostra o menu e executa uma opcao. A simulacao fica parada entretanto.
 * @param fabrica Fabrica em pausa.
 * @return false quando o operador escolhe sair.
 */
static bool menu(Fabrica& fabrica)
{
    cout << endl << "=== Menu (simulacao em pausa) ===" << endl;
    mostrarHora(fabrica);
    cout << "1  Listar" << endl;
    cout << "2  Ligar motor" << endl;
    cout << "3  Desligar motor" << endl;
    cout << "4  Manutencao" << endl;
    cout << "5  Adicionar utilizador" << endl;
    cout << "6  Adicionar motor" << endl;
    cout << "7  Remover" << endl;
    cout << "8  Estado de um motor" << endl;
    cout << "9  Motores de um tipo" << endl;
    cout << "10 Ranking das marcas que mais avariam" << endl;
    cout << "11 Ranking dos motores que mais trabalharam" << endl;
    cout << "12 Gerar relatorio XML agora" << endl;
    cout << "13 Gravar listagem HTML" << endl;
    cout << "14 Mudar de utilizador" << endl;
    cout << "0  Continuar a simulacao" << endl;
    cout << "S  Sair" << endl;

    string opcao;
    if (!lerTexto("Opcao: ", opcao))
    {
        return true;
    }

    if (opcao == "S" || opcao == "s")
    {
        return false;
    }

    if (opcao == "0")
    {
        return true;
    }

    if (opcao == "1")
    {
        fabrica.Listar(cout);
    }
    else if (opcao == "2" || opcao == "3" || opcao == "8")
    {
        int id = 0;
        if (!lerInteiro("ID do motor: ", id))
        {
            return true;
        }

        if (opcao == "2")
        {
            if (fabrica.Ligar(id))
            {
                cout << "Pedido de arranque registado." << endl;
            }
            else
            {
                cout << "Nao foi possivel ligar o motor." << endl;
            }
        }
        else if (opcao == "3")
        {
            fabrica.Desligar(id);
            cout << "Pedido de paragem registado." << endl;
        }
        else
        {
            cout << "Estado: ";
            ESTADO_MOTOR estado = fabrica.Get_Estado(id);
            if (estado == STOP)
            {
                cout << "STOP";
            }
            else if (estado == START)
            {
                cout << "START";
            }
            else if (estado == RUN)
            {
                cout << "RUN";
            }
            else
            {
                cout << "AVARIADO";
            }
            cout << endl;
        }
    }
    else if (opcao == "4")
    {
        if (!fabrica.Manutencao())
        {
            cout << "Manutencao recusada." << endl;
        }
    }
    else if (opcao == "5")
    {
        adicionarUtilizador(fabrica);
    }
    else if (opcao == "6")
    {
        adicionarMotor(fabrica);
    }
    else if (opcao == "7")
    {
        remover(fabrica);
    }
    else if (opcao == "9")
    {
        string tipo;
        if (!lerTexto("Tipo: ", tipo))
        {
            return true;
        }

        list<Motor*>* lista = fabrica.Listar_Tipo(tipo, cout);
        delete lista;
    }
    else if (opcao == "10")
    {
        list<string>* ranking = fabrica.Ranking_Dos_Fracos();
        if (ranking == NULL)
        {
            cout << "Ranking indisponivel." << endl;
        }
        else
        {
            int lugar = 1;
            for (list<string>::iterator it = ranking->begin(); it != ranking->end(); ++it)
            {
                cout << lugar << ". " << *it << endl;
                lugar = lugar + 1;
            }
            delete ranking;
        }
    }
    else if (opcao == "11")
    {
        list<Motor*>* ranking = fabrica.Ranking_Dos_Mais_Trabalhadores();
        if (ranking == NULL)
        {
            cout << "Ranking indisponivel." << endl;
        }
        else
        {
            int lugar = 1;
            for (list<Motor*>::iterator it = ranking->begin(); it != ranking->end(); ++it)
            {
                cout << lugar << ". id " << (*it)->GetId() << " " << (*it)->GetMarca();
                cout << " (" << (*it)->GetTempoTrabalho() << " min)" << endl;
                lugar = lugar + 1;
            }
            delete ranking;
        }
    }
    else if (opcao == "12")
    {
        cout << "A gerar relatorio..." << endl;
        fabrica.Relatorio("xml/relatorio.xml");
        cout << "Relatorio gravado em xml/relatorio.xml" << endl;
    }
    else if (opcao == "13")
    {
        fabrica.Listagem_HTML("xml/fabrica.html");
        cout << "Listagem gravada em xml/fabrica.html" << endl;
    }
    else if (opcao == "14")
    {
        fabrica.ListarUtilizadores(cout);

        int id = 0;
        if (!lerInteiro("ID do utilizador: ", id))
        {
            return true;
        }

        if (fabrica.MudarUtilizador(id))
        {
            cout << "Utilizador alterado com sucesso." << endl;
        }
        else
        {
            cout << "Nao existe nenhum utilizador com esse ID." << endl;
        }
    }
    else
    {
        cout << "Opcao desconhecida." << endl;
    }

    return true;
}

/**
 * @brief Carrega a fabrica, pede o intervalo do XML e corre o simulador.
 * @return 0 no fim normal, 1 se a configuracao ou o intervalo falharem.
 */
int main()
{
    srand(static_cast<unsigned int>(time(NULL)));

    User* atual = new Admin(1, "Gestor");
    Fabrica fabrica(atual);

    if (!fabrica.Load("xml/fabrica.xml"))
    {
        cout << "Nao foi possivel carregar a configuracao." << endl;
        cout << "Prima Enter para sair." << endl;
        cin.get();
        return 1;
    }

    cout << fabrica.GetNomeEmpresa() << " carregada." << endl;
    cout << "Cada segundo real avanca um minuto da fabrica." << endl;
    cout << "De quantos em quantos segundos deve ser gerado o relatorio XML? ";

   // int minutoPasso  = 8 * 60;
    //int horaSimulada = 8;

    //bool correr = true;

    //while (correr)
    int intervalo = 0;
    if (!(cin >> intervalo) || intervalo <= 0)
    {
        limparEntrada();
        cout << "O intervalo tem de ser um numero de segundos maior que zero." << endl;
        cout << "Prima Enter para sair." << endl;
        cin.get();
        return 1;
    }
    limparEntrada();

    fabrica.Relatorio("xml/relatorio.xml");
    cout << "Relatorio inicial gravado em xml/relatorio.xml" << endl;
    cout << "A simulacao esta a correr. Prima M para o menu." << endl;
    cout << "O ficheiro xml/relatorio.xml e atualizado de " << intervalo << " em " << intervalo << " segundos." << endl;

    int decorrido = 0;
    bool aCorrer = true;
    DWORD marca = GetTickCount();
    while (aCorrer && fabrica.EstaActiva())
    {
        if (_kbhit())
        {
            int tecla = _getch();
            if (tecla == 'M' || tecla == 'm')
            {
                aCorrer = menu(fabrica);
                marca = GetTickCount();
                continue;
            }
        }

        DWORD agora = GetTickCount();
        if (agora - marca >= 1000)
        {
            fabrica.AvancarMinuto();
            mostrarHora(fabrica);
            decorrido = decorrido + 1;
            if (decorrido >= intervalo)
            {
                fabrica.Relatorio("xml/relatorio.xml");
                cout << "Relatorio XML atualizado: xml/relatorio.xml" << endl;
                decorrido = 0;
            }

            // O video para o relogio. O minuto seguinte so conta depois de a janela fechar.
            marca = GetTickCount();
        }

        Sleep(50);
    }

    if (!fabrica.EstaActiva())
    {
        fabrica.Relatorio("xml/relatorio.xml");
        cout << "A fabrica foi desligada. Ultimo relatorio em xml/relatorio.xml" << endl;
    }

    cout << "Prima Enter para sair." << endl;
    cin.get();
    return 0;
}
