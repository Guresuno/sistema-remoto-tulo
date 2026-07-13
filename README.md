<div align="center">

# 🎮 Sistema Remoto TULO

**Motor de Streaming de Juegos de Ultra Baja Latencia (Zero-Copy) para Windows**

[![C++](https://img.shields.io/badge/C++-20-blue.svg)](https://isocpp.org/)
[![Platform](https://img.shields.io/badge/Platform-Windows-lightgrey.svg)]()
[![License](https://img.shields.io/badge/License-MIT-green.svg)]()
[![Status](https://img.shields.io/badge/Status-Prototype-orange.svg)]()

<p align="center">
  TULO es un motor de transmisión en tiempo real de bajo nivel diseñado para rivalizar con soluciones comerciales como GeForce Now o Parsec, construido directamente sobre DirectX 11 y AMD Advanced Media Framework (AMF).
</p>

</div>

---

## ✨ Características Principales

*   🚀 **Captura Zero-Copy:** Utiliza la API *DXGI Desktop Duplication* para extraer fotogramas directamente de la VRAM sin pasar por la CPU.
*   🎥 **Codificación por Hardware (AMF):** Integración nativa con tarjetas gráficas AMD Radeon para codificación H.264/HEVC en milisegundos usando el perfil `ULTRA_LOW_LATENCY`.
*   🕹️ **Multi-Seat / Juego en Segundo Plano:** Arquitectura diseñada para permitir jugar de forma remota en un monitor virtual usando un mando, sin interferir con el usuario físico de la PC (aislamiento de ratón/teclado).
*   ⚡ **Transmisión UDP Directa:** Diseñado para minimizar el *jitter* y descartar paquetes viejos en favor de la latencia absoluta.

## 🏗️ Arquitectura del Sistema

El proyecto se divide en dos componentes principales:

### TuloServer (Host)
El corazón del sistema. Se ejecuta en la máquina de juegos (Windows) y realiza las siguientes tareas de forma continua:
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
*   **Compilación:**
    *   MinGW-w64 (UCRT64) con GCC 11+
    *   CMake 3.20+

## 🚀 Compilación e Instalación

Para compilar el proyecto en un entorno MSYS2 (UCRT64):

```bash
mkdir build && cd build
cmake -G "MinGW Makefiles" ..
mingw32-make
```

## 📜 Licencia

Distribuido bajo la Licencia MIT. Consulta el archivo `LICENSE` para más información.

---
<div align="center">
  <i>Construido para el rendimiento puro.</i>
</div>
