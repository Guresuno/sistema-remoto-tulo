<div align="center">

# 🎮 Sistema Remoto TULO

**Motor de Streaming de Juegos de Ultra Baja Latencia (Zero-Copy) para Windows**

[![C++](https://img.shields.io/badge/C++-20-blue.svg)](https://isocpp.org/)
[![Platform](https://img.shields.io/badge/Platform-Windows-lightgrey.svg)]()
[![License](https://img.shields.io/badge/License-MIT-green.svg)]()
[![Status](https://img.shields.io/badge/Status-Prototype-orange.svg)]()

<p align="center">
  TULO es un motor de transmisión en tiempo real creado por mi para que julio no tenga el tiempo de espera del jifors nau <br>
  TULO IS LOVE TULO IS LIFE <br>
  swilfy es sendo pato y le gusta el yupi
</p>

### 📥 Descargas Oficiales (v1.0.0)

> **Instaladores automatizados incluidos. No requieren consola.**

[![Descargar Servidor](https://img.shields.io/badge/Descargar-TULO_Server-red?style=for-the-badge&logo=amd)](https://github.com/Guresuno/sistema-remoto-tulo/releases/download/v1.0.0/TuloServer.zip)
[![Descargar Cliente](https://img.shields.io/badge/Descargar-TULO_Client-blue?style=for-the-badge&logo=windows)](https://github.com/Guresuno/sistema-remoto-tulo/releases/download/v1.0.0/TuloClient.zip)

</div>

---
## 🏗️ Arquitectura del Sistema

El proyecto se divide en dos componentes principales:

### TuloServer (Host)
1. Captura la pantalla a 60 FPS (o más) usando D3D11/DXGI.
2. Inyecta la textura cruda en el codificador H.264 de AMD.
3. Envía los fragmentos de video (*NAL Units*) a través de un socket UDP.
4. Recibe eventos del mando desde la red e inyecta las pulsaciones usando el driver **ViGEmBus**.

### TuloClient (Receptor)
Aplicación ligera que se ejecuta en la máquina remota:
1. Ensambla los paquetes UDP.
2. Decodifica el flujo H.264 mediante aceleración por hardware local.
3. Lee el estado del mando de Xbox local y lo transmite al servidor.

## ⚙️ Requisitos del Sistema

*   **Host (Servidor):**
    *   Windows 10 / 11 (64-bit)
    *   Tarjeta Gráfica AMD (RX Series) soportada por AMD AMF.
    *   [ViGEmBus Driver](https://github.com/nefarius/ViGEmBus) instalado.
*   **Cliente:**
    *   La canaimita de Julio
