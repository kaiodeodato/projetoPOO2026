#include "Fabrica.h"

#include <cctype>
#include <cstddef>
#include <cstdlib>
#include <cmath>
#include <fstream>
#include <sstream>

using namespace std;

Fabrica::Fabrica(User* ut)
    : utilizadorAtual(ut),
      campoX(0),
      campoY(0),
      horaAbertura(0),
      horaEncerramento(0),
      vizinhancaAviso(0),
      nomeFabrica("")
{
    verde.minimo    = 0.0; verde.maximo    = 0.0;
    amarelo.minimo  = 0.0; amarelo.maximo  = 0.0;
    vermelho.minimo = 0.0; vermelho.maximo = 0.0;

    defCombustao.verde.minimo = 0.0; defCombustao.verde.maximo = 0.0;
    defCombustao.amarelo.minimo = 0.0; defCombustao.amarelo.maximo = 0.0;
    defCombustao.vermelho.minimo = 0.0; defCombustao.vermelho.maximo = 0.0;
    defCombustao.probAvaria = 0.0;

    defEletrico.verde.minimo = 0.0; defEletrico.verde.maximo = 0.0;
    defEletrico.amarelo.minimo = 0.0; defEletrico.amarelo.maximo = 0.0;
    defEletrico.vermelho.minimo = 0.0; defEletrico.vermelho.maximo = 0.0;
    defEletrico.probAvaria = 0.0;

    defInducao.verde.minimo = 0.0; defInducao.verde.maximo = 0.0;
    defInducao.amarelo.minimo = 0.0; defInducao.amarelo.maximo = 0.0;
    defInducao.vermelho.minimo = 0.0; defInducao.vermelho.maximo = 0.0;
    defInducao.probAvaria = 0.0;

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

    list<string> blocoDefinicoes;
    if (!extrairBloco(linhas, "DEFINICOES", blocoDefinicoes))
    {
        return false;
    }

    int    novoX            = 0;
    int    novoY            = 0;
    int    novaAbertura     = 0;
    int    novoEncerramento = 0;
    int    novaVizinhanca   = 0;
    string novoNome;
    DefinicoesTipoMotor novoComb, novoElet, novoInd;

    novoComb.verde.minimo = 0.0; novoComb.verde.maximo = 0.0;
    novoComb.amarelo.minimo = 0.0; novoComb.amarelo.maximo = 0.0;
    novoComb.vermelho.minimo = 0.0; novoComb.vermelho.maximo = 0.0;
    novoComb.probAvaria = 0.0;
    novoElet = novoComb;
    novoInd  = novoComb;

    if (!lerDefinicoes(blocoDefinicoes, novoX, novoY,
                       novaAbertura, novoEncerramento,
                       novaVizinhanca, novoNome,
                       novoComb, novoElet, novoInd))
    {
        return false;
    }

    // lerMotores e lerSensores precisam destes valores, por isso sao aplicados ja.
    // Se a carga falhar, restaurarDefinicoes volta ao estado anterior.
    const int vizinhancaAnterior = vizinhancaAviso;
    const DefinicoesTipoMotor combAnterior = defCombustao;
    const DefinicoesTipoMotor eletAnterior = defEletrico;
    const DefinicoesTipoMotor indAnterior  = defInducao;

    vizinhancaAviso = novaVizinhanca;

    defCombustao = novoComb;
    defEletrico  = novoElet;
    defInducao   = novoInd;

    list<string> blocoMotores;
    list<Motor*> novosMotores;
    if (extrairBloco(linhas, "MOTORES", blocoMotores))
    {
        if (!lerMotores(blocoMotores, novoX, novoY, novosMotores))
        {
            apagarMotores(novosMotores);
            restaurarDefinicoes(vizinhancaAnterior, combAnterior, eletAnterior, indAnterior);
            return false;
        }
    }

    list<string> blocoSensores;
    list<Sensor*> novosSensores;
    if (extrairBloco(linhas, "SENSORES", blocoSensores))
    {
        if (!lerSensores(blocoSensores, novoX, novoY, novosSensores))
        {
            apagarMotores(novosMotores);
            apagarSensores(novosSensores);
            restaurarDefinicoes(vizinhancaAnterior, combAnterior, eletAnterior, indAnterior);
            return false;
        }
    }

    campoX           = novoX;
    campoY           = novoY;
    horaAbertura     = novaAbertura;
    horaEncerramento = novoEncerramento;
    nomeFabrica      = novoNome;

    verde   = novoComb.verde;
    amarelo = novoComb.amarelo;
    vermelho= novoComb.vermelho;

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

bool Fabrica::Add(Sensor* s)
{
    if (s == NULL || !podeAdicionar())
    {
        return false;
    }

    if (s->GetId() <= 0 || s->GetMarca().empty())
    {
        return false;
    }

    if (!posicaoValida(s->GetX(), s->GetY(), campoX, campoY))
    {
        return false;
    }

    if (idSensorExiste(sensores, s->GetId()))
    {
        return false;
    }

    sensores.push_back(s);
    return true;
}

bool Fabrica::RemoverUtilizador(int id)
{
    if (!podeRemover())
    {
        return false;
    }

    for (list<User*>::iterator it = utilizadores.begin(); it != utilizadores.end(); ++it)
    {
        if ((*it)->GetId() == id)
        {
            if (*it == utilizadorAtual)
            {
                return false;
            }
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
            return true;
        }
    }

    return false;
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

    for (size_t i = 0; i < Tipo.size(); ++i)
    {
        Tipo[i] = (char)toupper((unsigned char)Tipo[i]);
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

    if (resultado->empty())
    {
        f << "Nenhum motor do tipo \"" << Tipo << "\". Tipos disponiveis: MCOMBUSTAO, MELETRICO, MINDUCAO" << endl;
    }

    return resultado;
}

bool Fabrica::Manutencao()
{
    if (!podeManutencao())
    {
        return false;
    }

    for (list<Motor*>::iterator it = motores.begin(); it != motores.end(); ++it)
    {
        Motor* m = *it;
        IntervaloTemperatura ivVerde = m->GetIntervaloVerde();
        double amplitude = ivVerde.maximo - ivVerde.minimo;
        double novaTemp = ivVerde.minimo;
        if (amplitude > 0.0)
        {
            novaTemp = ivVerde.minimo + (rand() % 10001) / 10000.0 * amplitude;
        }
        m->SetTemperaturaAtual(novaTemp);
        m->Reparar();
    }

    for (list<Sensor*>::iterator it = sensores.begin(); it != sensores.end(); ++it)
    {
        (*it)->Repor();
    }

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

    saida << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
    saida << "<relatorio>" << endl;
    saida << "  <fabrica campo_x=\"" << campoX << "\" campo_y=\"" << campoY << "\""
          << " abertura=\"" << horaAbertura << "\" encerramento=\"" << horaEncerramento << "\"/>" << endl;

    saida << "  <motores>" << endl;
    for (list<Motor*>::const_iterator it = motores.begin(); it != motores.end(); ++it)
    {
        Motor* m = *it;
        saida << "    <motor id=\"" << m->GetId() << "\""
              << " tipo=\"" << m->GetTipo() << "\""
              << " marca=\"" << m->GetMarca() << "\""
              << " estado=\"" << textoEstado(m->GetEstado()) << "\""
              << " temperatura=\"" << m->GetTemperaturaAtual() << "\""
              << " classe=\"" << textoClasse(m->GetClasseTemperatura()) << "\""
              << " consumo_dia=\"" << m->GetConsumoDia() << "\""
              << " tempo_trabalho=\"" << m->GetTempoTrabalho() << "\""
              << " avarias=\"" << m->GetTotalAvarias() << "\"/>" << endl;
    }
    saida << "  </motores>" << endl;

    saida << "  <sensores>" << endl;
    for (list<Sensor*>::const_iterator it = sensores.begin(); it != sensores.end(); ++it)
    {
        Sensor* s = *it;
        saida << "    <sensor id=\"" << s->GetId() << "\""
              << " tipo=\"" << s->GetTipo() << "\""
              << " marca=\"" << s->GetMarca() << "\""
              << " valor=\"" << s->GetValor() << "\""
              << " aviso=\"" << (s->EmAviso() ? "sim" : "nao") << "\""
              << " avarias=\"" << s->GetTotalAvarias() << "\"/>" << endl;
    }
    saida << "  </sensores>" << endl;

    saida << "</relatorio>" << endl;
    saida.close();
}

int Fabrica::Aviso_Humidade(list<Motor*>& lm)
{
    lm.clear();

    for (list<Sensor*>::iterator its = sensores.begin(); its != sensores.end(); ++its)
    {
        Sensor* s = *its;
        if (s->GetTipo() != "HUMIDADE" || !s->EmAviso())
        {
            continue;
        }

        int raio = s->GetRaioVizinhanca();

        for (list<Motor*>::iterator itm = motores.begin(); itm != motores.end(); ++itm)
        {
            Motor* m = *itm;
            int dx = m->GetX() - s->GetX();
            int dy = m->GetY() - s->GetY();
            double dist = sqrt((double)(dx * dx + dy * dy));

            bool jaNaLista = false;
            for (list<Motor*>::iterator ild = lm.begin(); ild != lm.end(); ++ild)
            {
                if (*ild == m) { jaNaLista = true; break; }
            }

            if (dist <= raio && m->GetEstado() != STOP && m->GetEstado() != AVARIADO && !jaNaLista)
            {
                m->STOP();
                lm.push_back(m);
            }
        }
    }

    if (!lm.empty())
    {
        cout << "[AVISO GESTOR] Sensor de HUMIDADE em aviso! " << lm.size() << " motor(es) desligado(s)." << endl;
    }

    return (int)lm.size();
}

int Fabrica::Aviso_Fumo(list<Motor*>& lm, string fich_video)
{
    lm.clear();

    for (list<Motor*>::iterator it = motores.begin(); it != motores.end(); ++it)
    {
        Motor* m = *it;
        if (m->GetEstado() != STOP && m->GetEstado() != AVARIADO)
        {
            m->STOP();
            lm.push_back(m);
        }
    }

    cout << "[AVISO GESTOR] Sensor de FUMO em aviso! Todos os motores desligados." << endl;

    if (!fich_video.empty())
    {
        string cmd = "start \"\" \"" + fich_video + "\"";
        system(cmd.c_str());
    }

    return (int)lm.size();
}

int Fabrica::Aviso_Luz(string fich_video)
{
    cout << "[AVISO] Luminosidade baixa — bom regresso a casa!" << endl;

    if (!fich_video.empty())
    {
        string cmd = "start \"\" \"" + fich_video + "\"";
        system(cmd.c_str());
    }

    return 1;
}

void Fabrica::Aviso_Missel(string fvideo, string festado)
{
    cout << "[ALERTA CRITICO] Missel detetado! A desligar a fabrica!" << endl;

    for (list<Motor*>::iterator it = motores.begin(); it != motores.end(); ++it)
    {
        Motor* m = *it;
        if (m->GetEstado() != AVARIADO)
        {
            m->STOP();
        }
    }

    for (list<Sensor*>::iterator it = sensores.begin(); it != sensores.end(); ++it)
    {
        (*it)->Repor();
    }

    if (!festado.empty())
    {
        ofstream saida(festado.c_str());
        if (saida.is_open())
        {
            Listar(saida);
            saida.close();
        }
    }

    if (!fvideo.empty())
    {
        string cmd = "start \"\" \"" + fvideo + "\"";
        system(cmd.c_str());
    }
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
    saida << "<html lang=\"pt\">" << endl;
    saida << "<head><meta charset=\"UTF-8\"><title>Estado da Fabrica</title>" << endl;
    saida << "<style>" << endl;
    saida << "body{font-family:Arial,sans-serif;margin:20px;background:#f4f4f4;}" << endl;
    saida << "h1{color:#333;}h2{color:#555;}" << endl;
    saida << "table{border-collapse:collapse;width:100%;margin-bottom:24px;}" << endl;
    saida << "th{background:#4a90d9;color:#fff;padding:8px;}td{padding:7px;border:1px solid #ccc;}" << endl;
    saida << "tr:nth-child(even){background:#e9f0fb;}" << endl;
    saida << ".VERDE{background:#c8f7c5;}.AMARELO{background:#fff3b0;}.VERMELHO{background:#ffb3b3;}" << endl;
    saida << ".STOP{color:#888;}.RUN{color:#27ae60;font-weight:bold;}.AVARIADO{color:#e74c3c;font-weight:bold;}" << endl;
    saida << "</style></head>" << endl;
    saida << "<body>" << endl;
    saida << "<h1>Estado da Fabrica</h1>" << endl;
    saida << "<p>Campo: " << campoX << " x " << campoY
          << " &nbsp;|&nbsp; Horario: " << horaAbertura << "h - " << horaEncerramento << "h</p>" << endl;

    saida << "<h2>Motores</h2>" << endl;
    saida << "<table>" << endl;
    saida << "<tr><th>ID</th><th>Tipo</th><th>Marca</th><th>Posicao</th><th>Estado</th>"
          << "<th>Temperatura</th><th>Classe</th><th>Consumo/h</th><th>Consumo dia</th>"
          << "<th>Tempo trabalho (min)</th><th>Avarias</th></tr>" << endl;

    for (list<Motor*>::const_iterator it = motores.begin(); it != motores.end(); ++it)
    {
        Motor* m = *it;
        string classe = textoClasse(m->GetClasseTemperatura());
        string estado = textoEstado(m->GetEstado());
        saida << "<tr class=\"" << classe << "\">"
              << "<td>" << m->GetId() << "</td>"
              << "<td>" << m->GetTipo() << "</td>"
              << "<td>" << m->GetMarca() << "</td>"
              << "<td>(" << m->GetX() << ", " << m->GetY() << ")</td>"
              << "<td class=\"" << estado << "\">" << estado << "</td>"
              << "<td>" << m->GetTemperaturaAtual() << "</td>"
              << "<td>" << classe << "</td>"
              << "<td>" << m->GetConsumoHora() << "</td>"
              << "<td>" << m->GetConsumoDia() << "</td>"
              << "<td>" << m->GetTempoTrabalho() << "</td>"
              << "<td>" << m->GetTotalAvarias() << "</td>"
              << "</tr>" << endl;
    }
    saida << "</table>" << endl;

    saida << "<h2>Sensores</h2>" << endl;
    saida << "<table>" << endl;
    saida << "<tr><th>ID</th><th>Tipo</th><th>Marca</th><th>Posicao</th><th>Valor</th><th>Estado</th><th>Avarias</th></tr>" << endl;

    for (list<Sensor*>::const_iterator it = sensores.begin(); it != sensores.end(); ++it)
    {
        Sensor* s = *it;
        string aviso = s->EmAviso() ? "AVISO" : "Normal";
        string cor = s->EmAviso() ? " style=\"background:#ffb3b3;\"" : "";
        saida << "<tr" << cor << ">"
              << "<td>" << s->GetId() << "</td>"
              << "<td>" << s->GetTipo() << "</td>"
              << "<td>" << s->GetMarca() << "</td>"
              << "<td>(" << s->GetX() << ", " << s->GetY() << ")</td>"
              << "<td>" << s->GetValor() << "</td>"
              << "<td>" << aviso << "</td>"
              << "<td>" << s->GetTotalAvarias() << "</td>"
              << "</tr>" << endl;
    }
    saida << "</table>" << endl;

    saida << "</body></html>" << endl;
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

void Fabrica::restaurarDefinicoes(int vizinhanca, const DefinicoesTipoMotor& comb,
                                  const DefinicoesTipoMotor& elet, const DefinicoesTipoMotor& ind)
{
    vizinhancaAviso = vizinhanca;
    defCombustao    = comb;
    defEletrico     = elet;
    defInducao      = ind;
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

/** @brief Devolve o nome da tag de abertura (ex: "<MOTOR>" -> "MOTOR"). Vazio se nao for tag. */
string Fabrica::tagAbertura(const string& linha) const
{
    if (linha.empty() || linha[0] != '<' || linha.size() < 3)
    {
        return "";
    }

    if (linha[1] == '/' || linha[1] == '?')
    {
        return "";
    }

    size_t fim = linha.find('>');
    if (fim == string::npos)
    {
        return "";
    }

    return linha.substr(1, fim - 1);
}

/** @brief Verifica se a linha e a tag de fecho da tag indicada. */
bool Fabrica::eTagFecho(const string& linha, const string& tag) const
{
    string fecho = "</" + tag + ">";
    return linha == fecho;
}

/** @brief Extrai o texto entre <tag> e </tag> numa so linha. */
string Fabrica::valorEntreTag(const string& linha, const string& tag) const
{
    string abre  = "<"  + tag + ">";
    string fecha = "</" + tag + ">";

    size_t posA = linha.find(abre);
    if (posA == string::npos)
    {
        return "";
    }

    size_t inicio = posA + abre.size();
    size_t posF   = linha.find(fecha, inicio);
    if (posF == string::npos)
    {
        return "";
    }

    return linha.substr(inicio, posF - inicio);
}

/** @brief Le um inteiro da primeira linha do bloco que contenha <tag>N</tag>. */
bool Fabrica::lerInteiroDaTag(const list<string>& bloco, const string& tag, int& dest) const
{
    for (list<string>::const_iterator it = bloco.begin(); it != bloco.end(); ++it)
    {
        string val = valorEntreTag(*it, tag);
        if (!val.empty())
        {
            stringstream ss(val);
            int v = 0;
            if (ss >> v)
            {
                dest = v;
                return true;
            }
        }
    }

    return false;
}

/** @brief Le um double da primeira linha do bloco que contenha <tag>N</tag>. */
bool Fabrica::lerRealDaTag(const list<string>& bloco, const string& tag, double& dest) const
{
    for (list<string>::const_iterator it = bloco.begin(); it != bloco.end(); ++it)
    {
        string val = valorEntreTag(*it, tag);
        if (!val.empty())
        {
            stringstream ss(val);
            double v = 0.0;
            if (ss >> v)
            {
                dest = v;
                return true;
            }
        }
    }

    return false;
}

/** @brief Le o texto da primeira linha do bloco que contenha <tag>texto</tag>. */
string Fabrica::lerTextoDaTag(const list<string>& bloco, const string& tag) const
{
    for (list<string>::const_iterator it = bloco.begin(); it != bloco.end(); ++it)
    {
        string val = valorEntreTag(*it, tag);
        if (!val.empty())
        {
            return val;
        }
    }

    return "";
}

/** @brief Parse "a,b" ou "a, b" para dois doubles. */
bool Fabrica::lerPar(const string& texto, double& a, double& b) const
{
    size_t virgula = texto.find(',');
    if (virgula == string::npos)
    {
        return false;
    }

    string sa = texto.substr(0, virgula);
    string sb = texto.substr(virgula + 1);

    stringstream ssa(sa), ssb(sb);
    return (ssa >> a) && (ssb >> b);
}

/** @brief Le <tag>min,max</tag> de um bloco e preenche o intervalo. */
bool Fabrica::lerIntervaloBloco(const list<string>& bloco, const string& tag,
                                IntervaloTemperatura& intervalo) const
{
    string texto = lerTextoDaTag(bloco, tag);
    if (texto.empty())
    {
        return false;
    }

    double a = 0.0, b = 0.0;
    if (!lerPar(texto, a, b))
    {
        return false;
    }

    if (a > b)
    {
        return false;
    }

    intervalo.minimo = a;
    intervalo.maximo = b;
    return true;
}

/** @brief Le <POSICAO>x,y</POSICAO> de um bloco. */
bool Fabrica::lerPosicao(const list<string>& bloco, int& posX, int& posY) const
{
    string texto = lerTextoDaTag(bloco, "POSICAO");
    if (texto.empty())
    {
        return false;
    }

    double a = 0.0, b = 0.0;
    if (!lerPar(texto, a, b))
    {
        return false;
    }

    posX = (int)a;
    posY = (int)b;
    return posX >= 0 && posY >= 0;
}

/**
 * @brief Extrai o PRIMEIRO bloco <tag>...</tag> da lista de linhas.
 * O conteudo (linhas internas, sem as tags de abertura/fecho) fica em bloco.
 */
bool Fabrica::extrairBloco(const list<string>& linhas, const string& tag,
                           list<string>& bloco) const
{
    string abre  = "<"  + tag + ">";
    string fecha = "</" + tag + ">";
    bool dentro  = false;

    for (list<string>::const_iterator it = linhas.begin(); it != linhas.end(); ++it)
    {
        const string& l = *it;

        if (!dentro)
        {
            if (l == abre)
            {
                dentro = true;
            }
            continue;
        }

        if (l == fecha)
        {
            return true;
        }

        bloco.push_back(l);
    }

    return false;
}

/**
 * @brief Extrai TODOS os blocos <tag>...</tag> directos (nao aninhados) da lista.
 */
bool Fabrica::extrairTodosOsBlocos(const list<string>& linhas, const string& tag,
                                   list< list<string> >& blocos) const
{
    string abre  = "<"  + tag + ">";
    string fecha = "</" + tag + ">";
    bool dentro  = false;
    list<string> atual;

    for (list<string>::const_iterator it = linhas.begin(); it != linhas.end(); ++it)
    {
        const string& l = *it;

        if (!dentro)
        {
            if (l == abre)
            {
                dentro = true;
                atual.clear();
            }
            continue;
        }

        if (l == fecha)
        {
            blocos.push_back(atual);
            dentro = false;
            continue;
        }

        atual.push_back(l);
    }

    return !blocos.empty();
}

bool Fabrica::lerTipoMotorBloco(const list<string>& bloco, DefinicoesTipoMotor& def) const
{
    if (!lerIntervaloBloco(bloco, "VERDE",    def.verde))    { return false; }
    if (!lerIntervaloBloco(bloco, "AMARELO",  def.amarelo))  { return false; }
    if (!lerIntervaloBloco(bloco, "VERMELHO", def.vermelho)) { return false; }
    lerRealDaTag(bloco, "PROB_AVARIA", def.probAvaria);
    return true;
}

bool Fabrica::lerDefinicoes(const list<string>& bloco,
                            int& novoX, int& novoY,
                            int& novaAbertura, int& novoEncerramento,
                            int& novaVizinhanca, string& novoNome,
                            DefinicoesTipoMotor& combDef,
                            DefinicoesTipoMotor& eletDef,
                            DefinicoesTipoMotor& indDef) const
{
    novoNome = lerTextoDaTag(bloco, "NOME_EMPRESA");

    if (!lerInteiroDaTag(bloco, "HORA_INICIO", novaAbertura))  { return false; }
    if (!lerInteiroDaTag(bloco, "HORA_FECHO",  novoEncerramento)) { return false; }
    if (novaAbertura >= novoEncerramento)                       { return false; }

    lerInteiroDaTag(bloco, "VIZINHANCA_AVISO", novaVizinhanca);

    string dimTexto = lerTextoDaTag(bloco, "DIMENSAO_FABRICA");
    if (dimTexto.empty()) { return false; }
    double dx = 0.0, dy = 0.0;
    if (!lerPar(dimTexto, dx, dy)) { return false; }
    novoX = (int)dx;
    novoY = (int)dy;
    if (novoX <= 0 || novoY <= 0) { return false; }

    list<string> bComb, bElet, bInd;
    if (extrairBloco(bloco, "MCOMBUSTAO", bComb)) { lerTipoMotorBloco(bComb, combDef); }
    if (extrairBloco(bloco, "MELETRICO",  bElet)) { lerTipoMotorBloco(bElet, eletDef); }
    if (extrairBloco(bloco, "MINDUCAO",   bInd))  { lerTipoMotorBloco(bInd,  indDef); }

    return true;
}

Motor* Fabrica::criarMotorDoBloco(const list<string>& bloco, const string& tipo,
                                  const DefinicoesTipoMotor& def) const
{
    int    id      = 0;
    double consumo = 0.0;
    int    posX    = 0;
    int    posY    = 0;

    string marca = lerTextoDaTag(bloco, "MARCA");
    if (!lerInteiroDaTag(bloco, "ID", id) || marca.empty()) { return NULL; }
    if (!lerRealDaTag(bloco, "CONSUMO_HORA", consumo))       { return NULL; }
    if (!lerPosicao(bloco, posX, posY))                      { return NULL; }
    if (id <= 0 || consumo < 0.0)                            { return NULL; }

    if (tipo == "MCOMBUSTAO")
    {
        return new MotorCombustao(id, marca, consumo, def.probAvaria,
                                  posX, posY, def.verde, def.amarelo, def.vermelho);
    }
    if (tipo == "MELETRICO")
    {
        return new MotorEletrico(id, marca, consumo, def.probAvaria,
                                 posX, posY, def.verde, def.amarelo, def.vermelho);
    }
    if (tipo == "MINDUCAO")
    {
        return new MotorInducao(id, marca, consumo, def.probAvaria,
                                posX, posY, def.verde, def.amarelo, def.vermelho);
    }

    return NULL;
}

Sensor* Fabrica::criarSensorDoBloco(const list<string>& bloco, const string& tipo) const
{
    int    id    = 0;
    double prob  = 0.0;
    double valor = 0.0;
    int    posX  = 0;
    int    posY  = 0;

    string marca = lerTextoDaTag(bloco, "MARCA");
    if (!lerInteiroDaTag(bloco, "ID", id) || marca.empty()) { return NULL; }
    lerRealDaTag(bloco, "PROB_AVARIA", prob);
    lerRealDaTag(bloco, "VALOR_ATUAL", valor);
    if (!lerPosicao(bloco, posX, posY))                     { return NULL; }

    double limiar = 0.0;
    lerRealDaTag(bloco, "VALOR_AVISO", limiar);

    if (tipo == "SHUMIDADE")
    {
        int raio = vizinhancaAviso > 0 ? vizinhancaAviso : 5;
        return new SensorHumidade(id, marca, valor, prob, posX, posY, limiar, raio);
    }
    if (tipo == "SLUZ")
    {
        return new SensorLuz(id, marca, valor, prob, posX, posY, limiar);
    }
    if (tipo == "SFUMO")
    {
        return new SensorFumo(id, marca, valor, prob, posX, posY, limiar);
    }
    if (tipo == "SMISSEL" || tipo == "SMISSIL")
    {
        return new SensorMissel(id, marca, valor, prob, posX, posY);
    }
    if (tipo == "STEMPERATURA")
    {
        return new SensorTemperatura(id, marca, valor, prob, posX, posY, limiar);
    }
    if (tipo == "SFOGO")
    {
        return new SensorFogo(id, marca, valor, prob, posX, posY, limiar);
    }

    return NULL;
}

bool Fabrica::lerMotores(const list<string>& blocoMotores,
                         int limX, int limY, list<Motor*>& dest) const
{
    static const char* tipos[] = { "MCOMBUSTAO", "MELETRICO", "MINDUCAO", NULL };

    for (int t = 0; tipos[t] != NULL; ++t)
    {
        string tipo = tipos[t];
        list< list<string> > blocos;
        extrairTodosOsBlocos(blocoMotores, tipo, blocos);

        const DefinicoesTipoMotor* def = &defCombustao;
        if (tipo == "MELETRICO") { def = &defEletrico; }
        if (tipo == "MINDUCAO")  { def = &defInducao; }

        for (list< list<string> >::iterator it = blocos.begin(); it != blocos.end(); ++it)
        {
            Motor* m = criarMotorDoBloco(*it, tipo, *def);
            if (m == NULL)                                                      { return false; }
            if (!posicaoValida(m->GetX(), m->GetY(), limX, limY))               { delete m; return false; }
            if (idMotorExiste(dest, m->GetId()) || idMotorExiste(motores, m->GetId())) { delete m; return false; }
            dest.push_back(m);
        }
    }

    return true;
}

bool Fabrica::lerSensores(const list<string>& blocoSensores,
                          int limX, int limY, list<Sensor*>& dest) const
{
    static const char* tipos[] = {
        "SHUMIDADE", "SLUZ", "SFUMO", "SMISSEL", "SMISSIL",
        "STEMPERATURA", "SFOGO", NULL
    };

    for (int t = 0; tipos[t] != NULL; ++t)
    {
        string tipo = tipos[t];
        list< list<string> > blocos;
        extrairTodosOsBlocos(blocoSensores, tipo, blocos);

        for (list< list<string> >::iterator it = blocos.begin(); it != blocos.end(); ++it)
        {
            Sensor* s = criarSensorDoBloco(*it, tipo);
            if (s == NULL)                                                          { return false; }
            if (!posicaoValida(s->GetX(), s->GetY(), limX, limY))                   { delete s; return false; }
            if (idSensorExiste(dest, s->GetId()) || idSensorExiste(sensores, s->GetId())) { delete s; return false; }
            dest.push_back(s);
        }
    }

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

void Fabrica::Arrancar(int id_motor)
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

    motor->START();
    cout << "Motor " << id_motor << " em START." << endl;
}

void Fabrica::PassoSimulacao(int minutoAtual, int horaAtual, int minutosPasso, double grausPasso, double arrefPasso)
{
    bool algumEmRun = false;
    for (list<Motor*>::iterator it = motores.begin(); it != motores.end(); ++it)
    {
        if ((*it)->GetEstado() == RUN)
        {
            algumEmRun = true;
            break;
        }
    }

    double fracaoHora = minutosPasso / 60.0;

    for (list<Motor*>::iterator it = motores.begin(); it != motores.end(); ++it)
    {
        Motor* m = *it;

        if (m->GetEstado() == START)
        {
            if (horaAtual >= horaAbertura && horaAtual < horaEncerramento)
            {
                m->RUN();
            }
            else
            {
                cout << "[AVISO] Motor " << m->GetId()
                     << " (" << m->GetMarca() << ") aguarda abertura da fabrica (hora actual: "
                     << horaAtual << "h)." << endl;
            }
        }

        if (m->GetEstado() == RUN)
        {
            m->Aquecer(grausPasso);
            m->RegistarTrabalho(minutosPasso);
            m->ESTOU_QUENTE();

            double sorteio = (rand() % 10000) / 10000.0;
            if (sorteio < m->GetProbabilidadeAvaria() * fracaoHora)
            {
                if (m->Avariar())
                {
                    cout << "[AVISO GESTOR] Motor " << m->GetId()
                         << " (" << m->GetMarca() << ") AVARIOU." << endl;
                }
            }
        }
        else if (m->GetEstado() == STOP)
        {
            m->Arrefecer(arrefPasso);
        }
    }

    for (list<Sensor*>::iterator it = sensores.begin(); it != sensores.end(); ++it)
    {
        (*it)->Actualizar(algumEmRun, horaAtual, horaAbertura, horaEncerramento, fracaoHora);

        double sorteio = (rand() % 10000) / 10000.0;
        if (sorteio < (*it)->GetProbabilidadeAvaria() * fracaoHora && !(*it)->EstaAvariado())
        {
            (*it)->RegistarAvaria();
            cout << "[AVISO] Sensor " << (*it)->GetId()
                 << " (" << (*it)->GetTipo() << ") avariou." << endl;
        }
    }

    (void)minutoAtual;
}

void Fabrica::VerificarAvisosSensores(list<Motor*>& desligados)
{
    for (list<Sensor*>::iterator it = sensores.begin(); it != sensores.end(); ++it)
    {
        Sensor* s = *it;
        if (!s->Disparou())
        {
            continue;
        }

        string tipo = s->GetTipo();

        if (tipo == "HUMIDADE")
        {
            list<Motor*> lm;
            Aviso_Humidade(lm);
            for (list<Motor*>::iterator im = lm.begin(); im != lm.end(); ++im)
            {
                desligados.push_back(*im);
            }
        }
        else if (tipo == "FUMO")
        {
            list<Motor*> lm;
            Aviso_Fumo(lm, "");
            for (list<Motor*>::iterator im = lm.begin(); im != lm.end(); ++im)
            {
                desligados.push_back(*im);
            }
        }
        else if (tipo == "LUZ")
        {
            Aviso_Luz("");
        }
        else if (tipo == "MISSEL")
        {
            Aviso_Missel("", "Estado.txt");
        }
        else if (tipo == "TEMPERATURA")
        {
            cout << "[AVISO SENSOR] Temperatura critica detetada pelo sensor " << s->GetId() << "." << endl;
            for (list<Motor*>::iterator im = motores.begin(); im != motores.end(); ++im)
            {
                Motor* m = *im;
                if (m->GetEstado() != AVARIADO && m->GetEstado() != ::STOP)
                {
                    m->STOP();
                    desligados.push_back(m);
                }
            }
        }
        else if (tipo == "FOGO")
        {
            cout << "[ALERTA CRITICO] Fogo detetado pelo sensor " << s->GetId() << "! A parar todos os motores!" << endl;
            for (list<Motor*>::iterator im = motores.begin(); im != motores.end(); ++im)
            {
                Motor* m = *im;
                if (m->GetEstado() != AVARIADO)
                {
                    m->STOP();
                    desligados.push_back(m);
                }
            }
            for (list<Sensor*>::iterator is = sensores.begin(); is != sensores.end(); ++is)
            {
                (*is)->Repor();
            }
        }
    }
}

void Fabrica::ReiniciarConsumoDia()
{
    for (list<Motor*>::iterator it = motores.begin(); it != motores.end(); ++it)
    {
        (*it)->ReiniciarConsumoDia();
    }
}

void Fabrica::SetUtilizadorAtual(User* novo)
{
    if (novo == NULL)
    {
        return;
    }

    bool estaNaLista = false;
    for (list<User*>::iterator it = utilizadores.begin(); it != utilizadores.end(); ++it)
    {
        if (*it == utilizadorAtual)
        {
            estaNaLista = true;
            break;
        }
    }

    if (!estaNaLista && utilizadorAtual != NULL)
    {
        delete utilizadorAtual;
    }

    utilizadorAtual = novo;
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
