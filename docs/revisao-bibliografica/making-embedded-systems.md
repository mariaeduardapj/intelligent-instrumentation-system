# 📚 Making Embedded Systems – Revisão Aplicada

## 📖 Referência

Making Embedded Systems
Elecia White
2ª edição, O’Reilly, 2022.

---

## 🎯 Objetivo

Correlacionar os conceitos do livro com o desenvolvimento do projeto:

> *Painel de Instrumentação Inteligente para Veículo Elétrico Autônomo com Monitoramento Adaptativo em Tempo Real*

---

# 📌 Capítulo 1 — Fundamentos de Sistemas Embarcados

## Conceitos principais

* Sistemas embarcados possuem função específica, diferente de computadores de uso geral
* Uso de **cross-compiler** para compilar código para outra arquitetura. Importância de:

  * Coesão
  * Baixo acoplamento
* Necessidade de **cross-debugger**. Dois grandes desafios:

  * Depuração em ambiente limitado
  * Restrição de recursos (RAM, CPU, energia, etc.)



## Aplicação no projeto

O sistema foi dividido em dois módulos principais:

* **HMI (Interface)**

  * Hardware: Raspberry Pi 5 + display touch

* **Aquisição de dados**

  * Hardware: ESP32 + sensores

Essa divisão segue o princípio de:

* Separação de responsabilidades
* Baixo acoplamento



## Decisões de engenharia

* Uso de **C++** para firmware
* Desenvolvimento em arquitetura diferente → uso de cross-compiler
* Implementação de testes antes e depois da integração
* Monitoramento de:

  * Taxa de atualização
  * Consumo de energia



## Estratégia de debug

* Detecção de falhas em sensores
* Problemas de comunicação
* Travamentos de interface
* Inconsistência de dados



## Foco futuro

* Otimização de:

  * Velocidade
  * Corrente
  * Temperatura
* Priorização de tarefas no sistema embarcado

---

# 📌 Capítulo 2 — Arquitetura de Sistemas

## Conceitos principais

* Projetos bons evoluem de arquiteturas bem definidas
* Uso de diagramas:

  * Contexto
  * Blocos
  * Organizacional
  * Camadas



## Princípios de arquitetura

* Encapsulamento
* Redução de dependências
* Separação em módulos
* Proteção contra mudanças futuras



## Boas práticas

* Uso de drivers bem estruturados
* Padrão **POSIX** para interface com sistema
* Uso de **Adapter** para desacoplamento de hardware
* Sistema de logs eficiente (evitar `printf`)
* Controle de versões no sistema



## Padrões aplicados

* **MVC (Model-View-Controller)**:

  * Model → dados dos sensores
  * View → display
  * Controller → lógica de interação

* Uso de **sandbox** para testes



## Aplicação no projeto

### Arquitetura definida

* Separação entre:

  * Aquisição de dados
  * Processamento
  * Interface

### Recursos críticos identificados

* Barramento CAN
* Histórico de dados
* Uso de CPU



## Decisões de engenharia

* Uso de adaptadores para permitir troca de:

  * Sensores
  * Protocolos
  * Display

* Componentes descritos de forma genérica (flexibilidade do projeto)

---

# 📌 Capítulo 3 — Hardware e Integração

## Conceitos principais

* Desenvolvimento embarcado exige:

  * Conhecimento de software
  * Conhecimento de hardware

* Software geralmente é desenvolvido após hardware inicial



## Processo de hardware

1. Análise de datasheets
2. Definição de componentes
3. Projeto de PCB
4. Testes de hardware
5. Integração com software



## Estratégias de teste

* Testar componentes individualmente
* Priorizar partes críticas
* Evoluir de simples → complexo
* Testes por terceiros



## Seleção de componentes

Critérios:

* Requisitos essenciais
* Requisitos desejáveis
* Custo
* Disponibilidade
* Complexidade de implementação



## Aplicação no projeto

### Escolha da SBC

Comparação entre:

* Raspberry Pi 5
* Orange Pi 5
* Rock Pi 4
* Banana Pi M5

Resultado: **Raspberry Pi 5** escolhido por maior equilíbrio geral



## Boas práticas adotadas

* Leitura de datasheets
* Planejamento antes da montagem
* Testes elétricos prévios
* Uso de equipamentos adequados

---

# ⚙️ Conclusão Geral

Os conceitos do livro fundamentam diretamente o desenvolvimento do projeto, principalmente em:

* Arquitetura de software
* Organização de firmware
* Integração hardware-software
* Estratégias de teste e debug

O projeto segue princípios de engenharia voltados para:

* Robustez
* Escalabilidade
* Manutenibilidade


