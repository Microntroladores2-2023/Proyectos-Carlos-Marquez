# Proyectos de Microcontroladores 2 - ESP32 con PlatformIO

¡Hola! Soy Carlos Marquez [carlosjmarq](https://github.com/carlosjmarq) y esta carpeta contiene mis proyectos de Microcontroladores 2 utilizando el ESP32 y PlatformIO.

## Descripción

Estos proyectos fueron creados para la materia de Microcontroladores 2 en es cuela de ingenieria electriuce de la Universidad Centarl de Venezuela (UCV), en la que aprendimos a implementar distintos sistemas embebidos utilizando el microcontrolador ESP32 y la plataforma PlatformIO. Los proyectos abarcan diferentes temas, desde enceder leds hasta protocolos de comunicacion industrial.

PlatformIO es una herramienta de desarrollo que nos permite escribir, compilar y cargar código en diferentes plataformas de microcontroladores, incluyendo el ESP32. PlatformIO genera una estructura de carpetas por defecto para cada proyecto, que incluye las siguientes carpetas:

- **`include`**: En esta carpeta se almacenan los archivos de encabezado (`*.h`) que son utilizados por el código fuente.
- **`lib`**: En esta carpeta se almacenan las bibliotecas externas que se utilizan en el proyecto. Las bibliotecas se pueden agregar utilizando el administrador de bibliotecas de PlatformIO o descargándolas manualmente.
- **`src`**: En esta carpeta se almacena el código fuente (`*.c`, `*.cpp` y `*.h`) del proyecto.
- **`test`**: En esta carpeta se pueden agregar pruebas de unidad para el proyecto.
- **`platformio.ini`**: Este archivo es el archivo de configuración principal de PlatformIO y contiene información sobre la plataforma y las herramientas de desarrollo utilizadas para el proyecto.

## Estructura de Carpetas

- **`led_sencillo`**: Parpadeo de un led.
- **`maquinas-estado-arduino`**: implemenacion sencilla de una maquina de estado en arduino.
- **`tareas_en_nucleos`**: creacion de diversas tareas y distrubucion de su carga en distintos nucleos del micro.
- **`tipos-variables`**: estudio de los distintos tipos de variables disponibles en c.

## Requisitos de Software

- [PlatformIO Core](https://docs.platformio.org/en/latest/core/installation.html)

## Requisitos de Hardware

- ESP32

## Licencia

Este proyecto está bajo la Licencia [MIT](https://opensource.org/licenses/MIT).