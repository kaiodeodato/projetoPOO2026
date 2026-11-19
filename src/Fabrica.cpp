/**
 * @file Fabrica.cpp
 * @brief Implementacao da gestao, da leitura XML e do passo de simulacao.
 */

#include "Fabrica.h"

#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <limits>
#include <cmath>

using namespace std;

Fabrica::Fabrica(User* ut)
    : utilizadorAtual(ut),
      campoX(0),
      campoY(0),
      horaAbertura(0),
      horaEncerramento(0),
      nomeEmpresa(""),
      vizinhancaAviso(0),
      horaSimulada(0),
      minutoSimulado(0),
      fabricaActiva(true),
      turnoAberto(false)
{
    verde.minimo = 0.0;
    verde.maximo = 0.0;
    amarelo.minimo = 0.0;
    amarelo.maximo = 0.0;
    vermelho.minimo = 0.0;
    vermelho.maximo = 0.0;

    if (ut != NULL)
    {
        utilizadores.push_back(ut);
    }
}

Fabrica::~Fabrica()
{
    apagarMotores(motores);
    apagarSensores(sensores);
    apagarUtilizadores(utilizadores);
    utilizadorAtual = NULL;
}

bool Fabrica::Load(const string& ficheiro)
{
    if (!motores.empty() || !sensores.empty())
    {
        return false;
    }

    list<string> linhas;
    if (!lerLinhas(ficheiro, linhas))
    {
        return false;
    }

    definicoesMotores.clear();

    string nome;
    int novoX = 0;
    int novoY = 0;
    int novaAbertura = 0;
    int novoEncerramento = 0;
    int novaVizinhanca = 0;
    IntervaloTemperatura novoVerde = {0.0, 0.0};
    IntervaloTemperatura novoAmarelo = {0.0, 0.0};
    IntervaloTemperatura novoVermelho = {0.0, 0.0};
    list<Motor*> novosMotores;
    list<Sensor*> novosSensores;

    if (!lerDados(linhas, nome, novoX, novoY, novaAbertura, novoEncerramento, novaVizinhanca,
                  novoVerde, novoAmarelo, novoVermelho, novosMotores, novosSensores))
    {
        definicoesMotores.clear();
        list<User*> vazia;
        libertarCargaParcial(vazia, novosMotores, novosSensores);
        return false;
    }

    nomeEmpresa = nome;
    campoX = novoX;
    campoY = novoY;
    horaAbertura = novaAbertura;
    horaEncerramento = novoEncerramento;
    vizinhancaAviso = novaVizinhanca;
    verde = novoVerde;
    amarelo = novoAmarelo;
    vermelho = novoVermelho;
    horaSimulada = horaAbertura;
    minutoSimulado = 0;
    fabricaActiva = true;
    turnoAberto = false;
    paragemManual.clear();

    moverMotores(novosMotores);
    moverSensores(novosSensores);
    return true;
}

bool Fabrica::Add(User* ut)
{
    if (ut == NULL || !podeAdicionar())
    {
        return false;
    }

    if (ut->GetId() <= 0 || ut->GetNome().empty())
    {
        return false;
    }

    if (idUtilizadorExiste(utilizadores, ut->GetId()))
    {
        return false;
    }

    utilizadores.push_back(ut);
    return true;
}

bool Fabrica::Add(Motor* m)
{
    if (m == NULL || !podeAdicionar())
    {
        return false;
    }

    if (m->GetId() <= 0 || m->GetTipo().empty() || m->GetMarca().empty())
    {
        return false;
    }

    if (!posicaoValida(m->GetX(), m->GetY(), campoX, campoY))
    {
        return false;
    }

    if (idMotorExiste(motores, m->GetId()))
    {
        return false;
    }

    motores.push_back(m);
    return true;
}

void Fabrica::Listar(ostream& f) const
{
    if (!podeListar())
    {
        return;
    }

    f << "=== " << nomeEmpresa << " ===" << endl;
    f << "Campo: " << campoX << " x " << campoY << endl;
    f << "Horario: " << horaAbertura << " - " << horaEncerramento << endl;
    f << "Hora simulada: " << horaSimulada << ":";
    if (minutoSimulado < 10)
    {
        f << "0";
    }
    f << minutoSimulado << endl;
    f << "Vizinhanca de aviso: " << vizinhancaAviso << endl;

    if (utilizadorAtual != NULL)
    {
        f << "Utilizador actual: " << utilizadorAtual->GetNome()
          << " (" << utilizadorAtual->GetTipo() << ")" << endl;
    }

    f << endl
      << "Utilizadores:" << endl;
    for (list<User*>::const_iterator it = utilizadores.begin(); it != utilizadores.end(); ++it)
    {
        User* u = *it;
        f << "- " << u->GetId() << " " << u->GetNome() << " [" << u->GetTipo() << "]" << endl;
    }

    f << endl
      << "Motores:" << endl;
    for (list<Motor*>::const_iterator it = motores.begin(); it != motores.end(); ++it)
    {
        Motor* m = *it;
        f << "- id " << m->GetId()
          << " | " << m->GetTipo()
          << " | " << m->GetMarca()
          << " | (" << m->GetX() << ", " << m->GetY() << ")"
          << " | " << textoEstado(m->GetEstado())
          << " | " << textoClasse(m->GetClasseTemperatura())
          << " | temp " << m->GetTemperaturaAtual()
          << " | aviso " << m->GetTemperaturaAviso()
          << " | paragem " << m->GetTemperaturaParagem()
          << " | consumo/h " << m->GetConsumoHora()
          << " | consumo dia " << m->GetConsumoDia()
          << " | trabalho " << m->GetTempoTrabalho()
          << " | avarias " << m->GetTotalAvarias()
          << endl;
    }

    f << endl
      << "Sensores:" << endl;
    for (list<Sensor*>::const_iterator it = sensores.begin(); it != sensores.end(); ++it)
    {
        Sensor* s = *it;
        f << "- id " << s->GetId()
          << " | " << s->GetTipo()
          << " | " << s->GetMarca()
          << " | (" << s->GetX() << ", " << s->GetY() << ")"
          << " | valor " << s->GetValor()
          << " | limiar " << s->GetLimiar();

        if (s->GetProbabilidadeUm() > 0.0)
        {
            f << " | prob. evento " << s->GetProbabilidadeUm();
        }

        if (s->EstaAvariado())
        {
            f << " | AVARIADO";
        }
        else if (s->EmAviso())
        {
            f << " | aviso";
        }
        else
        {
            f << " | normal";
        }

        f << endl;
    }
}

void Fabrica::Desligar(int id_motor)
{
    if (!podeRun())
    {
        return;
    }

    Motor* motor = procurarMotor(id_motor);
    if (motor == NULL)
    {
        return;
    }

    marcarParagemManual(id_motor);
    motor->STOP();
}

ESTADO_MOTOR Fabrica::Get_Estado(int id_motor) const
{
    if (!podeListar())
    {
        return STOP;
    }

    Motor* motor = procurarMotor(id_motor);
    if (motor == NULL)
    {
        return STOP;
    }

    return motor->GetEstado();
}

list<Motor*>* Fabrica::Listar_Tipo(string Tipo, ostream& f) const
{
    if (!podeListar())
    {
        return NULL;
    }

    list<Motor*>* resultado = new list<Motor*>();

    for (list<Motor*>::const_iterator it = motores.begin(); it != motores.end(); ++it)
    {
        Motor* m = *it;
        if (m->GetTipo() == Tipo)
        {
            resultado->push_back(m);
            f << m->GetId() << " " << m->GetMarca() << " " << textoEstado(m->GetEstado()) << endl;
        }
    }

    return resultado;
}

bool Fabrica::Manutencao()
{
    if (!podeManutencao() || !fabricaActiva)
    {
        return false;
    }

    for (list<Motor*>::const_iterator it = motores.begin(); it != motores.end(); ++it)
    {
        map<string, DefinicaoTipoMotor>::const_iterator def =
            definicoesMotores.find((*it)->GetTipo());

        if (def == definicoesMotores.end() ||
            def->second.verde.maximo < def->second.verde.minimo)
        {
            return false;
        }
    }

    for (list<Motor*>::iterator it = motores.begin(); it != motores.end(); ++it)
    {
        Motor* motor = *it;

        map<string, DefinicaoTipoMotor>::const_iterator def =
            definicoesMotores.find(motor->GetTipo());

        double minimo = def->second.verde.minimo;
        double maximo = def->second.verde.maximo;

        double fracao = (rand() % 1001) / 1000.0;

        double escolha = minimo + fracao * (maximo - minimo);

        motor->Reparar();

        motor->SetTemperaturaAtual(escolha);

    }

    for (list<Sensor*>::iterator it = sensores.begin(); it != sensores.end(); ++it)
    {
        (*it)->Repor();
    }

    avisarGestor("Manutencao concluida. Motores em temperatura verde e sensores repostos.");

    return true;
}

list<string>* Fabrica::Ranking_Dos_Fracos() const
{
    if (!podeListar())
    {
        return NULL;
    }

    map<string, int> totais;
    acumularAvarias(totais);

    list<MarcaAvarias> ordenada;
    for (map<string, int>::const_iterator it = totais.begin(); it != totais.end(); ++it)
    {
        inserirMarca(ordenada, it->first, it->second);
    }

    list<string>* ranking = new list<string>();
    for (list<MarcaAvarias>::const_iterator it = ordenada.begin(); it != ordenada.end(); ++it)
    {
        ranking->push_back(it->marca);
    }

    return ranking;
}

