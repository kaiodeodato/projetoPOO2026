#include "Fabrica.h"

#include <cstddef>
#include <fstream>
#include <sstream>

using namespace std;

Fabrica::Fabrica(User* ut)
    : utilizadorAtual(ut),
      campoX(0),
      campoY(0),
      horaAbertura(0),
      horaEncerramento(0)
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

    int novoX = 0;
    int novoY = 0;
    int novaAbertura = 0;
    int novoEncerramento = 0;
    IntervaloTemperatura novoVerde = {0.0, 0.0};
    IntervaloTemperatura novoAmarelo = {0.0, 0.0};
    IntervaloTemperatura novoVermelho = {0.0, 0.0};

    if (!lerEstrutura(linhas, novoX, novoY, novaAbertura, novoEncerramento,
                      novoVerde, novoAmarelo, novoVermelho))
    {
        return false;
    }

    list<User*> novosUsers;
    list<Motor*> novosMotores;
    list<Sensor*> novosSensores;

    if (!lerConteudo(linhas, novoX, novoY, novosUsers, novosMotores, novosSensores))
    {
        libertarCargaParcial(novosUsers, novosMotores, novosSensores);
        return false;
    }

    campoX = novoX;
    campoY = novoY;
    horaAbertura = novaAbertura;
    horaEncerramento = novoEncerramento;
    verde = novoVerde;
    amarelo = novoAmarelo;
    vermelho = novoVermelho;

    moverUtilizadores(novosUsers);
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

    f << "=== Fabrica ===" << endl;
    f << "Campo: " << campoX << " x " << campoY << endl;
    f << "Horario: " << horaAbertura << " - " << horaEncerramento << endl;

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
          << " | valor " << s->GetValor();

        if (s->EmAviso())
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
    if (!podeManutencao())
    {
        return false;
    }

    // Fase seguinte: por cada motor, escolher uma temperatura aleatoria
    // dentro do intervalo VERDE carregado do XML.
    return false;
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

void Fabrica::Relatorio(string fich_xml) const
{
    if (!podeListar() || fich_xml.empty())
    {
        return;
    }

    // Fase seguinte: gravar o XML com o estado e o consumo do dia de cada motor.
}

int Fabrica::Aviso_Humidade(list<Motor*>& lm)
{
    lm.clear();

    // Fase seguinte: desligar os motores dentro do raio de cada sensor
    // de humidade em aviso, avisar o gestor e devolver a contagem.
    return 0;
}

int Fabrica::Aviso_Fumo(list<Motor*>& lm, string fich_video)
{
    lm.clear();

    if (fich_video.empty())
    {
        return 0;
    }

    // Fase seguinte: desligar todos os motores e mostrar o video das saidas.
    return 0;
}

int Fabrica::Aviso_Luz(string fich_video)
{
    if (fich_video.empty())
    {
        return 0;
    }

    // Fase seguinte: mostrar o video de regresso a casa.
    return 0;
}

void Fabrica::Aviso_Missel(string fvideo, string festado)
{
    if (fvideo.empty() || festado.empty())
    {
        return;
    }

    // Fase seguinte: mostrar os abrigos, gravar o estado e desligar a fabrica.
}

void Fabrica::Listagem_HTML(string fich_html) const
{
    if (!podeListar() || fich_html.empty())
    {
        return;
    }

    // Fase seguinte: gravar um HTML com o estado actual, para abrir no browser.
}

int Fabrica::GetCampoX() const
{
    return campoX;
}

int Fabrica::GetCampoY() const
{
    return campoY;
}

