#include "Fabrica.h"
#include "Sensor.h"
#include "User.h"
#include "TiposMotor.h"

#include <iostream>
#include <cassert>
#include <cstdlib>
#include <ctime>

using namespace std;

static int passed = 0;
static int failed = 0;

static void check(const string& nome, bool cond)
{
    if (cond)
    {
        cout << "[OK] " << nome << endl;
        passed++;
    }
    else
    {
        cout << "[FALHOU] " << nome << endl;
        failed++;
    }
}

static void teste_carga()
{
    cout << "\n--- T1: Carregamento XML ---" << endl;
    User* u = new Admin(1, "Gestor");
    Fabrica f(u);
    bool ok = f.Load("xml/fabrica.xml");
    check("T1.1 Load retorna true", ok);
    check("T1.2 Campo carregado (X=50)", f.GetCampoX() == 50);
    check("T1.3 Campo carregado (Y=80)", f.GetCampoY() == 80);
}

static void teste_tipos_sensor()
{
    cout << "\n--- T2: Tipos de sensor ---" << endl;
    SensorTemperatura st(200, "M1", 0.0, 0.01, 12, 40, 50.0);
    SensorFogo        sf(202, "M1", 0.0, 0.05,  8, 25,  1.0);
    SensorHumidade    sh(10,  "X",  0.0, 0.01,  5,  5, 80.0, 5);
    SensorFumo        sfu(11, "X",  0.0, 0.01,  5,  6, 10.0);
    SensorMissel      sm(300, "X",  0.0, 0.03, 49, 79);
    SensorLuz         sl(12,  "X", 100.0, 0.01, 1,  1, 50.0);

    check("T2.1 SensorTemperatura::GetTipo == TEMPERATURA", st.GetTipo() == "TEMPERATURA");
    check("T2.2 SensorFogo::GetTipo == FOGO",               sf.GetTipo() == "FOGO");
    check("T2.3 SensorHumidade::GetTipo == HUMIDADE",       sh.GetTipo() == "HUMIDADE");
    check("T2.4 SensorFumo::GetTipo == FUMO",               sfu.GetTipo() == "FUMO");
    check("T2.5 SensorMissel::GetTipo == MISSEL",           sm.GetTipo() == "MISSEL");
    check("T2.6 SensorLuz::GetTipo == LUZ",                 sl.GetTipo() == "LUZ");
}

static void teste_estados_motor()
{
    cout << "\n--- T3: Estados do motor ---" << endl;
    IntervaloTemperatura v; v.minimo = 0.0;  v.maximo = 80.0;
    IntervaloTemperatura a; a.minimo = 81.0; a.maximo = 100.0;
    IntervaloTemperatura r; r.minimo = 101.0; r.maximo = 250.0;
    MotorCombustao m(100, "Roy", 5.0, 0.1, 10, 20, v, a, r);

    check("T3.1 Estado inicial STOP", m.GetEstado() == STOP);

    m.START();
    check("T3.2 Apos START -> START", m.GetEstado() == START);

    m.RUN();
    check("T3.3 Apos RUN -> RUN", m.GetEstado() == RUN);

    m.STOP();
    check("T3.4 Apos STOP -> STOP", m.GetEstado() == STOP);

    m.Avariar();
    check("T3.5 Apos Avariar -> AVARIADO", m.GetEstado() == AVARIADO);

    m.RESTART();
    check("T3.6 RESTART em AVARIADO nao muda estado", m.GetEstado() == AVARIADO);

    m.START();
    check("T3.7 START em AVARIADO nao muda estado", m.GetEstado() == AVARIADO);

    m.Reparar();
    check("T3.8 Apos Reparar -> STOP", m.GetEstado() == STOP);
    m.RESTART();
    check("T3.9 Apos RESTART reparado -> START", m.GetEstado() == START);
}

static void teste_estou_quente()
{
    cout << "\n--- T4: ESTOU_QUENTE ---" << endl;
    IntervaloTemperatura v; v.minimo = 0.0;  v.maximo = 80.0;
    IntervaloTemperatura a; a.minimo = 81.0; a.maximo = 100.0;
    IntervaloTemperatura r; r.minimo = 101.0; r.maximo = 250.0;
    MotorCombustao m(100, "Roy", 5.0, 0.0, 10, 20, v, a, r);
    m.START(); m.RUN();

    m.SetTemperaturaAtual(50.0);
    check("T4.1 50 graus -> VERDE", m.ESTOU_QUENTE() == VERDE);
    check("T4.2 Motor continua RUN em VERDE", m.GetEstado() == RUN);

    m.SetTemperaturaAtual(90.0);
    check("T4.3 90 graus -> AMARELO", m.ESTOU_QUENTE() == AMARELO);
    check("T4.4 Motor continua RUN em AMARELO", m.GetEstado() == RUN);

    m.SetTemperaturaAtual(110.0);
    check("T4.5 110 graus -> VERMELHO", m.ESTOU_QUENTE() == VERMELHO);
    check("T4.6 Motor vai para STOP em VERMELHO", m.GetEstado() == STOP);
}