list<Motor*>* Fabrica::Ranking_Dos_Mais_Trabalhadores() const
{
    if (!podeListar())
    {
        return NULL;
    }

    list<Motor*>* ranking = new list<Motor*>();

    for (list<Motor*>::const_iterator it = motores.begin(); it != motores.end(); ++it)
    {
        inserirPorTrabalho(*ranking, *it);
    }

    return ranking;
}

/**
 * @brief Escreve um numero sem zeros inuteis a direita.
 * @param valor Numero a converter.
 * @return Texto compacto, por exemplo 0.1 ou 80.
 */
static string textoNumero(double valor)
{
    if (valor < 0.0)
    {
        return "-" + textoNumero(-valor);
    }

    ostringstream fluxo;
    fluxo << fixed << setprecision(6) << valor;
    string texto = fluxo.str();
    size_t ponto = texto.find('.');
    if (ponto == string::npos)
    {
        return texto;
    }

    size_t fim = texto.size();
    while (fim > ponto && texto[fim - 1] == '0')
    {
        fim--;
    }
    if (fim > ponto && texto[fim - 1] == '.')
    {
        fim--;
    }
    return texto.substr(0, fim);
}

/**
 * @brief Etiqueta de sensor usada no XML exportado.
 * @param tipo Nome interno do sensor.
 * @return SHUMIDADE, SLUZ, SMISSIL ou a etiqueta original.
 */
static string etiquetaSensorExportada(const string& tipo)
{
    if (tipo == "HUMIDADE")
    {
        return "SHUMIDADE";
    }

    if (tipo == "LUZ")
    {
        return "SLUZ";
    }

    if (tipo == "MISSEL" || tipo == "SMISSEL")
    {
        return "SMISSEL";
    }

    if (tipo == "FUMO" || tipo == "SFOGO")
    {
        return "SFOGO";
    }

    return tipo;
}

/**
 * @brief Probabilidade do sensor no estilo do XML exportado.
 *
 * Uma fracao entre 0 e 1 volta a percentagem inteira: 0.05 fica 5.
 * @param fracao Probabilidade guardada no sensor.
 */
static string textoProbabilidadeSensor(double fracao)
{
    if (fracao >= 0.0 && fracao <= 1.0)
    {
        return textoNumero(fracao * 100.0);
    }
    return textoNumero(fracao);
}

void Fabrica::Relatorio(string fich_xml) const
{
    if (!podeListar() || fich_xml.empty())
    {
        return;
    }

    ofstream saida(fich_xml.c_str());
    if (!saida.is_open())
    {
        return;
    }

    saida << "<DADOS>" << endl;
    saida << "<DEFINICOES>" << endl;
    saida << "<NOME_EMPRESA>" << escaparXml(nomeEmpresa) << "</NOME_EMPRESA>" << endl;
    saida << "<HORA_INICIO>" << horaAbertura << "</HORA_INICIO>" << endl;
    saida << "<HORA_FECHO>" << horaEncerramento << "</HORA_FECHO>" << endl;
    saida << "<VIZINHANCA_AVISO>" << vizinhancaAviso << "</VIZINHANCA_AVISO>" << endl;
    saida << "<DIMENSAO_FABRICA>" << campoX << "," << campoY << "</DIMENSAO_FABRICA>" << endl;

    string ordemTipos[3];
    ordemTipos[0] = "MELETRICO";
    ordemTipos[1] = "MCOMBUSTAO";
    ordemTipos[2] = "MINDUCAO";

    for (int i = 0; i < 3; i++)
    {
        map<string, DefinicaoTipoMotor>::const_iterator def = definicoesMotores.find(ordemTipos[i]);
        if (def == definicoesMotores.end())
        {
            continue;
        }

        saida << "<" << def->first << ">" << endl;
        saida << "<VERDE>" << textoNumero(def->second.verde.minimo) << ", " << textoNumero(def->second.verde.maximo) << "</VERDE>" << endl;
        saida << "<AMARELO>" << textoNumero(def->second.amarelo.minimo) << ", " << textoNumero(def->second.amarelo.maximo) << "</AMARELO>" << endl;
        saida << "<VERMELHO>" << textoNumero(def->second.vermelho.minimo) << ", " << textoNumero(def->second.vermelho.maximo) << "</VERMELHO>" << endl;
        saida << "<PROB_AVARIA>" << textoNumero(def->second.probabilidade) << "</PROB_AVARIA>" << endl;
        saida << "</" << def->first << ">" << endl;
    }

    for (map<string, DefinicaoTipoMotor>::const_iterator def = definicoesMotores.begin(); def != definicoesMotores.end(); ++def)
    {
        if (def->first == "MELETRICO" || def->first == "MCOMBUSTAO" || def->first == "MINDUCAO")
        {
            continue;
        }

        saida << "<" << def->first << ">" << endl;
        saida << "<VERDE>" << textoNumero(def->second.verde.minimo) << ", " << textoNumero(def->second.verde.maximo) << "</VERDE>" << endl;
        saida << "<AMARELO>" << textoNumero(def->second.amarelo.minimo) << ", " << textoNumero(def->second.amarelo.maximo) << "</AMARELO>" << endl;
        saida << "<VERMELHO>" << textoNumero(def->second.vermelho.minimo) << ", " << textoNumero(def->second.vermelho.maximo) << "</VERMELHO>" << endl;
        saida << "<PROB_AVARIA>" << textoNumero(def->second.probabilidade) << "</PROB_AVARIA>" << endl;
        saida << "</" << def->first << ">" << endl;
    }

    saida << "</DEFINICOES>" << endl;
    saida << "<MOTORES>" << endl;

    for (list<Motor*>::const_iterator it = motores.begin(); it != motores.end(); ++it)
    {
        Motor* motor = *it;
        string tipo = motor->GetTipo();
        saida << "<" << tipo << ">" << endl;
        saida << "<ID>" << motor->GetId() << "</ID>" << endl;
        saida << "<MARCA>" << escaparXml(motor->GetMarca()) << "</MARCA>" << endl;
        saida << "<VALOR_TEMP_ATUAL>" << textoNumero(motor->GetTemperaturaAtual()) << "</VALOR_TEMP_ATUAL>" << endl;
        saida << "<ESTADO_ATUAL>" << textoEstado(motor->GetEstado()) << "</ESTADO_ATUAL>" << endl;
        saida << "<ESTADO_COR>" << textoClasse(motor->GetClasseTemperatura()) << "</ESTADO_COR>" << endl;
        saida << "<CONSUMO_HORA>" << textoNumero(motor->GetConsumoHora()) << "</CONSUMO_HORA>" << endl;
        saida << "<CONSUMO_DIA>" << textoNumero(motor->GetConsumoDia()) << "</CONSUMO_DIA>" << endl;
        saida << "<POSICAO>" << motor->GetX() << "," << motor->GetY() << "</POSICAO>" << endl;
        saida << "</" << tipo << ">" << endl;
    }

    double consumoTotalDia = 0.0;

    for (list<Motor*>::const_iterator it = motores.begin(); it != motores.end(); ++it)
    {
        consumoTotalDia += (*it)->GetConsumoDia();
    }

    saida << "</MOTORES>" << endl;

    saida << "<CONSUMO_TOTAL_DIA>"
         << textoNumero(consumoTotalDia)
        << "</CONSUMO_TOTAL_DIA>" << endl;

    saida << "<SENSORES>" << endl;
    

    for (list<Sensor*>::const_iterator it = sensores.begin(); it != sensores.end(); ++it)
    {
        Sensor* sensor = *it;
        string tipo = etiquetaSensorExportada(sensor->GetTipo());
        string valorAtual = textoNumero(sensor->GetValor());
        if (tipo == "SHUMIDADE")
        {
            valorAtual = valorAtual + "%";
        }

        saida << "<" << tipo << ">" << endl;
        saida << "<ID>" << sensor->GetId() << "</ID>" << endl;
        saida << "<MARCA>" << escaparXml(sensor->GetMarca()) << "</MARCA>" << endl;
        saida << "<VALOR_AVISO>" << textoNumero(sensor->GetLimiar()) << "</VALOR_AVISO>" << endl;
        saida << "<POSICAO>" << sensor->GetX() << "," << sensor->GetY() << "</POSICAO>" << endl;
        saida << "<PROB_AVARIA>" << textoProbabilidadeSensor(sensor->GetProbabilidadeAvaria()) << "</PROB_AVARIA>" << endl;
        saida << "<VALOR_ATUAL>" << valorAtual << "</VALOR_ATUAL>" << endl;
        saida << "</" << tipo << ">" << endl;
    }

    saida << "</SENSORES>" << endl;
    saida << "</DADOS>" << endl;
    saida.close();
}

int Fabrica::Aviso_Humidade(list<Motor*>& lm)
{
    lm.clear();

    int desligados = 0;
    for (list<Sensor*>::iterator sensorIt = sensores.begin(); sensorIt != sensores.end(); ++sensorIt)
    {
        Sensor* sensor = *sensorIt;
        if (sensor->EstaAvariado() ||
        sensor->GetTipo() != "HUMIDADE" ||
        !sensor->EmAviso())
        {
            continue;
        }

        int raio = sensor->GetRaioVizinhanca();
        if (raio <= 0)
        {
            raio = vizinhancaAviso;
        }

        for (list<Motor*>::iterator motorIt = motores.begin(); motorIt != motores.end(); ++motorIt)
        {
            Motor* motor = *motorIt;
            bool jaListado = false;
            for (list<Motor*>::iterator listaIt = lm.begin(); listaIt != lm.end(); ++listaIt)
            {
                if (*listaIt == motor)
                {
                    jaListado = true;
                    break;
                }
            }

            if (jaListado || !motorNaVizinhanca(motor, sensor, raio))
            {
                continue;
            }

            ESTADO_MOTOR estado = motor->GetEstado();
            if (estado == ::START || estado == ::RUN)
            {
                motor->STOP();
                lm.push_back(motor);
                desligados = desligados + 1;
            }
        }
    }

    if (desligados > 0)
    {
        ostringstream texto;
        texto << "Humidade em aviso. Motores desligados na vizinhanca: " << desligados;
        avisarGestor(texto.str());
    }

    return desligados;
}

