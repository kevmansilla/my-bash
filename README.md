# Sistemas Operativos: Laboratorio 1 (MyBash)

El Laboratorio consiste en codificar un *shell* al estilo de bash(Bourne Again Shell) al que denominaremos *Mybash. El cual debe contener las siguientes funcionalidades:

1. Ejecución de comandos en modo foreground y background.
2. Re dirección de entrada y salida.
3. Pipe entre comandos.

## Primera parte

Para poder ejecutar *MyBash* correctamente se deben descargar los archivos del repositorio generado en **Bitbucket**, luego buscar los archivos `parser.o` y `lexer.o`(generados por la cátedra) según la arquitectura del procesador. En el caso de 64bits utilizar objects-x86_64 y para 32bits objects-i386 y pasarlos al directorio principal.

Luego el proyecto debe compilarse por terminal, para ello es necesario escribir las siguientes lineas de código:
```
> $ make mybash
> $ ./mybash
```
## Testing

Para poner a prueba nuestro programa y ver si cumple los requisitos necesarios, la cátedra nos provee de una *test suite* que son *test* pensados para evaluar si un programa tiene cierto comportamiento específico.

Para ejecutarlos será necesario la instalación de la biblioteca `check.h`, para Ubuntu:
```
sudo apt-get install check
```

Luego de tener lista la librería corremos los test de la siguiente forma:

1. *Test* parar el archivo `command.c`:
```
> $ make test-command
```
2. *Test* para el programa en conjunto:
```
> $ make test
```
3. *Test* de uso de memoria:
```
> $ make memtest
```

## Desarrollo del laboratorio:

El desarrollo del proyecto consiste en la implementación de tres módulos, todos ellos fueron resueltos de manera conjunta por los integrantes del grupo. Primero se trabajó con **command**, el cual es fundamental para la comunicación con los modulos **parser** y **execute** debido a que está conformado por las dos estructuras de datos (TADs) principales; *scommand* y por otro lado *pipelines*. Ambos TADs hacen uso de listas simples enlazadas (GSList) provistas por la librería `glib.h`.

Se codearon cada una de las funciones propuestas por el archivo `command.h` siguiendo las especificaciones correspondientes. También se agregó una función auxiliar (`strmerge_and_free`) en el archivo `strextra.c` con el fin de poder liberar la memoria que pedia `strmerge`, ya que está función nos resulto clave para desarrollar `scommand_to_string` y `pipeline_to_string`. Luego se procedió a comprobar la funcionalidad del módulo haciendo uso de los test, un vez que se logró una funcionalidad del 100% se puso foco en resolver problemas de *Memory Leaks*, modificando algunas implementaciones ya realizadas.

Cabe resaltar que existen *Memory Leaks*, más precisamente *still reachable* que se encuentra fuera de nuestro control debido a que son causados por la propia librería. Cuentan la cantidad de memoria que está reservada en bloques accesibles a la hora de cerrar el programa. Para lograr un número cercano a cero se tendría que hacer `free` de absolutamente toda la memoria que pide el programa, pero no es necesario debido a que el sistema operativo se ocupa de liberar los recursos asignados. (Ver está última just)

En segundo lugar decidimos implementar **builtin** este identifica y ejecuta los comandos internos que se la pasan a *MyBash* y por ende también está fuertemente relacionado con **execute**. Se implementaron las funciones propuestas por `builtin.h`,
con el fin de poder realizar las *syscall* de los comandos internos `cd` y `exit`. Se trabajo con varias opciones para la salida del comando `exit` hasta poder llegar a la implementación correcta y no tener problemas con la memoria.

Una vez finalizada la implementación de estos módulos, se codeó **execute** que cumple el papel principal del programa, ya que depende de los demás modulos y a su vez recibe el flujo de información del **parser**.


## Integrantes del grupo:

* Castellaro, Agustín.
* Mansilla, Kevin Gaston.
* Ramirez, Lautaro.
