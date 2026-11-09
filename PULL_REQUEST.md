# Pull Request — feature/implementacao-melhorada

## Resumo

Implementação completa do projeto Gestão de Fábrica (POO 2026/2027), cobrindo todas as funcionalidades exigidas no enunciado.

---

## Novos ficheiros

### `include/TiposMotor.h` + `src/TiposMotor.cpp`
- Subclasses `MotorCombustao`, `MotorEletrico` e `MotorInducao`, derivadas de `Motor`
- Cada tipo tem os seus próprios intervalos de temperatura (VERDE / AMARELO / VERMELHO) e `probAvaria` lidos do XML
- `GetTipo()` devolve `"MCOMBUSTAO"`, `"MELETRICO"` ou `"MINDUCAO"` conforme o tipo

### `Doxyfile`
- Configuração para gerar documentação HTML com Doxygen
- Entrada: `include/` e `src/`; saída: `docs/html/`
- LaTeX desativado; browser de fonte ativado

---

## Ficheiros modificados

### `include/Sensor.h` + `src/Sensor.cpp`
- Adicionados `SensorTemperatura` e `SensorFogo`
  - `SensorTemperatura::Actualizar`: sobe 0,5 °C por passo com motores em RUN, desce 0,3 °C sem motores ativos; dispara ao atingir o limiar
  - `SensorFogo::Actualizar`: disparo probabilístico com motores em marcha (comportamento análogo ao `SensorMissel`)

### `include/Fabrica.h`
- Novo struct `DefinicoesTipoMotor` (verde/amarelo/vermelho/probAvaria)
- Novos membros privados: `vizinhancaAviso`, `nomeFabrica`, `defCombustao`, `defEletrico`, `defInducao`
- Novos métodos privados de parsing XML multi-linha: `tagAbertura`, `eTagFecho`, `valorEntreTag`, `lerInteiroDaTag`, `lerRealDaTag`, `lerTextoDaTag`, `lerPar`, `lerIntervaloBloco`, `lerPosicao`, `extrairBloco`, `extrairTodosOsBlocos`, `lerDefinicoes`, `lerTipoMotorBloco`, `criarMotorDoBloco`, `criarSensorDoBloco`, `lerMotores`, `lerSensores`
- Novos métodos públicos: `Add(Sensor*)`, `RemoverUtilizador(int)`, `RemoverMotor(int)`, `Arrancar(int)`, `PassoSimulacao(...)`, `VerificarAvisosSensores(...)`, `ReiniciarConsumoDia()`

### `src/Fabrica.cpp`
- `Load()` completamente reescrito para o formato XML multi-linha `<TAG>valor</TAG>` compatível com o `Simulador.exe` fornecido
- Bloco `<DEFINICOES>` com `HORA_INICIO`, `HORA_FECHO`, `DIMENSAO_FABRICA`, `VIZINHANCA_AVISO` e configuração por tipo de motor
- Todos os métodos do enunciado implementados:
  - `Manutencao()` — repõe temperatura de cada motor para um valor aleatório na zona VERDE
  - `Relatorio(fich_xml)` — grava XML com consumo diário de cada motor
  - `Listagem_HTML(fich_html)` — grava tabela HTML com estado atual de todos os dispositivos
  - `Aviso_Humidade(lm)` — desliga todos os motores na vizinhança do sensor; devolve contagem
  - `Aviso_Fumo(lm, fvideo)` — desliga todos os motores e invoca vídeo de evacuação
  - `Aviso_Luz(fvideo)` — invoca vídeo de bom regresso a casa
  - `Aviso_Missel(fvideo, festado)` — desliga tudo e guarda estado em ficheiro de texto
  - `PassoSimulacao(...)` — avança o tempo simulado; aquece/arrefece motores, pode avariar, notifica AMARELO/VERMELHO
  - `VerificarAvisosSensores(...)` — verifica todos os sensores e ativa os avisos no primeiro disparo
  - `ReiniciarConsumoDia()` — zera consumo diário no virar do dia

### `src/main.cpp`
- Reescrito com simulação contínua: cada iteração do loop equivale a 10 minutos simulados (500 ms reais)
- Tecla `M` / `m` pausa o simulador e abre o menu interativo (14 opções + S)
- Tecla `Q` / `q` termina o simulador
- Compatibilidade Windows (`_kbhit` / `_getch`) e Linux (`termios`, modo raw)

### `xml/fabrica.xml`
- Substituído pelo `Empresa1XML.xml` fornecido com o simulador
- Novo formato: `<DADOS> → <DEFINICOES> + <MOTORES> + <SENSORES>`

### `Makefile`
- Adicionado `src/TiposMotor.cpp` à lista de fontes

### `.gitignore`
- Adicionada entrada `docs/` (pasta gerada localmente pelo Doxygen)

---

## Funcionalidades implementadas (checklist do enunciado)

| Método | Estado |
|---|---|
| `Fabrica::Fabrica(User*)` | ✅ |
| `Fabrica::Load(string)` | ✅ |
| `Fabrica::Add(User*)` | ✅ |
| `Fabrica::Add(Motor*)` | ✅ |
| `Fabrica::Add(Sensor*)` | ✅ |
| `Fabrica::Listar(ostream&)` | ✅ |
| `Fabrica::Desligar(int)` | ✅ |
| `Fabrica::Get_Estado(int)` | ✅ |
| `Fabrica::Listar_Tipo(string, ostream&)` | ✅ |
| `Fabrica::Manutencao()` | ✅ |
| `Fabrica::Ranking_Dos_Fracos()` | ✅ |
| `Fabrica::Ranking_Dos_Mais_Trabalhadores()` | ✅ |
| `Fabrica::Relatorio(string)` | ✅ |
| `Fabrica::Aviso_Humidade(list<Motor*>&)` | ✅ |
| `Fabrica::Aviso_Fumo(list<Motor*>&, string)` | ✅ |
| `Fabrica::Aviso_Luz(string)` | ✅ |
| `Fabrica::Aviso_Missel(string, string)` | ✅ |
| `Fabrica::Listagem_HTML(string)` | ✅ |
| Subclasses Motor (Combustão/Eléctrico/Indução) | ✅ |
| Subclasses Sensor (Luz/Humidade/Fumo/Míssil/Temperatura/Fogo) | ✅ |
| Simulador contínuo com tecla M/Q | ✅ |
| Documentação Doxygen | ✅ |
| Gestão de memória (destrutor, sem `auto`) | ✅ |