/*int Fabrica::Aviso_Fumo(list<Motor*>& lm, string fich_video)
{
    lm.clear();

    if (fich_video.empty())
    {
        return 0;
    }

    int desligados = 0;
    for (list<Motor*>::iterator it = motores.begin(); it != motores.end(); ++it)
    {
        Motor* motor = *it;
        ESTADO_MOTOR estado = motor->GetEstado();
        if (estado == ::START || estado == ::RUN)
        {
            motor->STOP();
            lm.push_back(motor);
            desligados = desligados + 1;
        }
    }

    ostringstream video;
    video << "VIDEO: saidas de emergencia" << endl;
    video << "Empresa: " << nomeEmpresa << endl;
    video << "Dirija-se a saida mais proxima." << endl;
    video << "Campo: " << campoX << " x " << campoY << endl;
    video << "Saida norte: (0, 0)" << endl;
    video << "Saida sul: (" << campoX << ", " << campoY << ")" << endl;
    video << "Motores desligados: " << desligados << endl;
    gravarTexto(fich_video, video.str());

    ostringstream texto;
    texto << "Fumo em aviso. Todos os motores foram desligados. Video: " << fich_video;
    avisarGestor(texto.str());
    return desligados;
}*/

int Fabrica::Aviso_Fumo(list<Motor*>& lm, string fich_video)
{
    lm.clear();

    int desligados = 0;

    for (list<Motor*>::iterator it = motores.begin(); it != motores.end(); ++it)
    {
        Motor* motor = *it;
        ESTADO_MOTOR estado = motor->GetEstado();

        if (estado == ::START || estado == ::RUN)
        {
            motor->STOP();

            if (!paragemManualContem(motor->GetId()))
            {
                paragemManual.push_back(motor->GetId());
            }

            lm.push_back(motor);
            desligados++;
        }
    }

    bool videoGravado = false;

    if (!fich_video.empty())
    {
        ostringstream video;

        video << "VIDEO: saidas de emergencia" << endl;
        video << "Empresa: " << nomeEmpresa << endl;
        video << "Dirija-se a saida mais proxima." << endl;
        video << "Campo: " << campoX << " x " << campoY << endl;
        video << "Saida norte: (0, 0)" << endl;
        video << "Saida sul: (" << campoX << ", " << campoY << ")" << endl;
        video << "Motores desligados: " << desligados << endl;

        videoGravado = gravarTexto(fich_video, video.str());
    }

    ostringstream texto;

    texto << "Fumo detetado. Paragem de seguranca executada. ";
    texto << "Motores desligados: " << desligados << ".";

    if (fich_video.empty())
    {
        texto << " Ficheiro informativo nao indicado.";
    }
    else if (!videoGravado)
    {
        texto << " Nao foi possivel gravar: " << fich_video << ".";
    }
    else
    {
        texto << " Informacao guardada em: " << fich_video << ".";
    }

    avisarGestor(texto.str());

    return desligados;
}

int Fabrica::Aviso_Luz(string fich_video)
{
    if (fich_video.empty())
    {
        return 0;
    }

    ostringstream video;
    video << "VIDEO: regresso a casa" << endl;
    video << "A luz baixou e o encerramento aproxima-se." << endl;
    video << "Bom regresso a casa." << endl;
    if (!gravarTexto(fich_video, video.str()))
    {
        return 0;
    }

    ostringstream texto;
    texto << "Luz em aviso. Video: " << fich_video;
    avisarGestor(texto.str());
    return 1;
}

/*void Fabrica::Aviso_Missel(string fvideo, string festado)
{
    if (fvideo.empty() || festado.empty())
    {
        return;
    }

    ostringstream video;
    video << "VIDEO: abrigos" << endl;
    video << "Missel detetado na direcao da fabrica." << endl;
    video << "Dirija-se ao abrigo mais proximo e aguarde." << endl;
    gravarTexto(fvideo, video.str());

    ofstream estado(festado.c_str());
    if (estado.is_open())
    {
        Listar(estado);
        estado.close();
    }

    for (list<Motor*>::iterator it = motores.begin(); it != motores.end(); ++it)
    {
        (*it)->STOP();
    }

    fabricaActiva = false;

    ostringstream texto;
    texto << "Missel em aviso. Fabrica desligada. Video: " << fvideo << ". Estado: " << festado;
    avisarGestor(texto.str());
}*/
    void Fabrica::Aviso_Missel(string fvideo, string festado)
    {
        for (list<Motor*>::iterator it = motores.begin(); it != motores.end(); ++it)
        {
            (*it)->STOP();
        }

        fabricaActiva = false;

        bool videoGravado = false;

        if (!fvideo.empty())
        {
            ostringstream video;
            video << "VIDEO: abrigos" << endl;
            video << "Missel detetado na direcao da fabrica." << endl;
            video << "Dirija-se ao abrigo mais proximo e aguarde." << endl;

            videoGravado = gravarTexto(fvideo, video.str());
        }

        bool estadoGravado = false;

        if (!festado.empty())
        {
            ofstream estado(festado.c_str());

            if (estado.is_open())
            {
                estado << "=== " << nomeEmpresa << " ===" << endl;
                estado << "Estado da fabrica: INATIVA" << endl;
                estado << "Campo: " << campoX << " x " << campoY << endl;
                estado << "Horario: " << horaAbertura << " - " << horaEncerramento << endl;
                estado << "Hora simulada: " << horaSimulada << ":";

                if (minutoSimulado < 10)
                {
                    estado << "0";
                }

                estado << minutoSimulado << endl;
                estado << "Vizinhanca de aviso: " << vizinhancaAviso << endl;

                estado << endl << "Motores:" << endl;

                for (list<Motor*>::const_iterator it = motores.begin(); it != motores.end(); ++it)
                {
                    Motor* motor = *it;

                    estado << "- id " << motor->GetId()
                        << " | " << motor->GetTipo()
                        << " | " << motor->GetMarca()
                        << " | (" << motor->GetX() << ", " << motor->GetY() << ")"
                        << " | " << textoEstado(motor->GetEstado())
                        << " | " << textoClasse(motor->GetClasseTemperatura())
                        << " | temp " << motor->GetTemperaturaAtual()
                        << " | consumo/h " << motor->GetConsumoHora()
                        << " | consumo dia " << motor->GetConsumoDia()
                        << " | trabalho " << motor->GetTempoTrabalho()
                        << " | avarias " << motor->GetTotalAvarias()
                        << endl;
                }

                estado << endl << "Sensores:" << endl;

                for (list<Sensor*>::const_iterator it = sensores.begin(); it != sensores.end(); ++it)
                {
                    Sensor* sensor = *it;

                    estado << "- id " << sensor->GetId()
                        << " | " << sensor->GetTipo()
                        << " | " << sensor->GetMarca()
                        << " | (" << sensor->GetX() << ", " << sensor->GetY() << ")"
                        << " | valor " << sensor->GetValor()
                        << " | limiar " << sensor->GetLimiar()
                        << " | aviso " << (sensor->EmAviso() ? "sim" : "nao")
                        << endl;
                }

                estado.close();
                estadoGravado = !estado.fail();
            }
        }

        ostringstream texto;

        texto << "Missil detetado. Paragem de seguranca executada e fabrica inativa.";

        if (fvideo.empty())
        {
            texto << " Ficheiro de video nao indicado.";
        }
        else if (!videoGravado)
        {
            texto << " Nao foi possivel gravar o ficheiro de video: " << fvideo << ".";
        }

        if (festado.empty())
        {
            texto << " Caminho do estado nao indicado.";
        }
        else if (estadoGravado)
        {
            texto << " Estado guardado em: " << festado << ".";
        }
        else
        {
            texto << " Nao foi possivel guardar o estado em: " << festado << ".";
        }

        avisarGestor(texto.str());
    }


void Fabrica::Listagem_HTML(string fich_html) const
{
    if (!podeListar() || fich_html.empty())
    {
        return;
    }

    ofstream saida(fich_html.c_str());
    if (!saida.is_open())
    {
        return;
    }

    saida << "<!DOCTYPE html>" << endl;
    saida << "<html><head><meta charset=\"UTF-8\"><title>";
    saida << escaparXml(nomeEmpresa) << "</title></head><body>" << endl;
    saida << "<h1>" << escaparXml(nomeEmpresa) << "</h1>" << endl;
    saida << "<p>Campo " << campoX << " x " << campoY;
    saida << ". Hora simulada " << horaSimulada << ":";
    if (minutoSimulado < 10)
    {
        saida << "0";
    }
    saida << minutoSimulado << ".</p>" << endl;
    saida << "<h2>Motores</h2><table border=\"1\">" << endl;
    saida << "<tr><th>ID</th><th>Tipo</th><th>Marca</th><th>Estado</th><th>Temperatura</th><th>Consumo do dia</th></tr>" << endl;

    saida << fixed << setprecision(2);
    for (list<Motor*>::const_iterator it = motores.begin(); it != motores.end(); ++it)
    {
        Motor* motor = *it;
        saida << "<tr><td>" << motor->GetId() << "</td><td>" << escaparXml(motor->GetTipo());
        saida << "</td><td>" << escaparXml(motor->GetMarca()) << "</td><td>" << textoEstado(motor->GetEstado());
        saida << "</td><td>" << motor->GetTemperaturaAtual() << "</td><td>" << motor->GetConsumoDia();
        saida << "</td></tr>" << endl;
    }

    saida << "</table><h2>Sensores</h2><table border=\"1\">" << endl;
    saida << "<tr><th>ID</th><th>Tipo</th><th>Marca</th><th>Valor</th><th>Aviso</th></tr>" << endl;
    for (list<Sensor*>::const_iterator it = sensores.begin(); it != sensores.end(); ++it)
    {
        Sensor* sensor = *it;
        saida << "<tr><td>" << sensor->GetId() << "</td><td>" << escaparXml(sensor->GetTipo());
        saida << "</td><td>" << escaparXml(sensor->GetMarca()) << "</td><td>" << sensor->GetValor();
        saida << "</td><td>" << (sensor->EmAviso() ? "SIM" : "NAO") << "</td></tr>" << endl;
    }

    saida << "</table></body></html>" << endl;
    saida.close();
}

