/**
 * @file Fabrica.cpp
 * @brief Implementacao da gestao, do XML com TinyXML-2 e do passo de simulacao.
 */

#include "Fabrica.h"
#include "tinyxml2.h"

#include <cstring>
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <sstream>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

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

    if (!lerDados(ficheiro, nome, novoX, novoY, novaAbertura, novoEncerramento, novaVizinhanca,
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
          << " | consumo dia " << m->GetConsumoDia();

        if (m->TemDeposito())
        {
            int percentagem = static_cast<int>(m->GetCombustivelPercentagem() + 0.5);
            f << " | combustivel " << percentagem << "%";
        }

        f
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
 * @brief Etiqueta de sensor usada no XML exportado.
 * @param tipo Nome interno do sensor.
 * @return SHUMIDADE, SLUZ, SMISSEL ou a etiqueta original.
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
 * @brief Texto de um numero, no formato da TinyXML-2.
 * @param valor Numero a converter.
 * @return Texto produzido por XMLUtil::ToStr.
 */
static string textoDaBiblioteca(double valor)
{
    char buffer[64];
    tinyxml2::XMLUtil::ToStr(valor, buffer, 64);
    return buffer;
}

/**
 * @brief Probabilidade tal como o relatorio a mostra.
 *
 * Uma fracao entre 0 e 1 volta a percentagem: 0.05 fica 5.
 * @param fracao Probabilidade guardada no sensor.
 */
static double probabilidadeExportada(double fracao)
{
    if (fracao >= 0.0 && fracao <= 1.0)
    {
        return fracao * 100.0;
    }
    return fracao;
}

/**
 * @brief Acrescenta a definicao de um tipo de motor ao relatorio.
 * @param documento Documento do relatorio.
 * @param definicoes Elemento DEFINICOES.
 * @param tipo Etiqueta do tipo.
 * @param verdeMinimo Limite inferior da zona verde.
 * @param verdeMaximo Limite superior da zona verde.
 * @param amareloMinimo Limite inferior da zona amarela.
 * @param amareloMaximo Limite superior da zona amarela.
 * @param vermelhoMinimo Limite inferior da zona vermelha.
 * @param vermelhoMaximo Limite superior da zona vermelha.
 * @param probabilidade Probabilidade de avaria, em fracao.
 */
static void escreverTipoMotor(tinyxml2::XMLDocument& documento, tinyxml2::XMLElement* definicoes,
                              const string& tipo, double verdeMinimo, double verdeMaximo,
                              double amareloMinimo, double amareloMaximo,
                              double vermelhoMinimo, double vermelhoMaximo, double probabilidade)
{
    tinyxml2::XMLElement* bloco = documento.NewElement(tipo.c_str());
    definicoes->InsertEndChild(bloco);

    tinyxml2::XMLElement* verde = documento.NewElement("VERDE");
    verde->SetText((textoDaBiblioteca(verdeMinimo) + ", " + textoDaBiblioteca(verdeMaximo)).c_str());
    bloco->InsertEndChild(verde);

    tinyxml2::XMLElement* amarelo = documento.NewElement("AMARELO");
    amarelo->SetText((textoDaBiblioteca(amareloMinimo) + ", " + textoDaBiblioteca(amareloMaximo)).c_str());
    bloco->InsertEndChild(amarelo);

    tinyxml2::XMLElement* vermelho = documento.NewElement("VERMELHO");
    vermelho->SetText((textoDaBiblioteca(vermelhoMinimo) + ", " + textoDaBiblioteca(vermelhoMaximo)).c_str());
    bloco->InsertEndChild(vermelho);

    tinyxml2::XMLElement* avaria = documento.NewElement("PROB_AVARIA");
    avaria->SetText(probabilidade);
    bloco->InsertEndChild(avaria);
}

void Fabrica::Relatorio(string fich_xml) const
{
    if (!podeListar() || fich_xml.empty())
    {
        return;
    }

    tinyxml2::XMLDocument documento;
    tinyxml2::XMLElement* dados = documento.NewElement("DADOS");
    documento.InsertEndChild(dados);
    tinyxml2::XMLElement* definicoes = documento.NewElement("DEFINICOES");
    dados->InsertEndChild(definicoes);

    tinyxml2::XMLElement* empresa = documento.NewElement("NOME_EMPRESA");
    empresa->SetText(nomeEmpresa.c_str());
    definicoes->InsertEndChild(empresa);

    tinyxml2::XMLElement* inicio = documento.NewElement("HORA_INICIO");
    inicio->SetText(horaAbertura);
    definicoes->InsertEndChild(inicio);

    tinyxml2::XMLElement* fecho = documento.NewElement("HORA_FECHO");
    fecho->SetText(horaEncerramento);
    definicoes->InsertEndChild(fecho);

    tinyxml2::XMLElement* vizinhanca = documento.NewElement("VIZINHANCA_AVISO");
    vizinhanca->SetText(vizinhancaAviso);
    definicoes->InsertEndChild(vizinhanca);

    tinyxml2::XMLElement* dimensao = documento.NewElement("DIMENSAO_FABRICA");
    dimensao->SetText((to_string(campoX) + "," + to_string(campoY)).c_str());
    definicoes->InsertEndChild(dimensao);

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

        escreverTipoMotor(documento, definicoes, def->first,
                          def->second.verde.minimo, def->second.verde.maximo,
                          def->second.amarelo.minimo, def->second.amarelo.maximo,
                          def->second.vermelho.minimo, def->second.vermelho.maximo,
                          def->second.probabilidade);
    }

    for (map<string, DefinicaoTipoMotor>::const_iterator def = definicoesMotores.begin(); def != definicoesMotores.end(); ++def)
    {
        if (def->first == "MELETRICO" || def->first == "MCOMBUSTAO" || def->first == "MINDUCAO")
        {
            continue;
        }

        escreverTipoMotor(documento, definicoes, def->first,
                          def->second.verde.minimo, def->second.verde.maximo,
                          def->second.amarelo.minimo, def->second.amarelo.maximo,
                          def->second.vermelho.minimo, def->second.vermelho.maximo,
                          def->second.probabilidade);
    }

    tinyxml2::XMLElement* blocoMotores = documento.NewElement("MOTORES");
    dados->InsertEndChild(blocoMotores);
    double consumoTotalDia = 0.0;

    for (list<Motor*>::const_iterator it = motores.begin(); it != motores.end(); ++it)
    {
        Motor* motor = *it;
        tinyxml2::XMLElement* bloco = documento.NewElement(motor->GetTipo().c_str());
        blocoMotores->InsertEndChild(bloco);

        tinyxml2::XMLElement* id = documento.NewElement("ID");
        id->SetText(motor->GetId());
        bloco->InsertEndChild(id);

        tinyxml2::XMLElement* marca = documento.NewElement("MARCA");
        marca->SetText(motor->GetMarca().c_str());
        bloco->InsertEndChild(marca);

        tinyxml2::XMLElement* temperatura = documento.NewElement("VALOR_TEMP_ATUAL");
        temperatura->SetText(motor->GetTemperaturaAtual());
        bloco->InsertEndChild(temperatura);

        tinyxml2::XMLElement* estado = documento.NewElement("ESTADO_ATUAL");
        estado->SetText(textoEstado(motor->GetEstado()).c_str());
        bloco->InsertEndChild(estado);

        tinyxml2::XMLElement* cor = documento.NewElement("ESTADO_COR");
        cor->SetText(textoClasse(motor->GetClasseTemperatura()).c_str());
        bloco->InsertEndChild(cor);

        tinyxml2::XMLElement* consumoHora = documento.NewElement("CONSUMO_HORA");
        consumoHora->SetText(motor->GetConsumoHora());
        bloco->InsertEndChild(consumoHora);

        tinyxml2::XMLElement* consumoDia = documento.NewElement("CONSUMO_DIA");
        consumoDia->SetText(motor->GetConsumoDia());
        bloco->InsertEndChild(consumoDia);

        tinyxml2::XMLElement* posicao = documento.NewElement("POSICAO");
        posicao->SetText((to_string(motor->GetX()) + "," + to_string(motor->GetY())).c_str());
        bloco->InsertEndChild(posicao);

        consumoTotalDia = consumoTotalDia + motor->GetConsumoDia();
    }

    tinyxml2::XMLElement* total = documento.NewElement("CONSUMO_TOTAL_DIA");
    total->SetText(consumoTotalDia);
    dados->InsertEndChild(total);

    tinyxml2::XMLElement* blocoSensores = documento.NewElement("SENSORES");
    dados->InsertEndChild(blocoSensores);
    for (list<Sensor*>::const_iterator it = sensores.begin(); it != sensores.end(); ++it)
    {
        Sensor* sensor = *it;
        string tipo = etiquetaSensorExportada(sensor->GetTipo());
        string valorAtual = textoDaBiblioteca(sensor->GetValor());
        if (tipo == "SHUMIDADE")
        {
            valorAtual = valorAtual + "%";
        }

        tinyxml2::XMLElement* bloco = documento.NewElement(tipo.c_str());
        blocoSensores->InsertEndChild(bloco);

        tinyxml2::XMLElement* id = documento.NewElement("ID");
        id->SetText(sensor->GetId());
        bloco->InsertEndChild(id);

        tinyxml2::XMLElement* marca = documento.NewElement("MARCA");
        marca->SetText(sensor->GetMarca().c_str());
        bloco->InsertEndChild(marca);

        tinyxml2::XMLElement* aviso = documento.NewElement("VALOR_AVISO");
        aviso->SetText(sensor->GetLimiar());
        bloco->InsertEndChild(aviso);

        tinyxml2::XMLElement* posicao = documento.NewElement("POSICAO");
        posicao->SetText((to_string(sensor->GetX()) + "," + to_string(sensor->GetY())).c_str());
        bloco->InsertEndChild(posicao);

        tinyxml2::XMLElement* avaria = documento.NewElement("PROB_AVARIA");
        avaria->SetText(probabilidadeExportada(sensor->GetProbabilidadeAvaria()));
        bloco->InsertEndChild(avaria);

        tinyxml2::XMLElement* atual = documento.NewElement("VALOR_ATUAL");
        atual->SetText(valorAtual.c_str());
        bloco->InsertEndChild(atual);
    }

    documento.SaveFile(fich_xml.c_str());
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

/** @brief Video de regresso a casa, em Assets. */
static const char* VIDEO_LUZ = "Assets/video 1 - bom regresso.mp4";

/** @brief Video do alerta de missil, em Assets. */
static const char* VIDEO_MISSEL = "Assets/video 2 - alerta missel.mp4";

/** @brief Video do alerta de fumo, em Assets. */
static const char* VIDEO_FUMO = "Assets/video 3 - alerta fumo.mp4";

/** @brief Video de reabastecimento, em Assets. */
static const char* VIDEO_COMBUSTIVEL = "Assets/video 4 - combustivel.mp4";

/**
 * @brief Le um inteiro de 32 bits em big-endian.
 */
static unsigned int lerU32(const unsigned char* dados)
{
    return (static_cast<unsigned int>(dados[0]) << 24)
        | (static_cast<unsigned int>(dados[1]) << 16)
        | (static_cast<unsigned int>(dados[2]) << 8)
        | static_cast<unsigned int>(dados[3]);
}

/**
 * @brief Le um inteiro de 64 bits em big-endian.
 */
static unsigned long long lerU64(const unsigned char* dados)
{
    unsigned long long valor = 0;

    for (int i = 0; i < 8; i++)
    {
        valor = (valor << 8) | dados[i];
    }

    return valor;
}

/**
 * @brief Duracao do video, em milissegundos, lida do cabecalho MP4.
 * @return 0 se o cabecalho nao tiver a duracao.
 */
static DWORD duracaoVideoMs(const char* caminho)
{
    ifstream ficheiro(caminho, ios::binary);

    if (!ficheiro)
    {
        return 0;
    }

    ficheiro.seekg(0, ios::end);
    streamoff tamanho = ficheiro.tellg();
    ficheiro.seekg(0, ios::beg);

    if (tamanho < 32)
    {
        return 0;
    }

    unsigned char* dados = new unsigned char[static_cast<size_t>(tamanho)];
    ficheiro.read(reinterpret_cast<char*>(dados), tamanho);
    streamsize lidos = ficheiro.gcount();
    ficheiro.close();

    DWORD resultado = 0;

    for (streamsize i = 0; i + 32 < lidos; i++)
    {
        if (dados[i] != 'm' || dados[i + 1] != 'v' || dados[i + 2] != 'h' || dados[i + 3] != 'd')
        {
            continue;
        }

        streamsize payload = i + 4;

        if (payload >= lidos)
        {
            break;
        }

        unsigned char versao = dados[payload];
        unsigned long long duracao = 0;
        unsigned int escala = 0;

        if (versao == 0 && payload + 20 <= lidos)
        {
            streamsize base = payload + 12;
            escala = lerU32(dados + base);
            duracao = lerU32(dados + base + 4);
        }
        else if (versao == 1 && payload + 32 <= lidos)
        {
            streamsize base = payload + 20;
            escala = lerU32(dados + base);
            duracao = lerU64(dados + base + 4);
        }

        if (escala > 0)
        {
            resultado = static_cast<DWORD>((duracao * 1000ull) / escala);
        }

        break;
    }

    delete[] dados;
    return resultado;
}

/**
 * @brief Dados para saber se a janela do leitor ainda esta visivel.
 */
struct JanelaDoLeitor
{
    HANDLE trabalho;
    DWORD pid;
    bool visivel;
};

/**
 * @brief Procura uma janela visivel do leitor ou de um processo do mesmo trabalho.
 */
static BOOL CALLBACK procurarJanelaLeitor(HWND janela, LPARAM param)
{
    JanelaDoLeitor* dados = reinterpret_cast<JanelaDoLeitor*>(param);

    if (!IsWindowVisible(janela))
    {
        return TRUE;
    }

    DWORD pid = 0;
    GetWindowThreadProcessId(janela, &pid);

    if (pid == 0)
    {
        return TRUE;
    }

    if (pid == dados->pid)
    {
        dados->visivel = true;
        return FALSE;
    }

    if (dados->trabalho != NULL)
    {
        HANDLE processo = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);

        if (processo != NULL)
        {
            BOOL noTrabalho = FALSE;

            if (IsProcessInJob(processo, dados->trabalho, &noTrabalho) && noTrabalho)
            {
                dados->visivel = true;
            }

            CloseHandle(processo);
        }
    }

    if (dados->visivel)
    {
        return FALSE;
    }

    return TRUE;
}

/**
 * @brief Espera o video acabar e encerra o leitor assim que a janela fecha.
 *
 * A simulacao fica parada apenas enquanto esta funcao nao regressa.
 * @param processo Processo do leitor.
 * @param trabalho Trabalho que agrupa o leitor e os processos que ele abrir. Pode ser NULL.
 * @param pid Identificador do processo do leitor.
 * @param duracaoMs Duracao do video. Zero usa um limite de seguranca.
 */
static void esperarFimDoVideo(HANDLE processo, HANDLE trabalho, DWORD pid, DWORD duracaoMs)
{
    DWORD inicio = GetTickCount();
    bool janelaVista = false;
    DWORD limite = 30000;

    if (duracaoMs > 0)
    {
        limite = duracaoMs + 2000;
    }

    while (WaitForSingleObject(processo, 200) == WAIT_TIMEOUT)
    {
        JanelaDoLeitor dados;
        dados.trabalho = trabalho;
        dados.pid = pid;
        dados.visivel = false;
        EnumWindows(procurarJanelaLeitor, reinterpret_cast<LPARAM>(&dados));

        if (dados.visivel)
        {
            janelaVista = true;
        }
        else if (janelaVista)
        {
            break;
        }

        if (GetTickCount() - inicio >= limite)
        {
            break;
        }
    }

    if (trabalho != NULL)
    {
        TerminateJobObject(trabalho, 0);
    }
    else
    {
        TerminateProcess(processo, 0);
    }

    WaitForSingleObject(processo, 3000);
}

/**
 * @brief Abre um video no Windows Media Player e so regressa quando a janela fecha.
 *
 * O tempo gasto aqui nao avanca a fabrica. Quem chamou retoma o minuto seguinte.
 * @param caminhoRelativo Caminho do video a partir da pasta do programa.
 * @return false se o ficheiro ou o leitor nao abrirem.
 */
static bool reproduzirVideo(const char* caminhoRelativo)
{
    char caminho[MAX_PATH];
    DWORD tamanho = GetFullPathNameA(caminhoRelativo, MAX_PATH, caminho, NULL);
    if (tamanho == 0 || tamanho >= MAX_PATH)
    {
        return false;
    }

    if (GetFileAttributesA(caminho) == INVALID_FILE_ATTRIBUTES)
    {
        return false;
    }

    const char* reprodutores[2];
    reprodutores[0] = "C:\\Program Files\\Windows Media Player\\wmplayer.exe";
    reprodutores[1] = "C:\\Program Files (x86)\\Windows Media Player\\wmplayer.exe";

    DWORD duracao = duracaoVideoMs(caminho);

    for (int i = 0; i < 2; i++)
    {
        if (GetFileAttributesA(reprodutores[i]) == INVALID_FILE_ATTRIBUTES)
        {
            continue;
        }

        string linha = string("\"") + reprodutores[i] + "\" /play /close \"" + caminho + "\"";
        char* comando = new char[linha.size() + 1];
        for (size_t indice = 0; indice < linha.size(); indice++)
        {
            comando[indice] = linha[indice];
        }
        comando[linha.size()] = '\0';

        STARTUPINFOA arranque;
        PROCESS_INFORMATION processo;
        ZeroMemory(&arranque, sizeof(arranque));
        arranque.cb = sizeof(arranque);
        ZeroMemory(&processo, sizeof(processo));

        BOOL criado = CreateProcessA(reprodutores[i], comando, NULL, NULL, FALSE, 0, NULL, NULL, &arranque, &processo);
        delete[] comando;

        if (!criado)
        {
            continue;
        }

        HANDLE trabalho = CreateJobObjectA(NULL, NULL);

        if (trabalho != NULL)
        {
            JOBOBJECT_EXTENDED_LIMIT_INFORMATION limites;
            ZeroMemory(&limites, sizeof(limites));
            limites.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;

            if (!SetInformationJobObject(trabalho, JobObjectExtendedLimitInformation, &limites, sizeof(limites))
                || !AssignProcessToJobObject(trabalho, processo.hProcess))
            {
                CloseHandle(trabalho);
                trabalho = NULL;
            }
        }

        esperarFimDoVideo(processo.hProcess, trabalho, processo.dwProcessId, duracao);

        if (trabalho != NULL)
        {
            CloseHandle(trabalho);
        }

        CloseHandle(processo.hProcess);
        CloseHandle(processo.hThread);
        return true;
    }

    return false;
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

int Fabrica::Aviso_Fumo(list<Motor*>& lm, string)
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

    bool videoAberto = reproduzirVideo(VIDEO_FUMO);

    ostringstream texto;

    texto << "Fumo detetado. Paragem de seguranca executada. ";
    texto << "Motores desligados: " << desligados << ".";

    if (videoAberto)
    {
        texto << " Video das saidas reproduzido.";
    }
    else
    {
        texto << " Nao foi possivel reproduzir o video das saidas.";
    }

    avisarGestor(texto.str());

    return desligados;
}

int Fabrica::Aviso_Luz(string)
{
    if (!reproduzirVideo(VIDEO_LUZ))
    {
        avisarGestor("Luz em aviso. Nao foi possivel reproduzir o video de regresso.");
        return 0;
    }

    avisarGestor("Luz em aviso. Video de regresso reproduzido.");
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
void Fabrica::Aviso_Missel(string, string festado)
{
    for (list<Motor*>::iterator it = motores.begin(); it != motores.end(); ++it)
    {
        (*it)->STOP();
    }

    fabricaActiva = false;

    bool videoAberto = reproduzirVideo(VIDEO_MISSEL);

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
                        << " | consumo dia " << motor->GetConsumoDia();

                    if (motor->TemDeposito())
                    {
                        int percentagem = static_cast<int>(motor->GetCombustivelPercentagem() + 0.5);
                        estado << " | combustivel " << percentagem << "%";
                    }

                    estado
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

        if (videoAberto)
        {
            texto << " Video dos abrigos reproduzido.";
        }
        else
        {
            texto << " Nao foi possivel reproduzir o video dos abrigos.";
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
    bool depositoVazio = false;

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

        // 7. Registar trabalho, combustivel e aquecimento.
        motor->RegistarTrabalho(1);

        if (motor->ConsumirCombustivel(1))
        {
            depositoVazio = true;
        }

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

    if (depositoVazio)
    {
        bool videoAberto = reproduzirVideo(VIDEO_COMBUSTIVEL);

        for (list<Motor*>::iterator it = motores.begin(); it != motores.end(); ++it)
        {
            Motor* motor = *it;

            if (motor->TemDeposito() && motor->GetCombustivel() <= 0.0)
            {
                motor->Reabastecer();
            }
        }

        if (videoAberto)
        {
            avisarGestor("Combustivel esgotado. Video de reabastecimento reproduzido. Deposito a 100%.");
        }
        else
        {
            avisarGestor("Combustivel esgotado. Nao foi possivel reproduzir o video. Deposito a 100%.");
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
 * @brief Nome interno do sensor a partir da etiqueta do XML.
 * @param tipoXml Etiqueta usada no ficheiro.
 * @return LUZ, HUMIDADE ou a propria etiqueta.
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

/**
 * @brief Indica se ja existe um sensor com o mesmo identificador e o mesmo tipo.
 * @param lista Sensores ja aceites.
 * @param id Identificador.
 * @param tipoXml Etiqueta do sensor no XML.
 */
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

/**
 * @brief Le dois numeros separados por virgula no texto que a TinyXML-2 ja extraiu.
 * @param elemento Elemento cujo texto esta no formato primeiro,segundo.
 * @param primeiro Primeiro valor, convertido por XMLUtil::ToDouble.
 * @param segundo Segundo valor, convertido por XMLUtil::ToDouble.
 * @return false se o texto nao tiver exactamente uma virgula ou um numero falhar.
 */
static bool lerPar(const tinyxml2::XMLElement* elemento, double& primeiro, double& segundo)
{
    if (elemento == NULL)
    {
        return false;
    }

    const char* texto = elemento->GetText();
    if (texto == NULL)
    {
        return false;
    }

    const char* virgula = strchr(texto, ',');
    if (virgula == NULL || strchr(virgula + 1, ',') != NULL)
    {
        return false;
    }

    string esquerda(texto, static_cast<size_t>(virgula - texto));
    return tinyxml2::XMLUtil::ToDouble(esquerda.c_str(), &primeiro) &&
           tinyxml2::XMLUtil::ToDouble(virgula + 1, &segundo);
}

/**
 * @brief Le uma probabilidade com QueryDoubleText.
 *
 * Um valor igual ou superior a 1 e uma percentagem.
 * @param elemento Elemento com o numero.
 * @param destino Fracao entre 0 e 1.
 * @return false se o texto nao for um numero valido.
 */
static bool lerProbabilidade(const tinyxml2::XMLElement* elemento, double& destino)
{
    double bruto = 0.0;
    if (elemento == NULL || elemento->QueryDoubleText(&bruto) != tinyxml2::XML_SUCCESS || bruto < 0.0)
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

bool Fabrica::lerDados(const string& ficheiro, string& nome, int& novoX, int& novoY,
                       int& abertura, int& fecho, int& vizinhanca,
                       IntervaloTemperatura& novoVerde, IntervaloTemperatura& novoAmarelo,
                       IntervaloTemperatura& novoVermelho, list<Motor*>& novosMotores,
                       list<Sensor*>& novosSensores)
{
    tinyxml2::XMLDocument documento;
    if (documento.LoadFile(ficheiro.c_str()) != tinyxml2::XML_SUCCESS)
    {
        return false;
    }

    tinyxml2::XMLElement* dados = documento.FirstChildElement();
    if (dados == NULL || string(dados->Name()) != "DADOS" || dados->NextSiblingElement() != NULL)
    {
        return false;
    }

    tinyxml2::XMLElement* definicoes = NULL;
    tinyxml2::XMLElement* motores = NULL;
    tinyxml2::XMLElement* sensores = NULL;

    for (tinyxml2::XMLElement* filho = dados->FirstChildElement(); filho != NULL; filho = filho->NextSiblingElement())
    {
        string etiqueta = filho->Name();
        if (etiqueta == "DEFINICOES")
        {
            if (definicoes != NULL)
            {
                return false;
            }
            definicoes = filho;
        }
        else if (etiqueta == "MOTORES")
        {
            if (motores != NULL)
            {
                return false;
            }
            motores = filho;
        }
        else if (etiqueta == "SENSORES")
        {
            if (sensores != NULL)
            {
                return false;
            }
            sensores = filho;
        }
        else
        {
            return false;
        }
    }

    if (definicoes == NULL || motores == NULL || sensores == NULL)
    {
        return false;
    }

    if (!lerDefinicoesXml(definicoes, nome, novoX, novoY, abertura, fecho, vizinhanca,
                          novoVerde, novoAmarelo, novoVermelho))
    {
        return false;
    }

    if (!lerMotoresXml(motores, novoX, novoY, novosMotores))
    {
        return false;
    }

    return lerSensoresXml(sensores, novoX, novoY, novosSensores);
}

bool Fabrica::lerDefinicoesXml(tinyxml2::XMLElement* definicoes, string& nome, int& novoX, int& novoY,
                               int& abertura, int& fecho, int& vizinhanca,
                               IntervaloTemperatura& novoVerde, IntervaloTemperatura& novoAmarelo,
                               IntervaloTemperatura& novoVermelho)
{
    bool temNome = false;
    bool temInicio = false;
    bool temFecho = false;
    bool temVizinhanca = false;
    bool temDimensao = false;
    bool temIntervaloGlobal = false;

    nome = "";
    novoX = 0;
    novoY = 0;
    abertura = 0;
    fecho = 0;
    vizinhanca = 0;

    if (definicoes == NULL)
    {
        return false;
    }

    for (tinyxml2::XMLElement* filho = definicoes->FirstChildElement(); filho != NULL; filho = filho->NextSiblingElement())
    {
        string etiqueta = filho->Name();

        if (etiqueta == "NOME_EMPRESA")
        {
            const char* texto = filho->GetText();
            if (temNome || texto == NULL || texto[0] == '\0')
            {
                return false;
            }
            nome = texto;
            temNome = true;
        }
        else if (etiqueta == "HORA_INICIO" || etiqueta == "HORA_FECHO")
        {
            int hora = 0;
            if (filho->QueryIntText(&hora) != tinyxml2::XML_SUCCESS || !horaValida(hora))
            {
                return false;
            }
            if (etiqueta == "HORA_INICIO")
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
        else if (etiqueta == "VIZINHANCA_AVISO")
        {
            int valor = 0;
            if (temVizinhanca || filho->QueryIntText(&valor) != tinyxml2::XML_SUCCESS || valor < 0)
            {
                return false;
            }
            vizinhanca = valor;
            temVizinhanca = true;
        }
        else if (etiqueta == "DIMENSAO_FABRICA")
        {
            double eixoX = 0.0;
            double eixoY = 0.0;
            if (temDimensao || !lerPar(filho, eixoX, eixoY))
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
        else if (eTipoMotor(etiqueta))
        {
            if (!lerTipoMotorXml(filho, temIntervaloGlobal, novoVerde, novoAmarelo, novoVermelho))
            {
                return false;
            }
        }
        else
        {
            return false;
        }
    }

    if (!temNome || !temInicio || !temFecho || !temVizinhanca || !temDimensao)
    {
        return false;
    }

    if (!horaValida(abertura) || !horaValida(fecho) || abertura >= fecho)
    {
        return false;
    }

    return !definicoesMotores.empty() && temIntervaloGlobal;
}

bool Fabrica::lerTipoMotorXml(tinyxml2::XMLElement* elemento, bool& temIntervaloGlobal,
                              IntervaloTemperatura& novoVerde, IntervaloTemperatura& novoAmarelo,
                              IntervaloTemperatura& novoVermelho)
{
    if (elemento == NULL || elemento->Name() == NULL)
    {
        return false;
    }

    string tipo = elemento->Name();
    if (!eTipoMotor(tipo) || definicoesMotores.find(tipo) != definicoesMotores.end())
    {
        return false;
    }

    DefinicaoTipoMotor definicao;
    definicao.verde.minimo = 0.0;
    definicao.verde.maximo = 0.0;
    definicao.amarelo.minimo = 0.0;
    definicao.amarelo.maximo = 0.0;
    definicao.vermelho.minimo = 0.0;
    definicao.vermelho.maximo = 0.0;
    definicao.probabilidade = 0.0;

    bool temVerde = false;
    bool temAmarelo = false;
    bool temVermelho = false;
    bool temProbabilidade = false;

    for (tinyxml2::XMLElement* filho = elemento->FirstChildElement(); filho != NULL; filho = filho->NextSiblingElement())
    {
        string etiqueta = filho->Name();

        if (etiqueta == "VERDE" || etiqueta == "AMARELO" || etiqueta == "VERMELHO")
        {
            double minimo = 0.0;
            double maximo = 0.0;
            if (!lerPar(filho, minimo, maximo) || minimo > maximo)
            {
                return false;
            }

            IntervaloTemperatura* intervalo = NULL;
            bool* presente = NULL;
            if (etiqueta == "VERDE")
            {
                intervalo = &definicao.verde;
                presente = &temVerde;
            }
            else if (etiqueta == "AMARELO")
            {
                intervalo = &definicao.amarelo;
                presente = &temAmarelo;
            }
            else
            {
                intervalo = &definicao.vermelho;
                presente = &temVermelho;
            }

            if (*presente)
            {
                return false;
            }

            intervalo->minimo = minimo;
            intervalo->maximo = maximo;
            *presente = true;
        }
        else if (etiqueta == "PROB_AVARIA")
        {
            double probabilidade = 0.0;
            if (temProbabilidade || !lerProbabilidade(filho, probabilidade))
            {
                return false;
            }
            definicao.probabilidade = probabilidade;
            temProbabilidade = true;
        }
        else
        {
            return false;
        }
    }

    if (!temVerde || !temAmarelo || !temVermelho || !temProbabilidade)
    {
        return false;
    }

    if (!intervalosCoerentes(definicao.verde, definicao.amarelo, definicao.vermelho))
    {
        return false;
    }

    definicoesMotores[tipo] = definicao;
    if (!temIntervaloGlobal)
    {
        novoVerde = definicao.verde;
        novoAmarelo = definicao.amarelo;
        novoVermelho = definicao.vermelho;
        temIntervaloGlobal = true;
    }

    return true;
}

bool Fabrica::lerMotoresXml(tinyxml2::XMLElement* motores, int limiteX, int limiteY, list<Motor*>& novosMotores)
{
    if (motores == NULL)
    {
        return false;
    }

    for (tinyxml2::XMLElement* elemento = motores->FirstChildElement(); elemento != NULL; elemento = elemento->NextSiblingElement())
    {
        if (elemento->Name() == NULL || !eTipoMotor(elemento->Name()))
        {
            return false;
        }

        if (!lerUmMotorXml(elemento, elemento->Name(), limiteX, limiteY, novosMotores))
        {
            return false;
        }
    }

    return true;
}

bool Fabrica::lerUmMotorXml(tinyxml2::XMLElement* elemento, const string& tipo, int limiteX, int limiteY,
                            list<Motor*>& novosMotores)
{
    bool temId = false;
    bool temMarca = false;
    bool temConsumo = false;
    bool temPosicao = false;
    int id = 0;
    string marca;
    double consumo = 0.0;
    int posX = 0;
    int posY = 0;

    if (elemento == NULL)
    {
        return false;
    }

    for (tinyxml2::XMLElement* filho = elemento->FirstChildElement(); filho != NULL; filho = filho->NextSiblingElement())
    {
        string etiqueta = filho->Name();

        if (etiqueta == "ID")
        {
            if (temId || filho->QueryIntText(&id) != tinyxml2::XML_SUCCESS || id <= 0)
            {
                return false;
            }
            temId = true;
        }
        else if (etiqueta == "MARCA")
        {
            const char* texto = filho->GetText();
            if (temMarca || texto == NULL || texto[0] == '\0')
            {
                return false;
            }
            marca = texto;
            temMarca = true;
        }
        else if (etiqueta == "CONSUMO_HORA")
        {
            if (temConsumo || filho->QueryDoubleText(&consumo) != tinyxml2::XML_SUCCESS || consumo < 0.0)
            {
                return false;
            }
            temConsumo = true;
        }
        else if (etiqueta == "POSICAO")
        {
            double eixoX = 0.0;
            double eixoY = 0.0;
            if (temPosicao || !lerPar(filho, eixoX, eixoY))
            {
                return false;
            }
            if (eixoX != static_cast<double>(static_cast<int>(eixoX)) || eixoY != static_cast<double>(static_cast<int>(eixoY)))
            {
                return false;
            }
            posX = static_cast<int>(eixoX);
            posY = static_cast<int>(eixoY);
            temPosicao = true;
        }
        else
        {
            return false;
        }
    }

    if (!temId || !temMarca || !temConsumo || !temPosicao)
    {
        return false;
    }

    if (!posicaoValida(posX, posY, limiteX, limiteY))
    {
        return false;
    }

    if (idMotorExiste(novosMotores, id) || idMotorExiste(motores, id))
    {
        return false;
    }

    map<string, DefinicaoTipoMotor>::const_iterator def = definicoesMotores.find(tipo);
    if (def == definicoesMotores.end())
    {
        return false;
    }

    Motor* motor = NULL;
    if (tipo == "MELETRICO")
    {
        motor = new MotorEletrico(id, marca, consumo, def->second.amarelo.minimo,
                                  def->second.vermelho.minimo, def->second.probabilidade, posX, posY);
    }
    else if (tipo == "MCOMBUSTAO")
    {
        motor = new MotorCombustao(id, marca, consumo, def->second.amarelo.minimo,
                                   def->second.vermelho.minimo, def->second.probabilidade, posX, posY);
    }
    else if (tipo == "MINDUCAO")
    {
        motor = new MotorInducao(id, marca, consumo, def->second.amarelo.minimo,
                                 def->second.vermelho.minimo, def->second.probabilidade, posX, posY);
    }

    if (motor == NULL)
    {
        return false;
    }

    novosMotores.push_back(motor);
    return true;
}

bool Fabrica::lerSensoresXml(tinyxml2::XMLElement* sensores, int limiteX, int limiteY, list<Sensor*>& novosSensores)
{
    if (sensores == NULL)
    {
        return false;
    }

    for (tinyxml2::XMLElement* elemento = sensores->FirstChildElement(); elemento != NULL; elemento = elemento->NextSiblingElement())
    {
        if (elemento->Name() == NULL || !eTipoSensor(elemento->Name()))
        {
            return false;
        }

        if (!lerUmSensorXml(elemento, elemento->Name(), limiteX, limiteY, novosSensores))
        {
            return false;
        }
    }

    return true;
}

bool Fabrica::lerUmSensorXml(tinyxml2::XMLElement* elemento, const string& tipo, int limiteX, int limiteY,
                             list<Sensor*>& novosSensores)
{
    bool temId = false;
    bool temMarca = false;
    bool temAviso = false;
    bool temProbabilidade = false;
    bool temProbabilidadeUm = false;
    bool temPosicao = false;
    bool temRaio = false;
    int id = 0;
    string marca;
    double aviso = 0.0;
    double probabilidade = 0.0;
    double probabilidadeUm = 0.0;
    int raio = 0;
    int posX = 0;
    int posY = 0;

    if (elemento == NULL)
    {
        return false;
    }

    for (tinyxml2::XMLElement* filho = elemento->FirstChildElement(); filho != NULL; filho = filho->NextSiblingElement())
    {
        string etiqueta = filho->Name();

        if (etiqueta == "ID")
        {
            if (temId || filho->QueryIntText(&id) != tinyxml2::XML_SUCCESS || id <= 0)
            {
                return false;
            }
            temId = true;
        }
        else if (etiqueta == "MARCA")
        {
            const char* texto = filho->GetText();
            if (temMarca || texto == NULL || texto[0] == '\0')
            {
                return false;
            }
            marca = texto;
            temMarca = true;
        }
        else if (etiqueta == "VALOR_AVISO")
        {
            if (temAviso || filho->QueryDoubleText(&aviso) != tinyxml2::XML_SUCCESS)
            {
                return false;
            }
            temAviso = true;
        }
        else if (etiqueta == "POSICAO")
        {
            double eixoX = 0.0;
            double eixoY = 0.0;
            if (temPosicao || !lerPar(filho, eixoX, eixoY))
            {
                return false;
            }
            if (eixoX != static_cast<double>(static_cast<int>(eixoX)) || eixoY != static_cast<double>(static_cast<int>(eixoY)))
            {
                return false;
            }
            posX = static_cast<int>(eixoX);
            posY = static_cast<int>(eixoY);
            temPosicao = true;
        }
        else if (etiqueta == "PROB_AVARIA")
        {
            if (temProbabilidade || !lerProbabilidade(filho, probabilidade))
            {
                return false;
            }
            temProbabilidade = true;
        }
        else if (etiqueta == "PROB_1")
        {
            if ((tipo != "SFOGO" && tipo != "SMISSEL") || temProbabilidadeUm || !lerProbabilidade(filho, probabilidadeUm))
            {
                return false;
            }
            temProbabilidadeUm = true;
        }
        else if (etiqueta == "RAIO_VIZINHANCA")
        {
            if (tipo != "SHUMIDADE" || temRaio || filho->QueryIntText(&raio) != tinyxml2::XML_SUCCESS || raio < 0)
            {
                return false;
            }
            temRaio = true;
        }
        else
        {
            return false;
        }
    }

    bool exigeEvento = tipo == "SFOGO" || tipo == "SMISSEL";
    if (!temId || !temMarca || !temAviso || !temPosicao || !temProbabilidade)
    {
        return false;
    }

    if (exigeEvento && !temProbabilidadeUm)
    {
        return false;
    }

    if (!exigeEvento && temProbabilidadeUm)
    {
        return false;
    }

    if (!posicaoValida(posX, posY, limiteX, limiteY))
    {
        return false;
    }

    if (sensorDoMesmoTipo(novosSensores, id, tipo) || sensorDoMesmoTipo(sensores, id, tipo))
    {
        return false;
    }

    Sensor* sensor = NULL;
    if (tipo == "STEMPERATURA")
    {
        sensor = new SensorTemperatura(id, marca, 0.0, probabilidade, posX, posY, aviso);
    }
    else if (tipo == "SLUZ")
    {
        sensor = new SensorLuz(id, marca, aviso + 40.0, probabilidade, posX, posY, aviso);
    }
    else if (tipo == "SHUMIDADE")
    {
        sensor = new SensorHumidade(id, marca, 0.0, probabilidade, posX, posY, aviso, raio);
    }
    else if (tipo == "SFOGO")
    {
        sensor = new SensorFumo(id, marca, 0.0, probabilidade, posX, posY, aviso);
        sensor->SetProbabilidadeUm(probabilidadeUm);
    }
    else if (tipo == "SMISSEL")
    {
        sensor = new SensorMissel(id, marca, 0.0, probabilidade, posX, posY, aviso);
        sensor->SetProbabilidadeUm(probabilidadeUm);
    }

    if (sensor == NULL)
    {
        return false;
    }

    novosSensores.push_back(sensor);
    return true;
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

string Fabrica::GetNomeUtilizadorActual() const
{
    if (utilizadorAtual == NULL)
    {
        return "";
    }

    return utilizadorAtual->GetNome();
}

string Fabrica::GetTipoUtilizadorActual() const
{
    if (utilizadorAtual == NULL)
    {
        return "";
    }

    return utilizadorAtual->GetTipo();
}

bool Fabrica::MudarUtilizador(int id)
{
    for (list<User*>::iterator it = utilizadores.begin(); it != utilizadores.end(); ++it)
    {
        if ((*it)->GetId() == id)
        {
            utilizadorAtual = *it;
            return true;
        }
    }

    return false;
}

bool Fabrica::MudarTipoUtilizador(int perfil)
{
    if (utilizadorAtual == NULL)
    {
        return false;
    }

    string tipo;
    if (perfil == 1)
    {
        tipo = "ADMIN";
    }
    else if (perfil == 2)
    {
        tipo = "NORMAL";
    }
    else if (perfil == 3)
    {
        tipo = "VISITANTE";
    }
    else
    {
        return false;
    }

    if (utilizadorAtual->GetTipo() == tipo)
    {
        return true;
    }

    int id = utilizadorAtual->GetId();
    string nome = utilizadorAtual->GetNome();
    User* novo = NULL;

    if (perfil == 1)
    {
        novo = new Admin(id, nome);
    }
    else if (perfil == 2)
    {
        novo = new Normal(id, nome);
    }
    else
    {
        novo = new Visitante(id, nome);
    }

    for (list<User*>::iterator it = utilizadores.begin(); it != utilizadores.end(); ++it)
    {
        if (*it == utilizadorAtual)
        {
            delete *it;
            *it = novo;
            utilizadorAtual = novo;
            return true;
        }
    }

    delete utilizadorAtual;
    utilizadorAtual = novo;
    utilizadores.push_back(novo);
    return true;
}

void Fabrica::ListarUtilizadores(ostream& f) const
{
    f << endl << "=== Utilizadores ===" << endl;

    for (list<User*>::const_iterator it = utilizadores.begin(); it != utilizadores.end(); ++it)
    {
        f << "ID: " << (*it)->GetId()
          << " | Nome: " << (*it)->GetNome()
          << " | Tipo: " << (*it)->GetTipo()
          << endl;
    }
}