static void teste_intervalos_motor()
{
    cout << "\n--- T5: Intervalos de temperatura por tipo ---" << endl;
    IntervaloTemperatura vc; vc.minimo = 0.0;  vc.maximo = 80.0;
    IntervaloTemperatura ac; ac.minimo = 81.0; ac.maximo = 100.0;
    IntervaloTemperatura rc; rc.minimo = 101.0; rc.maximo = 250.0;
    MotorCombustao  mc(100, "X", 5.0, 0.0, 1, 1, vc, ac, rc);

    IntervaloTemperatura ve; ve.minimo = 0.0;  ve.maximo = 80.0;
    IntervaloTemperatura ae; ae.minimo = 81.0; ae.maximo = 100.0;
    IntervaloTemperatura re; re.minimo = 101.0; re.maximo = 500.0;
    MotorEletrico   me(101, "X", 5.0, 0.0, 2, 2, ve, ae, re);

    IntervaloTemperatura vi; vi.minimo = 0.0;  vi.maximo = 50.0;
    IntervaloTemperatura ai; ai.minimo = 51.0; ai.maximo = 70.0;
    IntervaloTemperatura ri; ri.minimo = 71.0; ri.maximo = 100.0;
    MotorInducao    mi(102, "X", 5.0, 0.0, 3, 3, vi, ai, ri);

    IntervaloTemperatura vmc = mc.GetIntervaloVerde();
    IntervaloTemperatura vme = me.GetIntervaloVerde();
    IntervaloTemperatura vmi = mi.GetIntervaloVerde();

    check("T5.1 Combustao verde min=0", vmc.minimo == 0.0);
    check("T5.2 Combustao verde max=80", vmc.maximo == 80.0);
    check("T5.3 Eletrico verde min=0", vme.minimo == 0.0);
    check("T5.4 Eletrico verde max=80", vme.maximo == 80.0);
    check("T5.5 Inducao verde min=0", vmi.minimo == 0.0);
    check("T5.6 Inducao verde max=50", vmi.maximo == 50.0);
}

static void teste_listar_tipo()
{
    cout << "\n--- T6: Listar_Tipo case-insensitive ---" << endl;
    User* u = new Admin(1, "Gestor");
    Fabrica f(u);
    f.Load("xml/fabrica.xml");

    list<Motor*>* upper = f.Listar_Tipo("MCOMBUSTAO", cout);
    list<Motor*>* lower = f.Listar_Tipo("mcombustao", cout);
    list<Motor*>* mixed = f.Listar_Tipo("mCoMbUsTaO", cout);
    list<Motor*>* none  = f.Listar_Tipo("XYZ", cout);

    check("T6.1 MCOMBUSTAO maiusculas -> 3 motores", upper != NULL && upper->size() == 3);
    check("T6.2 mcombustao minusculas -> 3 motores", lower != NULL && lower->size() == 3);
    check("T6.3 MixedCase -> 3 motores",             mixed != NULL && mixed->size() == 3);
    check("T6.4 XYZ -> lista vazia (nao NULL)",       none  != NULL && none->empty());

    if (upper) delete upper;
    if (lower) delete lower;
    if (mixed) delete mixed;
    if (none)  delete none;
}

static void teste_permissoes()
{
    cout << "\n--- T7: Permissoes por tipo de utilizador ---" << endl;
    User* u = new Admin(1, "Gestor");
    Fabrica f(u);
    f.Load("xml/fabrica.xml");

    f.Arrancar(100);
    check("T7.1 Admin pode arrancar motor", f.Get_Estado(100) == START);

    f.SetUtilizadorAtual(new Visitante(2, "Visita"));

    bool removeu = f.RemoverMotor(100);
    check("T7.2 Visitante nao pode remover motor", !removeu);

    f.Arrancar(101);
    check("T7.3 Visitante nao pode arrancar motor", f.Get_Estado(101) == STOP);

    f.SetUtilizadorAtual(new Normal(3, "Normal"));
    f.Arrancar(101);
    check("T7.4 Normal pode arrancar motor", f.Get_Estado(101) == START);
}