int Fabrica::GetCampoX() const
{
    return campoX;
}

int Fabrica::GetCampoY() const
{
    return campoY;
}

bool Fabrica::CriarMotor(int id, const string& tipo, const string& marca, double consumo, int x, int y)
{
    if (tipo.empty() || marca.empty() || consumo < 0.0)
    {
        return false;
    }

    map<string, DefinicaoTipoMotor>::const_iterator definicao = definicoesMotores.find(tipo);
    if (definicao == definicoesMotores.end())
    {
        return false;
    }

    Motor* motor = NULL;
    if (tipo == "MELETRICO")
    {
        motor = new MotorEletrico(id, marca, consumo, definicao->second.amarelo.minimo,
                                  definicao->second.vermelho.minimo, definicao->second.probabilidade, x, y);
    }
    else if (tipo == "MCOMBUSTAO")
    {
        motor = new MotorCombustao(id, marca, consumo, definicao->second.amarelo.minimo,
                                   definicao->second.vermelho.minimo, definicao->second.probabilidade, x, y);
    }
    else if (tipo == "MINDUCAO")
    {
        motor = new MotorInducao(id, marca, consumo, definicao->second.amarelo.minimo,
                                 definicao->second.vermelho.minimo, definicao->second.probabilidade, x, y);
    }

    if (motor == NULL)
    {
        return false;
    }
    if (!Add(motor))
    {
        delete motor;
        return false;
    }

    return true;
}

bool Fabrica::RemoverUtilizador(int id)
{
    if (!podeRemover() || utilizadorAtual == NULL || utilizadorAtual->GetId() == id)
    {
        return false;
    }

    for (list<User*>::iterator it = utilizadores.begin(); it != utilizadores.end(); ++it)
    {
        if ((*it)->GetId() == id)
        {
            delete *it;
            utilizadores.erase(it);
            return true;
        }
    }

    return false;
}

bool Fabrica::RemoverMotor(int id)
{
    if (!podeRemover())
    {
        return false;
    }

    for (list<Motor*>::iterator it = motores.begin(); it != motores.end(); ++it)
    {
        if ((*it)->GetId() == id)
        {
            delete *it;
            motores.erase(it);
            limparParagemManual(id);
            return true;
        }
    }

    return false;
}

bool Fabrica::RemoverSensor(int id, const string& tipo)
{
    if (!podeRemover())
    {
        return false;
    }

    for (list<Sensor*>::iterator it = sensores.begin(); it != sensores.end(); ++it)
    {
        if ((*it)->GetId() == id && (*it)->GetTipo() == tipo)
        {
            delete *it;
            sensores.erase(it);
            return true;
        }
    }

    return false;
}




bool Fabrica::Ligar(int id)
{
    if (!podeRun() || !fabricaActiva)
    {
        return false;
    }

    Motor* motor = procurarMotor(id);

    if (motor == NULL)
    {
        return false;
    }

    if (!fabricaAberta() || fumoEmAviso() || humidadeBloqueia(motor))
    {
        return false;
    }

    if (motor->EstaBloqueadoPorTemperatura())
    {
        return false;
    }

    if (motor->GetEstado() == AVARIADO)
    {
        motor->RESTART();
    }
    else if (motor->GetEstado() == ::STOP)
    {
        motor->START();
    }

    if (motor->GetEstado() == ::START || motor->GetEstado() == ::RUN)
    {
        limparParagemManual(id);
        return true;
    }

    return false;
}


void Fabrica::processarAlertas()
{
    bool alertaHumidade = false;
    bool alertaFumo = false;
    bool alertaLuz = false;
    bool alertaMissel = false;

    for (list<Sensor*>::iterator it = sensores.begin(); it != sensores.end(); ++it)
    {
        Sensor* sensor = *it;

        if (sensor->EstaAvariado())
        {
            continue;
        }

        if (!sensor->Disparou())
        {
            continue;
        }

        string tipo = sensor->GetTipo();

        if (tipo == "HUMIDADE")
        {
            alertaHumidade = true;
        }
        else if (tipo == "SFOGO" || tipo == "FUMO")
        {
            alertaFumo = true;
        }
        else if (tipo == "LUZ")
        {
            alertaLuz = true;
        }
        else if (tipo == "SMISSEL")
        {
            alertaMissel = true;
        }
        else if (tipo == "STEMPERATURA")
        {
            ostringstream texto;

            texto << "Temperatura em aviso no sensor id "
                  << sensor->GetId()
                  << ". Valor atual: "
                  << sensor->GetValor();

            avisarGestor(texto.str());
        }
    }

    if (alertaMissel)
    {
        Aviso_Missel("Video_Missel.txt", "Estado.txt");
        return;
    }

    if (alertaFumo)
    {
        list<Motor*> motoresDesligados;
        Aviso_Fumo(motoresDesligados, "Video_Fumo.txt");
    }

    if (alertaHumidade)
    {
        list<Motor*> motoresDesligados;
        Aviso_Humidade(motoresDesligados);
    }

    if (alertaLuz)
    {
        Aviso_Luz("Video_Luz.txt");
    }
}


void Fabrica::AvancarMinuto()
{
    if (!fabricaActiva)
    {
        return;
    }

    bool aberta = fabricaAberta();

    if (aberta && !turnoAberto)
    {
        paragemManual.clear();
        turnoAberto = true;
    }

    if (!aberta)
    {
        turnoAberto = false;
    }

    bool motoresEmMarcha = false;
    bool fumo = fumoEmAviso();

    for (list<Motor*>::iterator it = motores.begin(); it != motores.end(); ++it)
    {
        Motor* motor = *it;

        if (motor->EstaBloqueadoPorTemperatura())
        {
            motor->Arrefecer(1.0);
            continue;
        }

        if (motor->AguardaArranqueManual())
        {
            continue;
        }

        if (!aberta || paragemManualContem(motor->GetId()) ||
            fumo || humidadeBloqueia(motor))
        {
            ESTADO_MOTOR estado = motor->GetEstado();

            if (estado == ::START || estado == ::RUN)
            {
                motor->STOP();
            }

            continue;
        }

        if (motor->GetEstado() == AVARIADO || motor->EstaAvariado())
        {
            continue;
        }

        if (motor->GetEstado() == ::STOP)
        {
            motor->START();
        }
        else if (motor->GetEstado() == ::START)
        {
            motor->RUN();
        }

        if (motor->GetEstado() != ::RUN)
        {
            continue;
        }

        double sorteio = (rand() % 10000) / 10000.0;

        if (sorteio < motor->GetProbabilidadeAvaria() / 60.0)
        {
            if (motor->Avariar())
            {
                ostringstream texto;

                texto << "Motor id " << motor->GetId()
                      << " (" << motor->GetMarca() << ") avariou.";

                avisarGestor(texto.str());
            }

            continue;
        }

        // 7. Registar trabalho e aquecimento.
        motor->RegistarTrabalho(1);
        motoresEmMarcha = true;

        CLASSE_TEMPERATURA antes = motor->GetClasseTemperatura();

        motor->AplicarAquecimento();

        CLASSE_TEMPERATURA depois = motor->ESTOU_QUENTE();

        if (depois == VERMELHO && antes != VERMELHO)
        {
            ostringstream texto;

            texto << "Motor id " << motor->GetId()
                  << " em VERMELHO. Foi parado.";

            avisarGestor(texto.str());
        }
        else if (depois == AMARELO && antes == VERDE)
        {
            ostringstream texto;

            texto << "Motor id " << motor->GetId()
                  << " em AMARELO. Esta a aquecer.";

            avisarGestor(texto.str());
        }
    }

    actualizarSensores(motoresEmMarcha);

    processarAlertas();

    if (!fabricaActiva)
    {
        return;
    }

    minutoSimulado = minutoSimulado + 1;

    if (minutoSimulado < 60)
    {
        return;
    }

    minutoSimulado = 0;

    horaSimulada = horaSimulada + 1;

    if (horaSimulada < 24)
    {
        return;
    }

    horaSimulada = 0;

    // 10. Reiniciar consumo diario.
    for (list<Motor*>::iterator it = motores.begin(); it != motores.end(); ++it)
    {
        (*it)->ReiniciarConsumoDia();
    }
}


bool Fabrica::EstaActiva() const
{
    return fabricaActiva;
}

int Fabrica::GetHoraSimulada() const
{
    return horaSimulada;
}

int Fabrica::GetMinutoSimulado() const
{
    return minutoSimulado;
}

string Fabrica::GetNomeEmpresa() const
{
    return nomeEmpresa;
}

