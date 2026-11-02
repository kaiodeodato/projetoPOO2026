#include "Fabrica.h"
#include "User.h"

#include <iostream>

using namespace std;

/**
 * @file main.cpp
 * @brief Arranque da fabrica.
 *
 * Nesta fase so se carrega o XML e se lista o estado.
 * O simulador continuo, e o menu da tecla M, ficam para a fase seguinte.
 */
int main()
{
    User* atual = new Admin(1, "Gestor");
    Fabrica fabrica(atual);

    if (!fabrica.Load("xml/fabrica.xml"))
    {
        cout << "Nao foi possivel carregar a configuracao." << endl;
        return 1;
    }

    fabrica.Listar(cout);
    return 0;
}