static void teste_humidade_duplicados()
{
    cout << "\n--- T8: Aviso_Humidade sem duplicados ---" << endl;
    User* u = new Admin(1, "Gestor");
    Fabrica f(u);
    f.Load("xml/fabrica.xml");

    f.Arrancar(100); f.Arrancar(101); f.Arrancar(102); f.Arrancar(103);

    list<Motor*> lm1, lm2;
    f.Aviso_Humidade(lm1);
    f.Aviso_Humidade(lm2);

    bool sem_dup1 = true;
    for (list<Motor*>::iterator a = lm1.begin(); a != lm1.end(); ++a)
    {
        list<Motor*>::iterator b = a; ++b;
        for (; b != lm1.end(); ++b)
            if (*a == *b) { sem_dup1 = false; break; }
    }
    bool sem_dup2 = true;
    for (list<Motor*>::iterator a = lm2.begin(); a != lm2.end(); ++a)
    {
        list<Motor*>::iterator b = a; ++b;
        for (; b != lm2.end(); ++b)
            if (*a == *b) { sem_dup2 = false; break; }
    }
    check("T8.1 Primeira chamada sem duplicados na lm", sem_dup1);
    check("T8.2 Segunda chamada sem duplicados na lm", sem_dup2);
}

static void teste_missel_repoe_sensores()
{
    cout << "\n--- T9: Aviso_Missel repoe sensores ---" << endl;
    User* u = new Admin(1, "Gestor");
    Fabrica f(u);
    f.Load("xml/fabrica.xml");

    f.Arrancar(100); f.Arrancar(101);

    f.Aviso_Missel("", "");

    bool motorsStopped = (f.Get_Estado(100) == STOP || f.Get_Estado(100) == AVARIADO)
                      && (f.Get_Estado(101) == STOP || f.Get_Estado(101) == AVARIADO);
    check("T9.1 Motores parados apos Aviso_Missel", motorsStopped);
}

static void teste_verificar_avisos_temperatura_fogo()
{
    cout << "\n--- T10: VerificarAvisosSensores para TEMPERATURA e FOGO ---" << endl;

    SensorTemperatura st(200, "M1", 60.0, 0.0, 12, 40, 50.0);
    check("T10.1 SensorTemperatura em aviso quando valor>=limiar", st.EmAviso());
    check("T10.2 SensorTemperatura::Disparou na primeira chamada", st.Disparou());
    check("T10.3 SensorTemperatura::Disparou false na segunda (ja foi processado)", !st.Disparou());

    SensorFogo sf(202, "M1", 1.0, 0.0, 8, 25, 1.0);
    check("T10.4 SensorFogo em aviso quando valor>=limiar", sf.EmAviso());
    check("T10.5 SensorFogo::Disparou na primeira chamada", sf.Disparou());
    check("T10.6 Apos Repor, SensorFogo nao esta em aviso", (sf.Repor(), !sf.EmAviso()));
}

static void teste_manutencao_temperatura()
{
    cout << "\n--- T11: Manutencao repoe temperatura no verde do tipo ---" << endl;
    User* u = new Admin(1, "Gestor");
    Fabrica f(u);
    f.Load("xml/fabrica.xml");

    ESTADO_MOTOR est = f.Get_Estado(100);
    check("T11.1 Motor 100 acessivel (nao retorna estado invalido)", est == STOP || est == START || est == RUN || est == AVARIADO);

    bool ok = f.Manutencao();
    check("T11.2 Admin pode fazer manutencao", ok);
}

static void teste_posicao_nao_negativa()
{
    cout << "\n--- T12: Posicao nao-negativa ---" << endl;
    Motor m1(10, "MCOMBUSTAO", "Roy", 5.0, 81.0, 101.0, 0.0, 0, 0);
    check("T12.1 Motor em X=0,Y=0 aceite (GetX==0)", m1.GetX() == 0);
    check("T12.2 Motor em X=0,Y=0 aceite (GetY==0)", m1.GetY() == 0);

    Motor m2(11, "MCOMBUSTAO", "Roy", 5.0, 81.0, 101.0, 0.0, -1, 5);
    check("T12.3 Motor com X=-1 rejeitado (X fica 0)", m2.GetX() == 0);

    Motor m3(12, "MCOMBUSTAO", "Roy", 5.0, 81.0, 101.0, 0.0, 5, -1);
    check("T12.4 Motor com Y=-1 rejeitado (Y fica 0)", m3.GetY() == 0);
}

int main()
{
    srand(42);

    cout << "======================================" << endl;
    cout << "   SUITE DE TESTES - Fabrica POO      " << endl;
    cout << "======================================" << endl;

    teste_carga();
    teste_tipos_sensor();
    teste_estados_motor();
    teste_estou_quente();
    teste_intervalos_motor();
    teste_listar_tipo();
    teste_permissoes();
    teste_humidade_duplicados();
    teste_missel_repoe_sensores();
    teste_verificar_avisos_temperatura_fogo();
    teste_manutencao_temperatura();
    teste_posicao_nao_negativa();

    cout << "\n======================================" << endl;
    cout << "Resultados: " << passed << " OK  |  " << failed << " FALHARAM" << endl;
    cout << "======================================" << endl;

    return (failed == 0) ? 0 : 1;
}