bool Fabrica::podeAdicionar() const
{
    if (utilizadorAtual == NULL)
    {
        return false;
    }

    return utilizadorAtual->PodeAdicionar();
}

bool Fabrica::podeRemover() const
{
    if (utilizadorAtual == NULL)
    {
        return false;
    }

    return utilizadorAtual->PodeRemover();
}

bool Fabrica::podeListar() const
{
    if (utilizadorAtual == NULL)
    {
        return false;
    }

    return utilizadorAtual->PodeListar();
}

bool Fabrica::podeRun() const
{
    if (utilizadorAtual == NULL)
    {
        return false;
    }

    return utilizadorAtual->PodeRun();
}

bool Fabrica::podeManutencao() const
{
    if (utilizadorAtual == NULL)
    {
        return false;
    }

    return utilizadorAtual->PodeManutencao();
}

bool Fabrica::fabricaAberta() const
{
    return horaSimulada >= horaAbertura && horaSimulada < horaEncerramento;
}

bool Fabrica::paragemManualContem(int id) const
{
    for (list<int>::const_iterator it = paragemManual.begin(); it != paragemManual.end(); ++it)
    {
        if (*it == id)
        {
            return true;
        }
    }

    return false;
}

void Fabrica::marcarParagemManual(int id)
{
    if (!paragemManualContem(id))
    {
        paragemManual.push_back(id);
    }
}

void Fabrica::limparParagemManual(int id)
{
    for (list<int>::iterator it = paragemManual.begin(); it != paragemManual.end(); ++it)
    {
        if (*it == id)

        {
            paragemManual.erase(it);
            return;
        }
    }
}

int Fabrica::distancia(int x1, int y1, int x2, int y2) const
{
    int eixoX = x1 - x2;
    int eixoY = y1 - y2;
    if (eixoX < 0)
    {
        eixoX = -eixoX;
    }
    if (eixoY < 0)
    {
        eixoY = -eixoY;
    }
    if (eixoX > eixoY)
    {
        return eixoX;
    }
    return eixoY;
}

bool Fabrica::motorNaVizinhanca(const Motor* motor, const Sensor* sensor, int raio) const
{
    if (motor == NULL || sensor == NULL || raio < 0)
    {
        return false;
    }

    return distancia(motor->GetX(), motor->GetY(), sensor->GetX(), sensor->GetY()) <= raio;
}

bool Fabrica::fumoEmAviso() const
{
    for (list<Sensor*>::const_iterator it = sensores.begin(); it != sensores.end(); ++it)
    {
        Sensor* sensor = *it;

        if (sensor->EstaAvariado())
        {
            continue;
        }

        string tipo = sensor->GetTipo();

        if ((tipo == "SFOGO" || tipo == "FUMO") && sensor->EmAviso())
        {
            return true;
        }
    }

    return false;
}
bool Fabrica::humidadeBloqueia(const Motor* motor) const
{
    for (list<Sensor*>::const_iterator it = sensores.begin(); it != sensores.end(); ++it)
    {
        Sensor* sensor = *it;

        if (sensor->EstaAvariado() ||
            sensor->GetTipo() != "HUMIDADE" ||
            !sensor->EmAviso())
        {
            continue;
        }

        int raio = sensor->GetRaioVizinhanca();

        if (raio <= 0)
        {
            raio = vizinhancaAviso;
        }

        if (motorNaVizinhanca(motor, sensor, raio))
        {
            return true;
        }
    }

    return false;
}

void Fabrica::avisarGestor(const string& mensagem) const
{
    cout << "GESTOR: " << mensagem << endl;
}

string Fabrica::escaparXml(const string& texto) const
{
    string resultado;
    for (size_t i = 0; i < texto.size(); i++)
    {
        char c = texto[i];
        if (c == '&')
        {
            resultado = resultado + "&amp;";
        }
        else if (c == '<')
        {
            resultado = resultado + "&lt;";
        }
        else if (c == '>')
        {
            resultado = resultado + "&gt;";
        }
        else if (c == '"')
        {
            resultado = resultado + "&quot;";
        }
        else
        {
            resultado = resultado + c;
        }
    }

    return resultado;
}


bool Fabrica::gravarTexto(const string& ficheiro, const string& conteudo) const
{
    ofstream saida(ficheiro.c_str());

    if (!saida.is_open())
    {
        return false;
    }

    saida << conteudo;

    if (!saida.good())
    {
        saida.close();
        return false;
    }

    saida.close();

    if (saida.fail())
    {
        return false;
    }

    return true;
}



void Fabrica::actualizarSensores(bool motoresEmMarcha)
{
    double fracao = 1.0 / 60.0;

    for (list<Sensor*>::iterator it = sensores.begin(); it != sensores.end(); ++it)
    {
        Sensor* sensor = *it;

        if (sensor->EstaAvariado())
        {
            continue;
        }

        if (sensor->GetTipo() == "STEMPERATURA")
        {
            double maisQuente = 0.0;
            bool achou = false;

            for (list<Motor*>::iterator motorIt = motores.begin();
                 motorIt != motores.end(); ++motorIt)
            {
                Motor* motor = *motorIt;

                if (!motorNaVizinhanca(motor, sensor, vizinhancaAviso))
                {
                    continue;
                }

                achou = true;

                if (motor->GetTemperaturaAtual() > maisQuente)
                {
                    maisQuente = motor->GetTemperaturaAtual();
                }
            }

            if (achou)
            {
                sensor->SetValor(maisQuente);
            }
        }

        if (!sensor->EstaAvariado())
        {
            double sorteio = (rand() % 10000) / 10000.0;

            if (sorteio < sensor->GetProbabilidadeAvaria() * fracao)
            {
                sensor->RegistarAvaria();

                ostringstream texto;

                texto << "Sensor " << sensor->GetTipo()
                      << " id " << sensor->GetId()
                      << " avariou.";

                avisarGestor(texto.str());

                continue;
            }
        }

        sensor->Actualizar(
            motoresEmMarcha,
            horaSimulada,
            horaAbertura,
            horaEncerramento,
            fracao
        );
    }
}

bool Fabrica::posicaoValida(int posX, int posY, int limiteX, int limiteY) const
{
    if (limiteX <= 0 || limiteY <= 0)
    {
        return false;
    }

    if (posX < 0 || posY < 0)
    {
        return false;
    }

    if (posX > limiteX || posY > limiteY)
    {
        return false;
    }

    return true;
}

bool Fabrica::horaValida(int hora) const
{
    return hora >= 0 && hora <= 23;
}

bool Fabrica::intervalosCoerentes(const IntervaloTemperatura& zonaVerde,
                                  const IntervaloTemperatura& zonaAmarela,
                                  const IntervaloTemperatura& zonaVermelha) const
{
    if (zonaVerde.minimo > zonaVerde.maximo)
    {
        return false;
    }

    if (zonaAmarela.minimo > zonaAmarela.maximo)
    {
        return false;
    }

    if (zonaVermelha.minimo > zonaVermelha.maximo)
    {
        return false;
    }

    if (zonaVerde.maximo >= zonaAmarela.minimo)
    {
        return false;
    }

    if (zonaAmarela.maximo >= zonaVermelha.minimo)
    {
        return false;
    }

    return true;
}

bool Fabrica::idUtilizadorExiste(const list<User*>& lista, int id) const
{
    for (list<User*>::const_iterator it = lista.begin(); it != lista.end(); ++it)
    {
        if ((*it)->GetId() == id)
        {
            return true;
        }
    }

    return false;
}

bool Fabrica::idMotorExiste(const list<Motor*>& lista, int id) const
{
    for (list<Motor*>::const_iterator it = lista.begin(); it != lista.end(); ++it)
    {
        if ((*it)->GetId() == id)
        {
            return true;
        }
    }

    return false;
}

bool Fabrica::idSensorExiste(const list<Sensor*>& lista, int id) const
{
    for (list<Sensor*>::const_iterator it = lista.begin(); it != lista.end(); ++it)
    {
        if ((*it)->GetId() == id)
        {
            return true;
        }
    }

    return false;
}

Motor* Fabrica::procurarMotor(int id) const
{
    for (list<Motor*>::const_iterator it = motores.begin(); it != motores.end(); ++it)
    {
        if ((*it)->GetId() == id)
        {
            return *it;
        }
    }

    return NULL;
}

void Fabrica::apagarUtilizadores(list<User*>& lista)
{
    for (list<User*>::iterator it = lista.begin(); it != lista.end(); ++it)
    {
        delete *it;
        *it = NULL;
    }

    lista.clear();
}

void Fabrica::apagarMotores(list<Motor*>& lista)
{
    for (list<Motor*>::iterator it = lista.begin(); it != lista.end(); ++it)
    {
        delete *it;
        *it = NULL;
    }

    lista.clear();
}

void Fabrica::apagarSensores(list<Sensor*>& lista)
{
    for (list<Sensor*>::iterator it = lista.begin(); it != lista.end(); ++it)
    {
        delete *it;
        *it = NULL;
    }

    lista.clear();
}

void Fabrica::moverUtilizadores(list<User*>& origem)
{
    for (list<User*>::iterator it = origem.begin(); it != origem.end(); ++it)
    {
        utilizadores.push_back(*it);
    }

    origem.clear();
}

void Fabrica::moverMotores(list<Motor*>& origem)
{
    for (list<Motor*>::iterator it = origem.begin(); it != origem.end(); ++it)
    {
        motores.push_back(*it);
    }

    origem.clear();
}

void Fabrica::moverSensores(list<Sensor*>& origem)
{
    for (list<Sensor*>::iterator it = origem.begin(); it != origem.end(); ++it)
    {
        sensores.push_back(*it);
    }

    origem.clear();
}

