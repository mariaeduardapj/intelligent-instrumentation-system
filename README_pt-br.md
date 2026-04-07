![Status](https://img.shields.io/badge/status-em%20planejamento-yellow)
![License](https://img.shields.io/badge/license-MIT-blue)
![C++](https://img.shields.io/badge/language-C%2B%2B-blue)

# 📟 Painel de Instrumentação para Veículos

## 📌 Objetivo 
Desenvolver um painel de instrumentação inteligente para veículo elétrico autônomo, com monitoramento adaptativo em tempo real de parâmetros críticos do sistema.

## ℹ️ Descrição
Este projeto propõe o desenvolvimento de um sistema embarcado capaz de coletar, processar e exibir informações críticas do veículo, como velocidade, nível de bateria e temperatura, de forma eficiente e em tempo real.

A solução envolve a integração de sensores, microcontroladores e interfaces de comunicação, com foco em desempenho, confiabilidade e escalabilidade.

## 🏗️ System Architecture
O sistema está dividido em dois módulos principais:

- **Interface Homem-Máquina (IHM)**
    - Hardware: Raspberry Pi 5 com tela sensível ao toque
- **Módulo de Aquisição de Dados**
    - Hardware: ESP32 com sensores

Essa separação segue princípios fundamentais da engenharia de software:
- Baixo acoplamento
- Alta coesão
- Separação de responsabilidades

## 🚧 Status do Projeto
Em fase inicial de planejamento e levantamento teórico.

## 🛠️ Tecnologias previstas
- C/C++
- Computador de Placa Única
- Microcontroladores
- Sensores
- Display Touch
- Comunicação SPI
- Comunicação serial (UART, CAN, etc.)

## 🔬 Engineering Decisions
- Utilização de **C++** para desenvolvimento de firmware
- Arquitetura modular para escalabilidade
- Utilização de ferramentas de compilação cruzada
- Ênfase em testabilidade e validação do sistema
- Monitoramento de:
    - Taxa de atualização
    - Consumo de energia
    - Estabilidade do sistema

## 🎯 Objetivos Futuros
- Definição da arquitetura do sistema
- Integração com sensores reais
- Implementação do firmware inicial
- Testes em bancada
- Otimização de desempenho (latência, consumo de energia, comportamento térmico)

## 📚 Contexto da Pesquisa
Este projeto faz parte de uma iniciativa de pesquisa de graduação focada em sistemas embarcados e interfaces inteligentes para aplicações automotivas, do grupo de pesquisa Siva.

## 📄 Licença
Este projeto está sob a licença MIT - veja o arquivo LICENSE.md para detalhes.

---

Desenvolvido por **Maria Eduarda P. de Jesus** 