bool Fabrica::podeAdicionar() const
{
    if (utilizadorAtual == NULL)
    {
        return false;
    }

    return utilizadorAtual->PodeAdicionar();
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

bool Fabrica::posicaoValida(int posX, int posY, int limiteX, int limiteY) const
{
    if (limiteX <= 0 || limiteY <= 0)
    {
        return false;
    }

    if (posX <= 0 || posY <= 0)
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

    entrada.close();
    return true;
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

bool Fabrica::linhaDoTipo(const string& linha, const string& tag) const
{
    string marca = "<" + tag;
    if (linha.size() < marca.size())
    {
        return false;
    }

    if (linha.compare(0, marca.size(), marca) != 0)
    {
        return false;
    }

    if (linha.size() == marca.size())
    {
        return true;
    }

    char seguinte = linha[marca.size()];
    return seguinte == ' ' || seguinte == '>' || seguinte == '/';
}

bool Fabrica::linhaIgnorada(const string& linha) const
{
    if (linha.compare(0, 2, "<?") == 0)
    {
        return true;
    }

    if (linha.size() >= 4 && linha.compare(0, 4, "<!--") == 0)
    {
        return true;
    }

    if (linha.compare(0, 2, "</") == 0)
    {
        return true;
    }

    if (linhaDoTipo(linha, "fabrica"))
    {
        return true;
    }

    if (linhaDoTipo(linha, "utilizadores"))
    {
        return true;
    }

    if (linhaDoTipo(linha, "sensores"))
    {
        return true;
    }

    if (linhaDoTipo(linha, "motores"))
    {
        return true;
    }

    return false;
}

bool Fabrica::linhaEstrutural(const string& linha) const
{
    if (linhaDoTipo(linha, "dimensoes"))
    {
        return true;
    }

    if (linhaDoTipo(linha, "horario"))
    {
        return true;
    }

    if (linhaDoTipo(linha, "verde"))
    {
        return true;
    }

    if (linhaDoTipo(linha, "amarelo"))
    {
        return true;
    }

    if (linhaDoTipo(linha, "vermelho"))
    {
        return true;
    }

    return false;
}

string Fabrica::lerAtributo(const string& linha, const string& atributo) const
{
    string chave = atributo + "=\"";
    size_t pos = 0;

    while (pos < linha.size())
    {
        pos = linha.find(chave, pos);
        if (pos == string::npos)
        {
            return "";
        }

        if (pos == 0 || linha[pos - 1] == ' ')
        {
            size_t inicio = pos + chave.size();
            size_t fim = linha.find('"', inicio);
            if (fim == string::npos)
            {
                return "";
            }

            return linha.substr(inicio, fim - inicio);
        }

        pos = pos + chave.size();
    }

    return "";
}

bool Fabrica::atributoInteiro(const string& linha, const string& atributo, int& destino) const
{
    string texto = lerAtributo(linha, atributo);
    if (texto.empty())
    {
        return false;
    }

    stringstream fluxo(texto);
    int valor = 0;
    char resto = '\0';

    if (!(fluxo >> valor))
    {
        return false;
    }

    if (fluxo >> resto)
    {
        return false;
    }

    destino = valor;
    return true;
}

bool Fabrica::atributoReal(const string& linha, const string& atributo, double& destino) const
{
    string texto = lerAtributo(linha, atributo);
    if (texto.empty())
    {
        return false;
    }

    stringstream fluxo(texto);
    double valor = 0.0;
    char resto = '\0';

    if (!(fluxo >> valor))
    {
        return false;
    }

    if (fluxo >> resto)
    {
        return false;
    }

    destino = valor;
    return true;
}

bool Fabrica::lerLocalizacao(const string& linha, int& id, string& marca,
                             double& probabilidade, int& posX, int& posY) const
{
    if (!atributoInteiro(linha, "id", id))
    {
        return false;
    }

    marca = lerAtributo(linha, "marca");
    if (marca.empty())
    {
        return false;
    }

    if (!atributoReal(linha, "probabilidade_avaria", probabilidade))
    {
        return false;
    }

    if (probabilidade < 0.0 || probabilidade > 1.0)
    {
        return false;
    }

    if (!atributoInteiro(linha, "x", posX) || !atributoInteiro(linha, "y", posY))
    {
        return false;
    }

    return id > 0;
}

bool Fabrica::lerDimensoes(const string& linha, int& novoX, int& novoY) const
{
    if (!atributoInteiro(linha, "x", novoX) || !atributoInteiro(linha, "y", novoY))
    {
        return false;
    }

    return novoX > 0 && novoY > 0;
}

bool Fabrica::lerHorario(const string& linha, int& abertura, int& encerramento) const
{
    if (!atributoInteiro(linha, "abertura", abertura) || !atributoInteiro(linha, "encerramento", encerramento))
    {
        return false;
    }

    if (!horaValida(abertura) || !horaValida(encerramento))
    {
        return false;
    }

    return abertura < encerramento;
}

bool Fabrica::lerIntervalo(const string& linha, IntervaloTemperatura& intervalo) const
{
    if (!atributoReal(linha, "min", intervalo.minimo) || !atributoReal(linha, "max", intervalo.maximo))
    {
        return false;
    }

    return intervalo.minimo <= intervalo.maximo;
}

bool Fabrica::lerEstrutura(const list<string>& linhas, int& novoX, int& novoY,
                           int& novaAbertura, int& novoEncerramento,
                           IntervaloTemperatura& novoVerde, IntervaloTemperatura& novoAmarelo,
                           IntervaloTemperatura& novoVermelho) const
{
    bool temCampo = false;
    bool temHorario = false;
    bool temVerde = false;
    bool temAmarelo = false;
    bool temVermelho = false;

    for (list<string>::const_iterator it = linhas.begin(); it != linhas.end(); ++it)
    {
        const string& linha = *it;

        if (linhaDoTipo(linha, "dimensoes"))
        {
            if (temCampo || !lerDimensoes(linha, novoX, novoY))
            {
                return false;
            }
            temCampo = true;
        }
        else if (linhaDoTipo(linha, "horario"))
        {
            if (temHorario || !lerHorario(linha, novaAbertura, novoEncerramento))
            {
                return false;
            }
            temHorario = true;
        }
        else if (linhaDoTipo(linha, "verde"))
        {
            if (temVerde || !lerIntervalo(linha, novoVerde))
            {
                return false;
            }
            temVerde = true;
        }
        else if (linhaDoTipo(linha, "amarelo"))
        {
            if (temAmarelo || !lerIntervalo(linha, novoAmarelo))
            {
                return false;
            }
            temAmarelo = true;
        }
        else if (linhaDoTipo(linha, "vermelho"))
        {
            if (temVermelho || !lerIntervalo(linha, novoVermelho))
            {
                return false;
            }
            temVermelho = true;
        }
    }

    if (!temCampo || !temHorario || !temVerde || !temAmarelo || !temVermelho)
    {
        return false;
    }

    return intervalosCoerentes(novoVerde, novoAmarelo, novoVermelho);
}

bool Fabrica::lerConteudo(const list<string>& linhas, int limiteX, int limiteY,
                          list<User*>& novosUsers, list<Motor*>& novosMotores, list<Sensor*>& novosSensores) const
{
    for (list<string>::const_iterator it = linhas.begin(); it != linhas.end(); ++it)
    {
        const string& linha = *it;

        if (linhaIgnorada(linha) || linhaEstrutural(linha))
        {
            continue;
        }

        if (linhaDoTipo(linha, "admin") || linhaDoTipo(linha, "normal") || linhaDoTipo(linha, "visitante"))
        {
            if (!registarUtilizadorXml(linha, novosUsers))
            {
                return false;
            }
            continue;
        }

        if (linhaDoTipo(linha, "luz") || linhaDoTipo(linha, "humidade") || linhaDoTipo(linha, "fumo") || linhaDoTipo(linha, "missel"))
        {
            if (!registarSensorXml(linha, limiteX, limiteY, novosSensores))
            {
                return false;
            }
            continue;
        }

        if (linhaDoTipo(linha, "motor"))
        {
            if (!registarMotorXml(linha, limiteX, limiteY, novosMotores))
            {
                return false;
            }
            continue;
        }

        return false;
    }

    return true;
}

User* Fabrica::criarUtilizador(const string& linha) const
{
    int id = 0;
    if (!atributoInteiro(linha, "id", id))
    {
        return NULL;
    }

    string nome = lerAtributo(linha, "nome");
    if (id <= 0 || nome.empty())
    {
        return NULL;
    }

    if (linhaDoTipo(linha, "admin"))
    {
        return new Admin(id, nome);
    }

    if (linhaDoTipo(linha, "normal"))
    {
        return new Normal(id, nome);
    }

    if (linhaDoTipo(linha, "visitante"))
    {
        return new Visitante(id, nome);
    }

    return NULL;
}

Sensor* Fabrica::criarSensor(const string& linha) const
{
    int id = 0;
    string marca;
    double probabilidade = 0.0;
    int posX = 0;
    int posY = 0;
    double valor = 0.0;

    if (!lerLocalizacao(linha, id, marca, probabilidade, posX, posY))
    {
        return NULL;
    }

    if (!atributoReal(linha, "valor", valor))
    {
        return NULL;
    }

    if (linhaDoTipo(linha, "luz"))
    {
        double limiar = 0.0;
        if (!atributoReal(linha, "limiar", limiar))
        {
            return NULL;
        }

        return new SensorLuz(id, marca, valor, probabilidade, posX, posY, limiar);
    }

    if (linhaDoTipo(linha, "humidade"))
    {
        double limiar = 0.0;
        int raio = 0;
        if (!atributoReal(linha, "limiar", limiar) || !atributoInteiro(linha, "raio", raio) || raio <= 0)
        {
            return NULL;
        }

        return new SensorHumidade(id, marca, valor, probabilidade, posX, posY, limiar, raio);
    }

    if (linhaDoTipo(linha, "fumo"))
    {
        double limiar = 0.0;
        if (!atributoReal(linha, "limiar", limiar))
        {
            return NULL;
        }

        return new SensorFumo(id, marca, valor, probabilidade, posX, posY, limiar);
    }

    if (linhaDoTipo(linha, "missel"))
    {
        return new SensorMissel(id, marca, valor, probabilidade, posX, posY);
    }

    return NULL;
}

Motor* Fabrica::criarMotor(const string& linha) const
{
    int id = 0;
    string marca;
    double probabilidade = 0.0;
    int posX = 0;
    int posY = 0;
    double consumo = 0.0;
    double aviso = 0.0;
    double paragem = 0.0;

    string tipo = lerAtributo(linha, "tipo");
    if (tipo.empty())
    {
        return NULL;
    }

    if (!lerLocalizacao(linha, id, marca, probabilidade, posX, posY))
    {
        return NULL;
    }

    if (!atributoReal(linha, "consumo_hora", consumo) || !atributoReal(linha, "temperatura_aviso", aviso) || !atributoReal(linha, "temperatura_paragem", paragem))
    {
        return NULL;
    }

    if (consumo < 0.0 || aviso >= paragem)
    {
        return NULL;
    }

    return new Motor(id, tipo, marca, consumo, aviso, paragem, probabilidade, posX, posY);
}

bool Fabrica::registarUtilizadorXml(const string& linha, list<User*>& destino) const
{
    User* utilizador = criarUtilizador(linha);
    if (utilizador == NULL)
    {
        return false;
    }

    if (idUtilizadorExiste(destino, utilizador->GetId()))
    {
        delete utilizador;
        return false;
    }

    if (idUtilizadorExiste(utilizadores, utilizador->GetId()))
    {
        delete utilizador;
        return true;
    }

    destino.push_back(utilizador);
    return true;
}

bool Fabrica::registarSensorXml(const string& linha, int limiteX, int limiteY, list<Sensor*>& destino) const
{
    Sensor* sensor = criarSensor(linha);
    if (sensor == NULL)
    {
        return false;
    }

    if (!posicaoValida(sensor->GetX(), sensor->GetY(), limiteX, limiteY) || idSensorExiste(destino, sensor->GetId()) || idSensorExiste(sensores, sensor->GetId()))
    {
        delete sensor;
        return false;
    }

    destino.push_back(sensor);
    return true;
}

bool Fabrica::registarMotorXml(const string& linha, int limiteX, int limiteY, list<Motor*>& destino) const
{
    Motor* motor = criarMotor(linha);
    if (motor == NULL)
    {
        return false;
    }

    if (!posicaoValida(motor->GetX(), motor->GetY(), limiteX, limiteY) || idMotorExiste(destino, motor->GetId()) || idMotorExiste(motores, motor->GetId()))
    {
        delete motor;
        return false;
    }

    destino.push_back(motor);
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