void Fabrica::libertarCargaParcial(list<User*>& users, list<Motor*>& mots, list<Sensor*>& sens)
{
    apagarUtilizadores(users);
    apagarMotores(mots);
    apagarSensores(sens);
}

bool Fabrica::lerLinhas(const string& ficheiro, list<string>& linhas) const
{
    ifstream entrada(ficheiro.c_str());

    if (!entrada.is_open())
    {
        return false;
    }

    string linha;

    while (getline(entrada, linha))
    {
        linha = limparLinha(linha);

        if (!linha.empty())
        {
            linhas.push_back(linha);
        }
    }

    bool leituraValida = entrada.eof() && !entrada.bad();

    entrada.close();

    return leituraValida;
}

string Fabrica::limparLinha(const string& linha) const
{
    size_t inicio = 0;
    size_t fim = linha.size();

    while (inicio < fim)
    {
        char c = linha[inicio];
        if (c != ' ' && c != '\t' && c != '\r')
        {
            break;
        }
        inicio++;
    }

    while (fim > inicio)
    {
        char c = linha[fim - 1];
        if (c != ' ' && c != '\t' && c != '\r')
        {
            break;
        }
        fim--;
    }

    return linha.substr(inicio, fim - inicio);
}

/** @brief Indica se a etiqueta e um tipo de motor do XML. */
static bool eTipoMotor(const string& tag)
{
    return tag == "MELETRICO" || tag == "MCOMBUSTAO" || tag == "MINDUCAO";
}

/** @brief Indica se a etiqueta e um tipo de sensor do XML. */
static bool eTipoSensor(const string& tag)
{
     return tag == "STEMPERATURA" || tag == "SFOGO" || tag == "SMISSEL" || tag == "SLUZ" || tag == "SHUMIDADE";
}

/**
 * @brief Reconhece uma linha que so tem uma etiqueta de abertura ou de fecho.
 * @param linha Linha ja limpa.
 * @param tag Fica com o nome da etiqueta.
 * @param fecho true para procurar um fecho.
 * @return false se a linha tiver texto ou outra etiqueta.
 */
static bool tagSimples(const string& linha, string& tag, bool fecho)
{
    if (linha.size() < 3 || linha[0] != '<' || linha[linha.size() - 1] != '>')
    {
        return false;
    }

    if (linha.find('>') != linha.size() - 1)
    {
        return false;
    }

    size_t inicio = 1;
    if (fecho)
    {
        if (linha.size() < 4 || linha[1] != '/')
        {
            return false;
        }
        inicio = 2;
    }
    else if (linha[1] == '/' || linha[1] == '?' || linha[1] == '!')
    {
        return false;
    }

    tag = linha.substr(inicio, linha.size() - 1 - inicio);
    if (tag.empty() || tag.find(' ') != string::npos)
    {
        return false;
    }

    return true;
}

/**
 * @brief Le um elemento com texto na mesma linha.
 * @param linha Linha ja limpa.
 * @param tag Fica com o nome da etiqueta.
 * @param texto Fica com o conteudo.
 * @return false se a linha nao for um elemento completo.
 */
static bool elementoTexto(const string& linha, string& tag, string& texto)
{
    if (linha.size() < 5 || linha[0] != '<' || linha[1] == '/' || linha[1] == '?' || linha[1] == '!')
    {
        return false;
    }

    size_t fimAbertura = linha.find('>');
    if (fimAbertura == string::npos || fimAbertura < 2 || fimAbertura == linha.size() - 1)
    {
        return false;
    }

    tag = linha.substr(1, fimAbertura - 1);
    if (tag.empty() || tag.find(' ') != string::npos)
    {
        return false;
    }

    string fecho = "</" + tag + ">";
    if (linha.size() <= fimAbertura + fecho.size())
    {
        return false;
    }

    if (linha.compare(linha.size() - fecho.size(), fecho.size(), fecho) != 0)
    {
        return false;
    }

    texto = linha.substr(fimAbertura + 1, linha.size() - fecho.size() - (fimAbertura + 1));
    return !texto.empty();
}

/**
 * @brief Converte um texto num numero real, sem lixo a seguir.
 * @param texto Texto do XML.
 * @param destino Valor lido.
 */
static bool lerRealTexto(const string& texto, double& destino)
{
    stringstream fluxo(texto);
    double valor = 0.0;
    char extra = 0;

    if (!(fluxo >> valor))
    {
        return false;
    }

    if (fluxo >> extra)
    {
        return false;
    }

    if (!isfinite(valor))
    {
        return false;
    }

    destino = valor;
    return true;
}

/**
 * @brief Converte um texto num inteiro.
 * @param texto Texto do XML.
 * @param destino Valor lido.
 */
static bool lerInteiroTexto(const string& texto, int& destino)
{
    double valor = 0.0;

    if (!lerRealTexto(texto, valor))
    {
        return false;
    }

    if (valor < numeric_limits<int>::min() ||
        valor > numeric_limits<int>::max())
    {
        return false;
    }

    if (valor != static_cast<double>(static_cast<int>(valor)))
    {
        return false;
    }

    destino = static_cast<int>(valor);
    return true;
}

/**
 * @brief Le dois numeros separados por uma virgula.
 * @param texto Texto no formato primeiro,segundo.
 * @param primeiro Primeiro valor.
 * @param segundo Segundo valor.
 */
static bool lerParTexto(const string& texto, double& primeiro, double& segundo)
{
    size_t virgula = texto.find(',');
    if (virgula == string::npos || texto.find(',', virgula + 1) != string::npos)
    {
        return false;
    }

    return lerRealTexto(texto.substr(0, virgula), primeiro) && lerRealTexto(texto.substr(virgula + 1), segundo);
}

/**
 * @brief Converte uma probabilidade do XML para uma fracao entre 0 e 1.
 *
 * Um valor igual ou superior a 1 e tratado como percentagem.
 * @param bruto Valor lido.
 * @param destino Fracao resultante.
 */
static bool probabilidadeDoXml(double bruto, double& destino)
{
    if (bruto < 0.0)
    {
        return false;
    }

    if (bruto >= 1.0)
    {
        bruto = bruto / 100.0;
    }

    if (bruto > 1.0)
    {
        return false;
    }

    destino = bruto;
    return true;
}

/**
 * @brief Indica se ja existe um sensor com o mesmo identificador e o mesmo tipo.
 * @param lista Sensores ja aceites.
 * @param id Identificador.
 * @param tipo Etiqueta do sensor.
 */
 static string tipoSensorInterno(const string& tipoXml)
{
    if (tipoXml == "SLUZ")
    {
        return "LUZ";
    }

    if (tipoXml == "SHUMIDADE")
    {
        return "HUMIDADE";
    }

    return tipoXml;
}

static bool sensorDoMesmoTipo(const list<Sensor*>& lista, int id, const string& tipoXml)
{
    string tipoInterno = tipoSensorInterno(tipoXml);

    for (list<Sensor*>::const_iterator it = lista.begin(); it != lista.end(); ++it)
    {
        if ((*it)->GetId() == id && (*it)->GetTipo() == tipoInterno)
        {
            return true;
        }
    }

    return false;
}

