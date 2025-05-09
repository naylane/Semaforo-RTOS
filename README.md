<div>
    <img src="./img\embarcatech_logo.png" alt="Logo Embarcatech" height="90">
    <img align="right" src="./img\cepedi_logo.png" alt="Logo Embarcatech" height="90">
</div>

<h2 align="center"> Semáforo Inteligente com Modo Noturno e Acessibilidade </h2>
<h5 align="center"> Atividade promovida pela Residência (fase 2) do programa EmbarcaTech. </h5>

<br>

## Sobre o projeto
O projeto pretende simular o funcionamento de um semáforo inteligente com recursos adicionais, como **modo noturno**, **alertas sonoros para acessibilidade** e **exibição de informações visuais** em um display OLED e matriz de LEDs. A descrição completa do projeto pode ser encontrada na **[Ficha de proposta de projeto](docs\TrabalhoSE_FSA_3_Naylane_Ribeiro.pdf)**.

## Tecnologias e Ferramentas utilizadas
- **Kit BitDogLab**
- **FreeRTOS**
- **Linguagem de programação:** C
- **Ambiente de Desenvolvimento:** Visual Studio Code


## Instruções de uso
- É necessário instalar o SDK da Raspberry Pi Pico e realizar as configurações iniciais.

1. Para ter acesso ao projeto, **clone o repositório**:
```bash
git clone https://github.com/naylane/Semaforo-RTOS.git
```
2. Certifique de configurar o projeto, checando o ```CMakeLists.txt``` e o SDK.
3. Compile o projeto pela extensão da Raspberry Pi Pico ou terminal. Opção pelo terminal abaixo:
```bash
mkdir build && cd build
cmake ..
make 
```
4. Coloque a BitDogLab em modo BOOTSEL (pressione o botão BOOTSEL ao conectar o cabo USB).


## Desenvolvedora
<table>
  <tr>
    <td align="center"><img style="" src="https://avatars.githubusercontent.com/u/89545660?v=4" width="100px;" ><br /> <sub> <b> Naylane Ribeiro </b> </sub>
    </td>
</table>

<img width=100% src="https://capsule-render.vercel.app/api?type=waving&color=2c9ad6&height=120&section=footer"/>