bool Fabrica::lerDados(const list<string>& linhas, string& nome, int& novoX, int& novoY,
                       int& abertura, int& fecho, int& vizinhanca,
                       IntervaloTemperatura& novoVerde, IntervaloTemperatura& novoAmarelo,
                       IntervaloTemperatura& novoVermelho, list<Motor*>& novosMotores,
                       list<Sensor*>& novosSensores)
{
    /**
     * @brief Definicao de tipo enquanto a etiqueta ainda nao fechou.
     */
    struct DefinicaoMotor
    {
        IntervaloTemperatura verde; ///< Zona verde ja lida.
        IntervaloTemperatura amarelo; ///< Zona amarela ja lida.
        IntervaloTemperatura vermelho; ///< Zona vermelha ja lida.
        double probabilidade; ///< Probabilidade ja convertida para fracao.
        bool temVerde; ///< true depois de aparecer VERDE.
        bool temAmarelo; ///< true depois de aparecer AMARELO.
        bool temVermelho; ///< true depois de aparecer VERMELHO.
        bool temProbabilidade; ///< true depois de aparecer PROB_AVARIA.

        DefinicaoMotor()
            : probabilidade(0.0), temVerde(false), temAmarelo(false), temVermelho(false), temProbabilidade(false)
        {
            verde.minimo = 0.0;
            verde.maximo = 0.0;
            amarelo.minimo = 0.0;
            amarelo.maximo = 0.0;
            vermelho.minimo = 0.0;
            vermelho.maximo = 0.0;
        }
    };

    map<string, DefinicaoMotor> definicoes;
    list<string> pilha;

    bool jaAbriuDados = false;
    bool viuDefinicoes = false;
    bool viuMotores = false;
    bool viuSensores = false;
    bool fechouDefinicoes = false;
    bool fechouMotores = false;
    bool fechouSensores = false;
    bool temNome = false;
    bool temInicio = false;
    bool temFecho = false;
    bool temVizinhanca = false;
    bool temDimensao = false;
    bool temIntervaloGlobal = false;

    bool definicaoActiva = false;
    string nomeDefinicao;
    DefinicaoMotor definicaoActual;

    bool motorActivo = false;
    string motorTipo;
    int motorId = 0;
    string motorMarca;
    double motorConsumo = 0.0;
    int motorX = 0;
    int motorY = 0;
    bool temMotorId = false;
    bool temMotorMarca = false;
    bool temMotorConsumo = false;
    bool temMotorPosicao = false;

    bool sensorActivo = false;
    string sensorTipo;
    int sensorId = 0;
    string sensorMarca;
    double sensorAviso = 0.0;
    double sensorProb = 0.0;
    double sensorProb1 = 0.0;
    int sensorRaio = 0;
    int sensorX = 0;
    int sensorY = 0;
    bool temSensorId = false;
    bool temSensorMarca = false;
    bool temSensorAviso = false;
    bool temSensorProb = false;
    bool temSensorProb1 = false;
    bool temSensorPosicao = false;
    bool temSensorRaio = false;

    nome = "";
    novoX = 0;
    novoY = 0;
    abertura = 0;
    fecho = 0;
    vizinhanca = 0;

    for (list<string>::const_iterator it = linhas.begin(); it != linhas.end(); ++it)
    {
        const string& linha = *it;
        if (linha.size() >= 2 && linha[0] == '<' && linha[1] == '?')
        {
            continue;
        }

        string tag;
        string texto;
        if (elementoTexto(linha, tag, texto))
        {
            if (pilha.empty())
            {
                return false;
            }

            string pai = pilha.back();
            if (tag == "NOME_EMPRESA" && pai == "DEFINICOES" && !definicaoActiva)
            {
                if (temNome)
                {
                    return false;
                }
                nome = texto;
                temNome = true;
            }
            else if ((tag == "HORA_INICIO" || tag == "HORA_FECHO") && pai == "DEFINICOES" && !definicaoActiva)
            {
                int hora = 0;
                if (!lerInteiroTexto(texto, hora) || !horaValida(hora))
                {
                    return false;
                }
                if (tag == "HORA_INICIO")
                {
                    if (temInicio)
                    {
                        return false;
                    }
                    abertura = hora;
                    temInicio = true;
                }
                else
                {
                    if (temFecho)
                    {
                        return false;
                    }
                    fecho = hora;
                    temFecho = true;
                }
            }
            else if (tag == "VIZINHANCA_AVISO" && pai == "DEFINICOES" && !definicaoActiva)
            {
                int valor = 0;
                if (temVizinhanca || !lerInteiroTexto(texto, valor) || valor < 0)
                {
                    return false;
                }
                vizinhanca = valor;
                temVizinhanca = true;
            }
            else if (tag == "DIMENSAO_FABRICA" && pai == "DEFINICOES" && !definicaoActiva)
            {
                double eixoX = 0.0;
                double eixoY = 0.0;
                if (temDimensao || !lerParTexto(texto, eixoX, eixoY))
                {
                    return false;
                }
                if (eixoX != static_cast<double>(static_cast<int>(eixoX)) || eixoY != static_cast<double>(static_cast<int>(eixoY)))
                {
                    return false;
                }
                if (eixoX <= 0.0 || eixoY <= 0.0)
                {
                    return false;
                }
                novoX = static_cast<int>(eixoX);
                novoY = static_cast<int>(eixoY);
                temDimensao = true;
            }
            else if ((tag == "VERDE" || tag == "AMARELO" || tag == "VERMELHO") && definicaoActiva && pai == nomeDefinicao)
            {
                double minimo = 0.0;
                double maximo = 0.0;
                if (!lerParTexto(texto, minimo, maximo) || minimo > maximo)
                {
                    return false;
                }
                if (tag == "VERDE")
                {
                    if (definicaoActual.temVerde)
                    {
                        return false;
                    }
                    definicaoActual.verde.minimo = minimo;
                    definicaoActual.verde.maximo = maximo;
                    definicaoActual.temVerde = true;
                }
                else if (tag == "AMARELO")
                {
                    if (definicaoActual.temAmarelo)
                    {
                        return false;
                    }
                    definicaoActual.amarelo.minimo = minimo;
                    definicaoActual.amarelo.maximo = maximo;
                    definicaoActual.temAmarelo = true;
                }
                else
                {
                    if (definicaoActual.temVermelho)
                    {
                        return false;
                    }
                    definicaoActual.vermelho.minimo = minimo;
                    definicaoActual.vermelho.maximo = maximo;
                    definicaoActual.temVermelho = true;
                }
            }
            else if (tag == "PROB_AVARIA" && definicaoActiva && pai == nomeDefinicao)
            {
                double bruto = 0.0;
                double probabilidade = 0.0;
                if (definicaoActual.temProbabilidade || !lerRealTexto(texto, bruto) || !probabilidadeDoXml(bruto, probabilidade))
                {
                    return false;
                }
                definicaoActual.probabilidade = probabilidade;
                definicaoActual.temProbabilidade = true;
            }
            else if (tag == "ID" && motorActivo && pai == motorTipo)
            {
                int id = 0;
                if (temMotorId || !lerInteiroTexto(texto, id) || id <= 0)
                {
                    return false;
                }
                motorId = id;
                temMotorId = true;
            }
            else if (tag == "MARCA" && motorActivo && pai == motorTipo)
            {
                if (temMotorMarca || texto.empty())
                {
                    return false;
                }
                motorMarca = texto;
                temMotorMarca = true;
            }
            else if (tag == "CONSUMO_HORA" && motorActivo && pai == motorTipo)
            {
                double consumo = 0.0;
                if (temMotorConsumo || !lerRealTexto(texto, consumo) || consumo < 0.0)
                {
                    return false;
                }
                motorConsumo = consumo;
                temMotorConsumo = true;
            }
            else if (tag == "POSICAO" && motorActivo && pai == motorTipo)
            {
                double posX = 0.0;
                double posY = 0.0;
                if (temMotorPosicao || !lerParTexto(texto, posX, posY))
                {
                    return false;
                }
                if (posX != static_cast<double>(static_cast<int>(posX)) || posY != static_cast<double>(static_cast<int>(posY)))
                {
                    return false;
                }
                motorX = static_cast<int>(posX);
                motorY = static_cast<int>(posY);
                temMotorPosicao = true;
            }
            else if (tag == "ID" && sensorActivo && pai == sensorTipo)
            {
                int id = 0;
                if (temSensorId || !lerInteiroTexto(texto, id) || id <= 0)
                {
                    return false;
                }
                sensorId = id;
                temSensorId = true;
            }
            else if (tag == "MARCA" && sensorActivo && pai == sensorTipo)
            {
                if (temSensorMarca || texto.empty())
                {
                    return false;
                }
                sensorMarca = texto;
                temSensorMarca = true;
            }
            else if (tag == "VALOR_AVISO" && sensorActivo && pai == sensorTipo)
            {
                double aviso = 0.0;
                if (temSensorAviso || !lerRealTexto(texto, aviso))
                {
                    return false;
                }
                sensorAviso = aviso;
                temSensorAviso = true;
            }
            else if (tag == "POSICAO" && sensorActivo && pai == sensorTipo)
            {
                double posX = 0.0;
                double posY = 0.0;
                if (temSensorPosicao || !lerParTexto(texto, posX, posY))
                {
                    return false;
                }
                if (posX != static_cast<double>(static_cast<int>(posX)) || posY != static_cast<double>(static_cast<int>(posY)))
                {
                    return false;
                }
                sensorX = static_cast<int>(posX);
                sensorY = static_cast<int>(posY);
                temSensorPosicao = true;
            }
            else if (tag == "PROB_AVARIA" && sensorActivo && pai == sensorTipo)
            {
                double bruto = 0.0;
                double probabilidade = 0.0;
                if (temSensorProb || !lerRealTexto(texto, bruto) || !probabilidadeDoXml(bruto, probabilidade))
                {
                    return false;
                }
                sensorProb = probabilidade;
                temSensorProb = true;
            }
            else if (tag == "PROB_1" && sensorActivo && pai == sensorTipo)
            {
                double bruto = 0.0;
                double probabilidade = 0.0;
                if (sensorTipo != "SFOGO" && sensorTipo != "SMISSEL")
                {
                    return false;
                }
                if (temSensorProb1 || !lerRealTexto(texto, bruto) || !probabilidadeDoXml(bruto, probabilidade))
                {
                    return false;
                }
                sensorProb1 = probabilidade;
                temSensorProb1 = true;
            }
            else if (tag == "RAIO_VIZINHANCA" && sensorActivo && pai == sensorTipo)
            {
                int raio = 0;

                if (sensorTipo != "SHUMIDADE" || temSensorRaio || !lerInteiroTexto(texto, raio) || raio < 0)
                    {
                         return false;
                    }

                sensorRaio = raio;
                temSensorRaio = true;
            }
            else
            {
                return false;
            }

            continue;
        }

        if (tagSimples(linha, tag, true))
        {
            if (pilha.empty() || pilha.back() != tag)
            {
                return false;
            }

            pilha.pop_back();

            if (tag == "DADOS")
            {
                if (!pilha.empty() || !fechouDefinicoes || !fechouMotores || !fechouSensores)
                {
                    return false;
                }
            }
            else if (tag == "DEFINICOES")
            {
                if (definicaoActiva || !temNome || !temInicio || !temFecho || !temVizinhanca || !temDimensao)
                {
                    return false;
                }
                if (!horaValida(abertura) || !horaValida(fecho) || abertura >= fecho)
                {
                    return false;
                }
                if (definicoes.empty() || !temIntervaloGlobal)
                {
                    return false;
                }
                fechouDefinicoes = true;
            }
            else if (tag == "MOTORES")
            {
                if (motorActivo)
                {
                    return false;
                }
                fechouMotores = true;
            }
            else if (tag == "SENSORES")
            {
                if (sensorActivo)
                {
                    return false;
                }
                fechouSensores = true;
            }
            else if (eTipoMotor(tag) && definicaoActiva && tag == nomeDefinicao)
            {
                if (!definicaoActual.temVerde || !definicaoActual.temAmarelo || !definicaoActual.temVermelho || !definicaoActual.temProbabilidade)
                {
                    return false;
                }
                if (!intervalosCoerentes(definicaoActual.verde, definicaoActual.amarelo, definicaoActual.vermelho))
                {
                    return false;
                }
                if (definicoes.find(tag) != definicoes.end())
                {
                    return false;
                }
                definicoes[tag] = definicaoActual;
                DefinicaoTipoMotor guardada;
                guardada.verde = definicaoActual.verde;
                guardada.amarelo = definicaoActual.amarelo;
                guardada.vermelho = definicaoActual.vermelho;
                guardada.probabilidade = definicaoActual.probabilidade;
                definicoesMotores[tag] = guardada;
                if (!temIntervaloGlobal)
                {
                    novoVerde = definicaoActual.verde;
                    novoAmarelo = definicaoActual.amarelo;
                    novoVermelho = definicaoActual.vermelho;
                    temIntervaloGlobal = true;
                }
                definicaoActiva = false;
            }
            else if (eTipoMotor(tag) && motorActivo && tag == motorTipo)
            {
                if (!temMotorId || !temMotorMarca || !temMotorConsumo || !temMotorPosicao)
                {
                    return false;
                }
                if (!posicaoValida(motorX, motorY, novoX, novoY))
                {
                    return false;
                }
                if (idMotorExiste(novosMotores, motorId) || idMotorExiste(motores, motorId))
                {
                    return false;
                }

                map<string, DefinicaoMotor>::const_iterator def = definicoes.find(motorTipo);
                if (def == definicoes.end())
                {
                    return false;
                }

                Motor* motor = NULL;
                if (motorTipo == "MELETRICO")
                {
                    motor = new MotorEletrico(motorId, motorMarca, motorConsumo, def->second.amarelo.minimo,
                                              def->second.vermelho.minimo, def->second.probabilidade, motorX, motorY);
                }
                else if (motorTipo == "MCOMBUSTAO")
                {
                    motor = new MotorCombustao(motorId, motorMarca, motorConsumo, def->second.amarelo.minimo,
                                               def->second.vermelho.minimo, def->second.probabilidade, motorX, motorY);
                }
                else if (motorTipo == "MINDUCAO")
                {
                    motor = new MotorInducao(motorId, motorMarca, motorConsumo, def->second.amarelo.minimo,
                                             def->second.vermelho.minimo, def->second.probabilidade, motorX, motorY);
                }

                if (motor == NULL)
                {
                    return false;
                }

                novosMotores.push_back(motor);
                motorActivo = false;
            }
            else if (eTipoSensor(tag) && sensorActivo && tag == sensorTipo)
            {
                bool exigeEvento = sensorTipo == "SFOGO" || sensorTipo == "SMISSEL";
                if (!temSensorId || !temSensorMarca || !temSensorAviso || !temSensorPosicao || !temSensorProb)
                {
                    return false;
                }
                if (exigeEvento && !temSensorProb1)
                {
                    return false;
                }
                if (!exigeEvento && temSensorProb1)
                {
                    return false;
                }
                if (!posicaoValida(sensorX, sensorY, novoX, novoY))
                {
                    return false;
                }
                if (sensorDoMesmoTipo(novosSensores, sensorId, sensorTipo) || sensorDoMesmoTipo(sensores, sensorId, sensorTipo))
                {
                    return false;
                }

                Sensor* sensor = NULL;
                if (sensorTipo == "STEMPERATURA")
                {
                    sensor = new SensorTemperatura(sensorId, sensorMarca, 0.0,
                                                sensorProb, sensorX, sensorY, sensorAviso);
                }
                else if (sensorTipo == "SLUZ")
                {
                    sensor = new SensorLuz(sensorId, sensorMarca, 0.0,
                                        sensorProb, sensorX, sensorY, sensorAviso);
                }
                else if (sensorTipo == "SHUMIDADE")
                {
                    sensor = new SensorHumidade(sensorId, sensorMarca, 0.0,
                                                sensorProb, sensorX, sensorY,
                                                sensorAviso, sensorRaio);
                }
                else if (sensorTipo == "SFOGO")
                {
                    sensor = new SensorFumo(sensorId, sensorMarca, 0.0,
                                            sensorProb, sensorX, sensorY, sensorAviso);

                    sensor->SetProbabilidadeUm(sensorProb1);
                }
                else if (sensorTipo == "SMISSEL")
                {
                    sensor = new SensorMissel(sensorId, sensorMarca, 0.0,
                                            sensorProb, sensorX, sensorY, sensorAviso);

                    sensor->SetProbabilidadeUm(sensorProb1);
                }
                novosSensores.push_back(sensor);
                sensorActivo = false;
            }
            else
            {
                return false;
            }

            continue;
        }

        if (!tagSimples(linha, tag, false))
        {
            return false;
        }

        if (pilha.empty())
        {
            if (tag != "DADOS" || jaAbriuDados)
            {
                return false;
            }
            jaAbriuDados = true;
            pilha.push_back(tag);
            continue;
        }

        string pai = pilha.back();
        if (tag == "DEFINICOES" || tag == "MOTORES" || tag == "SENSORES")
        {
            if (pai != "DADOS" || definicaoActiva || motorActivo || sensorActivo)
            {
                return false;
            }
            if (tag == "DEFINICOES" && viuDefinicoes)
            {
                return false;
            }
            if (tag == "MOTORES" && viuMotores)
            {
                return false;
            }
            if (tag == "SENSORES" && viuSensores)
            {
                return false;
            }
            if (tag == "DEFINICOES")
            {
                viuDefinicoes = true;
            }
            else if (tag == "MOTORES")
            {
                viuMotores = true;
            }
            else
            {
                viuSensores = true;
            }
            pilha.push_back(tag);
            continue;
        }

        if (eTipoMotor(tag))
        {
            if (pai == "DEFINICOES")
            {
                if (definicaoActiva)
                {
                    return false;
                }
                definicaoActiva = true;
                nomeDefinicao = tag;
                definicaoActual = DefinicaoMotor();
                pilha.push_back(tag);
                continue;
            }
            if (pai == "MOTORES")
            {
                if (motorActivo || !fechouDefinicoes)
                {
                    return false;
                }
                motorActivo = true;
                motorTipo = tag;
                motorId = 0;
                motorMarca = "";
                motorConsumo = 0.0;
                motorX = 0;
                motorY = 0;
                temMotorId = false;
                temMotorMarca = false;
                temMotorConsumo = false;
                temMotorPosicao = false;
                pilha.push_back(tag);
                continue;
            }
            return false;
        }

        if (eTipoSensor(tag))
        {
            if (pai != "SENSORES" || sensorActivo || !fechouDefinicoes)
            {
                return false;
            }
            sensorActivo = true;
            sensorTipo = tag;
            sensorId = 0;
            sensorMarca = "";
            sensorAviso = 0.0;
            sensorProb = 0.0;
            sensorProb1 = 0.0;
            sensorX = 0;
            sensorY = 0;
            temSensorId = false;
            temSensorMarca = false;
            temSensorAviso = false;
            temSensorProb = false;
            temSensorProb1 = false;
            temSensorPosicao = false;
            sensorRaio = 0;
            temSensorRaio = false;
            pilha.push_back(tag);
            continue;
        }

        return false;
    }

    return jaAbriuDados && pilha.empty();
}

string Fabrica::textoEstado(ESTADO_MOTOR estado) const
{
    if (estado == STOP)
    {
        return "STOP";
    }

    if (estado == START)
    {
        return "START";
    }

    if (estado == RUN)
    {
        return "RUN";
    }

    return "AVARIADO";
}

string Fabrica::textoClasse(CLASSE_TEMPERATURA classe) const
{
    if (classe == VERDE)
    {
        return "VERDE";
    }

    if (classe == AMARELO)
    {
        return "AMARELO";
    }

    return "VERMELHO";
}

void Fabrica::acumularAvarias(map<string, int>& totais) const
{
    for (list<Motor*>::const_iterator it = motores.begin(); it != motores.end(); ++it)
    {
        totais[(*it)->GetMarca()] = totais[(*it)->GetMarca()] + (*it)->GetTotalAvarias();
    }

    for (list<Sensor*>::const_iterator it = sensores.begin(); it != sensores.end(); ++it)
    {
        totais[(*it)->GetMarca()] = totais[(*it)->GetMarca()] + (*it)->GetTotalAvarias();
    }
}

void Fabrica::inserirMarca(list<MarcaAvarias>& ranking, const string& marca, int avarias) const
{
    MarcaAvarias entrada;
    entrada.marca = marca;
    entrada.avarias = avarias;

    for (list<MarcaAvarias>::iterator it = ranking.begin(); it != ranking.end(); ++it)
    {
        if (entrada.avarias > it->avarias)
        {
            ranking.insert(it, entrada);
            return;
        }

        if (entrada.avarias == it->avarias && entrada.marca < it->marca)
        {
            ranking.insert(it, entrada);
            return;
        }
    }

    ranking.push_back(entrada);
}

void Fabrica::inserirPorTrabalho(list<Motor*>& destino, Motor* motor) const
{
    for (list<Motor*>::iterator it = destino.begin(); it != destino.end(); ++it)
    {
        if (motor->GetTempoTrabalho() > (*it)->GetTempoTrabalho())
        {
            destino.insert(it, motor);
            return;
        }

        if (motor->GetTempoTrabalho() == (*it)->GetTempoTrabalho() && motor->GetId() < (*it)->GetId())
        {
            destino.insert(it, motor);
            return;
        }
    }

    destino.push_back(motor);
}
